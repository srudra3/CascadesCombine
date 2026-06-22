#!/usr/bin/env python3

import os
import glob
import json

BASE_DIR = "runs/zzlikefix/condor"

results = []

# Find all group directories
group_dirs = sorted(glob.glob(os.path.join(BASE_DIR, "group_*")))

for gdir in group_dirs:

    json_dir = os.path.join(gdir, "json")

    if not os.path.isdir(json_dir):
        continue

    # Only keep ZZTo4L JSON files
    json_files = glob.glob(
        os.path.join(json_dir, "*ZZto4L*.json")
    )

    if not json_files:
        continue

    # Usually one matching file per folder
    jf = json_files[0]

    with open(jf, "r") as f:
        data = json.load(f)

    # Top-level cut name
    cut_name = list(data.keys())[0]

    sample_block = data[cut_name]

    # First dataset entry
    dataset_name = list(sample_block.keys())[0]

    files_block = sample_block[dataset_name]["files"]

    # First ROOT file entry
    rootfile = list(files_block.keys())[0]

    nominal = files_block[rootfile]["nominal"]

    # nominal = [events, yield, error]
    yield_val = nominal[1]
    error_val = nominal[2]

    results.append((cut_name, yield_val, error_val))

# Print cutflow table
print()
print(f"{'Cut':<35} {'Yield':>15} {'Error':>15}")
print("-" * 70)

for cut, yld, err in results:
    print(f"{cut:<35} {yld:15.4f} {err:15.4f}")
