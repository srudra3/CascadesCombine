#!/usr/bin/env python3
import sys, os, argparse, datetime, time, fcntl, subprocess, time, glob, json, yaml, re, shutil
from pathlib import Path
from typing import Optional, Dict, Any
from CondorJobCountMonitor import CondorJobCountMonitor
from fileLock import FileLock, FileLockTimeout, LOCK_FILENAME

def parse_args():
    p = argparse.ArgumentParser(description="Top-level workflow runner")
    p.add_argument("--max-resubmits", type=int, default=3,
                   help="Max resubmit cycles to attempt")
    p.add_argument("--bins-cfg", dest="bins_cfg", type=str, default="config/bin_cfgs/bin_examples.yaml",
                   help="Path to YAML config file containing bin definitions")
    p.add_argument("--processes-cfg", dest="processes_cfg", type=str, default="config/process_cfgs/processes.yaml",
                   help="YAML config file containing process definitions")
    p.add_argument("--hist-cfg", dest="hist_cfg", type=str, default="config/hist_cfgs/hist_examples.yaml",
                   help="YAML config file containing histogram definitions")
    p.add_argument("--FDpattern-cfg", dest="FDpattern_cfg", type=str, default="config/FDpattern_cfgs/FDpattern_examples.yaml",
                   help="YAML config file containing FD bin pattern definitions")
    p.add_argument("--stress-test", dest="stress_test", action="store_true",
                   help="Run stress test")
    p.add_argument("--make-json", action="store_true",
                   help="Generate JSON outputs")
    p.add_argument("--make-root", action="store_true",
                   help="Generate ROOT outputs")
    p.add_argument("--make-impacts", action="store_true",
                   help="Generate Impacts")
    p.add_argument("--make-FD", action="store_true",
                   help="Generate FitDiagnostics")
    p.add_argument("--make-trig-fit", action="store_true",
                   help="Generate trigger fit outputs")
    p.add_argument("--lumi", dest="lumi", type=str, default="-1",
                   help="Lumi to scale everything to overriding SampleTool values")
    p.add_argument("--run-name", dest="run_name", type=str, default=None,
                   help="Optional run name prefix to prepend to timestamp for the run directory")
    p.add_argument("--existing-BFI-dir", dest="existing_BFI_dir", type=str, default=None,
                   help="Optional pass in existing run dir and make new run dir that takes the existing BFI output as input to do BF and later steps")
    p.add_argument("--existing-BF-dir", dest="existing_BF_dir", type=str, default=None,
                   help="Optional pass in existing run dir and make new run dir that takes the existing BF output as input and starts after BF (launching limits step). Also copies qualifying datacards/ subdirs.")
    p.add_argument("--skip-compile", action="store_true",
                   help="Skip running the compile step")
    p.add_argument("--skip-plot-yields", action="store_true",
                   help="Skip running the plot yields from json step")
    p.add_argument("--only-yields", action="store_true",
                   help="Stop after getting yields")
    p.add_argument("--bins-per-job", type=int, default=10,
                   help="Number of bins to group per job")
    p.add_argument("--max-materialize", type=str, default="50",
                   help="max number of jobs per cluster to condor")
    p.add_argument("--cutflow", action="store_true",
                   help="make cutflow hist")
    return p.parse_args()

def early_setup(run_name, existing_BFI_name: Optional[str]=None, existing_BF_name: Optional[str]=None):
    """
    Early logging/setup called before the main workflow begins.

    Returns:
      (run_info: dict, try_acquire_lock_or_exit: callable)
    The returned try_acquire_lock_or_exit(non_blocking: bool=True) will return a FileLock.
    """

    if run_name:
        # Use exactly what caller passed
        run_name = run_name
    else:
        # Fallback: generate one here (minute resolution)
        ts = datetime.datetime.now().strftime("%B%d_%Y_%H%M")
        run_name = f"run_{ts}"

    # If an existing run name is provided, prefix the new run with the existing run base name
    existing_name = existing_BFI_name or existing_BF_name
    if existing_name:
        # use the basename of the provided path so this works with absolute/relative paths
        existing_base = Path(existing_name).name
        run_name = existing_base + "_" + run_name 

    # Create the run_dir and debug file immediately (so wrapper can tail it)
    run_dir = os.path.join("runs", run_name)
    os.makedirs(run_dir, exist_ok=True)

    debug_log_path = os.path.join(run_dir, "debug_run_combine.debug")

    # Print outward confirmation to the original terminal before redirecting
    print(f"[run_combine] Using run directory: {run_dir}", file=sys.__stdout__)

    # Redirect stdout/stderr to the per-run debug file (line-buffered)
    log_file = open(debug_log_path, "w", buffering=1)
    sys.stdout = log_file
    sys.stderr = log_file

    # Save early info in a dict for downstream use
    run_info: Dict[str, Any] = {
        "run_dir": run_dir,
        "debug_log": debug_log_path,
        "run_name": run_name
    }

    # Helper wrappers that print friendly messages to the original terminal if lock cannot be obtained.
    def try_acquire_lock_or_exit(non_blocking: bool = True) -> FileLock:
        """
        Try to acquire the global lock. If non_blocking and lock is held, prints a friendly message
        to the original terminal (sys.__stdout__) and exits(1). If non_blocking is False, this
        will block until the lock is acquired.
        Returns the FileLock instance (which the caller must release when done).
        """
        lock = FileLock(LOCK_FILENAME)
        if non_blocking:
            got = lock.try_acquire()
            if not got:
                print("[run_combine] ERROR: Another run is currently building/staging assets.", file=sys.__stdout__, flush=True)
                print(f"[run_combine] Lock file: {LOCK_FILENAME}", file=sys.__stdout__, flush=True)
                print("[run_combine] Please wait until that run finishes before starting another.", file=sys.__stdout__, flush=True)
                # optionally give the user a hint where to look
                recent_runs = sorted(Path("runs").glob("run_*"), key=os.path.getmtime, reverse=True)[:5]
                if recent_runs:
                    print("[run_combine] Recent runs (newest first):", file=sys.__stdout__, flush=True)
                    for p in recent_runs:
                        print(f"  {p}", file=sys.__stdout__, flush=True)
                sys.exit(1)
            return lock
        else:
            lock.acquire()
            return lock

    return run_info, try_acquire_lock_or_exit

# ----- helper functions -----
def _read_condor_bins_list(condor_dir="condor"):
    """
    Reads the automatically generated bins list file from condor/
    Returns list of bin names. Picks the most recent file matching bins_list_*.txt
    """
    pattern = os.path.join(condor_dir, "bins_list_*.txt")
    files = glob.glob(pattern)
    if not files:
        return []
    # pick the latest file by modification time
    files.sort(key=os.path.getmtime, reverse=True)
    latest_file = files[0]
    with open(latest_file) as f:
        return [ln.strip() for ln in f if ln.strip()]

def _joined_bins_from_yaml(bins_cfg):
    """
    Return joined bin names (with '__') directly from the YAML file
    """
    try:
        with open(bins_cfg, "r") as f:
            bins_data = yaml.safe_load(f) or {}
        names = list(bins_data.keys())
        if names:
            # sanitize names to remove any characters unsafe for filenames
            import re
            safe_names = [re.sub(r"[^A-Za-z0-9_]", "__", n) for n in names]
            return "__".join(safe_names)
    except Exception:
        pass
    return None

def _rename_with_suffix(src_path, suffix, target_dir):
    """
    Move/rename src_path into target_dir with suffix inserted before the extension.
    e.g. myconfig.yaml -> myconfig_bin.yaml
    Overwrites existing target file if present.
    Returns the new path (string).
    """
    src = Path(src_path)
    target_dir = Path(target_dir)
    ext = src.suffix                # .yaml / .yml / etc
    base = src.stem                 # filename without suffix/extension

    new_name = f"{base}{suffix}{ext}"
    dest = target_dir / new_name
    dest.parent.mkdir(parents=True, exist_ok=True)

    # If destination exists, remove it so replace() will succeed
    if dest.exists():
        dest.unlink()

    # move/rename (replace will overwrite if necessary)
    return str(src.replace(dest))

def _copy_file(src, dst_dir_or_file):
    srcp = Path(src)
    dstp = Path(dst_dir_or_file)
    # If dstp has a file extension, treat it as a full file path
    if dstp.suffix:
        dst = dstp
    # If caller passed the exact filename, also treat as file path
    elif dstp.name == srcp.name:
        dst = dstp
    else:
        # Otherwise treat dstp as a directory
        dst = dstp / srcp.name
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(srcp, dst)
    return dst

def prepare_run_and_stage_assets_copy(
    run_info: Dict[str, Any],
    bins_cfg: str,
    processes_cfg: str,
    hists_cfg: Optional[str] = None,
    FDpattern_cfg: Optional[str] = None,
    existing_BFI_dir: Optional[bool] = False,
    existing_BF_dir: Optional[bool] = False,
):
    """
    Copy-only staging for run_dir.
    Returns:
      dict mapping keys like 'bins_cfg','hist_cfg','processes_cfg','exe_dir','configs_dir',...
    """
    run_dir = run_info["run_dir"]
    dirs_to_make = ["exe", "configs", "datacards", "include", "python", "src", "macro", "plots"]
    if not existing_BFI_dir:
        dirs_to_make.extend([
                             "condor",
                            ])
    if not existing_BF_dir:
        dirs_to_make.extend([
                             "condor_BF",
                            ])
    for sub in dirs_to_make:
        os.makedirs(os.path.join(run_dir, sub), exist_ok=True)
    run_path = Path(run_dir)
    # directories to maintain inside run_dir
    exe_dir = run_path / "exe"
    macro_dir = run_path / "macro"
    configs_dir = run_path / "configs"
    datacards_dir = run_path / "datacards"
    condor_dir = run_path / "condor"
    plots_dir = run_path / "plots"
    include_dir = run_path / "include"
    python_dir = run_path / "python"
    src_dir = run_path / "src"
    combine_dir = run_path / "combine"
    condor_BF_dir = run_path / "condor_BF"

    # -------------------------
    # 1) Copy selected config files
    # Note: .yaml files have extension added on just in case user repeated the same name
    # -------------------------
    config_bin_path = _copy_file(bins_cfg, configs_dir)
    config_bin_path = _rename_with_suffix(config_bin_path, "_bins", configs_dir)
    configs_processes_path = _copy_file(processes_cfg, configs_dir)
    configs_processes_path = _rename_with_suffix(configs_processes_path, "_processes", configs_dir)
    configs_hist_path = None
    if hists_cfg:
        configs_hist_path = _copy_file(hists_cfg, configs_dir)
        configs_hist_path = _rename_with_suffix(configs_hist_path, "_hists", configs_dir)
    if FDpattern_cfg:
        configs_FDpattern_path = _copy_file(FDpattern_cfg, configs_dir)
        configs_FDpattern_path = _rename_with_suffix(configs_FDpattern_path, "_FDpattern", configs_dir)

    # -------------------------
    # 2) Copy all *.x exes
    # -------------------------
    staged_exes = {}
    for exe_file in Path(".").glob("*.x"):
        dst = exe_dir / exe_file.name
        _copy_file(exe_file, dst)
        staged_exes[exe_file.name] = str(dst)

    # -------------------------
    # 3) Copy include_items into run_dir/include/
    #    and src_items into run_dir/src/
    #    and macro_items into run_dir/macro/
    #    (items may be files or directories; directories are copied recursively preserving basename)
    # -------------------------
    include_items = [
        "BuildFit.h",
    ]
    src_items = [
        "BuildFit.cpp",
    ]
    macro_items = [
        "Zscore.x",
        "CollectSignificance.py",
        "launchLimits.sh",
        "launchSignificances.sh",
        "launchCollectLimits.sh",
        "launchT2W.sh",
        "launchImpacts.sh",
        "launchFitDiagnostics.sh",
    ]
    python_items = [
        "make_Regions.py",
        "make_GSB_Regions.py",
    ]

    if not existing_BFI_dir and not existing_BF_dir:
        include_items.extend([
            "DefineUserHists.h",
            "BFICondorTools.h", # for systematics
        ])
        src_items.extend([
            "BFI_condor.cpp",
            "SampleTool.cpp",
            "PredefinedCutsBFI.cpp",
            "UserCutsBFI.cpp",
        ])

    for item in include_items:
        p = Path(Path("include") / item)
        if not p.exists():
            print(f"[run_combine] WARNING: include item '{item}' not found, skipping.", file=os.sys.stderr, flush=True)
            continue
        dst = include_dir / p.name
        _copy_file(p, dst)

    for item in src_items:
        p = Path(Path("src") / item)
        if not p.exists():
            print(f"[run_combine] WARNING: src item '{item}' not found, skipping.", file=os.sys.stderr, flush=True)
            continue
        dst = src_dir / p.name
        _copy_file(p, dst)

    for item in macro_items:
        p = Path(Path("macro") / item)
        if not p.exists():
            print(f"[run_combine] WARNING: macro item '{item}' not found, skipping.", file=os.sys.stderr, flush=True)
            continue
        dst = macro_dir / p.name
        _copy_file(p, dst)

    for item in python_items:
        p = Path(Path("python") / item)
        if not p.exists():
            print(f"[run_combine] WARNING: python item '{item}' not found, skipping.", file=os.sys.stderr, flush=True)
            continue
        dst = python_dir / p.name
        _copy_file(p, dst)

    # -------------------------
    # 5) Return mapping for use by the workflow
    # -------------------------
    mapping = {
        "run_dir": str(run_path),
        "exe_dir": str(exe_dir),
        "configs_dir": str(configs_dir),
        "datacards_dir": str(datacards_dir),
        "condor_dir": str(condor_dir),
        "plots_dir": str(plots_dir),
        "include_dir": str(include_dir),
        "python_dir": str(python_dir),
        "src_dir": str(src_dir),
        "macro_dir": str(macro_dir),
        "combine": str(combine_dir),
        "condor_BF": str(condor_BF_dir),
        "debug_log": str(run_path / "debug_run_combine.debug"),
        "bins_cfg": str(config_bin_path),
        "processes_cfg": str(configs_processes_path),
        "hist_cfg": str(configs_hist_path) if hists_cfg else None,
        "FDpattern_cfg": str(configs_FDpattern_path) if FDpattern_cfg else None,
        "staged_exes": staged_exes,
    }

    return mapping

def clean_binaries():
    """
    Run `make clean` to clean out binaries.
    """
    print("[run_combine] Cleaning previous builds...", flush=True)
    subprocess.run(["make", "clean"], check=True, stdout=subprocess.DEVNULL)

def build_binaries():
    """
    Run `make all -j8` to build the latest binaries.
    """
    print("[run_combine] Building all binaries...", flush=True)
    try:
        # Run make, capture stdout/stderr
        subprocess.run(
            ["make", "all", "-j8", "--output-sync=recurse"],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=True,
            text=True
        )
    except subprocess.CalledProcessError as e:
        # If make fails, print the captured output
        print("[run_combine] Build failed. Output from make:", flush=True)
        print(e.stdout, flush=True)
        raise
    print("[run_combine] Build finished.", flush=True)

def submit_jobs(config, processes, hist, make_json=False, make_root=False, make_cutflow=False, lumi="1.", run_dir=None, bins_per_job=1, max_materialize="100"):
    """
    Runs submitJobs.py to generate Condor scripts.
    """
    if not run_dir:
        print("[run_combine] submit_jobs needs a run directory!", flush=True)
        sys.exit(0)
    cmd = ["python3", "python/submitJobs.py", "--bins-cfg", config, "--processes-cfg", processes, "--lumi", lumi, "--run-dir", run_dir, "--bins-per-job", str(bins_per_job), "--max-materialize", max_materialize]

    if make_json:
        cmd.append("--make-json")
    if make_root:
        cmd.append("--make-root")
        if hist:
            cmd.append("--hist-yaml")
            cmd.append(hist)
            if make_cutflow:
                cmd.append("--cutflow")
    print(f"[run_combine] Running submitJobs.py with cmd: {' '.join(cmd)}", flush=True)
    subprocess.run(cmd, check=True, stdout=sys.stdout, stderr=sys.stderr)

def create_mergers(make_json=False, make_root=False, run_dir=None):
    """
    Runs createMergers.py to generate merger scripts.
    """
    if not run_dir:
        print("[run_combine] create_mergers needs a run directory!", flush=True)
        sys.exit(0)
    cmd = ["python3", "python/createMergers.py", "--make-master", "--run-dir", run_dir]

    if make_json:
        cmd.append("--do-json")
    if make_root:
        cmd.append("--do-hadd")
    subprocess.run(cmd, check=True, stdout=sys.stdout, stderr=sys.stderr)

def get_flattened_json_path(run_dir=None):
    """
    Return the path to the flattened JSON produced by the merge scripts.
    """
    if not run_dir:
        print("[run_combine] get_flattened_json_path needs a run_dir!")
        sys.exit(1)
    flattened_file = os.path.join(run_dir, "flattened.json")
    if not flattened_file:
        raise FileNotFoundError(f"No flattened JSON files found in {run_dir}/")
    return flattened_file

def get_flattened_root_path(run_dir=None):
    """
    Return the path to the merged root file produced by the merge scripts.
    """
    if not run_dir:
        print("[run_combine] get_flattened_root_path needs a run_dir!")
        sys.exit(1)
    hadd_file = os.path.join(run_dir, "final_hadded.root")
    if not hadd_file:
        raise FileNotFoundError(f"No final root files found in {run_dir}/")
    return hadd_file

def extract_signals(json_file):
    """
    Takes in a flattened json and extracts the names of the signals using the first bin
    """
    signals = []
    with open(json_file, "r") as f:
        data = json.load(f)
    for bin_name, processes in data.items():
        for proc_name, values in processes.items():
            nominal = values.get("nominal", [])
            if len(nominal) > 1:
                if "SMS" in proc_name or "Cascades" in proc_name:
                    signals.append(proc_name)
        break # only use first bin
    return signals

def print_events(json_file):
    with open(json_file, "r") as f:
        data = json.load(f)
    for bin_name, processes in data.items():
        print(f"Bin: {bin_name}", flush=True)
        for proc_name, values in processes.items():
            if len(values) > 1:
                weighted_events = round(values[1], 2)
                print(f"  {proc_name}: {weighted_events}", flush=True)

def get_all_work_dirs():
    """
    Return the directories from condor/.
    """
    return [name for name in os.listdir("condor/") if os.path.isdir(os.path.join("condor", name))]

def load_bins(bins_cfg):
    """
    Return the bin names (condor work dirs) from the input YAML.
    Sanitizes names so they are safe for filesystem/condor.
    If it fails, fall back to get_all_work_dirs() above.
    """
    try:
        with open(bins_cfg, "r") as f:
            bins_data = yaml.safe_load(f) or {}
        # The bin names are just the top-level keys
        raw_bin_names = list(bins_data.keys())
        if len(raw_bin_names) > 0:
            # Sanitize: replace bad chars with "__"
            safe_bin_names = [
                re.sub(r"[^A-Za-z0-9_]", "__", name) for name in raw_bin_names
            ]
            return safe_bin_names
    except Exception as e:
        # if YAML can't be read, fall through to fallback
        print(f"[run_combine] Warning reading bins cfg ({bins_cfg}): {e}", file=sys.stderr, flush=True)
    return get_all_work_dirs()

def load_submitted_clusters(condor_dir):
    clusters = []
    path = condor_dir / "submitted_clusters.txt"
    if path.exists():
        with open(path) as f:
            for line in f:
                parts = line.strip().split()
                if len(parts) == 2:
                    clusters.append((parts[0], parts[1]))  # (cluster_id, schedd)
                elif parts:
                    clusters.append((parts[0], None))
    return clusters

def wait_for_jobs(work_dirs = None, condor="condor"):
    """
    Block until current user's Condor job count is below the monitor threshold.
    Uses CondorJobCountMonitor to check the current user's jobs.
    """
    idle_time_start = time.time()
    monitor = CondorJobCountMonitor(threshold=1, verbose=False)
    clusters = None
    if work_dirs:
        clusters = CondorJobCountMonitor.load_clusters_for_dirs(work_dirs, condor_root=condor)
    monitor.wait_until_no_idle_jobs(clusters=clusters)
    idle_time_end = time.time()
    monitor.wait_until_jobs_below(clusters=clusters)
    return idle_time_end - idle_time_start

def run_checkjobs_loop_parallel(condor_dir=None, work_dirs=None, no_resubmit=False, max_resubmits=3, check_json=False, check_root=False):
    """
    Check all work directories with checkJobs.py, resubmit failing jobs across
    all directories in one cycle, then wait once for all resubmitted jobs to finish.
    Returns True if no failed jobs remain (proceed), False on error or if max resubmits reached.
    """
    if not condor_dir:
        print("[run_combine] run_checkjobs_loop_parallel needs a condor_dir!", flush=True)
        sys.exit(0)
    if not work_dirs:
        print("[run_combine] run_checkjobs_loop_parallel needs work_dirs!", flush=True)
        sys.exit(0)

    attempt = 0
    check_marker_no_failed = "[checkJobs] No failed jobs to resubmit."
    check_marker_resub_ok = "[checkJobs] Resubmit submitted successfully."

    while attempt < max_resubmits:
        attempt += 1
        print(f"[run_combine] Running checkJobs for {work_dirs} (attempt {attempt}/{max_resubmits})...", flush=True)

        resubmitted_dirs = []
        any_unexpected = False

        for work_dir in work_dirs:
            check_cmd = ["python3", "python/checkJobs.py", work_dir, "--root-dir", condor_dir]
            if check_json:
                check_cmd.append("--check-json")
            if check_root:
                check_cmd.append("--check-root")
            proc = subprocess.run(check_cmd, capture_output=True, text=True)

            # Print outputs (labeled)
            if proc.stderr:
                print(f"----- checkJobs stderr ({work_dir}) -----", file=sys.stderr, flush=True)
                print(proc.stderr, file=sys.stderr, flush=True)

            if proc.returncode != 0:
                print(f"[run_combine] checkJobs.py returned non-zero ({proc.returncode}) for {work_dir}. Aborting.", file=sys.stderr, flush=True)
                return False

            stdout_lines = [line.strip() for line in (proc.stdout or "").splitlines()]
            if any(check_marker_no_failed == line for line in stdout_lines):
                continue
            elif any(check_marker_resub_ok == line for line in stdout_lines):
                resubmitted_dirs.append(work_dir)
            else:
                print(f"[run_combine] Unexpected checkJobs output for {work_dir}. See printed stdout/stderr above.", file=sys.stderr, flush=True)
                any_unexpected = True

        if any_unexpected:
            return False

        if not resubmitted_dirs:
            print("[run_combine] No failed jobs remaining in any work_dir. Proceeding.", flush=True)
            return True

        if no_resubmit:
            print(f"[run_combine] Resubmissions would be performed in {resubmitted_dirs}, but no_resubmit=True. Stopping.", flush=True)
            return False

        # wait once for all resubmitted jobs across all dirs
        print(f"[run_combine] Resubmitted jobs in {resubmitted_dirs}. Waiting for all resubmitted jobs to finish...", flush=True)
        wait_for_jobs(work_dirs, condor_dir)
        time.sleep(3) # buffer time for new outputs to transfer before recheck
        # after wait, loop again to re-run checkJobs across all dirs

    # reached max attempts
    print(f"[run_combine] Reached max_resubmits ({max_resubmits}). Giving up.", file=sys.stderr, flush=True)
    return False

def run_checkjobs_loop_parallel_BF(condor_dir, work_dirs, no_resubmit=False, max_resubmits=3):
    """
    Check all BF work directories, resubmit failing jobs across all dirs, then wait for them to finish.
    Returns True if no failed jobs remain, False if max resubmits reached or unexpected errors.
    """
    attempt = 0
    while attempt < max_resubmits:
        attempt += 1
        print(f"[run_combine] Running checkJobsBF (attempt {attempt}/{max_resubmits})...", flush=True)
        any_failed_this_round = False
        resubmitted_dirs = []
        for work_dir in work_dirs:
            check_cmd = ["python3", "python/checkJobsBF.py", work_dir, "--root-dir", condor_dir]
            if no_resubmit:
                check_cmd.append("--no-submit")
            proc = subprocess.run(check_cmd, capture_output=True, text=True)
            # Print outputs
            if proc.stdout:
                print(f"----- checkJobsBF stdout ({work_dir}) -----", flush=True)
                print(proc.stdout, flush=True)
            if proc.stderr:
                print(f"----- checkJobsBF stderr ({work_dir}) -----", file=sys.stderr, flush=True)
                print(proc.stderr, file=sys.stderr, flush=True)
            if proc.returncode != 0:
                print(f"[run_combine] checkJobsBF returned non-zero ({proc.returncode}) for {work_dir}. Aborting.", file=sys.stderr)
                return False
            # Check if any resubmit files were created
            resub_files = [f for f in os.listdir(os.path.join(condor_dir, work_dir)) if f.startswith("resubmit_") and f.endswith(".sub")]
            if resub_files:
                any_failed_this_round = True
                resubmitted_dirs.append(work_dir)
        if not any_failed_this_round:
            print("[run_combine] No failed jobs remaining. Proceeding.", flush=True)
            return True
        if no_resubmit:
            print(f"[run_combine] Failed jobs exist in {resubmitted_dirs}, but no_resubmit=True. Stopping.", flush=True)
            return False
        # Wait for all resubmitted jobs to finish
        print(f"[run_combine] Waiting for resubmitted jobs in {resubmitted_dirs} to finish...", flush=True)
        wait_for_jobs(work_dirs, condor_dir)
        time.sleep(3)  # small buffer for output transfer
    # Reached max resubmits
    print(f"[run_combine] Reached max_resubmits ({max_resubmits}). Some jobs may still be failing.", file=sys.stderr, flush=True)
    return False

def run_checkjobs_loop_parallel_Combine(
    condor_dir,
    work_dirs,
    checker="python/checkJobsCombine.py",
    checker_args=None,
    no_resubmit=False,
    max_resubmits=3,
):
    """
    Check combine jobs, resubmit failures across all dirs, then wait once for all resubmitted jobs.
    """
    if checker_args is None:
        checker_args = []

    attempt = 0

    while attempt < max_resubmits:
        attempt += 1
        print(
            f"[run_combine] Running combine check ({attempt}/{max_resubmits})...",
            flush=True,
        )

        resubmitted_dirs = []

        for work_dir in work_dirs:
            check_cmd = [
                "python3",
                checker,
                work_dir,
                "--root-dir", condor_dir,
            ] + checker_args

            if no_resubmit:
                check_cmd.append("--no-submit")

            proc = subprocess.run(check_cmd, capture_output=True, text=True)

            if proc.stdout:
                print(f"----- combine check stdout ({work_dir}) -----", flush=True)
                print(proc.stdout, flush=True)
            if proc.stderr:
                print(
                    f"----- combine check stderr ({work_dir}) -----",
                    file=sys.stderr,
                    flush=True,
                )
                print(proc.stderr, file=sys.stderr, flush=True)

            if proc.returncode != 0:
                print(
                    f"[run_combine] Combine check returned non-zero "
                    f"({proc.returncode}) for {work_dir}",
                    file=sys.stderr,
                    flush=True,
                )
                return False

            resub_dir = os.path.join(condor_dir, work_dir)
            if not os.path.isdir(resub_dir):
                continue
            resub_files = [
                f for f in os.listdir(resub_dir)
                if f.startswith("resubmit_") and f.endswith(".sub")
            ]
            if resub_files:
                resubmitted_dirs.append(work_dir)

        if not resubmitted_dirs:
            print(
                "[run_combine] No failed combine jobs remaining. Proceeding.",
                flush=True,
            )
            return True

        if no_resubmit:
            print(
                f"[run_combine] Failed combine jobs in {resubmitted_dirs}, "
                "but no_resubmit=True. Stopping.",
                flush=True,
            )
            return False

        print(
            f"[run_combine] Waiting for resubmitted combine jobs in {resubmitted_dirs}...",
            flush=True,
        )
        wait_for_jobs(work_dirs=work_dirs, condor=condor_dir)
        time.sleep(3)

    print(
        f"[run_combine] Reached max_resubmits ({max_resubmits}) for combine jobs.",
        file=sys.stderr,
        flush=True,
    )
    return False

# ----- main workflow -----
def main(args, run_info, try_acquire_lock_or_exit, start_time):
    # canonical run directory / name
    run_dir = run_info.get("run_dir")
    run_name = run_info.get("run_name")
    print(f"[run_combine] Running on host: {os.environ['HOSTNAME']}", flush=True)
    print(f"[run_combine] Using run directory: {run_dir}", flush=True)

    make_impacts = args.make_impacts
    make_FD = args.make_FD
    cutflow = False

    # Disallow passing both existing-BFI-dir and existing-BF-dir
    if args.existing_BFI_dir and args.existing_BF_dir:
        print("[run_combine] ERROR: --existing-BFI-dir and --existing-BF-dir are mutually exclusive.", file=sys.stderr, flush=True)
        sys.exit(1)

    # If either existing_BFI or existing_BF is provided, load configs from the existing run's configs/
    if args.existing_BFI_dir or args.existing_BF_dir:
        existing_dir = args.existing_BFI_dir if args.existing_BFI_dir else args.existing_BF_dir
        config_dir = os.path.join(existing_dir, "configs")
        bins_files = glob.glob(os.path.join(config_dir, "*bins.yaml"))
        hists_files = glob.glob(os.path.join(config_dir, "*hists.yaml"))
        processes_files = glob.glob(os.path.join(config_dir, "*processes.yaml"))
        FDpatterns_files = glob.glob(os.path.join(config_dir, "*FDpattern.yaml"))

        if not bins_files:
            raise FileNotFoundError(f"No '*bins.yaml' file found in {config_dir}")
        elif len(bins_files) > 1:
            print(f"[run_combine] Warning: Multiple '*bins.yaml' files found. Using the first one.")
        if not hists_files:
            raise FileNotFoundError(f"No '*hists.yaml' file found in {config_dir}")
        elif len(hists_files) > 1:
            print(f"[run_combine] Warning: Multiple '*hists.yaml' files found. Using the first one.")
        if not FDpatterns_files and args.make_FD:
            raise FileNotFoundError(f"No '*FDpattern.yaml' file found in {config_dir}")
        elif len(FDpatterns_files) > 1:
            print(f"[run_combine] Warning: Multiple '*FDpattern.yaml' files found. Using the first one.")
        if not processes_files:
            raise FileNotFoundError(f"No '*processes.yaml' file found in {config_dir}")
        elif len(processes_files) > 1:
            print(f"[run_combine] Warning: Multiple '*processes.yaml' files found. Using the first one.")

        bins_cfg = bins_files[0]
        hist_cfg = hists_files[0]
        processes_cfg = processes_files[0]
        FDpattern_cfg = ""
        if len(FDpatterns_files) > 0:
            FDpattern_cfg = FDpatterns_files[0]
        make_json = os.path.isfile(os.path.join(existing_dir, "flattened.json"))
        make_root = False
        
        if not make_json:
            print("[run_combine] ERROR: Could not find flattened.json in", existing_dir)
            sys.exit(1)
        else:
            _copy_file(os.path.join(existing_dir, "flattened.json"), run_dir)
            flattened_json = os.path.join(run_dir, "flattened.json")

        # If this is an existing BF run, copy its datacards under the new run's datacards/,
        # but only copy subdirectories which contain both:
        #   1) a datacard named <subdirname>.txt
        #   2) json_shapes_flat.root
        if args.existing_BF_dir:
            src_datacards = os.path.join(existing_dir, "datacards")
            dst_datacards = os.path.join(run_dir, "datacards")
            if os.path.isdir(src_datacards):
                print(f"[run_combine] Copying qualifying datacards from {src_datacards} -> {dst_datacards}", flush=True)
                # Walk all directories under src_datacards and look for qualifying folders
                for root, dirs, files in os.walk(src_datacards):
                    # basename of the directory we're inspecting
                    base = os.path.basename(root)
                    # skip the top-level datacards directory itself (it won't have base datacard)
                    if root == src_datacards:
                        continue
                    expected_datacard = os.path.join(root, f"{base}.txt")
                    expected_shapes = os.path.join(root, "json_shapes_flat.root")
                    if os.path.isfile(expected_datacard) and os.path.isfile(expected_shapes):
                        rel = os.path.relpath(root, src_datacards)
                        dest_dir = os.path.join(dst_datacards, rel)
                        os.makedirs(dest_dir, exist_ok=True)
                        try:
                            _copy_file(expected_datacard, dest_dir)
                            _copy_file(expected_shapes, dest_dir)
                        except Exception as e:
                            print(f"[run_combine] Warning: failed copying datacard files for {rel}: {e}", file=sys.stderr, flush=True)
                    else:
                        # skip if the two required files are not both present
                        print(f"[run_combine] Warning: no datacard or shapes file for {base}", file=sys.stderr, flush=True)
                        pass
                print(f"[run_combine] Copied datacard and shapes from {src_datacards}", flush=True)
            else:
                print(f"[run_combine] ERROR: no datacards directory found in {existing_dir}!", flush=True)
                sys.exit(1)

    else:
        # use args when no existing run provided
        bins_cfg = args.bins_cfg
        hist_cfg = args.hist_cfg
        processes_cfg = args.processes_cfg
        FDpattern_cfg = args.FDpattern_cfg
        make_json = args.make_json
        make_root = args.make_root
        if not make_json and not make_root:
            make_json = True # if user passed neither option then make json
        if args.stress_test:
            print("[run_combine] Running stress test. Using stress yamls instead of loaded arg yamls", flush=True)
            bins_cfg = "config/bin_cfgs/bin_stress.yaml"
            hist_cfg = "config/hist_cfgs/hist_stress.yaml"
            processes_cfg = "config/process_cfgs/processes_stress.yaml"
            make_json = True
            make_root = True
        if args.make_root and args.cutflow:
            cutflow = True

    # Acquire lock (blocking or not)
    lock = None
    try:
        # NOTE: set non_blocking=True to exit if lock is held, False to block until acquired
        lock = try_acquire_lock_or_exit(non_blocking=False)
        print("[run_combine] Build/stage lock acquired; starting compile and staging...", file=sys.__stdout__, flush=True)

        # Compile framework (inside the lock)
        if not args.skip_compile:
            clean_binaries()
            build_binaries()

        # Stage files into the run directory (configs, exe, src, include, condor, plots, macro, etc.)
        # For either existing_BFI_dir or existing_BF_dir want the same "existing" behavior for staging
        run_dir_map = prepare_run_and_stage_assets_copy(
            run_info,
            bins_cfg,
            processes_cfg,
            hist_cfg,
            FDpattern_cfg,
            existing_BFI_dir=bool(args.existing_BFI_dir),
            existing_BF_dir=bool(args.existing_BF_dir),
        )
        # merge staged mapping into run_info so downstream code can use run_info everywhere
        run_info.update(run_dir_map)

    except Exception as e:
        # Print immediate feedback to the original terminal, then re-raise so the debug log also captures the stack.
        print(f"[run_combine] ERROR during build/stage: {e}", file=sys.__stdout__, flush=True)
        # Ensure lock is released if held
        if lock:
            try:
                lock.release()
                print("[run_combine] Build/stage lock released (due to error).", file=sys.__stdout__, flush=True)
            except Exception:
                pass
        raise
    else:
        # Normal successful completion of the critical section: release lock and continue.
        if lock:
            try:
                lock.release()
                print("[run_combine] Build/stage complete; lock released.", file=sys.__stdout__, flush=True)
            except Exception:
                pass

    # update locals to point at staged versions
    run_dir = run_info["run_dir"]
    print(f"[run_combine] Final run directory: {run_dir}", flush=True)
    bins_cfg = run_info.get("bins_cfg", bins_cfg)
    processes_cfg = run_info.get("processes_cfg", processes_cfg)
    hist_cfg = run_info.get("hist_cfg", hist_cfg)
    FDpattern_cfg = run_info.get("FDpattern_cfg", FDpattern_cfg)

    # convenience local paths
    condor_dir = run_info.get("condor_dir")
    plots_dir = run_info.get("plots_dir")
    configs_dir = run_info.get("configs_dir")
    exe_dir = run_info.get("exe_dir")
    macro_dir = run_info.get("macro_dir")
    condor_BF = run_info["condor_BF"]
    output_dir = run_info["datacards_dir"]
    lumi = args.lumi
    plot_lumi = lumi # lumi used for labels in plots
    if plot_lumi == "-1":
        plot_lumi = "400" # set to estimated Run2 + Run3 for now

    # Skip BFI steps if BFI_dir or BF_dir
    if not (args.existing_BFI_dir or args.existing_BF_dir):
        # Submit jobs (give submit_jobs the run-local condor dir so everything stays inside the run)
        print("[run_combine] Submitting jobs...", flush=True)
        submit_jobs(
            config=bins_cfg,
            processes=processes_cfg,
            hist=hist_cfg,
            make_json=make_json,
            make_root=make_root,
            make_cutflow=cutflow,
            lumi=lumi,
            run_dir=condor_dir,
            bins_per_job=args.bins_per_job,
            max_materialize=args.max_materialize
        )

        # Create merge scripts (master merge should live in the run condor dir)
        print("[run_combine] Creating merger scripts...", flush=True)
        create_mergers(make_json=make_json, make_root=make_root, run_dir=run_dir)

        # Wait for jobs to finish; pass condor path to functions that need it
        condor_time_start = time.time()
        print("[run_combine] Waiting for condor jobs to finish...", flush=True)
        
        # Prefer bins_list* text file if present, since it records actual job group dirs
        auto_bins = _read_condor_bins_list(condor_dir)
        if auto_bins:
            loaded_bins = auto_bins
        else:
            # Fallback: detect subdirectories created by createJobs.py
            try:
                condor_path = Path(condor_dir)
                if condor_path.is_dir():
                    detected = [d.name for d in condor_path.iterdir() if d.is_dir()]
                else:
                    detected = []
            except Exception as e:
                print(f"[run_combine] Warning: failed to list condor dir '{condor_dir}': {e}", flush=True)
                detected = []
        
            if detected:
                loaded_bins = sorted(detected)
                print(f"[run_combine] Using detected condor work dirs", flush=True)
            else:
                # Final fallback: YAML bins
                loaded_bins = load_bins(bins_cfg)
                print(f"[run_combine] No condor work dirs found; falling back to YAML bin list ({len(loaded_bins)} bins).", flush=True)
        
        idle_time_seconds = wait_for_jobs(work_dirs=loaded_bins, condor=condor_dir)

        # Run checkJobs loop and resubmit if necessary
        print("[run_combine] Checking for failed jobs and resubmitting if necessary...", flush=True)
        ok = run_checkjobs_loop_parallel(
            condor_dir=condor_dir,
            work_dirs=loaded_bins,
            no_resubmit=False,
            max_resubmits=args.max_resubmits,
            check_json=make_json,
            check_root=make_root,
        )
        if not ok:
            print("[run_combine] checkJobs step did not complete successfully. Aborting further steps.", file=sys.stderr)
            sys.exit(1)
        condor_time_end = time.time()
        condor_time_seconds = condor_time_end - condor_time_start

        # Run all merge scripts - master merge script lives in the run condor dir
        master_merge_sh = os.path.join(condor_dir, f"master_merge.sh")
        if not os.path.exists(master_merge_sh):
            print(f"[run_combine] ERROR: master merge script not found: {master_merge_sh}", file=sys.stderr)
            sys.exit(1)
        print(f"[run_combine] Running master merge script: {master_merge_sh}", flush=True)
        subprocess.run(["bash", master_merge_sh], check=True, stdout=sys.stdout, stderr=sys.stderr)

        if make_root and not args.make_trig_fit:
            # Plot Histograms
            hadd_file = get_flattened_root_path(run_dir=run_dir)
            plot_cmd = [
                "./"+exe_dir+"/PlotHistograms.x",
                "-i", hadd_file,
                "-o", plots_dir,
                "-l", plot_lumi,
                "--ratios", hist_cfg,
            ]
            print("[run_combine] Plotting histograms with command:", " ".join(plot_cmd), flush=True)
            subprocess.run(plot_cmd, check=True, stdout=sys.stdout, stderr=sys.stderr)
        if make_root and args.make_trig_fit:
            hadd_file = get_flattened_root_path(run_dir=run_dir)
            plot_cmd = [
                "./"+exe_dir+"/PlotTriggers.x",
                "-i", hadd_file,
                "-o", plots_dir,
                "--ratios", hist_cfg,
            ]
            print("[run_combine] Plotting triggers with command:", " ".join(plot_cmd), flush=True)
            subprocess.run(plot_cmd, check=True, stdout=sys.stdout, stderr=sys.stderr)
            trig_fit_cmd = [
                "./"+exe_dir+"/Trigger_SFs.x",
                "-f", plots_dir+"/output_triggers_final_hadded.root",
            ]
            print("[run_combine] Fitting trigger turn-ons with command:", " ".join(trig_fit_cmd), flush=True)
            subprocess.run(trig_fit_cmd, check=True, stdout=sys.stdout, stderr=sys.stderr)

    idle_time_seconds_BF = 0

    plot_yields_proc = None
    if make_json:
        flattened_json = get_flattened_json_path(run_dir=run_dir)
        if not args.existing_BFI_dir and not args.existing_BF_dir and not args.skip_plot_yields:
            plot_cmd = [
                "./"+exe_dir+"/PlotYields.x",
                "-i", flattened_json,
                "-o", plots_dir,
                "-l", plot_lumi,
                "--config", FDpattern_cfg,
            ]
            print("[run_combine] Plotting yields with command:", " ".join(plot_cmd), flush=True)
            plot_yields_proc = subprocess.Popen(
                plot_cmd,
                stdout=sys.stdout,
                stderr=sys.stderr,
                start_new_session=True  # recommended for long jobs
            )
        if not args.only_yields:
            signals = extract_signals(flattened_json)
            if not args.existing_BF_dir:
                condor_time_start_BF = time.time()
                for sig in signals:
                    BF_condor_cmd = [
                        "python3", "python/submitBFJobs.py",
                        "--output-dir", output_dir,
                        "--executable", "./"+exe_dir+"/BF.x",
                        "--logs-dir", f'{condor_BF}/{sig}/',
                        "--json", flattened_json, 
                        "--signal", sig,
                        "--submit-file", f'{condor_BF}/{sig}/job_{sig}.sub',
                        "--record-dir", f'{condor_BF}/{sig}/',
                    ]
                    subprocess.run(BF_condor_cmd, check=True, capture_output=True, text=True)

                # Run checkJobs loop and resubmit if necessary
                idle_time_seconds_BF = wait_for_jobs(work_dirs=signals, condor=condor_BF)
                print("[run_combine] Checking for failed jobs and resubmitting if necessary...", flush=True)
                ok = run_checkjobs_loop_parallel_BF(
                    condor_dir=condor_BF,
                    work_dirs=signals,
                    no_resubmit=False,
                    max_resubmits=args.max_resubmits,
                )
                if not ok:
                    print("[run_combine] BF checkJobs step did not complete successfully. Aborting further steps.", file=sys.stderr)
                    sys.exit(1)
                condor_time_end_BF = time.time()
                condor_time_seconds_BF = condor_time_end_BF - condor_time_start_BF

            if not make_FD and not make_impacts:
                print("[run_combine] Launching limit jobs...", flush=True)
                condor_time_start_combine = time.time()
                
                for sig in signals:
                    limits_submit_cmd = [
                        "python3", "python/submitCombineJobs.py",
                        "--signal", sig,
                        "--output-dir", output_dir,
                        "--method", "AsymptoticLimits",
                        "--extra-args", "-n .limit",
                    ]
                    subprocess.run(limits_submit_cmd, check=True, stdout=sys.stdout, stderr=sys.stderr)
                
                print("[run_combine] Launched limit jobs", flush=True)
                
                if len(signals) < 20: # don't run significance for all points
                    print("[run_combine] Launching significance jobs...", flush=True)
                    for sig in signals:
                        significances_submit_cmd = [
                            "python3", "python/submitCombineJobs.py",
                            "--signal", sig,
                            "--output-dir", output_dir,
                            "--method", "Significance",
                            "--extra-args", "-n .Test --expectSignal=1 -t -1",
                        ]
                        subprocess.run(significances_submit_cmd, check=True, stdout=sys.stdout, stderr=sys.stderr)
                    
                    print("[run_combine] Launched significance jobs", flush=True)
                
                # Wait once for ALL combine jobs
                print("[run_combine] Waiting for combine jobs...", flush=True)
                wait_for_jobs(work_dirs=signals, condor=output_dir)

                # Check limits
                ok = run_checkjobs_loop_parallel_Combine(
                    condor_dir=output_dir,
                    work_dirs=signals,
                    checker="python/checkJobsCombine.py",
                    checker_args=["--method", "AsymptoticLimits"],
                    no_resubmit=False,
                    max_resubmits=args.max_resubmits,
                )
                
                if not ok:
                    print("[run_combine] Limit combine jobs failed. Aborting.", file=sys.stderr)
                    sys.exit(1)
                
                if len(signals) < 20: # don't run significance for all points
                    # Check significances
                    ok = run_checkjobs_loop_parallel_Combine(
                        condor_dir=output_dir,
                        work_dirs=signals,
                        checker="python/checkJobsCombine.py",
                        checker_args=["--method", "Significance"],
                        no_resubmit=False,
                        max_resubmits=args.max_resubmits,
                    )
                    
                    if not ok:
                        print("[run_combine] Significance combine jobs failed. Aborting.", file=sys.stderr)
                        sys.exit(1)
                
                condor_time_end_combine = time.time()
                condor_time_seconds_combine = condor_time_end_combine - condor_time_start_combine

                print("[run_combine] Launching CollectLimits...", flush=True)
                subprocess.run(["bash", macro_dir+"/launchCollectLimits.sh", output_dir, run_dir], check=True, stdout=sys.stdout, stderr=sys.stderr)

                # collect significances
                if len(signals) < 20: # don't run significance for all points
                    try:
                        print("[run_combine] Collecting significances...", flush=True)
                        subprocess.run(["python3", "-u", macro_dir+"/CollectSignificance.py", output_dir, run_dir], check=True, stdout=sys.stdout, stderr=sys.stderr)
                    except Exception: # typical failure is because a signal process has 0 events in any bins
                        pass

                    # plot significances
                    plot_cmd = [
                        "./"+exe_dir+"/PlotSignificances.x",
                        "-i", run_dir+"/Significance_datacards.txt",
                        "-o", plots_dir
                    ]
                    print("[run_combine] Plotting significances with command:", " ".join(plot_cmd), flush=True)
                    subprocess.run(plot_cmd, check=True, stdout=sys.stdout, stderr=sys.stderr)

            # Note impacts, T2W, and FD in macro only run on first signal
            if make_impacts or make_FD:
                # T2W
                T2W_cmd = [
                    "bash",
                    macro_dir+"/launchT2W.sh",
                    output_dir,
                    run_dir
                ]
                print("[run_combine] Running T2W with command:", " ".join(T2W_cmd), flush=True)
                subprocess.run(T2W_cmd, check=True, stdout=sys.stdout, stderr=sys.stderr)

            # Impacts
            if make_impacts:
                # Impacts
                impacts_cmd = [
                    "bash",
                    macro_dir+"/launchImpacts.sh",
                    output_dir,
                    run_dir
                ]
                print("[run_combine] Running impacts with command:", " ".join(impacts_cmd), flush=True)
                subprocess.run(impacts_cmd, check=True, stdout=sys.stdout, stderr=sys.stderr)

                # Collect all impacts.pdf into plots/pdfs/impacts
                impacts_src_root = output_dir  # datacards dir
                impacts_dst_root = os.path.join(plots_dir, "pdfs", "impacts")
                os.makedirs(impacts_dst_root, exist_ok=True)

                print(f"[run_combine] Collecting impacts.pdf files into {impacts_dst_root}", flush=True)
                for subdir in os.listdir(impacts_src_root):
                    subdir_path = os.path.join(impacts_src_root, subdir)
                    impacts_pdf = os.path.join(subdir_path, "impacts.pdf")
                    if os.path.isdir(subdir_path) and os.path.isfile(impacts_pdf):
                        dst_file = os.path.join(impacts_dst_root, f"impacts__{run_name}__{subdir}.pdf")
                        try:
                            _copy_file(impacts_pdf, dst_file)
                        except Exception as e:
                            print(f"[run_combine] Warning: failed to copy {impacts_pdf}: {e}", flush=True)
                    impacts_alpha_pdf = os.path.join(subdir_path, "impacts_alpha.pdf")
                    if os.path.isdir(subdir_path) and os.path.isfile(impacts_alpha_pdf):
                        dst_file = os.path.join(impacts_dst_root, f"impacts_alpha__{run_name}__{subdir}.pdf")
                        try:
                            _copy_file(impacts_alpha_pdf, dst_file)
                        except Exception as e:
                            print(f"[run_combine] Warning: failed to copy {impacts_alpha_pdf}: {e}", flush=True)

            # FitDiagnostics
            if make_FD:
                # local
                FD_cmd = [
                    "bash",
                    macro_dir+"/launchFitDiagnostics.sh",
                    output_dir,
                    run_dir
                ]
                print("[run_combine] Running FitDiagnostics with command:", " ".join(FD_cmd), flush=True)
                subprocess.run(FD_cmd, check=True, stdout=sys.stdout, stderr=sys.stderr)
                # Plot FD
                FD_output_name = 'fitDiagnostics.Test.root'
                FD_files = [os.path.abspath(f) for f in glob.glob(os.path.join(output_dir, '**', FD_output_name), recursive=True)]
                for FD_file in FD_files:
                    FD_plot_cmd_prefit = [
                        "./"+exe_dir+"/PlotFitDiagnostics.x",
                        "-i", FD_file,
                        "-o", plots_dir,
                        "--config", FDpattern_cfg,
                        "-t", "shapes_prefit"
                    ]
                    print("[run_combine] Running prefit FitDiagnostics plotter with command:", " ".join(FD_plot_cmd_prefit), flush=True)
                    subprocess.run(FD_plot_cmd_prefit, check=True, stdout=sys.stdout, stderr=sys.stderr)
                    FD_plot_cmd_postfit = [
                        "./"+exe_dir+"/PlotFitDiagnostics.x",
                        "-i", FD_file,
                        "-o", plots_dir,
                        "--config", FDpattern_cfg,
                        "-t", "shapes_fit_b"
                    ]
                    print("[run_combine] Running postfit FitDiagnostics plotter with command:", " ".join(FD_plot_cmd_postfit), flush=True)
                    subprocess.run(FD_plot_cmd_postfit, check=True, stdout=sys.stdout, stderr=sys.stderr)
                    # make CSV
                    FD_CSV_cmd = [
                        "root",
                        "-l",
                        "-b",
                        f'macro/CreatePrePostCSV.C("{FD_file}","{FD_file.replace(".Test.root","CSV.csv")}")'
                    ]
                    print("[run_combine] Making csv/zscore files with command:", " ".join(FD_CSV_cmd), flush=True)
                    subprocess.run(FD_CSV_cmd, check=True, stdout=sys.stdout, stderr=sys.stderr)
                    FD_Zscore_cmd = [
                        "./"+macro_dir+"/Zscore.x",
                        "-f 2",
                        "-i", f'{FD_file.replace(".Test.root","CSV_zscore.txt")}',
                        "-o", output_dir+"/"+str(signals[0]),
                    ]
                    print("[run_combine] Running Zscore exe with command:", " ".join(FD_Zscore_cmd), flush=True)
                    subprocess.run(FD_Zscore_cmd, check=True, stdout=sys.stdout, stderr=sys.stderr)

    # Clean up tar
    if not args.only_yields and make_json:
        cmssw_tarball = condor_BF + '/../cmssw_runtime.tgz'
        if os.path.exists(cmssw_tarball):
            os.remove(cmssw_tarball)

    if plot_yields_proc is not None:
        print("[run_combine] Waiting for PlotYields to finish...", flush=True)
        ret = plot_yields_proc.wait()
        if ret != 0:
            print("[run_combine] PlotYields failed with return code", ret, file=sys.stderr)
            sys.exit(1)
    print("[run_combine] All steps completed.", flush=True)
    end_time = time.time()
    # total time
    total_time_seconds = end_time - start_time
    if not args.existing_BFI_dir and not args.existing_BF_dir:
        print("Time for all condor jobs to start running: {:.2f} seconds = {:.2f} minutes = {:.2f} hours".format(
            idle_time_seconds, idle_time_seconds/60, idle_time_seconds/3600), flush=True)
        print("Time for condor processing: {:.2f} seconds = {:.2f} minutes = {:.2f} hours".format(
            condor_time_seconds, condor_time_seconds/60, condor_time_seconds/3600), flush=True)
    if not args.only_yields and make_json:
        if not args.existing_BF_dir:
            print("Time for all BF condor jobs to start running: {0:.2f} seconds = {1:.2f} minutes = {2:.2f} hours".format(
                idle_time_seconds_BF, idle_time_seconds_BF/60, idle_time_seconds_BF/3600), flush=True)
            print("Total for BF condor processing: {0:.2f} seconds = {1:.2f} minutes = {2:.2f} hours".format(
                condor_time_seconds_BF, condor_time_seconds_BF/60, condor_time_seconds_BF/3600), flush=True)
        if not make_FD and not make_impacts:
            print("Total for combine condor processing: {0:.2f} seconds = {1:.2f} minutes = {2:.2f} hours".format(
                condor_time_seconds_combine, condor_time_seconds_combine/60, condor_time_seconds_combine/3600), flush=True)
    print("Total time: {0:.2f} seconds = {1:.2f} minutes = {2:.2f} hours".format(
        total_time_seconds, total_time_seconds/60, total_time_seconds/3600), flush=True)

if __name__ == "__main__":
    start_time = time.time()
    args = parse_args()
    # call early_setup first so run_dir and logging redirection exist early
    if args.existing_BFI_dir:
        run_info, try_acquire_lock_or_exit = early_setup(args.run_name, args.existing_BFI_dir)
    elif args.existing_BF_dir:
        run_info, try_acquire_lock_or_exit = early_setup(args.run_name, args.existing_BF_dir)
    else:
        run_info, try_acquire_lock_or_exit = early_setup(args.run_name)
    main(args, run_info, try_acquire_lock_or_exit, start_time)
