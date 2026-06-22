#!/usr/bin/env python3
import os, sys, argparse, subprocess, yaml, re, time, json, hashlib
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

# ---------------------------------
# DEFAULT CONFIGURATION PARAMETERS
# ---------------------------------
cpus = "1"
memory = "1 GB"
dryrun = False
max_workers = 4
limit_submit = None  # limit number of job submissions (None=no limit)

# ---------------------------------
# HELPERS
# ---------------------------------
def strip_inline_comments(s):
    if not isinstance(s, str):
        return s
    lines = []
    for line in s.splitlines():
        line = line.split("#", 1)[0].rstrip()
        if line.strip():
            lines.append(line)
    return "\n".join(lines)

def load_processes(cfg_path):
    with open(cfg_path, "r") as f:
        cfg = yaml.safe_load(f)
    processes = cfg.get("processes", {}) or {}
    bkg = processes.get("bkg", []) or []
    sig = processes.get("sig", []) or []
    data = processes.get("data", []) or []
    sms_filters = cfg.get("sms_filters", [])
    return bkg, sig, data, sms_filters

def _sanitize_for_condor_dir(s: str) -> str:
    return re.sub(r'[^A-Za-z0-9_.-]', '_', s)[:200]

def build_command(bin_name, cfg, bkg_processes, sig_processes, data_processes, sms_filters,
                  make_json, make_root, make_cutflow, hist_yaml, lumi, run_dir,
                  bins_cfg_path, max_materialize):
    """
    bins_cfg_path: path to the per-group JSON file that contains bin->cfg mapping.
                   This file will be passed to createJobs.py as --bins-cfg so that
                   createJobs can stage it into the job and forward per-bin cuts.
    """
    cmd = [
        "python3", "python/createJobs.py",
        "--bkg_processes", *bkg_processes,
        "--sig_processes", *sig_processes,
        "--data_processes", *data_processes,
        "--bin", bin_name,
        "--bins-cfg", str(bins_cfg_path),
        "--cuts", cfg.get("cuts","").replace('\n',''),
        "--lep-cuts", cfg.get("lep-cuts","").replace('\n',''),
        "--predefined-cuts", cfg.get("predefined-cuts","").replace('\n',''),
        "--user-cuts", cfg.get("user-cuts","").replace('\n',''),
        "--cpus", cpus,
        "--memory", memory,
        "--max-materialize", max_materialize,
        "--lumi", lumi,
        "--run-dir", run_dir
    ]
    if sms_filters:
        cmd += ["--sms-filters", *sms_filters]
    if dryrun:
        cmd.append("--dryrun")

    # Add histogram/ROOT options
    if make_json:
        cmd.append("--make-json")
    if make_root:
        cmd.append("--make-root")
    if hist_yaml:
        cmd += ["--hist-yaml", hist_yaml]
    if make_root and make_cutflow:
        cmd.append("--cutflow")

    return cmd

def submit_job(cmd):
    if not dryrun:
        subprocess.run(cmd)
        time.sleep(0.5)

# -----------------------------
# MAIN
# -----------------------------
def main():
    global dryrun

    parser = argparse.ArgumentParser(description="Submit BFI jobs by calling python/createJobs.py")
    parser.add_argument("--dryrun", action="store_true")
    parser.add_argument("--lumi", type=str, default="1.")
    parser.add_argument("--bins-cfg", type=str, default="config/bin_cfgs/examples.yaml",
                        help="YAML file with bin definitions")
    parser.add_argument("--processes-cfg", type=str, default="config/process_cfgs/processes.yaml")
    parser.add_argument("--make-json", action="store_true", help="Pass --make-json to createJobs.py")
    parser.add_argument("--make-root", action="store_true", help="Pass --make-root to createJobs.py")
    parser.add_argument("--cutflow", action="store_true", help="Pass --cutflow to createJobs.py")
    parser.add_argument("--hist-yaml", type=str, default=None, help="YAML file for histogram configuration")
    parser.add_argument("--run-dir", type=str, default="condor", help="directory for holding condor info")
    parser.add_argument("--bins-per-job", type=int, default=1,
                        help="Number of bins to pass to each createJobs invocation (grouped as semicolon-separated list). Default: 1")
    parser.add_argument("--max-materialize", type=str, default="100", help="max materialize jobs to condor")
    args = parser.parse_args()

    # Default behavior: make JSON if neither specified
    make_json = args.make_json
    make_root = args.make_root
    max_materialize = args.max_materialize
    if not (make_json or make_root):
        make_json = True
    if make_root: # extra resources for histograms
        global memory, cpus
        memory = "2 GB"
        cpus = "1"

    dryrun = args.dryrun
    lumi = args.lumi
    run_dir = args.run_dir
    bins_per_job = max(1, int(args.bins_per_job))
    os.makedirs(run_dir, exist_ok=True)

    # Load processes
    bkg_processes, sig_processes, data_processes, sms_filters = load_processes(args.processes_cfg)

    # Load bins YAML (master)
    bins_cfg_path = Path(args.bins_cfg)
    bins = {}
    if bins_cfg_path.exists():
        with open(bins_cfg_path) as f:
            bins = yaml.safe_load(f) or {}
            # normalize and strip inline comments
            for k, v in bins.items():
                if isinstance(v, dict):
                    for key in ("cuts", "lep-cuts", "predefined-cuts", "user-cuts"):
                        v.setdefault(key, "")
                        v[key] = strip_inline_comments(v[key])
    else:
        print(f"[submitJobs] ERROR: bins cfg not found: {args.bins_cfg}", file=sys.stderr)
        sys.exit(1)

    # Build commands (group bins into chunks of size bins_per_job)
    jobs = []

    # preserve YAML order
    bin_items = list(bins.items())

    # make groups
    groups = []
    if bins_per_job <= 1:
        # trivial grouping: one bin per group
        for (bin_name, cfg) in bin_items:
            groups.append([(bin_name, cfg)])
    else:
        # Use slicing to guarantee every bin is included, even in a partial final chunk
        for i in range(0, len(bin_items), bins_per_job):
            chunk = bin_items[i:i + bins_per_job]
            if chunk:
                groups.append(chunk)

    # For each group write a JSON file that contains only that group's bins
    # then call createJobs with --bin "A;B;C" and --bins-cfg pointing to that per-group JSON
    for gi, group in enumerate(groups):
        group_bin_names = [bn for (bn, _) in group]
        combined_bin_arg = ";".join(group_bin_names)
        # For createJobs we still need a 'cfg' argument for build_command; pick the first bin's cfg
        first_cfg = group[0][1] if group else {}

        # prepare per-group JSON file (so createJobs/condor job will have exact per-bin cut configs)
        safe_first = re.sub(r'[^A-Za-z0-9_.-]', '_', group_bin_names[0]) if group_bin_names else f"group{gi}"
        group_json_name = f"bin_group_{gi:03d}__{safe_first}.json"
        group_json_path = Path(run_dir) / group_json_name

        # content: mapping bin_name -> its dict (cuts, lep-cuts, etc)
        group_dict = {}
        for (bn, bcfg) in group:
            # ensure only primitive serializable values
            entry = {}
            for key in ("cuts", "lep-cuts", "predefined-cuts", "user-cuts"):
                entry[key] = bcfg.get(key, "")
            # preserve any other metadata in the bin YAML block
            for k,v in bcfg.items():
                if k not in entry:
                    entry[k] = v
            group_dict[bn] = entry

        with open(group_json_path, "w") as gj:
            json.dump(group_dict, gj, indent=2)

        # Build command that points createJobs at the group JSON (not the full master YAML)
        cmd = build_command(combined_bin_arg, first_cfg, bkg_processes, sig_processes, data_processes, sms_filters,
                            make_json, make_root, args.cutflow, args.hist_yaml, lumi, run_dir, group_json_path, max_materialize)
        jobs.append(cmd)

    # Write a bins_list file so downstream tools know the exact condor work dirs created.
    # The list contains the actual condor directory names (what createJobs will create).
    bins_list_path = Path(run_dir) / f"bins_list_{int(time.time())}.txt"
    with open(bins_list_path, "w") as blf:
        for gi, group in enumerate(groups):
            group_bin_names = [bn for (bn, _) in group]
            combined = ";".join(group_bin_names)
    
            # safe_first: sanitized first bin name (falls back to group index)
            if group_bin_names:
                safe_first = re.sub(r'[^A-Za-z0-9_.-]', '_', group_bin_names[0])[:50]
            else:
                safe_first = f"group{gi:03d}"
    
            digest = hashlib.sha1(combined.encode()).hexdigest()[:8]
            condor_dir_name = f"group_{gi:03d}__{safe_first}__{digest}"
            blf.write(condor_dir_name + "\n")

    print(f"[submitJobs] Wrote condor bins list to {bins_list_path}")

    if limit_submit is not None:
        jobs = jobs[:limit_submit]

    # Submit jobs in parallel
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        executor.map(submit_job, jobs)

    print("[submitJobs] All submissions dispatched")

if __name__ == "__main__":
    main()

