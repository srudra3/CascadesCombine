#!/usr/bin/env bash
# macro/launchFitDiagnostics.sh
source "macro/utils.sh"

# First argument = datacard directory (default datacards_cascades)
dcdir="${1:-datacards_cascades}"
# Second argument = run_dir (default: runs/latest)
rundir="${2:-runs/latest}"

echo "[launchFitDiagnostics] Using datacard directory: $dcdir"

# Run FitDiagnostics initial fit for first workspace
for WS in "${dcdir}"/*/*_workspace.root; do
    WSDIR="$(dirname "$WS")"
    WSFILE="$(basename "$WS")"
    MASS=$(extract_mass "$(basename "$WSDIR")")
    echo "[launchFitDiagnostics] Running FitDiagnostics in $WSDIR for $WSFILE"
    pushd "$WSDIR" > /dev/null || exit 1
    combineTool.py \
      -M FitDiagnostics \
      -d "$WSFILE" \
      -m "120" \
      --skipSBFit \
      --saveShapes \
      --saveWithUncertainties \
      --robustHesse 1 \
      --saveNormalizations
    popd > /dev/null || exit 1
    break
done

# options:
#-v 10 \
#-m "$MASS" \
      #--robustFit 1 \
      #--robustHesse 1 \
