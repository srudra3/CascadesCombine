// overlay_ZZCR.C
// Overlay normalized, unfilled histograms of quad_PT_lep0 / quad_PT_lep1
// for ZZ_2022, TB_2022, ttbar_2022 samples in the M2lZVeto_ZZCR region.
//
// Usage:
//   root -l -b -q overlay_ZZCR.C

#include <algorithm>
#include <vector>
#include <string>
#include "TFile.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TLatex.h"
#include "TStyle.h"

void DrawOverlay(TFile* f, const std::string& var, const std::string& outname) {

    // --- Build histogram names ---
    std::vector<std::string> samples = {"ZZ_2022", "TB_2022", "ttbar_2022"};
    std::vector<std::string> labels  = {"ZZ", "TB", "t#bar{t}"};
    std::vector<int> colors = {kBlue+1, kOrange+7, kGray+2};

    TCanvas* c = new TCanvas(("c_" + var).c_str(), "", 900, 700);
    c->SetLeftMargin(0.13);
    c->SetRightMargin(0.30); // room for legend on the right, like the reference plot
    // c->SetLogy(); // uncomment if you want log-y like the CMS style plot above

    TLegend* leg = new TLegend(0.72, 0.55, 0.98, 0.88);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->SetTextSize(0.03);

    // --- First pass: load, clone, normalize, and find the true global max ---
    std::vector<TH1D*> hists;
    double globalMax = 0;

    for (size_t i = 0; i < samples.size(); ++i) {
        std::string hname = "M2lZVeto_ZZCR__" + samples[i] + "__" + var;
        TH1D* h = (TH1D*)f->Get(hname.c_str());
        if (!h) {
            std::cerr << "WARNING: histogram not found: " << hname << std::endl;
            hists.push_back(nullptr);
            continue;
        }
        h = (TH1D*)h->Clone((hname + "_clone").c_str());
        h->SetDirectory(0);
        h->Sumw2(); // make sure per-bin errors reflect weight variance, not just sqrt(N)

        // Normalize to unit area
        double integral = h->Integral();
        if (integral != 0) h->Scale(1.0 / integral);

        // No fill, just colored outline
        h->SetFillStyle(0);
        h->SetLineColor(colors[i]);
        h->SetLineWidth(2);
        h->SetLineStyle(1);
        h->SetStats(0);

        globalMax = std::max(globalMax, h->GetMaximum());
        hists.push_back(h);
        leg->AddEntry(h, labels[i].c_str(), "l");
    }

    // --- Second pass: draw, using globalMax so nothing gets clipped ---
    TH1D* hFirst = nullptr;
    for (size_t i = 0; i < hists.size(); ++i) {
        TH1D* h = hists[i];
        if (!h) continue;

        if (!hFirst) {
            hFirst = h;
            hFirst->SetTitle("");
            hFirst->GetYaxis()->SetTitle("Normalized events");
            hFirst->GetXaxis()->SetTitle((var == "quad_PT_lep0") ? "PT[0] [GeV]" : "PT[1] [GeV]");
            hFirst->GetYaxis()->SetTitleOffset(1.4);
            hFirst->SetMaximum(globalMax * 1.15); // headroom so peaks aren't clipped
            hFirst->Draw("HIST E"); // draw with error bars to expose low-stat/high-weight bins
        } else {
            h->Draw("HIST E SAME");
        }
    }

    leg->Draw();

    // "CMS Preliminary" style label, like the reference plot
    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(62);
    latex.SetTextSize(0.04);
    latex.DrawLatex(0.13, 0.92, "CMS");
    latex.SetTextFont(52);
    latex.DrawLatex(0.20, 0.92, "Preliminary");

    c->SaveAs((outname + ".png").c_str());
    c->SaveAs((outname + ".pdf").c_str());
}

void overlay_ZZCR() {
    gStyle->SetOptStat(0);

    TFile* f = TFile::Open("runs/ZZCR_notfirstZmassCut/final_hadded.root", "READ");
    if (!f || f->IsZombie()) {
        std::cerr << "ERROR: could not open input ROOT file." << std::endl;
        return;
    }

    DrawOverlay(f, "quad_PT_lep0", "overlay_quad_PT_lep0");
    DrawOverlay(f, "quad_PT_lep1", "overlay_quad_PT_lep1");

    f->Close();
}
