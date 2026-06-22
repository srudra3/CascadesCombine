#!/bin/bash
echo "[$(date)] Starting job on $(hostname)"
echo "[$(date)] Working directory: $(pwd)"

# --- CMS environment ---
source /cvmfs/cms.cern.ch/cmsset_default.sh
export SCRAM_ARCH=el9_amd64_gcc12

cmsenv_dir=/cvmfs/cms.cern.ch/el9_amd64_gcc12/cms/cmssw/CMSSW_14_1_0_pre4/src
if [ -d "$cmsenv_dir" ]; then
    cd "$cmsenv_dir"
    eval `scram runtime -sh`
    cd -
else
    echo "CMSSW release not found on CVMFS!"
    exit 1
fi

chmod +x BFI_condor.x

clean_arg() { echo "$1" | tr -d '\n' | tr -d '\r' | xargs; }

# --- initialize variables ---
BIN=""
ROOTFILE=""
OUTPUT_JSON=""
OUTPUT_HIST=""
HIST_YAML=""
PROC_YAML=""
CUTS_MULTI=""
LEP_CUTS_MULTI=""
PREDEF_CUTS_MULTI=""
USER_CUTS_MULTI=""
BINS_CFG=""
SIG_TYPE=""
LUMI=""
SMS_FILTERS=""
JSON_FLAG=""
HIST_FLAG=""
CUTFLOW_FLAG=""
PROC_NAME=""

# --- Parse arguments (updated to collect repeated per-bin flags) ---
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        --bin) BIN=$(clean_arg "$2"); shift 2;;
        --file) ROOTFILE=$(clean_arg "$2"); shift 2;;

        # Standalone flags
        --json) JSON_FLAG="--json"; shift;;
        --hist) HIST_FLAG="--hist"; shift;;
        --cutflow) CUTFLOW_FLAG="--cutflow"; shift;;

        # Output filenames
        --json-output) OUTPUT_JSON=$(clean_arg "$2"); shift 2;;
        --root-output) OUTPUT_HIST=$(clean_arg "$2"); shift 2;;
        --hist-yaml) HIST_YAML=$(clean_arg "$2"); shift 2;;
        --proc-yaml) PROC_YAML=$(clean_arg "$2"); shift 2;;

        # Cuts (collect repeated occurrences)
        --cuts)
            val=$(clean_arg "$2")
            [[ "$val" == "__EMPTY__" ]] && val=""
            if [[ -z "$CUTS_MULTI" ]]; then
                CUTS_MULTI="$val"
            else
                CUTS_MULTI="${CUTS_MULTI}|||$val"
            fi
            shift 2
            ;;
        --lep-cuts)
            val=$(clean_arg "$2")
            [[ "$val" == "__EMPTY__" ]] && val=""
            if [[ -z "$LEP_CUTS_MULTI" ]]; then
                LEP_CUTS_MULTI="$val"
            else
                LEP_CUTS_MULTI="${LEP_CUTS_MULTI}|||$val"
            fi
            shift 2
            ;;
        --predefined-cuts)
            val=$(clean_arg "$2")
            [[ "$val" == "__EMPTY__" ]] && val=""
            if [[ -z "$PREDEF_CUTS_MULTI" ]]; then
                PREDEF_CUTS_MULTI="$val"
            else
                PREDEF_CUTS_MULTI="${PREDEF_CUTS_MULTI}|||$val"
            fi
            shift 2
            ;;
        --user-cuts)
            val=$(clean_arg "$2")
            [[ "$val" == "__EMPTY__" ]] && val=""
            if [[ -z "$USER_CUTS_MULTI" ]]; then
                USER_CUTS_MULTI="$val"
            else
                USER_CUTS_MULTI="${USER_CUTS_MULTI}|||$val"
            fi
            shift 2
            ;;

        # bins-cfg (accept and keep basename)
        --bins-cfg) BINS_CFG=$(clean_arg "$2"); shift 2;;

        # Other options
        --sig-type) SIG_TYPE=$(clean_arg "$2"); shift 2;;
        --lumi) LUMI=$(clean_arg "$2"); shift 2;;
        --proc-name)
            PROC_NAME=$(clean_arg "$2")
            shift 2
            ;;

        # Multi-value argument handling for sms filters
        --sms-filters)
            shift
            SMS_FILTERS=""
            while [[ $# -gt 0 && ! $1 == --* ]]; do
                SMS_FILTERS+="$1,"
                shift
            done
            SMS_FILTERS=${SMS_FILTERS%,}
            ;;

        *) echo "Unknown option $1"; shift;;
    esac
done

CUTS_MULTI="${CUTS_MULTI//__SEMI__/;}"
LEP_CUTS_MULTI="${LEP_CUTS_MULTI//__SEMI__/;}"
PREDEF_CUTS_MULTI="${PREDEF_CUTS_MULTI//__SEMI__/;}"
USER_CUTS_MULTI="${USER_CUTS_MULTI//__SEMI__/;}"
CUTS_MULTI="${CUTS_MULTI//__EMPTY__/}"
LEP_CUTS_MULTI="${LEP_CUTS_MULTI//__EMPTY__/}"
PREDEF_CUTS_MULTI="${PREDEF_CUTS_MULTI//__EMPTY__/}"
USER_CUTS_MULTI="${USER_CUTS_MULTI//__EMPTY__/}"

# --- Auto-generate output filenames if not provided ---
base_name=$(basename "$ROOTFILE" .root)
if [[ -z "$OUTPUT_JSON" ]]; then
    OUTPUT_JSON="${BIN}_${base_name}.json"
fi
if [[ -z "$OUTPUT_HIST" ]]; then
    OUTPUT_HIST="${BIN}_${base_name}.root"
fi

# Force basenames to stay local
OUTPUT_JSON=$(basename "$OUTPUT_JSON")
OUTPUT_HIST=$(basename "$OUTPUT_HIST")
[[ -n "$HIST_YAML" ]] && HIST_YAML=$(basename "$HIST_YAML")
[[ -n "$PROC_YAML" ]] && PROC_YAML=$(basename "$PROC_YAML")

# --- Build command as a single quoted string ---
CMD="./BFI_condor.x --bin \"$BIN\" --file \"$ROOTFILE\""

[[ -n "$JSON_FLAG" ]] && CMD="$CMD $JSON_FLAG"
[[ -n "$OUTPUT_JSON" ]] && CMD="$CMD --json-output \"$OUTPUT_JSON\""

[[ -n "$HIST_FLAG" ]] && CMD="$CMD $HIST_FLAG"
[[ -n "$OUTPUT_HIST" ]] && CMD="$CMD --root-output \"$OUTPUT_HIST\""
[[ -n "$HIST_YAML" ]] && CMD="$CMD --hist-yaml \"$HIST_YAML\""
[[ -n "$PROC_YAML" ]] && CMD="$CMD --proc-yaml \"$PROC_YAML\""
[[ -n "$CUTFLOW_FLAG" ]] && CMD="$CMD $CUTFLOW_FLAG"
[[ -n "$PROC_NAME" ]] && CMD="$CMD --proc-name \"$PROC_NAME\""

# If we collected per-bin repeated cuts, pass them as a single --cuts-multi argument (||| as delimiter)
if [[ -n "$CUTS_MULTI" ]]; then
    CMD="$CMD --cuts-multi \"$CUTS_MULTI\""
elif [[ -n "$CUTS" ]]; then
    CMD="$CMD --cuts \"$CUTS\""
fi

if [[ -n "$LEP_CUTS_MULTI" ]]; then
    CMD="$CMD --lep-cuts-multi \"$LEP_CUTS_MULTI\""
elif [[ -n "$LEP_CUTS" ]]; then
    CMD="$CMD --lep-cuts \"$LEP_CUTS\""
fi

if [[ -n "$PREDEF_CUTS_MULTI" ]]; then
    CMD="$CMD --predefined-cuts-multi \"$PREDEF_CUTS_MULTI\""
elif [[ -n "$PREDEF_CUTS" ]]; then
    CMD="$CMD --predefined-cuts \"$PREDEF_CUTS\""
fi

if [[ -n "$USER_CUTS_MULTI" ]]; then
    CMD="$CMD --user-cuts-multi \"$USER_CUTS_MULTI\""
elif [[ -n "$USER_CUTS" ]]; then
    CMD="$CMD --user-cuts \"$USER_CUTS\""
fi

# Keep passing --bins-cfg basename for debugging / future expansion
if [[ -n "$BINS_CFG" ]]; then
    CMD="$CMD --bins-cfg \"$BINS_CFG\""
fi

[[ -n "$SIG_TYPE" ]] && CMD="$CMD --sig-type \"$SIG_TYPE\""
[[ -n "$LUMI" ]] && CMD="$CMD --lumi \"$LUMI\""
[[ -n "$SMS_FILTERS" ]] && CMD="$CMD --sms-filters \"$SMS_FILTERS\""

# --- Echo and run ---
echo "$CMD"
eval "$CMD"

echo "[$(date)] Job finished."
