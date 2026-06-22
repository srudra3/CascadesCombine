#!/usr/bin/env python3
import subprocess
import time

# ======== CONFIG ========
BASE = "root://cmseos.fnal.gov//store/user/lpcsusylep/NTUPLES_Cascades_v9"
DRY_RUN = False
MAX_RETRIES = 3
RETRY_DELAY = 5  # seconds

JOBS = [

    # ----------------------------------------------------------------
    # Signal jobs (explicit file lists)
    # ----------------------------------------------------------------
    # ----------------------------------------------------------------
    # Step 1: Summer22_130X_SMS -> all 106X and 130X dirs
    # Simple swap naming (Summer22_130X -> target)
    # Just for TSlepSlep_TEST and TChiWZTEST single-mass files
    # ----------------------------------------------------------------
    {
        "source_dir": "Summer22_130X_SMS",
        "source_suffix": "Summer22_130X",
        "dest_dirs": [
            "Summer20UL16APV_106X_SMS",
            "Summer20UL16_106X_SMS",
            "Summer20UL17_106X_SMS",
            "Summer20UL18_106X_SMS",
            "Summer22EE_130X_SMS",
            "Summer23_130X_SMS",
            "Summer23BPix_130X_SMS",
            "Summer24_130X_SMS",
            "Summer25_130X_SMS",
            "Summer26_130X_SMS",
        ],
        "compound_naming": False,
        "files": [
            "TChiWZ_MNLSP300_MLSP290_EDFilterOR_TuneCP5_13p6TeV-madgraph-pythia8_Summer22_130X.root",
            "TSlepSlep_MSlep250_MLSP245_TuneCP5_13p6TeV-madgraph-pythia8_Summer22_130X.root",
        ],
    },

    # ----------------------------------------------------------------
    # Step 2: Fan-out with compound naming
    # ----------------------------------------------------------------
    {
        "source_dir": "Summer20UL16APV_106X_SMS",
        "source_suffix": "Summer20UL16APV_106X",
        "dest_dirs": [
            "Summer22EE_130X_SMS",
        ],
        "compound_naming": True,
        "files": [
            "SMS-TChiWZ_ZToLL_mZMin-0p1_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
        ],
    },
    {
        "source_dir": "Summer20UL16_106X_SMS",
        "source_suffix": "Summer20UL16_106X",
        "dest_dirs": [
            "Summer22_130X_SMS",
        ],
        "compound_naming": True,
        "files": [
            "SMS-TChiWZ_ZToLL_mZMin-0p1_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
        ],
    },
    {
        "source_dir": "Summer20UL17_106X_SMS",
        "source_suffix": "Summer20UL17_106X",
        "dest_dirs": [
            "Summer23_130X_SMS",
            "Summer24_130X_SMS",
            "Summer26_130X_SMS",
        ],
        "compound_naming": True,
        "files": [
            "SMS-TChiWZ_ZToLL_mZMin-0p1_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
        ],
    },
    {
        "source_dir": "Summer20UL18_106X_SMS",
        "source_suffix": "Summer20UL18_106X",
        "dest_dirs": [
            "Summer23BPix_130X_SMS",
            "Summer25_130X_SMS",
        ],
        "compound_naming": True,
        "files": [
            "SMS-TChiWZ_ZToLL_mZMin-0p1_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
        ],
    },
    {
        "source_dir": "Summer16_102X_SMS",
        "source_suffix": "Summer16_102X",
        "dest_dirs": [
            "Summer22_130X_SMS",
            "Summer22EE_130X_SMS",
        ],
        "compound_naming": True,
        "files": [
            "SMS-TChiWZ_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_102X.root",
            "SMS-TChiWZ_dM-60to90_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_102X.root",
            "SMS-TChipmWW_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_102X.root",
            "SMS-TChipmWW_dM-3to50_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_102X.root",
            "SMS-TChipmWW_dM-60to90_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_102X.root",
            "SMS-TSlepSlep_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_102X.root",
            "SMS-TSlepSlep_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_102X.root",
            "TChiWZ_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_102X.root",
        ],
    },
    {
        "source_dir": "Fall17_102X_SMS",
        "source_suffix": "Fall17_102X",
        "dest_dirs": [
            "Summer23_130X_SMS",
            "Summer23BPix_130X_SMS",
        ],
        "compound_naming": True,
        "files": [
            "SMS-TChiWZ_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root",
            "SMS-TChiWZ_dM-60to90_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root",
            "SMS-TChipmWW_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root",
            "SMS-TChipmWW_dM-3to50_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root",
            "SMS-TChipmWW_dM-60to90_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root",
            "SMS-TSlepSlep_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root",
            "SMS-TSlepSlep_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root",
            "TChiWZ_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root",
        ],
    },
    {
        "source_dir": "Autumn18_102X_SMS",
        "source_suffix": "Autumn18_102X",
        "dest_dirs": [
            "Summer24_130X_SMS",
            "Summer25_130X_SMS",
            "Summer26_130X_SMS",
        ],
        "compound_naming": True,
        "files": [
            "SMS-TChiWZ_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_102X.root",
            "SMS-TChiWZ_dM-60to90_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_102X.root",
            "SMS-TChipmWW_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_102X.root",
            "SMS-TChipmWW_dM-3to50_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_102X.root",
            "SMS-TChipmWW_dM-60to90_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_102X.root",
            "SMS-TSlepSlep_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_102X.root",
            "SMS-TSlepSlep_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_102X.root",
            "TChiWZ_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_102X.root",
        ],
    },

    # ----------------------------------------------------------------
    # Background jobs (auto-discover all files in source dir)
    # ----------------------------------------------------------------
    {
        "source_dir": "Summer24_130X",
        "source_suffix": "Summer24_130X",
        "dest_dirs": [
            "Summer25_130X",
            "Summer26_130X",
        ],
        "compound_naming": True,
        "files": None,  # None means auto-discover via eosls
    },

]
# ========================


def run_cmd(cmd):
    for attempt in range(1, MAX_RETRIES + 1):
        try:
            subprocess.run(cmd, check=True)
            return True
        except subprocess.CalledProcessError:
            print(f"[WARN] Attempt {attempt} failed: {' '.join(cmd)}")
            if attempt < MAX_RETRIES:
                time.sleep(RETRY_DELAY)
            else:
                print(f"[ERROR] Failed after {MAX_RETRIES} attempts.")
                return False


def eos_ls(path):
    """List files in an EOS directory. Returns list of filenames."""
    cmd = ["eos", "root://cmseos.fnal.gov", "ls", path]
    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        return [f.strip() for f in result.stdout.splitlines() if f.strip()]
    except subprocess.CalledProcessError:
        print(f"[ERROR] Could not list directory: {path}")
        return []


def get_dest_filename(src_filename, source_suffix, target_suffix, compound):
    if compound:
        # e.g. _Summer24_130X -> _Summer24_Summer25_130X
        # Strip the trailing _130X or _102X or _106X version token from source_suffix
        base = source_suffix.rsplit("_", 2)[0]  # "Summer24"
        return src_filename.replace(source_suffix, f"{base}_{target_suffix}", 1)
    else:
        return src_filename.replace(source_suffix, target_suffix, 1)


n_total = 0
n_failed = 0

for job in JOBS:
    source_dir    = job["source_dir"]
    source_suffix = job["source_suffix"]
    compound      = job["compound_naming"]

    # Auto-discover files if not explicitly listed
    if job["files"] is None:
        eos_path = f"/store/user/lpcsusylep/NTUPLES_Cascades_v9/{source_dir}"
        filelist = eos_ls(eos_path)
        if not filelist:
            print(f"[WARN] No files found in {eos_path}, skipping job.")
            continue
    else:
        filelist = job["files"]

    for dest_dir in job["dest_dirs"]:
        target_suffix = dest_dir.replace("_SMS", "")

        for src_filename in filelist:
            dest_filename = get_dest_filename(src_filename, source_suffix, target_suffix, compound)

            source = f"{BASE}/{source_dir}/{src_filename}"
            dest   = f"{BASE}/{dest_dir}/{dest_filename}"
            cmd    = ["xrdcp", "-f", source, dest]
            n_total += 1

            if DRY_RUN:
                print("[DRY-RUN]", " ".join(cmd))
            else:
                print("[COPY]", " ".join(cmd))
                if not run_cmd(cmd):
                    n_failed += 1

print("\n===== SUMMARY =====")
print(f"Total operations: {n_total}")
print(f"Failed:           {n_failed}")
