#!/usr/bin/env python3
"""
run_plotImpacts.py.py

Wrapper around plotImpacts.py that loops over multiple impact JSON files
and plots a set of predefined nuisance groups for each.

Input JSON paths are expected to follow the convention:
  <run_dir>/datacards/<mass_point>/impacts.json

The datacard is resolved automatically as:
  <run_dir>/datacards/<mass_point>/<mass_point>.txt

Outputs are written to:
  <run_dir>/plots/pdfs/impacts/<mass_point>/<group_name>.pdf/.root

Usage:
  python run_plotImpacts.py.py -j path/to/impacts.json [more impacts.json ...]
  python run_plotImpacts.py.py -j runs/.../datacards/*/impacts.json --dry-run
  python run_plotImpacts.py.py -j path/to/impacts.json --only-groups Btag PTISR
"""

from __future__ import print_function
import argparse
import os
import subprocess
import sys

# ---------------------
# SHARED ARGS
# Passed to every plotImpacts.py call (can be overridden per-group in NUISANCE_GROUPS).
# 'datacard' is auto-derived from the JSON path; only set this to override globally.
# ---------------------
SHARED = {
    "datacard":       None,   # None = auto-derive as <json_dir>/<mass_point>.txt
    "sort":           "impact_r",
    "max_label_len":  45,
    "absolute":       True,   # convert Gaussian lnN nuisances to multiplicative scale factors
}

# ---------------------
# NUISANCE GROUPS
# Each entry defines one plotImpacts.py call per JSON file.
#   name       : used in the output filename
#   substrings : list of substrings passed to -s (ignored if use_all=True)
#   use_all    : if True, pass --all instead of -s
#   overrides  : optional dict to override any key in SHARED for this group only
# ---------------------
NUISANCE_GROUPS = [
    {
        "name":       "Btag",
        "substrings": ["Btag"],
        "use_all":    False,
        "overrides":  {},
    },
    {
        "name":       "PTISR",
        "substrings": ["PTISR"],
        "use_all":    False,
        "overrides":  {},
    },
    {
        "name":       "OSOF",
        "substrings": ["OSOF"],
        "use_all":    False,
        "overrides":  {},
    },
    {
        "name":       "SameSign",
        "substrings": ["SameSign"],
        "use_all":    False,
        "overrides":  {},
    },
    {
        "name":       "LepID",
        "substrings": ["IDISO", "SIP3D"],
        "use_all":    False,
        "overrides":  {},
    },
    {
        "name":       "LepHemi",
        "substrings": ["LepHemi"],
        "use_all":    False,
        "overrides":  {},
    },
    {
        "name":       "scale_rateParams",
        "substrings": ["scale_"],
        "use_all":    False,
        "overrides":  {},
    },
    {
        "name":       "all_name",
        "substrings": [],
        "use_all":    True,
        "overrides":  {"sort": "name"},
    },
    {
        "name":       "all_impact",
        "substrings": [],
        "use_all":    True,
        "overrides":  {"sort": "impact_r"},
    },
]

# Path to plotImpacts.py (assumed to live alongside this wrapper)
PLOT_SCRIPT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "plotImpacts.py")


# ---------------------
# Path helpers
# ---------------------
def derive_paths(json_path):
    """
    Given e.g.:
      runs/<run_dir>/datacards/SMS_TChiWZ_SMS_500_450/impacts.json
    Returns:
      out_dir       = runs/<run_dir>/plots/pdfs/impacts/SMS_TChiWZ_SMS_500_450
      mass_point    = SMS_TChiWZ_SMS_500_450
      datacard_path = runs/<run_dir>/datacards/SMS_TChiWZ_SMS_500_450/SMS_TChiWZ_SMS_500_450.txt

    Falls back gracefully if the path does not contain a 'datacards' component.
    """
    json_path = os.path.normpath(os.path.abspath(json_path))
    parts = json_path.split(os.sep)
    try:
        dc_idx = len(parts) - 1 - parts[::-1].index("datacards")
    except ValueError:
        dc_idx = None

    if dc_idx is not None and dc_idx + 1 < len(parts) - 1:
        run_root      = os.sep.join(parts[:dc_idx])
        mass_point    = parts[dc_idx + 1]
        json_dir      = os.path.dirname(json_path)
        out_dir       = os.path.join(run_root, "plots", "pdfs", "impacts", mass_point)
        datacard_path = os.path.join(json_dir, mass_point + ".txt")
    else:
        json_dir      = os.path.dirname(json_path)
        mass_point    = os.path.basename(json_dir)
        out_dir       = os.path.join(json_dir, "impacts_plots")
        datacard_path = os.path.join(json_dir, mass_point + ".txt")
        print(f"  [warn] Could not find 'datacards' in path; writing outputs to {out_dir}")

    return out_dir, mass_point, datacard_path


# ---------------------
# Command builder
# ---------------------
def build_cmd(json_path, group, out_dir, datacard_path):
    """Build the subprocess command list for one (json, group) combination."""
    cfg = dict(SHARED)
    cfg.update(group.get("overrides", {}))

    out_base = os.path.join(out_dir, group["name"])

    cmd = [
        sys.executable, PLOT_SCRIPT,
        "-j", json_path,
        "-o", out_base,
        "--sort", str(cfg["sort"]),
        "--max-label-len", str(cfg["max_label_len"]),
    ]

    # Use explicit datacard override from SHARED if set, otherwise use auto-derived path
    dc = cfg.get("datacard") or datacard_path
    if cfg.get("absolute", True):
        if dc and os.path.isfile(dc):
            cmd += ["-d", dc]
        else:
            print(f"  [warn] Datacard not found: {dc} — falling back to --no-absolute for group '{group['name']}'")
            cmd.append("--no-absolute")
    else:
        cmd.append("--no-absolute")

    if group["use_all"]:
        cmd.append("--all")
    else:
        cmd += ["-s"] + group["substrings"]

    return cmd


def run_cmd(cmd, dry_run=False):
    label = " ".join(cmd)
    if dry_run:
        print(f"  [DRY-RUN] {label}")
        return True
    print(f"  [RUN] {label}")
    result = subprocess.run(cmd)
    if result.returncode != 0:
        print(f"  !! FAILED (exit {result.returncode})")
        return False
    return True


# ---------------------
# Main
# ---------------------
def main():
    parser = argparse.ArgumentParser(
        description="Wrapper: run plotImpacts.py for one or more impacts.json files across predefined nuisance groups.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "-j", "--json", nargs="+", required=True, metavar="PATH",
        help="One or more impacts.json paths (e.g. runs/.../datacards/SMS_500_450/impacts.json)",
    )
    parser.add_argument("--dry-run", action="store_true",
                        help="Print commands without executing them.")
    parser.add_argument("--only-groups", nargs="+", metavar="GROUP_NAME",
                        help="Run only these nuisance group names (e.g. Btag PTISR all).")
    args = parser.parse_args()

    # filter groups if requested
    groups = NUISANCE_GROUPS
    if args.only_groups:
        groups = [g for g in groups if g["name"] in args.only_groups]
        if not groups:
            available = [g["name"] for g in NUISANCE_GROUPS]
            print(f"ERROR: none of {args.only_groups} matched. Available groups: {available}")
            sys.exit(1)

    total   = len(args.json) * len(groups)
    success = 0
    failed  = []

    tag = "[DRY-RUN] " if args.dry_run else ""
    print(f"{tag}Running {len(groups)} group(s) x {len(args.json)} JSON file(s) = {total} plot(s)\n")

    for json_path in args.json:
        if not os.path.isfile(json_path):
            print(f"[SKIP] File not found: {json_path}")
            failed += [(json_path, g["name"]) for g in groups]
            continue

        out_dir, mass_point, datacard_path = derive_paths(json_path)

        print(f"--- {json_path}")
        print(f"    mass point : {mass_point}")
        print(f"    datacard   : {datacard_path}")
        print(f"    output dir : {out_dir}")

        if not args.dry_run:
            os.makedirs(out_dir, exist_ok=True)

        for group in groups:
            cmd = build_cmd(json_path, group, out_dir, datacard_path)
            ok  = run_cmd(cmd, dry_run=args.dry_run)
            if ok:
                success += 1
            else:
                failed.append((json_path, group["name"]))

        print()

    print(f"{tag}Done: {success}/{total} succeeded.")
    if failed:
        print("Failed combinations:")
        for jp, gn in failed:
            print(f"  json={jp}  group={gn}")
        sys.exit(1)


if __name__ == "__main__":
    main()
