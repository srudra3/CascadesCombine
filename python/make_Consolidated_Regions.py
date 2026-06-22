#!/usr/bin/env python3
"""
make_Consolidated_Regions.py

Consolidates pairs (or groups) of YAML region files that differ only in
jet-multiplicity (0J / 1J / ...) and PTISR bin (lPTISR / hPTISR, or P<value>
embedded in bin names).

Usage:
    python make_Consolidated_Regions.py file1.yaml file2.yaml [file3.yaml ...]

All input files must share the same "base" name once the J-count and PTISR
tokens are stripped (e.g. Regions_2L_0J_lPTISR_Silver.yaml and
Regions_2L_1J_hPTISR_Silver.yaml both reduce to Regions_2L_Silver).

The output file is written next to the first input file (or in the current
directory) as <base>.yaml.

Consolidation rules
-------------------
Filename  : strip  <NJ>  and  <lPTISR|hPTISR>  tokens.
Bin key   : strip  <NJ>_P<value>  or  P<value>  prefixed by _ from the key.
cuts      : semi-colon-separated list; rules per token:
              PTISR_LEP>=<v>   -> keep token with the MINIMUM (loosest) value
              PTISR_LEP<<v>    -> DROP (upper bound disappears after merging)
              Njet_S==<v>      -> DROP (covered by the union of jet bins)
              Njet_S>=<v>      -> DROP
              everything else  -> must be IDENTICAL across all files;
                                  raise an error if they differ.
lep-cuts, predefined-cuts, user-cuts:
              must be identical across all files; raise an error if they differ.
"""

import re
import sys
import os
from pathlib import Path

# ---------------------------------------------------------------------------
# Filename helpers
# ---------------------------------------------------------------------------

# Tokens to strip from the filename stem (order doesn't matter)
_FNAME_JET_RE   = re.compile(r'_\d+J(?=_|$)')      # _0J, _1J, _2J, …
_FNAME_PTISR_RE = re.compile(r'_[lh]PTISR(?=_|$)')# _lPTISR, _hPTISR


def stem_base(stem: str) -> str:
    """Return the filename stem with jet and PTISR tokens removed."""
    s = _FNAME_JET_RE.sub('', stem)
    s = _FNAME_PTISR_RE.sub('', s)
    # collapse any double underscores introduced by the removal
    while '__' in s:
        s = s.replace('__', '_')
    return s.strip('_')


# ---------------------------------------------------------------------------
# Bin-key helpers
# ---------------------------------------------------------------------------

# Matches patterns like  _0J_P250  or  _1J_P350  inside a bin key
_KEY_JET_PTISR_RE = re.compile(r'_\d+J_P\d+')
# Fallback: just  _P<digits>  in case there is no leading NJ
_KEY_PTISR_ONLY_RE = re.compile(r'_P\d+')


def key_base(key: str) -> str:
    """Strip jet+PTISR tokens from a bin key."""
    k = _KEY_JET_PTISR_RE.sub('', key)
    k = _KEY_PTISR_ONLY_RE.sub('', k)
    while '__' in k:
        k = k.replace('__', '_')
    return k.strip('_')


# ---------------------------------------------------------------------------
# Cut-string helpers
# ---------------------------------------------------------------------------

_PTISR_GE_RE = re.compile(r'^PTISR_LEP>=(\d+(?:\.\d*)?)$')   # PTISR_LEP>=250
_PTISR_LT_RE = re.compile(r'^PTISR_LEP<(\d+(?:\.\d*)?)$')    # PTISR_LEP<350
_NJET_S_RE   = re.compile(r'^Njet_S(?:==|>=|<=|>|<)\S+$')    # Njet_S==0, Njet_S>=1


def parse_cuts(cuts_str: str) -> list[str]:
    """Split a semicolon-delimited cuts string, dropping empty tokens."""
    return [t.strip() for t in cuts_str.split(';') if t.strip()]


def merge_cuts(all_cuts: list[list[str]]) -> str:
    """
    Merge cut lists from multiple bins according to the consolidation rules.

    Parameters
    ----------
    all_cuts : list of cut lists, one per input file/bin

    Returns
    -------
    Merged semicolon-separated cuts string (trailing semicolon included).
    """
    # Collect PTISR lower bounds; everything else must be identical.
    ptisr_ge_values: list[float] = []
    # Build a reference list of "stable" cuts (non-PTISR, non-NjetS)
    # keyed by their first occurrence order.
    stable_cuts_per_file: list[list[str]] = []

    for cut_list in all_cuts:
        stable = []
        for token in cut_list:
            if _PTISR_GE_RE.match(token):
                val = float(_PTISR_GE_RE.match(token).group(1))
                ptisr_ge_values.append(val)
            elif _PTISR_LT_RE.match(token):
                pass  # drop upper PTISR bound
            elif _NJET_S_RE.match(token):
                pass  # drop Njet_S cuts
            else:
                stable.append(token)
        stable_cuts_per_file.append(stable)

    # Validate that stable cuts are identical across all files
    reference = stable_cuts_per_file[0]
    for i, other in enumerate(stable_cuts_per_file[1:], start=2):
        if other != reference:
            # Give a helpful diff
            only_ref  = [c for c in reference if c not in other]
            only_other = [c for c in other if c not in reference]
            msg_parts = []
            if only_ref:
                msg_parts.append(f"  only in file 1 : {only_ref}")
            if only_other:
                msg_parts.append(f"  only in file {i}: {only_other}")
            raise ValueError(
                "Non-consolidatable cuts differ between files:\n" +
                "\n".join(msg_parts)
            )

    # Build merged cut list: stable cuts + the loosest PTISR lower bound
    merged: list[str] = []
    if ptisr_ge_values:
        min_ptisr = min(ptisr_ge_values)
        merged.append(f"PTISR_LEP>={int(min_ptisr) if min_ptisr == int(min_ptisr) else min_ptisr}")
    merged.extend(reference)

    return ';'.join(merged) + ';'


# ---------------------------------------------------------------------------
# Simple YAML loader that preserves insertion order and literal blocks
# ---------------------------------------------------------------------------
# We use PyYAML for parsing but reconstruct the output manually so we can
# keep the exact quoting style and literal-block scalars that the original
# files use.

try:
    import yaml
except ImportError:
    sys.exit("PyYAML is required.  Install it with:  pip install pyyaml")


def load_yaml_ordered(path: Path) -> dict:
    """Load a YAML file, returning an ordered dict of bin entries."""
    with path.open() as fh:
        data = yaml.safe_load(fh)
    if not isinstance(data, dict):
        raise ValueError(f"{path}: expected a top-level mapping, got {type(data)}")
    return data


# ---------------------------------------------------------------------------
# Output serialiser  (keeps quoting + literal blocks)
# ---------------------------------------------------------------------------

def _quote(s: str) -> str:
    """Wrap a string in double quotes, escaping any internal double quotes."""
    return '"' + s.replace('"', '\\"') + '"'


def dump_bin(key: str, entry: dict) -> str:
    """Serialise a single bin entry back to YAML text."""
    lines = [f"{key}:"]
    for field, value in entry.items():
        if isinstance(value, str) and '\n' in value:
            # literal block scalar
            lines.append(f"  {field}: |")
            for sub in value.splitlines():
                lines.append(f"    {sub}")
        else:
            lines.append(f"  {field}: {_quote(str(value))}")
    return '\n'.join(lines)


# ---------------------------------------------------------------------------
# Core consolidation logic
# ---------------------------------------------------------------------------

def consolidate(input_paths: list[Path]) -> tuple[str, str]:
    """
    Consolidate a group of YAML files.

    Returns
    -------
    (output_stem, yaml_text)
    """
    # Load all files
    datasets: list[dict] = [load_yaml_ordered(p) for p in input_paths]

    # Verify output stems all agree
    stems = [stem_base(p.stem) for p in input_paths]
    if len(set(stems)) != 1:
        raise ValueError(
            f"Files do not share the same base stem after stripping J/PTISR tokens.\n"
            f"Stems found: {stems}"
        )
    output_stem = stems[0]

    # Group bins by their base key
    from collections import defaultdict
    groups: dict[str, list[dict]] = defaultdict(list)
    group_order: list[str] = []   # preserve first-seen order

    for data in datasets:
        for raw_key, entry in data.items():
            bk = key_base(raw_key)
            if bk not in groups:
                group_order.append(bk)
            groups[bk].append(entry)

    # Consolidate each group
    output_bins: list[str] = []

    for bk in group_order:
        entries = groups[bk]

        if len(entries) == 1:
            # Nothing to merge; just re-emit with the cleaned key
            output_bins.append(dump_bin(bk, entries[0]))
            continue

        # Validate that non-cuts fields are identical
        ref_entry = entries[0]
        for field in ('lep-cuts', 'predefined-cuts', 'user-cuts'):
            ref_val = ref_entry.get(field)
            for i, e in enumerate(entries[1:], start=2):
                if e.get(field) != ref_val:
                    raise ValueError(
                        f"Bin '{bk}': field '{field}' differs between files:\n"
                        f"  file 1  : {ref_val!r}\n"
                        f"  file {i} : {e.get(field)!r}"
                    )

        # Merge cuts
        all_cuts = [parse_cuts(e['cuts']) for e in entries]
        merged_cuts = merge_cuts(all_cuts)

        merged_entry = dict(ref_entry)   # copy preserving field order
        merged_entry['cuts'] = merged_cuts

        output_bins.append(dump_bin(bk, merged_entry))

    yaml_text = '\n\n'.join(output_bins) + '\n'
    return output_stem, yaml_text


# ---------------------------------------------------------------------------
# CLI entry point
# ---------------------------------------------------------------------------

def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)

    input_paths = [Path(a) for a in sys.argv[1:]]
    for p in input_paths:
        if not p.exists():
            sys.exit(f"File not found: {p}")

    output_stem, yaml_text = consolidate(input_paths)

    out_dir = input_paths[0].parent
    out_path = out_dir / f"{output_stem}.yaml"

    out_path.write_text(yaml_text)
    print(f"Wrote {out_path}")


if __name__ == '__main__':
    main()
