#!/usr/bin/env python3
"""
Make Silver/Bronze YAMLs and optional lPTISR Golds from Gold YAMLs; enforce Regions predef/user
and ensure a baseline preselection of cuts in each bin's `cuts:` string.

Behavior:
-- For *_Gold.yaml files (in config/bin_cfgs by default):
-  1) If filename contains _hPTISR and is 2L or 3L, create a corresponding lPTISR Gold:
-     - 2L: _P350 -> _P250 and PTISR_LEP>=350 -> PTISR_LEP>=250;PTISR_LEP<350
-     - 3L: _P300 -> _P200 and PTISR_LEP>=300 -> PTISR_LEP>=200;PTISR_LEP<300
-  2) For any file whose filename starts with "Regions" (case-insensitive),
-     set every bin's `predefined-cuts` and `user-cuts` to the hard-coded strings below.
-  3) Produce Silver and Bronze YAMLs (Gold -> Silver / Bronze) with lep-cuts modified
-     according to NEW_LEP_CUTS mapping.
-     CUTS_PRESELECTION: list of cut substrings (include trailing ';') which will be ensured to appear in each bin's `cuts:` line.
-     REMOVE_CUTS: list of cut substrings (include trailing ';') which will be removed if present.

Run inside CMSSW after cmsenv with ruamel.yaml installed.

CLI flags:
  --no-preselection    : disable enforcing CUTS_PRESELECTION
  --remove "A;B;C;"    : semicolon-separated string of cuts to remove (overrides REMOVE_CUTS default)
  --dry-run            : don't write files; only print actions
"""
from pathlib import Path
import sys
import re
from ruamel.yaml import YAML
from ruamel.yaml.scalarstring import PreservedScalarString
from ruamel.yaml.comments import CommentedMap

# ----------------------------
# User-tunable defaults below
# ----------------------------

# Baseline cuts to ensure are present in every bin's cuts string.
# IMPORTANT: include the trailing semicolon in each substring so matching is unambiguous.
CUTS_PRESELECTION = [
    "MET>=150;",
    "METORtrigger==1;",
    "EventFilter==1;",
    "PassesJVM==1;",
    "MIN(abs(Eta_lep))<1.4442;",
    "Njet>0",
]

# Cuts to remove if present. Empty by default; set e.g. ["PassesJVM==1;"] to remove.
REMOVE_CUTS = [
    # example: "PassesJVM==1;",
]

# mapping for lep-cuts transforms
NEW_LEP_CUTS = {
    "Silver": {
        "2L": ["<=1Gold;", "=0Bronze;"],
        "3L": [">=1Silver;", "=0Bronze;"],
        "4L": [">=1Gold;", "=1Bronze;"],
    },
    "Bronze": {
        "2L": ["=1Bronze;"],
        "3L": [">=1Gold;", "=1Bronze;"],
        "4L": [">=1Gold;", "=2Bronze;"],
    },
}

# Hard-coded strings for Regions* files
REGIONS_PREDEFINED_CUTS = "Cleaning_LEP;dphiMETV_LEP;"
REGIONS_USER_CUTS = "minMll_minDR_2D_low;HEM_Veto;leadSjet_pt;"

# ----------------------------
# Helper functions
# ----------------------------

def detect_mult_from_key(key: str):
    """
    Return '2L'|'3L'|'4L' or None.
    Works with:
      - Old: Bin2L_Gold_0J_...
      - New: Bin_Run2_2L_Bronze_0J_...
      - Sideband / PTISR variations
    """
    if not key.startswith("Bin"):
        return None
    tokens = key.split("_")  # split by underscores
    # If format is Bin_RunX_2L_..., multiplicity is usually the second or third token
    for t in tokens[1:4]:  # skip "Bin", look at next 3 tokens
        if t in ("2L", "3L", "4L"):
            return t
    # fallback: anywhere in the key as a full token
    for t in tokens:
        if t in ("2L", "3L", "4L"):
            return t
    return None

def normalize_cuts_string(cuts_text: str):
    """
    Normalize a cuts string: split on semicolons, strip whitespace,
    rejoin with single semicolons and ensure trailing semicolon if non-empty.
    """
    if cuts_text is None:
        return ""
    s = str(cuts_text)
    # remove surrounding quotes if ruamel kept them visually, but will keep raw string handling
    parts = [p.strip() for p in s.split(';') if p.strip() != ""]
    if not parts:
        return ""
    return ";".join(parts) + ";"

def ensure_preselection_on_entry(entry, preselection_list):
    """
    Ensure every substring in preselection_list appears in entry['cuts'].
    Returns True if modified.
    """
    if not isinstance(entry, dict):
        return False
    cuts = str(entry.get("cuts", "") or "")
    orig = cuts
    # Normalize existing cuts for robust substring checks
    # but keep the original casing/format for parts untouched
    # use normalized reconstruction at the end.
    normalized = normalize_cuts_string(cuts)
    present_parts = [p for p in normalized.split(';') if p]
    changed = False
    for req in preselection_list:
        # req should include trailing ';' match presence by exact substring without trailing whitespace
        req_stripped = req.strip()
        if req_stripped.endswith(';'):
            req_core = req_stripped[:-1]
        else:
            req_core = req_stripped
        if req_core not in present_parts:
            present_parts.append(req_core)
            changed = True

    if changed:
        newcuts = ";".join(present_parts) + ";"
        entry["cuts"] = newcuts
        return True
    return False

def remove_cuts_from_entry(entry, remove_list):
    """
    Remove any substrings in remove_list from entry['cuts'].
    Returns True if modified.
    """
    if not isinstance(entry, dict):
        return False
    cuts = str(entry.get("cuts", "") or "")
    normalized = normalize_cuts_string(cuts)
    parts = [p for p in normalized.split(';') if p]
    original_len = len(parts)
    # for each removal, remove matching parts exactly (remove trailing semicolon assumed)
    to_remove_cores = []
    for r in remove_list:
        rstr = r.strip()
        if rstr.endswith(';'):
            rcore = rstr[:-1]
        else:
            rcore = rstr
        to_remove_cores.append(rcore)
    parts = [p for p in parts if p not in to_remove_cores]
    if len(parts) != original_len:
        newcuts = ";".join(parts) + (";" if parts else "")
        entry["cuts"] = newcuts
        return True
    return False

def enforce_common_cuts(doc):
    """
    Overwrite predefined-cuts and user-cuts for every bin in the document.
    Operates in-place on the ruamel mapping.
    """
    for key, entry in doc.items():
        if not isinstance(entry, dict):
            continue
        entry["predefined-cuts"] = REGIONS_PREDEFINED_CUTS
        entry["user-cuts"] = REGIONS_USER_CUTS

def transform_lep_cuts_ruamel(old_scalar, multiplicity, tier):
    """
    Replace any Gold marker lines in lep-cuts and prepend the new tier lines.
    Returns a PreservedScalarString to keep block style.
    """
    old_text = "" if old_scalar is None else str(old_scalar)
    old_lines = [ln.rstrip() for ln in old_text.splitlines() if ln.strip() != ""]
    kept = [ln for ln in old_lines if "Gold" not in ln]
    new_lines = NEW_LEP_CUTS[tier].get(multiplicity, [])
    final = new_lines + kept
    if not final:
        return PreservedScalarString("")
    return PreservedScalarString("\n".join(final) + "\n")

def make_lptisr_doc_ruamel(doc):
    """
    Produce a new ruamel CommentedMap for the lPTISR Gold version (operates on ruamel mapping).
    """
    from ruamel.yaml.comments import CommentedMap
    out = CommentedMap()
    for key in list(doc.keys()):
        value = doc[key]
        entry = value.copy() if hasattr(value, "copy") else value

        mult = detect_mult_from_key(key)  # robust 2L/3L/4L detection

        new_key = key
        # rename _P350 -> _P250 for 2L and _P300 -> _P200 for 3L
        if mult == "2L" and "_P350" in key:
            new_key = key.replace("_P350", "_P250")
        elif mult == "3L" and "_P300" in key:
            new_key = key.replace("_P300", "_P200")

        # update cuts strings as before
        if isinstance(entry, dict) and "cuts" in entry:
            old_text = str(entry.get("cuts", ""))
            new_text = old_text
            if mult == "2L":
                new_text = new_text.replace(
                    "PTISR_LEP>=350;", "PTISR_LEP>=250;PTISR_LEP<350;"
                ).replace(
                    "PTISR_LEP>=350", "PTISR_LEP>=250;PTISR_LEP<350"
                )
            elif mult == "3L":
                new_text = new_text.replace(
                    "PTISR_LEP>=300;", "PTISR_LEP>=200;PTISR_LEP<300;"
                ).replace(
                    "PTISR_LEP>=300", "PTISR_LEP>=200;PTISR_LEP<300"
                )
            if new_text != old_text:
                entry["cuts"] = new_text

        out[new_key] = entry
        try:
            out.ca.items[new_key] = doc.ca.items[key]
        except Exception:
            pass
    return out

def process_ruamel_doc_for_tier(doc, tier):
    """
    Return a new ruamel mapping with Gold->tier in keys and lep-cuts transformed.
    """
    out = CommentedMap()
    for key in list(doc.keys()):
        value = doc[key]
        new_key = key.replace("Gold", tier)
        if hasattr(value, "copy"):
            entry = value.copy()
        else:
            entry = value
        multiplicity = detect_mult_from_key(key)
        if isinstance(entry, dict) and multiplicity is not None and "lep-cuts" in entry:
            old_lep = entry.get("lep-cuts")
            entry["lep-cuts"] = transform_lep_cuts_ruamel(old_lep, multiplicity, tier)
        out[new_key] = entry
        try:
            out.ca.items[new_key] = doc.ca.items[key]
        except Exception:
            pass
    return out

# ----------------------------
# Main processing
# ----------------------------
def main(indir="config/bin_cfgs", outdir=None, dry_run=False,
         apply_preselection=True, remove_cuts_list=None):
    indir = Path(indir)
    outdir = Path(outdir) if outdir else indir
    outdir.mkdir(parents=True, exist_ok=True)

    yaml = YAML(typ="rt")
    yaml.preserve_quotes = True
    yaml.indent(mapping=2, sequence=4, offset=2)
    yaml.width = 4096

    remove_list = list(remove_cuts_list) if remove_cuts_list else list(REMOVE_CUTS)

    gold_files = sorted(indir.glob("*_Gold.yaml"))
    if not gold_files:
        print("No *_Gold.yaml files found in", indir)
        return 0

    for gf in gold_files:
        print("Loading Gold file:", gf.name)

        with gf.open("r", encoding="utf-8") as f:
            doc = yaml.load(f)

        # --------------------------------------------------
        # Enforce common Regions cuts
        # --------------------------------------------------
        if gf.name.lower().startswith("regions"):
            enforce_common_cuts(doc)

        # --------------------------------------------------
        # Apply preselection / removals to Gold
        # --------------------------------------------------
        added_count = removed_count = 0

        if apply_preselection:
            for key, entry in doc.items():
                if isinstance(entry, dict):
                    if ensure_preselection_on_entry(entry, CUTS_PRESELECTION):
                        added_count += 1

        if remove_list:
            for key, entry in doc.items():
                if isinstance(entry, dict):
                    if remove_cuts_from_entry(entry, remove_list):
                        removed_count += 1

        if (apply_preselection and added_count) or (remove_list and removed_count):
            print(
                f"  -> modified Gold in-memory cuts: "
                f"+{added_count} added, -{removed_count} removed"
            )

        # --------------------------------------------------
        # Write updated Gold back out (Regions only)
        # --------------------------------------------------
        if gf.name.lower().startswith("regions"):
            if not dry_run:
                with gf.open("w", encoding="utf-8") as f:
                    yaml.dump(doc, f)
                print(f"  wrote updated Gold: {gf}")
            else:
                print(f"  [dry-run] would write updated Gold: {gf}")

        # --------------------------------------------------
        # Generate lPTISR Gold from hPTISR Gold
        # --------------------------------------------------
        fname = gf.name
        fname_lower = fname.lower()

        if "_hptisr" in fname_lower and ("2l" in fname_lower or "3l" in fname_lower):

            if "hPTISR" in fname:
                l_name = fname.replace("hPTISR", "lPTISR")
            elif "HPTISR" in fname:
                l_name = fname.replace("HPTISR", "lPTISR")
            else:
                l_name = fname.replace("_hptisr", "_lptisr")

            l_doc = make_lptisr_doc_ruamel(doc)

            added_l = removed_l = 0

            if apply_preselection:
                for key, entry in l_doc.items():
                    if isinstance(entry, dict):
                        if ensure_preselection_on_entry(entry, CUTS_PRESELECTION):
                            added_l += 1

            if remove_list:
                for key, entry in l_doc.items():
                    if isinstance(entry, dict):
                        if remove_cuts_from_entry(entry, remove_list):
                            removed_l += 1

            if added_l or removed_l:
                print(
                    f"  -> modified lPTISR in-memory cuts: "
                    f"+{added_l} added, -{removed_l} removed"
                )

            l_path = outdir / l_name

            if dry_run:
                print(f"  [dry-run] would write generated lPTISR Gold: {l_path}")
            else:
                with l_path.open("w", encoding="utf-8") as f:
                    yaml.dump(l_doc, f)

                print("  wrote lPTISR Gold:", l_path)

        # --------------------------------------------------
        # Produce Silver and Bronze
        # --------------------------------------------------
        for tier in ("Silver", "Bronze"):

            newdoc = process_ruamel_doc_for_tier(doc, tier)

            added_t = removed_t = 0

            if apply_preselection:
                for key, entry in newdoc.items():
                    if isinstance(entry, dict):
                        if ensure_preselection_on_entry(entry, CUTS_PRESELECTION):
                            added_t += 1

            if remove_list:
                for key, entry in newdoc.items():
                    if isinstance(entry, dict):
                        if remove_cuts_from_entry(entry, remove_list):
                            removed_t += 1

            if fname.lower().startswith("regions"):
                enforce_common_cuts(newdoc)

            if added_t or removed_t:
                print(
                    f"  -> modified {fname.replace('Gold', tier)} cuts: "
                    f"+{added_t} added, -{removed_t} removed"
                )

            outname = fname.replace("Gold", tier)
            outpath = outdir / outname

            if dry_run:
                print(f"  [dry-run] would write {outpath}")
            else:
                with outpath.open("w", encoding="utf-8") as f:
                    yaml.dump(newdoc, f)

                print("  wrote", outpath)
    return 0

if __name__ == "__main__":
    import argparse
    ap = argparse.ArgumentParser(description="Make Silver/Bronze YAMLs and optional lPTISR Golds from Gold YAMLs; enforce Regions predef/user and cuts preselection")
    ap.add_argument("-i", "--indir", default="config/bin_cfgs", help="input directory")
    ap.add_argument("-o", "--outdir", default=None, help="output directory (defaults to input)")
    ap.add_argument("--dry-run", action="store_true", help="don't write files, just show actions")
    ap.add_argument("--no-preselection", action="store_true", help="do not ensure CUTS_PRESELECTION in each bin")
    ap.add_argument("--remove", type=str, default=None,
                    help="semicolon-separated cuts to remove, e.g. \"PassesJVM==1;Other==1;\"")
    args = ap.parse_args()

    remove_list = None
    if args.remove:
        # parse into list of semicolon-terminated strings
        parts = [p.strip() for p in args.remove.split(';') if p.strip() != ""]
        remove_list = [p + ";" for p in parts]

    rc = main(indir=args.indir, outdir=args.outdir, dry_run=args.dry_run,
              apply_preselection=(not args.no_preselection), remove_cuts_list=remove_list)
    sys.exit(rc)

