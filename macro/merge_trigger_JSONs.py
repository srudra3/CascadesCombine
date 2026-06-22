#!/usr/bin/env python3
import json
import sys
from pathlib import Path

def merge_fit_params(old_path: str, new_path: str, output_path: str):
    with open(old_path) as f:
        old = json.load(f)
    with open(new_path) as f:
        new = json.load(f)

    merged = {}

    all_keys = set(old.keys()) | set(new.keys())

    for key in all_keys:
        in_old = key in old
        in_new = key in new

        if in_new and not in_old:
            # Key only in new: take everything from new
            merged[key] = new[key]
        elif in_old and not in_new:
            # Key only in old: take everything from old
            merged[key] = old[key]
        else:
            # Key in both: merge with rules
            old_entry = old[key]
            new_entry = new[key]

            # Start from new entry as base (captures new fit function results)
            merged[key] = dict(new_entry)

            # Rule 1: fit params -> use NEW (already there from new_entry base)
            # Rule 2: bands params -> use OLD if it exists
            if "bands" in old_entry:
                merged[key]["bands"] = old_entry["bands"]
            elif "bands" in new_entry:
                merged[key]["bands"] = new_entry["bands"]
            # (if neither has bands, no bands key is set)

    with open(output_path, "w") as f:
        json.dump(merged, f, indent=4)
        f.write("\n")

    print(f"Merged {len(all_keys)} keys -> {output_path}")
    only_old = sum(1 for k in all_keys if k in old and k not in new)
    only_new = sum(1 for k in all_keys if k not in old and k in new)
    both     = sum(1 for k in all_keys if k in old and k in new)
    print(f"  {both} keys in both, {only_old} only in old, {only_new} only in new")

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: merge_fit_params.py <old.json> <new.json> <output.json>")
        sys.exit(1)
    merge_fit_params(sys.argv[1], sys.argv[2], sys.argv[3])
