#!/usr/bin/env python3
"""
Make three kinds of plots from the merged BuildFitInput ROOT file:

  dist    : overlaid per-process distributions (log y), yields in legend,
            S, S/B, S/sqrt(B), Z_Bi(1%/3%), Asimov Z in a text box
  scan    : significance metrics vs. cut value, from the same histogram
  cutflow : the per-process CutFlow histograms, same style

Histogram names in the file are  <region>__<process>__<hist>.
Processes starting with "Cascades" are treated as signal; everything else is background.

How to run (command-line examples):
  # every nm1_* variable: one plot each with all samples overlaid, plus cut scans
  python3 plot_nm1.py -f group_000__NoCuts__0b2cddb5.root --all-nm1

  # a single variable, distribution plot only
  python3 plot_nm1.py -f group_000__NoCuts__0b2cddb5.root --hist nm1_quad_maxMassHemi4l_3p1 --mode dist

  # only some signal points
  python3 plot_nm1.py -f group_000__NoCuts__0b2cddb5.root --all-nm1 --signal Cascades_180 --signal Cascades_220
"""
import argparse
import math
import os
import ROOT

ROOT.gROOT.SetBatch(True)
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetOptTitle(1)
ROOT.TH1.AddDirectory(False)

# ------------------------------------------------------------------
# Defaults for each N-1 histogram: (how the cut passes, nominal cut value)
#   lt    : event passes if x <= cut   (upper cut)
#   gt    : event passes if x >= cut   (lower cut)
#   absgt : event passes if |x| >= cut (e.g. off-Z)
#   None  : no scan (e.g. m4l window veto)
# ------------------------------------------------------------------
CUT_DEFAULTS = {
    "nm1_quad_minmll":            ("gt", 4.0),
    "nm1_quad_devZ":              ("absgt", 7.5),
    "nm1_quad_pt4l":              ("gt", 10.0),   # 10 GeV for Njet==0, 30 GeV for Njet>=1
    "nm1_quad_m4l":               (None, None),
    "nm1_quad_maxMassHemi4l":     ("lt", None),
    "nm1_quad_maxMassHemi4l_2p2": ("lt", 23.0),
    "nm1_quad_maxMassHemi4l_3p1": ("lt", 39.0),
    "nm1_quad_maxsip3d":          ("lt", 5.0),
    "nm1_quad_pt1":               ("gt", 18.0),
    "nm1_quad_pt2":               ("gt", 8.0),
    "nm1_quad_pt3":               ("gt", 6.0),
    "nm1_quad_pt4":               ("gt", 2.0),
}

XTITLES = {
    "nm1_quad_minmll": "min m_{ll} [GeV]",
    "nm1_quad_devZ": "m_{ll}^{best OSSF} - m_{Z} [GeV]",
    "nm1_quad_pt4l": "p_{T}^{4l(+jets)} [GeV]",
    "nm1_quad_m4l": "m_{4l} [GeV]",
    "nm1_quad_maxMassHemi4l": "Max (Thrust-)Hemisphere Mass [GeV]",
    "nm1_quad_maxMassHemi4l_2p2": "Max (Thrust-)Hemisphere Mass (2+2 events) [GeV]",
    "nm1_quad_maxMassHemi4l_3p1": "Max (Thrust-)Hemisphere Mass (3+1 events) [GeV]",
    "nm1_quad_maxsip3d": "max SIP_{3D}",
    "nm1_quad_pt1": "p_{T}^{l1} [GeV]",
    "nm1_quad_pt2": "p_{T}^{l2} [GeV]",
    "nm1_quad_pt3": "p_{T}^{l3} [GeV]",
    "nm1_quad_pt4": "p_{T}^{l4} [GeV]",
    "sr_quad_nlepMaxHemi4l": "N_{lep} in fuller hemisphere",
}

# Colours roughly matching the reference plots
COLORS = {
    "WZ": ROOT.kRed + 1, "ZZ": ROOT.kMagenta, "WW": ROOT.kGreen + 3,
    "VVV": ROOT.kGreen - 7, "Hto4l": ROOT.kCyan - 7, "ttbar": ROOT.kYellow + 2,
    "DY": ROOT.kOrange + 1, "Wjets": ROOT.kViolet + 1, "W": ROOT.kViolet + 1,
    "TB": ROOT.kGray + 2,
}
FALLBACK_COLORS = [ROOT.kAzure + 7, ROOT.kPink + 6, ROOT.kSpring - 6, ROOT.kTeal + 3, ROOT.kOrange + 7]


# ------------------------------------------------------------------
# Significance metrics (Z_Bi matches RooStats::NumberCountingUtils::BinomialExpZ)
# ------------------------------------------------------------------
def z_bi(s, b, rel_unc):
    if b <= 0 or s <= 0:
        return 0.0
    sigma_b = b * rel_unc
    tau = b / (sigma_b * sigma_b)
    n_on = s + b
    n_off = b * tau
    p = ROOT.TMath.BetaIncomplete(1.0 / (1.0 + tau), n_on, n_off + 1.0)
    return math.sqrt(2.0) * ROOT.TMath.ErfInverse(1.0 - 2.0 * p)


def z_asimov(s, b):
    if b <= 0 or s <= 0:
        return 0.0
    return math.sqrt(2.0 * ((s + b) * math.log(1.0 + s / b) - s))


def metrics(s, b):
    return {
        "S": s, "B": b,
        "SoB": s / b if b > 0 else 0.0,
        "SosqrtB": s / math.sqrt(b) if b > 0 else 0.0,
        "ZBi1": z_bi(s, b, 0.01),
        "ZBi3": z_bi(s, b, 0.03),
        "ZA": z_asimov(s, b),
    }


# ------------------------------------------------------------------
# Reading the file
# ------------------------------------------------------------------
def list_processes(tfile, region, hist):
    procs = []
    for key in tfile.GetListOfKeys():
        parts = key.GetName().split("__")
        if len(parts) == 3 and parts[0] == region and parts[2] == hist:
            procs.append(parts[1])
    return procs


def list_hists(tfile, region, prefix):
    names = []
    for key in tfile.GetListOfKeys():
        parts = key.GetName().split("__")
        if len(parts) == 3 and parts[0] == region and parts[2].startswith(prefix) and parts[2] not in names:
            names.append(parts[2])
    return names


def get_hist(tfile, region, proc, hist, scale, rebin):
    h = tfile.Get(f"{region}__{proc}__{hist}")
    if not h:
        return None
    h = h.Clone(f"{proc}_{hist}_clone")
    if h.GetSumw2N() == 0:
        h.Sumw2()
    if rebin > 1:
        h.Rebin(rebin)
    h.Scale(scale)
    return h


def fold_flows(h):
    """Put underflow into the first bin and overflow into the last bin, with errors."""
    n = h.GetNbinsX()
    for src, dst in ((0, 1), (n + 1, n)):
        c = h.GetBinContent(dst) + h.GetBinContent(src)
        e = math.hypot(h.GetBinError(dst), h.GetBinError(src))
        h.SetBinContent(dst, c)
        h.SetBinError(dst, e)
        h.SetBinContent(src, 0.0)
        h.SetBinError(src, 0.0)


def yield_and_error(h):
    """Integral including under/overflow, with its statistical error."""
    import ctypes
    e = ctypes.c_double(0.0)
    val = h.IntegralAndError(0, h.GetNbinsX() + 1, e)
    return val, e.value


def split_sig_bkg(procs, signal_prefix):
    sigs = [p for p in procs if p.startswith(signal_prefix)]
    bkgs = [p for p in procs if not p.startswith(signal_prefix)]
    return sigs, bkgs


def pretty(proc):
    """ZZ_2022 -> ZZ ; Cascades_180 -> Cascades_180"""
    if proc.startswith("Cascades"):
        return proc
    return proc.rsplit("_", 1)[0] if proc.rsplit("_", 1)[-1].isdigit() else proc


def color_for(proc, i):
    return COLORS.get(pretty(proc), FALLBACK_COLORS[i % len(FALLBACK_COLORS)])


def stats_box(x1, y1, x2, y2, label, m):
    box = ROOT.TPaveText(x1, y1, x2, y2, "NDC")
    box.SetFillColor(0)
    box.SetBorderSize(1)
    box.SetTextFont(42)
    box.SetTextAlign(12)
    box.AddText(label)
    box.AddText(f"S = {m['S']:.2f}, S/B = {m['SoB']:.4f}")
    box.AddText(f"S/#sqrt{{B}} = {m['SosqrtB']:.2f}, Z_{{Bi}}^{{1%/3%}} = {m['ZBi1']:.2f}/{m['ZBi3']:.2f}")
    box.AddText(f"med[Z_{{0}}^{{A}} | #mu=1] = {m['ZA']:.2f}")
    return box


# ------------------------------------------------------------------
# 1. Distribution plot
# ------------------------------------------------------------------
SIGNAL_COLORS = [ROOT.kBlue, ROOT.kAzure + 10, ROOT.kBlue + 3, ROOT.kViolet - 3, ROOT.kCyan + 3, ROOT.kAzure - 4]


def plot_dist(tfile, args, hist, sigs):
    """All backgrounds, their total, and every signal in `sigs` on one plot."""
    procs = list_processes(tfile, args.region, hist)
    _, bkgs = split_sig_bkg(procs, args.signal_prefix)

    hs_list = []
    for i, sig in enumerate(sigs):
        h = get_hist(tfile, args.region, sig, hist, args.scale, args.rebin)
        if h is None:
            print(f"  [skip] {hist}: no histogram for {sig}")
            continue
        fold_flows(h)
        hs_list.append((sig, h, SIGNAL_COLORS[i % len(SIGNAL_COLORS)]))
    if not hs_list:
        return

    hb_list = []
    for i, p in enumerate(bkgs):
        h = get_hist(tfile, args.region, p, hist, args.scale, args.rebin)
        if h is None:
            continue
        fold_flows(h)
        hb_list.append((p, h, color_for(p, i)))
    if not hb_list:
        print(f"  [skip] {hist}: no background histograms")
        return

    htot = hb_list[0][1].Clone("htot")
    htot.Reset()
    for _, h, _ in hb_list:
        htot.Add(h)

    b, be = yield_and_error(htot)

    c = ROOT.TCanvas("c", "c", 1100, 800)
    c.SetLogy()
    c.SetGridx()
    c.SetGridy()
    c.SetLeftMargin(0.12)
    c.SetRightMargin(0.04)

    htot.SetTitle(f"Reco Level Study {args.lumi_label}")
    htot.GetXaxis().SetTitle(args.xtitle or XTITLES.get(hist, hist))
    htot.GetYaxis().SetTitle("Events per bin")
    htot.GetYaxis().SetTitleOffset(1.3)
    htot.SetLineColor(ROOT.kBlack)
    htot.SetLineWidth(3)
    ymax = max([htot.GetMaximum()] + [h.GetMaximum() for _, h, _ in hs_list])
    htot.SetMaximum(max(ymax, 1.0) * 1e4)   # headroom for legend and box
    htot.SetMinimum(args.ymin)
    htot.Draw("HIST")

    for _, h, col in hb_list:
        h.SetLineColor(col)
        h.SetLineWidth(2)
        h.Draw("HIST SAME")
    for _, h, col in hs_list:
        h.SetLineColor(col)
        h.SetLineWidth(3)
        h.Draw("HIST SAME")
    htot.Draw("HIST SAME")

    # Legend: every signal, total background, every background, with yields
    leg = ROOT.TLegend(0.58, 0.50, 0.95, 0.89)
    leg.SetTextFont(42)
    leg.SetTextSize(0.025)
    leg.SetFillColor(0)
    for sig, h, _ in hs_list:
        s, se = yield_and_error(h)
        leg.AddEntry(h, f"{sig} [{s:.2f}#pm{se:.2f}]", "l")
    leg.AddEntry(htot, f"Total Bkgd. [{b:.2f}#pm{be:.2f}]", "l")
    for p, h, _ in hb_list:
        y, e = yield_and_error(h)
        leg.AddEntry(h, f"{pretty(p)} [{y:.2f}#pm{e:.2f}]", "l")
    leg.Draw()

    # Box: one line of metrics per signal (B is common)
    nlines = 2 + len(hs_list)
    box = ROOT.TPaveText(0.14, 0.89 - 0.045 * nlines, 0.57, 0.89, "NDC")
    box.SetFillColor(0)
    box.SetBorderSize(1)
    box.SetTextFont(42)
    box.SetTextAlign(12)
    box.AddText(f"{args.sel_label},  B = {b:.2f}")
    box.AddText("S,  S/B,  S/#sqrt{B},  Z_{Bi}^{1%/3%},  Z^{A}")
    for sig, h, col in hs_list:
        m = metrics(yield_and_error(h)[0], b)
        t = box.AddText(f"{sig}: {m['S']:.2f}, {m['SoB']:.3f}, {m['SosqrtB']:.2f}, "
                        f"{m['ZBi1']:.2f}/{m['ZBi3']:.2f}, {m['ZA']:.2f}")
        t.SetTextColor(col)
    box.Draw()

    tag = "allsig" if len(hs_list) > 1 else hs_list[0][0]
    out = os.path.join(args.outdir, f"{hist}__{tag}.{args.ext}")
    c.SaveAs(out)
    c.Close()


# ------------------------------------------------------------------
# 2. Cut-value scan
# ------------------------------------------------------------------
def scan_values(hs, hb, direction):
    """Return list of (cut, S, B) using the bin edges as cut values."""
    n = hs.GetNbinsX()
    out = []
    if direction == "lt":      # pass if x <= cut : sum bins 0..i (underflow passes)
        s = hs.GetBinContent(0)
        b = hb.GetBinContent(0)
        for i in range(1, n + 1):
            s += hs.GetBinContent(i)
            b += hb.GetBinContent(i)
            out.append((hs.GetXaxis().GetBinUpEdge(i), s, b))
    elif direction == "gt":    # pass if x >= cut : sum bins i..n+1 (overflow passes)
        for i in range(1, n + 1):
            s = hs.Integral(i, n + 1)
            b = hb.Integral(i, n + 1)
            out.append((hs.GetXaxis().GetBinLowEdge(i), s, b))
    elif direction == "absgt": # pass if |x| >= cut, using bin centres
        edges = sorted({abs(hs.GetXaxis().GetBinLowEdge(i)) for i in range(1, n + 2)})
        for cut in edges:
            s = b = 0.0
            for i in range(0, n + 2):
                x = hs.GetXaxis().GetBinCenter(i)
                if abs(x) >= cut:
                    s += hs.GetBinContent(i)
                    b += hb.GetBinContent(i)
            out.append((cut, s, b))
    return out


def plot_scan(tfile, args, hist, sig):
    direction, nominal = CUT_DEFAULTS.get(hist, (None, None))
    if args.cut_dir:
        direction = args.cut_dir
    if args.cut_value is not None:
        nominal = args.cut_value
    if direction is None:
        print(f"  [skip scan] {hist}: no scan direction (use --cut-dir lt|gt|absgt)")
        return

    procs = list_processes(tfile, args.region, hist)
    _, bkgs = split_sig_bkg(procs, args.signal_prefix)
    hs = get_hist(tfile, args.region, sig, hist, args.scale, args.rebin)
    if hs is None:
        return
    hb = None
    for p in bkgs:
        h = get_hist(tfile, args.region, p, hist, args.scale, args.rebin)
        if h is None:
            continue
        if hb is None:
            hb = h.Clone("hb_tot")
        else:
            hb.Add(h)
    if hb is None:
        return

    pts = scan_values(hs, hb, direction)
    series = {
        "S/#sqrt{B}":        (ROOT.kBlue,     lambda m: m["SosqrtB"]),
        "Z_{Bi}^{1%}":       (ROOT.kMagenta,  lambda m: m["ZBi1"]),
        "Z_{Bi}^{3%}":       (ROOT.kOrange,   lambda m: m["ZBi3"]),
        "10 S/B":            (ROOT.kRed + 1,  lambda m: 10 * m["SoB"]),
        "S/10":              (ROOT.kBlack,    lambda m: m["S"] / 10.0),
        "B/100":             (ROOT.kGreen + 2, lambda m: m["B"] / 100.0),
    }
    graphs = {}
    ymax = 0.0
    best = (None, -1.0)
    for name, (col, f) in series.items():
        g = ROOT.TGraph()
        g.SetLineColor(col)
        g.SetLineWidth(3)
        graphs[name] = g
    for cut, s, b in pts:
        m = metrics(s, b)
        for name, (col, f) in series.items():
            v = f(m) if b > 0 else 0.0
            graphs[name].SetPoint(graphs[name].GetN(), cut, v)
            ymax = max(ymax, v)
        if b > 0 and m["ZBi3"] > best[1]:
            best = (cut, m["ZBi3"])

    c = ROOT.TCanvas("c", "c", 1000, 750)
    c.SetGridx()
    c.SetGridy()
    c.SetLeftMargin(0.12)
    xlo = hs.GetXaxis().GetXmin() if direction != "absgt" else 0.0
    xhi = hs.GetXaxis().GetXmax()
    ytop = args.scan_ymax if args.scan_ymax else ymax * 1.15
    frame = c.DrawFrame(xlo, 0.0, xhi, ytop)
    frame.SetTitle("Reco level cut optimization")
    xt = args.xtitle or XTITLES.get(hist, hist)
    frame.GetXaxis().SetTitle(("|" + xt.replace(" [GeV]", "") + "| [GeV]" if direction == "absgt" else xt) + " cut value")
    frame.GetYaxis().SetTitle("Performance metric")

    for g in graphs.values():
        g.Draw("L SAME")

    if nominal is not None:
        line = ROOT.TLine(nominal, 0.0, nominal, ytop)
        line.SetLineStyle(2)
        line.SetLineWidth(2)
        line.Draw()

    leg = ROOT.TLegend(0.66, 0.60, 0.90, 0.89)
    leg.SetHeader("Performance Metrics")
    leg.SetTextFont(42)
    leg.SetFillColor(0)
    for name, g in graphs.items():
        leg.AddEntry(g, name, "l")
    leg.Draw()

    box = ROOT.TPaveText(0.15, 0.70, 0.52, 0.89, "NDC")
    box.SetFillColor(0)
    box.SetTextFont(42)
    box.SetTextAlign(12)
    box.AddText(args.sel_label)
    box.AddText(f"Scaled to {args.lumi_label}")
    box.AddText(f"Signal = {sig}")
    pass_txt = {"lt": "x #leq cut", "gt": "x #geq cut", "absgt": "|x| #geq cut"}[direction]
    box.AddText(f"Pass: {pass_txt}" + (f", current = {nominal:g}" if nominal is not None else ""))
    if best[0] is not None:
        box.AddText(f"Best Z_{{Bi}}^{{3%}} = {best[1]:.2f} at {best[0]:g}")
    box.Draw()

    out = os.path.join(args.outdir, f"scan__{hist}__{sig}.{args.ext}")
    c.SaveAs(out)
    c.Close()


# ------------------------------------------------------------------
# 3. Cutflow
# ------------------------------------------------------------------
def plot_cutflow(tfile, args, sig):
    hist = "CutFlow"
    procs = list_processes(tfile, args.region, hist)
    _, bkgs = split_sig_bkg(procs, args.signal_prefix)
    hs = get_hist(tfile, args.region, sig, hist, args.scale, 1)
    if hs is None:
        print(f"  [skip] no CutFlow for {sig} in region {args.region}")
        return
    n = hs.GetNbinsX()

    hb_list = []
    for i, p in enumerate(bkgs):
        h = get_hist(tfile, args.region, p, hist, args.scale, 1)
        if h is not None:
            hb_list.append((p, h, color_for(p, i)))
    htot = hb_list[0][1].Clone("cf_tot")
    htot.Reset()
    for _, h, _ in hb_list:
        htot.Add(h)

    s, se = hs.GetBinContent(n), hs.GetBinError(n)
    b, be = htot.GetBinContent(n), htot.GetBinError(n)
    m = metrics(s, b)

    c = ROOT.TCanvas("c", "c", 1100, 800)
    c.SetLogy()
    c.SetGridx()
    c.SetGridy()
    c.SetBottomMargin(0.18)
    c.SetLeftMargin(0.12)
    htot.SetTitle(f"Reco Level Study {args.lumi_label}")
    htot.GetXaxis().SetTitle("Cut Flow")
    htot.GetXaxis().SetLabelSize(0.035)
    htot.GetXaxis().LabelsOption("d")
    htot.GetYaxis().SetTitle("Events per bin")
    htot.SetLineColor(ROOT.kBlack)
    htot.SetLineWidth(3)
    htot.SetMaximum(max(htot.GetMaximum(), 1.0) * 1e3)
    htot.SetMinimum(args.ymin if args.ymin > 0 else 1.0)
    htot.Draw("HIST")
    for _, h, col in hb_list:
        h.SetLineColor(col)
        h.SetLineWidth(2)
        h.Draw("HIST SAME")
    hs.SetLineColor(ROOT.kBlue)
    hs.SetLineWidth(3)
    hs.Draw("HIST SAME")
    htot.Draw("HIST SAME")

    leg = ROOT.TLegend(0.53, 0.55, 0.95, 0.89)
    leg.SetTextFont(42)
    leg.SetTextSize(0.028)
    leg.SetFillColor(0)
    leg.AddEntry(hs, f"{sig} [{s:.2f}#pm{se:.2f}]", "l")
    leg.AddEntry(htot, f"Total Bkgd. [{b:.2f}#pm{be:.2f}]", "l")
    for p, h, _ in hb_list:
        leg.AddEntry(h, f"{pretty(p)} [{h.GetBinContent(n):.2f}#pm{h.GetBinError(n):.2f}]", "l")
    leg.Draw()
    box = stats_box(0.15, 0.68, 0.50, 0.89, args.sel_label, m)
    box.Draw()

    out = os.path.join(args.outdir, f"CutFlow__{args.region}__{sig}.{args.ext}")
    c.SaveAs(out)
    c.Close()


# ------------------------------------------------------------------
def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-f", "--file", required=True, help="merged ROOT file")
    ap.add_argument("--region", default="NoCuts", help="region prefix in histogram names (default NoCuts)")
    ap.add_argument("--hist", action="append", default=[], help="histogram name, e.g. nm1_quad_minmll (repeatable)")
    ap.add_argument("--all-nm1", action="store_true", help="plot every nm1_* histogram in the region")
    ap.add_argument("--mode", default="both", choices=["dist", "scan", "both", "cutflow"])
    ap.add_argument("--signal", action="append", default=[], help="signal process, e.g. Cascades_180 (default: all)")
    ap.add_argument("--signal-prefix", default="Cascades")
    ap.add_argument("--scale", type=float, default=1.0, help="multiply all histograms by this (lumi rescaling)")
    ap.add_argument("--lumi-label", default="400 fb^{-1}")
    ap.add_argument("--sel-label", default="4l selection (N-1)")
    ap.add_argument("--rebin", type=int, default=1)
    ap.add_argument("--cut-dir", choices=["lt", "gt", "absgt"], help="override scan direction")
    ap.add_argument("--cut-value", type=float, help="override nominal cut line in the scan")
    ap.add_argument("--xtitle", default=None)
    ap.add_argument("--ymin", type=float, default=0.05)
    ap.add_argument("--scan-ymax", type=float, default=None)
    ap.add_argument("--outdir", default="plots_nm1")
    ap.add_argument("--ext", default="png")
    args = ap.parse_args()

    os.makedirs(args.outdir, exist_ok=True)
    tfile = ROOT.TFile.Open(args.file)
    if not tfile or tfile.IsZombie():
        raise SystemExit(f"cannot open {args.file}")

    if args.mode == "cutflow":
        procs = list_processes(tfile, args.region, "CutFlow")
        sigs = args.signal or split_sig_bkg(procs, args.signal_prefix)[0]
        for sig in sigs:
            plot_cutflow(tfile, args, sig)
        return

    hists = list(args.hist)
    if args.all_nm1:
        hists += [h for h in list_hists(tfile, args.region, "nm1_") if h not in hists]
    if not hists:
        raise SystemExit("give --hist NAME or --all-nm1")

    for hist in hists:
        procs = list_processes(tfile, args.region, hist)
        if not procs:
            print(f"[warn] nothing found for {args.region}__*__{hist}")
            continue
        sigs = args.signal or split_sig_bkg(procs, args.signal_prefix)[0]
        print(f"{hist}: {len(procs)} processes, signals {sigs}")
        if args.mode in ("dist", "both"):
            plot_dist(tfile, args, hist, sigs)          # one plot, all signals overlaid
        if args.mode in ("scan", "both"):
            for sig in sigs:                            # scan is per signal
                plot_scan(tfile, args, hist, sig)


if __name__ == "__main__":
    main()
