run_all() {
    mkdir -p runs/
    echo "[run_all] Launching ./scripts/run_all.sh with arguments: $*"
    # Start detached using setsid (stdout to /dev/null, stderr -> terminal)
    setsid ./scripts/run_all.sh "$@" > /dev/null &
    PID=$!

    # Wait briefly to ensure a new run directory is created
    sleep 1
    RUN_DIR=$(ls -td runs/run_* 2>/dev/null | head -n1)
    if [[ -z "$RUN_DIR" ]]; then
        return
    fi

    DEBUG_LOG="$RUN_DIR/debug_run_combine.debug"
    if [[ -f "$DEBUG_LOG" ]]; then
        echo "[run_all] Debug log located at: $DEBUG_LOG"
        echo "[run_all] You can monitor it with:"
        echo "          tail -f $DEBUG_LOG"
    else
        echo "[run_all] Warning: Debug log does not exist yet at $DEBUG_LOG"
    fi
}
SLEEP=70
VERSION="v408"
make clean
make all -j 8
echo "Submitting Regions..."

# Call script to make regions
python3 python/make_Regions.py

# All Regions
rm -f config/bin_cfgs/Regions.yaml # Clean up yaml
rm -f config/bin_cfgs/Regions_Run2.yaml # Clean up yaml
rm -f config/bin_cfgs/Regions_Run3.yaml # Clean up yaml

cat config/bin_cfgs/Regions_Run2_2L_0J_lPTISR_Gold.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_2L_0J_lPTISR_Bronze.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_2L_1J_lPTISR_Gold.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_2L_1J_lPTISR_Bronze.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_2L_0J_hPTISR_Gold.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_2L_0J_hPTISR_Bronze.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_2L_1J_hPTISR_Gold.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_2L_1J_hPTISR_Bronze.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_2L_Silver.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_3L_Jincl_lPTISR_Gold.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_3L_Jincl_lPTISR_Bronze.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_3L_Jincl_hPTISR_Gold.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_3L_Jincl_Silver.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_3L_Jincl_hPTISR_Bronze.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_4L_Gold.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_4L_Silver.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_4L_Bronze.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_top_sideband_Gold.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_top_sideband_Silver.yaml >> config/bin_cfgs/Regions_Run2.yaml
cat config/bin_cfgs/Regions_Run2_top_sideband_Bronze.yaml >> config/bin_cfgs/Regions_Run2.yaml

cat config/bin_cfgs/Regions_Run3_2L_0J_lPTISR_Gold.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_2L_0J_lPTISR_Bronze.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_2L_1J_lPTISR_Gold.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_2L_1J_lPTISR_Bronze.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_2L_0J_hPTISR_Gold.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_2L_0J_hPTISR_Bronze.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_2L_1J_hPTISR_Gold.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_2L_1J_hPTISR_Bronze.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_2L_Silver.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_3L_Jincl_lPTISR_Gold.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_3L_Jincl_lPTISR_Bronze.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_3L_Jincl_hPTISR_Gold.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_3L_Jincl_Silver.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_3L_Jincl_hPTISR_Bronze.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_4L_Gold.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_4L_Silver.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_4L_Bronze.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_top_sideband_Gold.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_top_sideband_Silver.yaml >> config/bin_cfgs/Regions_Run3.yaml
cat config/bin_cfgs/Regions_Run3_top_sideband_Bronze.yaml >> config/bin_cfgs/Regions_Run3.yaml

cat config/bin_cfgs/Regions_Run2.yaml >> config/bin_cfgs/Regions.yaml
cat config/bin_cfgs/Regions_Run3.yaml >> config/bin_cfgs/Regions.yaml

# Set BPJ to % of total
BIN_COUNT=$(grep -c 'Bin' config/bin_cfgs/Regions.yaml)
BINS_PER_JOB=$(awk -v n="$BIN_COUNT" 'BEGIN { printf "%d\n", n*0.02 + 0.5 }')

# Run2+Run3 Sensitivity
run_all --make-json --skip-compile --bins-cfg config/bin_cfgs/Regions.yaml --bins-per-job ${BINS_PER_JOB} --run-name Cascades_Regions_234L_Run2Run3_${VERSION}
sleep ${SLEEP}

# Run2 Only Sensitivity
#run_all --make-json --skip-compile --processes-cfg config/process_cfgs/processes_Run2.yaml --bins-cfg config/bin_cfgs/Regions_Run2.yaml --bins-per-job ${BINS_PER_JOB} --run-name Cascades_Regions_234L_Run2_${VERSION}
#sleep ${SLEEP}

# all TChiWZ Run2 only
#run_all --skip-compile --skip-plot-yields --make-json --bins-cfg config/bin_cfgs/Regions_Run2.yaml --bins-per-job ${BINS_PER_JOB} --processes-cfg config/process_cfgs/processes_allTChiWZ_Run2.yaml --run-name Cascades_Regions_234L_allTChiWZ_Run2_${VERSION}
#sleep ${SLEEP}

# all preUL TChiWZ Run2 only
#run_all --skip-compile --skip-plot-yields --make-json --bins-cfg config/bin_cfgs/Regions_Run2.yaml --bins-per-job ${BINS_PER_JOB} --processes-cfg config/process_cfgs/processes_allpreULTChiWZ_Run2.yaml --run-name Cascades_Regions_234L_allpreULTChiWZ_Run2_${VERSION}
#sleep ${SLEEP}

# all TChiWZ Run2+Run3
#run_all --skip-compile --skip-plot-yields --make-json --bins-cfg config/bin_cfgs/Regions.yaml --bins-per-job ${BINS_PER_JOB} --processes-cfg config/process_cfgs/processes_allTChiWZ.yaml --run-name Cascades_Regions_234L_Run2_Run3_allTChiWZ_${VERSION}
#sleep ${SLEEP}

# all preULTChiWZ Run2+Run3
#run_all --skip-compile --skip-plot-yields --make-json --bins-cfg config/bin_cfgs/Regions.yaml --bins-per-job ${BINS_PER_JOB} --processes-cfg config/process_cfgs/processes_allpreULTChiWZ.yaml --run-name Cascades_Regions_234L_Run2_Run3_allpreULTChiWZ_${VERSION}
#sleep ${SLEEP}

# all preULTSlepSlep Run2+Run3
#run_all --skip-compile --skip-plot-yields --make-json --bins-cfg config/bin_cfgs/Regions.yaml --bins-per-job ${BINS_PER_JOB} --processes-cfg config/process_cfgs/processes_allpreULTSlepSlep.yaml --run-name Cascades_Regions_234L_Run2_Run3_allpreULTSlepSlep_${VERSION}
#sleep ${SLEEP}

# CR Fit
rm -f config/bin_cfgs/Regions_CR_Run2.yaml # Clean up yaml
rm -f config/bin_cfgs/Regions_CR_Run3.yaml # Clean up yaml
rm -f config/bin_cfgs/Regions_CR.yaml # Clean up yaml

cat config/bin_cfgs/Regions_Run2_2L_0J_lPTISR_Bronze.yaml >> config/bin_cfgs/Regions_CR_Run2.yaml
cat config/bin_cfgs/Regions_Run2_2L_1J_lPTISR_Bronze.yaml >> config/bin_cfgs/Regions_CR_Run2.yaml
cat config/bin_cfgs/Regions_Run2_2L_0J_hPTISR_Bronze.yaml >> config/bin_cfgs/Regions_CR_Run2.yaml
cat config/bin_cfgs/Regions_Run2_2L_1J_hPTISR_Bronze.yaml >> config/bin_cfgs/Regions_CR_Run2.yaml
cat config/bin_cfgs/Regions_Run2_2L_Silver.yaml >> config/bin_cfgs/Regions_CR_Run2.yaml
cat config/bin_cfgs/Regions_Run2_3L_Jincl_lPTISR_Bronze.yaml >> config/bin_cfgs/Regions_CR_Run2.yaml
cat config/bin_cfgs/Regions_Run2_3L_Jincl_Silver.yaml >> config/bin_cfgs/Regions_CR_Run2.yaml
cat config/bin_cfgs/Regions_Run2_3L_Jincl_hPTISR_Bronze.yaml >> config/bin_cfgs/Regions_CR_Run2.yaml
cat config/bin_cfgs/Regions_Run2_4L_Silver.yaml >> config/bin_cfgs/Regions_CR_Run2.yaml
cat config/bin_cfgs/Regions_Run2_4L_Bronze.yaml >> config/bin_cfgs/Regions_CR_Run2.yaml
cat config/bin_cfgs/Regions_Run2_top_sideband_Silver.yaml >> config/bin_cfgs/Regions_CR_Run2.yaml
cat config/bin_cfgs/Regions_Run2_top_sideband_Bronze.yaml >> config/bin_cfgs/Regions_CR_Run2.yaml

cat config/bin_cfgs/Regions_Run3_2L_0J_lPTISR_Bronze.yaml >> config/bin_cfgs/Regions_CR_Run3.yaml
cat config/bin_cfgs/Regions_Run3_2L_1J_lPTISR_Bronze.yaml >> config/bin_cfgs/Regions_CR_Run3.yaml
cat config/bin_cfgs/Regions_Run3_2L_0J_hPTISR_Bronze.yaml >> config/bin_cfgs/Regions_CR_Run3.yaml
cat config/bin_cfgs/Regions_Run3_2L_1J_hPTISR_Bronze.yaml >> config/bin_cfgs/Regions_CR_Run3.yaml
cat config/bin_cfgs/Regions_Run3_2L_Silver.yaml >> config/bin_cfgs/Regions_CR_Run3.yaml
cat config/bin_cfgs/Regions_Run3_3L_Jincl_lPTISR_Bronze.yaml >> config/bin_cfgs/Regions_CR_Run3.yaml
cat config/bin_cfgs/Regions_Run3_3L_Jincl_Silver.yaml >> config/bin_cfgs/Regions_CR_Run3.yaml
cat config/bin_cfgs/Regions_Run3_3L_Jincl_hPTISR_Bronze.yaml >> config/bin_cfgs/Regions_CR_Run3.yaml
cat config/bin_cfgs/Regions_Run3_4L_Silver.yaml >> config/bin_cfgs/Regions_CR_Run3.yaml
cat config/bin_cfgs/Regions_Run3_4L_Bronze.yaml >> config/bin_cfgs/Regions_CR_Run3.yaml
cat config/bin_cfgs/Regions_Run3_top_sideband_Silver.yaml >> config/bin_cfgs/Regions_CR_Run3.yaml
cat config/bin_cfgs/Regions_Run3_top_sideband_Bronze.yaml >> config/bin_cfgs/Regions_CR_Run3.yaml

cat config/bin_cfgs/Regions_CR_Run2.yaml >> config/bin_cfgs/Regions_CR.yaml
cat config/bin_cfgs/Regions_CR_Run3.yaml >> config/bin_cfgs/Regions_CR.yaml

# Run2 Only CR Fit
run_all --skip-plot-yields --make-impacts --make-FD --processes-cfg config/process_cfgs/data_processes_Run2.yaml --make-json --skip-compile --bins-cfg config/bin_cfgs/Regions_CR_Run2.yaml --bins-per-job ${BINS_PER_JOB} --run-name Cascades_CRFit_Impacts_FD_234L_Run2_${VERSION}
sleep ${SLEEP}

# Run3 Only CR Fit
run_all --skip-plot-yields --make-impacts --make-FD --processes-cfg config/process_cfgs/data_processes_Run3.yaml --make-json --skip-compile --bins-cfg config/bin_cfgs/Regions_CR_Run3.yaml --bins-per-job ${BINS_PER_JOB} --run-name Cascades_CRFit_Impacts_FD_234L_Run3_${VERSION}
sleep ${SLEEP}

# Run2 + Run3 CR Fit
run_all --skip-plot-yields --make-impacts --make-FD --processes-cfg config/process_cfgs/data_processes.yaml --make-json --skip-compile --bins-cfg config/bin_cfgs/Regions_CR.yaml --bins-per-job ${BINS_PER_JOB} --run-name Cascades_CRFit_Impacts_FD_234L_Run2Run3_${VERSION}
sleep ${SLEEP}

sleep 30 # final sleep just to hold user from accidentally submitting something before last sub is out the door
echo "Submitted Regions!"
