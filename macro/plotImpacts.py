#!/usr/bin/env python3
"""
plotImpacts.py

PyROOT script to plot combine impacts JSON, with optional conversion of Gaussian lnN nuisances
from pulls (theta) -> absolute multiplicative scale factors using lnN values from the datacard.
Shape systematics (declared as 'shape' in the datacard) are left as pulls.

Usage examples:
  # absolute conversion (default)
  python plotImpacts.py -j impacts.json -d myDatacard.txt -s Btag Run2_PTISR -o out_abs

  # plot everything but leave pulls (no absolute conversion)
  python plotImpacts.py -j impacts.json --all --no-absolute -o out_pulls

  # sort by impact
  python plotImpacts.py -j impacts.json -d myDatacard.txt --all --sort impact_r

Notes:
 - Requires PyROOT (ROOT 6+).
 - Runs in batch mode (no X11).
"""

from __future__ import print_function
import argparse
import json
import re
import os
import math
import numpy as np
import sys

# PyROOT imports
import ROOT
ROOT.gROOT.SetBatch(True)
ROOT.gStyle.SetOptStat(0)

# ---------------------
# Datacard parsing
# ---------------------
def parse_datacard_for_lnN_and_shape(datacard_path):
    """
    Parse the datacard to extract:
      - lnN representative multiplicative values per nuisance (dict: name -> value)
      - shape flags per nuisance (dict: name -> True/False)

    Heuristics:
      - Look for lines starting with the nuisance name followed by a type token (e.g., 'lnN', 'shape', 'param', etc.)
      - If 'lnN' appears in the type token or line, attempt to collect numeric tokens (floats) after the type and pick
        a representative value (the numeric with largest deviation from 1.0 if multiple).
      - If the type token contains 'shape' or the line contains 'shape', mark as shape.
    """
    lnN_map = {}
    shape_map = {}

    float_re = re.compile(r'[-+]?\d*\.\d+|[-+]?\d+')

    with open(datacard_path, 'r') as f:
        for raw in f:
            line = raw.strip()
            if not line or line.startswith('#'):
                continue
            # tokenise by whitespace but keep '/' forms for later float parsing
            tokens = line.split()
            if len(tokens) < 2:
                continue
            name = tokens[0]
            type_token = tokens[1].lower()

            is_shape = ('shape' in type_token) or ('shape' in line.lower())
            shape_map[name] = is_shape or shape_map.get(name, False)

            # try to detect lnN or other multiplicative types
            if 'lnn' in type_token or 'lnn' in line.lower():
                # collect numeric floats from tokens after token[1]
                floats = []
                # examine the rest of the tokens on the line
                for tok in tokens[2:]:
                    # sometimes token like "1.05/0.95" or "1.05/1.08" appear -> split by '/'
                    if '/' in tok:
                        parts = tok.split('/')
                    else:
                        parts = [tok]
                    for p in parts:
                        p = p.strip()
                        if p in ('-', '0', '0.0', '1', '1.0'):
                            # '-' means not applicable for that process; skip
                            continue
                        m = float_re.search(p)
                        if m:
                            try:
                                v = float(m.group(0))
                                floats.append(v)
                            except:
                                pass
                # fallback: try to find floats anywhere on the line
                if not floats:
                    found = float_re.findall(line)
                    floats = [float(x) for x in found] if found else []

                if floats:
                    # pick the float with largest abs deviation from 1.0 (representative)
                    def deviation(v): return abs(v - 1.0)
                    floats_sorted = sorted(floats, key=deviation, reverse=True)
                    chosen = floats_sorted[0]
                    lnN_map[name] = float(chosen)
                else:
                    # nothing numeric found; skip
                    continue
            else:
                # Also consider gmN/lnN written without being second token sometimes (rare) - look for 'lnN' anywhere
                if 'lnn' in line.lower():
                    found = float_re.findall(line)
                    if found:
                        floats = [float(x) for x in found]
                        floats_sorted = sorted(floats, key=lambda v: abs(v - 1.0), reverse=True)
                        lnN_map[name] = floats_sorted[0]

    return lnN_map, shape_map

# ---------------------
# JSON helpers
# ---------------------
def load_impacts_json(path):
    with open(path, 'r') as f:
        return json.load(f)

def select_params(data, substrings=None, use_all=False):
    """
    Return list of params (dicts) selected from data['params'].
    substrings: list of substrings; if any substring is contained in the nuisance name, select it.
    use_all: if True, select everything.
    """
    if use_all:
        #return list(data.get('params', []))
        return [p for p in data.get('params', []) if not p.get('name', '').startswith('prop_')] # ignore MC stats
    if not substrings:
        return []
    sel = []
    for p in data.get('params', []):
        name = p.get('name', '')
        for s in substrings:
            if s in name:
                sel.append(p)
                break
    return sel

# ---------------------
# Conversion / compute values
# ---------------------
def compute_plot_values(p, lnN_map, shape_map, absolute=True):
    """
    For a given param dict p, return (central, err_lo, err_hi, mode)
    mode: 'absolute' for multiplicative scale (center ~1), 'pull' for theta (center ~0), 'rateParam' for Unconstrained
    If absolute==False, will plot pulls for everything (i.e., no lnN conversion).
    Shape systematics (shape_map[name]==True) are always returned as 'pull'.
    """
    name = p.get('name', '')
    ptype = p.get('type', '').lower()  # e.g., 'gaussian' or 'unconstrained'
    fit = p.get('fit', [])
    # defensive defaults
    if len(fit) >= 3:
        low, cen, high = float(fit[0]), float(fit[1]), float(fit[2])
    elif len(fit) == 2:
        low, cen = float(fit[0]), float(fit[1])
        high = cen
    elif len(fit) == 1:
        low = cen = high = float(fit[0])
    else:
        low = cen = high = 0.0

    err_lo = cen - low
    err_hi = high - cen
    if err_lo < 0: err_lo = 0.0
    if err_hi < 0: err_hi = 0.0

    is_shape = shape_map.get(name, False)

    # Rate parameters: treated as absolute factors (they are already scale-like)
    if ptype == 'unconstrained' or ptype == 'rateparam':
        # central value might already be a multiplicative factor; we'll return that with asymmetric errors
        return float(cen), float(err_lo), float(err_hi), 'rateParam'

    # shape systematics: always treat as pulls (plot theta)
    if is_shape:
        return float(cen), float(err_lo), float(err_hi), 'pull'

    # Gaussian nuisances: if absolute conversion requested and lnN exists -> convert
    if absolute and ptype == 'gaussian' and name in lnN_map:
        k = float(lnN_map[name])
        # protect against non-positive k
        if k <= 0:
            # fallback to pulls
            return float(cen), float(err_lo), float(err_hi), 'pull'
        # convert theta -> multiplicative factor: k^theta
        val = (k ** float(cen))
        val_hi = (k ** (float(cen) + float(err_hi)))
        val_lo = (k ** (float(cen) - float(err_lo)))
        # errors are absolute offsets from central multiplicative factor
        err_lo_abs = val - val_lo
        err_hi_abs = val_hi - val
        # guard negative rounding
        if err_lo_abs < 0: err_lo_abs = 0.0
        if err_hi_abs < 0: err_hi_abs = 0.0
        return float(val), float(err_lo_abs), float(err_hi_abs), 'absolute'

    # otherwise default: plot as pull
    return float(cen), float(err_lo), float(err_hi), 'pull'


# ---------------------
# Nuisance display label translation
# Priority: LABEL_OVERRIDES (exact match) -> parse_nuisance_name (rules) -> raw name (fallback)
# ---------------------

# Hard-coded overrides for names that don't follow the standard pattern,
# or where you want to force a specific label. Add entries here as needed.
# Any name not listed here falls through to the rule-based parser below.
LABEL_OVERRIDES = {
    # Example irregular names -- populate as you encounter them:
    # "some_weird_nuisance":  "#kappa_{weird}^{special}",
    "scale_triboson":       "#kappa_{tri-boson}",
}

def parse_nuisance_name(name):
    """
    Rule-based conversion of a combine nuisance name to a TLatex display label.
    Returns None if the name doesn't match any known pattern (triggers raw fallback).

    Conventions:
      Gaussian lnN   -> #kappa_{subscript}^{superscript}
      Unconstrained  -> #theta_{subscript}^{superscript}
    """

    # --- Unconstrained rate params: scale_FAKES_<Era>_<LF|HF><Elec|Muon> ---
    m = re.match(r'^scale_FAKES_(Run\d)_(LF|HF)(Elec|Muon)$', name)
    if m:
        era, hf, flavour = m.group(1), m.group(2), m.group(3)
        era_str  = era.replace('Run', 'Run-')
        flav_str = 'e' if flavour == 'Elec' else '#mu'
        return f"#theta_{{Fakes}}^{{{era_str},{hf}{flav_str}}}"

    # --- Other simple scale_ rate params: scale_<bkg> ---
    m = re.match(r'^scale_(\w+)$', name)
    if m:
        bkg = m.group(1)
        label = bkg.replace('diboson', 'di-boson').replace('triboson', 'tri-boson')
        return f"#theta_{{{label}}}"

    # --- All remaining names are expected to start with Run<N>_ ---
    m = re.match(r'^(Run\d)_(.+)$', name)
    if not m:
        return None   # unrecognised, trigger fallback
    era     = m.group(1)            # e.g. Run2
    rest    = m.group(2)            # e.g. Btag_2L_0J_hPTISR
    era_str = era.replace('Run', 'Run-')
    tokens  = rest.split('_')
    if not tokens:
        return None

    category  = tokens[0]
    remainder = tokens[1:]

    def token_to_str(t):
        return {
            'lPTISR': '-ISR',
            'hPTISR': '+ISR',
            'Silver': 'Silver',
            'LFElec': 'LFe',
            'HFElec': 'HFe',
            'LFMuon': 'LF#mu',
            'HFMuon': 'HF#mu',
        }.get(t, t)   # 2L, 3L, 4L, 0J, 1J etc. pass through unchanged

    def build_sub(toks):
        return ','.join(token_to_str(t) for t in toks)

    # --- Lepton ID / SIP3D: Run2_IDISO_LFElec, Run2_SIP3D_HFMuon, etc. ---
    if category in ('IDISO', 'SIP3D') and remainder:
        flav_token = '_'.join(remainder)
        hf   = 'LF' if 'LF' in flav_token else 'HF'
        flav = 'e' if 'Elec' in flav_token else '#mu'
        return f"#kappa_{{{category}}}^{{{era_str},{hf}{flav}}}"

    # --- LepHemi: Run2_4L_LepHemi ---
    if category == '4L' and remainder and remainder[0] == 'LepHemi':
        return f"#kappa_{{LepHemi}}^{{{era_str},4L}}"

    # --- Standard categories ---
    CAT_LABELS = {
        'Btag':     'B-tag',
        'PTISR':    'p_{T}^{ISR}',
        'OSOF':     'OSOF',
        'SameSign': 'SS',
    }
    if category not in CAT_LABELS:
        return None   # unknown category, trigger fallback

    cat_label = CAT_LABELS[category]
    sub = build_sub(remainder)
    # If the category label itself contains TLatex super/subscripts, use a flat format
    # to avoid illegal nesting (e.g. p_{T}^{ISR} can't live inside ^{...})
    if '^' in cat_label:
        if sub:
            return f"#kappa^{{{era_str},{sub}}}_{{{cat_label}}}"
        else:
            return f"#kappa^{{{era_str}}}_{{{cat_label}}}"
    else:
        if sub:
            return f"#kappa_{{{cat_label}}}^{{{era_str},{sub}}}"
        else:
            return f"#kappa_{{{cat_label}}}^{{{era_str}}}"

def get_display_label(name):
    """
    Three-step priority chain:
      1. Hard-coded override in LABEL_OVERRIDES (exact match)
      2. Rule-based parser (parse_nuisance_name)
      3. Raw name fallback (prints a warning so you know to add it to LABEL_OVERRIDES)
    """
    if name in LABEL_OVERRIDES:
        return LABEL_OVERRIDES[name]

    parsed = parse_nuisance_name(name)
    if parsed is not None:
        return parsed

    print(f"  [label] No rule matched '{name}' -- using raw name. Consider adding to LABEL_OVERRIDES.")
    return name

# ---------------------
# ROOT plotting helpers
# ---------------------
def shorten_label(s, maxlen=40):
    if len(s) <= maxlen:
        return s
    return s[:maxlen-3] + '...'

def make_graph_from_entries(entries, max_label_len=45, marker_style=20):
    """
    entries: list of tuples (name, central, err_lo, err_hi, mode)
    returns: TGraphAsymmErrors, labels (list), modes (list)
    """
    N = len(entries)
    xs = np.arange(1, N+1, dtype=np.float64)
    xlo = np.zeros(N, dtype=np.float64)
    xhi = np.zeros(N, dtype=np.float64)
    ys = np.zeros(N, dtype=np.float64)
    ylo = np.zeros(N, dtype=np.float64)
    yhi = np.zeros(N, dtype=np.float64)
    labels = []
    modes = []

    for i, ent in enumerate(entries):
        name, cen, dlo, dhi, mode = ent
        xs[i] = float(i + 1)
        ys[i] = float(cen)
        ylo[i] = float(dlo)
        yhi[i] = float(dhi)
        labels.append(shorten_label(get_display_label(name), max_label_len))
        modes.append(mode)

    gr = ROOT.TGraphAsymmErrors(int(N),
                                xs,
                                ys,
                                xlo, xhi,
                                ylo, yhi)
    gr.SetMarkerStyle(marker_style)
    gr.SetMarkerSize(0.95)
    gr.SetLineWidth(1)
    return gr, labels, modes

def draw_impacts_graph(gr, labels, modes, entries, outbase, title="", ytitle=None):
    """
    Draws the graph with proper axis labels. modes list tells us which points are 'absolute', 'pull', or 'rateParam'.
    We'll draw horizontal reference line(s): y=1 for absolute/rateParam, y=0 for pull.
    """
    N = len(labels)
    # compute y-range manually considering asymmetric errors
    y_mins = []
    y_maxs = []
    for ent in entries:
        _, cen, dlo, dhi, mode = ent
        if mode == 'pull':
            y_mins.append(cen - dlo)
            y_maxs.append(cen + dhi)
        else:
            # absolute / rateParam around ~1 typically: cen +/- errs
            y_mins.append(cen - dlo)
            y_maxs.append(cen + dhi)

    if not y_mins:
        ymin, ymax = -1.0, 1.0
    else:
        ymin = min(y_mins)
        ymax = max(y_maxs)
        # padding
        if math.isclose(ymin, ymax):
            delta = abs(ymax) * 0.2 + 0.5
        else:
            delta = 0.12 * (ymax - ymin if ymax - ymin != 0 else 1.0)
        ymin -= delta
        ymax += delta

    # Create canvas: height scales with number of points to keep labels readable
    canvas_w = 1200
    canvas_h = max(600, 20 * N + 200)
    cv = ROOT.TCanvas(os.path.basename(outbase), os.path.basename(outbase), canvas_w, canvas_h)
    cv.SetLeftMargin(0.08)
    cv.SetRightMargin(0.04)
    cv.SetTopMargin(0.06)
    cv.SetBottomMargin(0.2)

    # Draw an empty histogram frame to host axes and bin labels
    h = ROOT.TH1F("hframe_"+os.path.basename(outbase), "", N, 0.5, N + 0.5)
    h.GetYaxis().SetTitle(ytitle if ytitle else "")
    h.GetYaxis().SetTitleSize(0.037)
    h.GetYaxis().SetLabelSize(0.03)
    h.GetYaxis().SetNdivisions(405)
    h.GetYaxis().SetTitleOffset(0.92)
    h.GetYaxis().CenterTitle()
    h.SetMinimum(ymin)
    h.SetMaximum(ymax)
    # set x-labels
    for i, lab in enumerate(labels):
        h.GetXaxis().SetBinLabel(i + 1, lab)
    h.GetXaxis().LabelsOption("v")  # vertical labels
    h.GetXaxis().SetLabelSize(0.028)
    h.GetXaxis().SetTitle("")

    h.Draw("HIST")

    # draw graph points
    gr.Draw("P same")

    # draw reference lines conditionally
    any_pull = any(m == 'pull' for m in modes)
    any_absolute = any(m in ('absolute', 'rateParam') for m in modes)

    if any_pull:
        l0 = ROOT.TLine(0.5, 0.0, N + 0.5, 0.0)
        l0.SetLineStyle(7)
        l0.SetLineWidth(1)
        l0.Draw("same")
    if any_absolute:
        l1 = ROOT.TLine(0.5, 1.0, N + 0.5, 1.0)
        l1.SetLineStyle(7)
        l1.SetLineWidth(1)
        l1.Draw("same")

    # legend explaining point modes
    leg = ROOT.TLegend(0.75, 0.70, 0.95, 0.88)
    leg.SetFillStyle(0)
    leg.SetBorderSize(0)
    # add entries based on what is present
    # create small proxy graphs for legend
    proxy = ROOT.TGraph(1)
    proxy.SetMarkerStyle(20)
    if any_absolute:
        leg.AddEntry(proxy, "Absolute multiplicative factor (converted from lnN)", "p")
    if any_pull:
        proxy2 = ROOT.TGraph(1)
        proxy2.SetMarkerStyle(20)
        leg.AddEntry(proxy2, "Pull (shape nuisances / non-lnN)", "p")
    #leg.Draw("same")

    # CMS label and info
    l = ROOT.TLatex()
    l.SetNDC()
    l.SetTextFont(42)
    l.SetTextSize(0.036)
    l.DrawLatex(0.07, 0.96, "#bf{#it{CMS}} Preliminary")
    #if title:
    #    l.SetTextSize(0.030)
    #    l.DrawLatex(0.02, 0.92, title)

    # draw nuisance names as left-column text (better than vertical labels if many)
    # But we already have vertical labels; optionally draw full names on the left if truncated
    # Here we attempt to show full names at left if they were truncated.
    # Compute whether any label was shortened:
    show_full_left = any(len(lbl) < len(entries[idx][0]) for idx, lbl in enumerate(labels))
    if show_full_left:
        txt = ROOT.TLatex()
        txt.SetNDC()
        txt.SetTextFont(42)
        txt.SetTextSize(0.023)
        txt.SetTextAlign(13)  # left, center vertically
        # print each full name aligned with its y bin
        for i, ent in enumerate(entries):
            name = ent[0]
            y = N - i - 0.5
            txt.DrawLatex(0.01, (0.12 + 0.78 * (y / float(N + 1))), name)

    cv.RedrawAxis()
    # Save
    out_pdf = outbase + ".pdf"
    out_root = outbase + ".root"
    cv.SaveAs(out_pdf)
    cv.SaveAs(out_root)
    print("Saved:", out_pdf, out_root)

# ---------------------
# CLI / main
# ---------------------
def main():
    parser = argparse.ArgumentParser(description="Plot impacts: option to convert Gaussian lnN nuisances to absolute multiplicative factors using datacard lnN values. Shape nuisances remain pulls.")
    parser.add_argument('-j', '--json', required=True, help='input impacts JSON (from combineTool -M Impacts)')
    parser.add_argument('-d', '--datacard', required=False, help='datacard to read lnN and shape declarations (required for absolute conversion)')
    group = parser.add_mutually_exclusive_group(required=False)
    group.add_argument('-s', '--sys', nargs='+', help='substring(s) to match nuisance names (default: required unless --all)')
    group.add_argument('--all', action='store_true', help='select all nuisances')
    parser.add_argument('-o', '--output', default='Impacts_absolute', help='output basename (no extension)')
    parser.add_argument('--no-absolute', dest='absolute', action='store_false', help='do NOT convert Gaussian lnN nuisances to absolute multiplicative factors; plot pulls instead')
    parser.add_argument('--max-label-len', type=int, default=45, help='max x-axis label length before truncation')
    parser.add_argument('--sort', choices=['impact_r', 'name', 'magnitude'], default='impact_r', help='sort nuisances (default: impact_r)')
    parser.add_argument('--reverse', action='store_true', help='reverse sort order')
    parser.set_defaults(absolute=True)  # default: convert to absolute terms if datacard provided
    args = parser.parse_args()

    # load JSON
    data = load_impacts_json(args.json)

    # decide selection
    if not args.all and not args.sys:
        parser.error("Provide --sys substrings or use --all.")

    params = select_params(data, substrings=args.sys or [], use_all=args.all)
    if not params:
        print("No matching nuisances found. Exiting.")
        return

    # parse datacard only if absolute conversion requested
    if args.absolute:
        if not args.datacard:
            print("ERROR: absolute conversion requested (default) but no datacard provided.")
            print("Either provide -d/--datacard or use --no-absolute to plot pulls.")
            sys.exit(1)
        lnN_map, shape_map = parse_datacard_for_lnN_and_shape(args.datacard)
        # Warn if duplicates or empty
        if not lnN_map:
            print("Warning: no lnN entries found in datacard (lnN map empty). Absolute conversion will be skipped for all nuisances.")
    else:
        lnN_map = {}
        shape_map = {}

    # compute entries: list of tuples (name, central, err_lo, err_hi, mode)
    entries = []
    for p in params:
        cen, dlo, dhi, mode = compute_plot_values(p, lnN_map, shape_map, absolute=args.absolute)
        entries.append((p.get('name', ''), cen, dlo, dhi, mode))

    # sort entries
    if args.sort == 'impact_r':
        entries = sorted(entries, key=lambda e: next((p.get('impact_r', 0.0) for p in params if p.get('name','') == e[0]), 0.0), reverse=not args.reverse)
    elif args.sort == 'name':
        entries = sorted(entries, key=lambda e: e[0], reverse=args.reverse)
    elif args.sort == 'magnitude':
        # sort by absolute size of (central-1) for absolute OR abs(central) for pulls
        def mag(e):
            _, cen, _, _, mode = e
            if mode in ('absolute', 'rateParam'):
                return abs(cen - 1.0)
            else:
                return abs(cen)
        entries = sorted(entries, key=mag, reverse=not args.reverse)

    # prepare graph and plot
    gr, labels, modes = make_graph_from_entries(entries, max_label_len=args.max_label_len)
    # set y-axis title
    # if all points are absolute/rateParam then title = "postfit scale factor"
    # if all points are pulls then "postfit pull (#theta)"
    any_absolute = any(m in ('absolute', 'rateParam') for m in modes)
    any_pull = any(m == 'pull' for m in modes)
    if any_absolute and not any_pull:
        ytitle = "post-fit scale factor #pm uncertainty"
    elif any_pull and not any_absolute:
        ytitle = "post-fit pull (#theta) #pm uncertainty"
    else:
        ytitle = "value (mix of pulls and absolute scale factors)"

    draw_impacts_graph(gr, labels, modes, entries, args.output, title=os.path.basename(args.json), ytitle=ytitle)


if __name__ == "__main__":
    main()
