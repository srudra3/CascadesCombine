#include "PlottingHelpers.h"

// ----------------------
// Plot 1D
// ----------------------
void Plot_Hist1D(TH1* h) {
    if (!h) return;
    string title = h->GetName();
    TCanvas* can = new TCanvas(("can_"+title).c_str(), ("can_"+title).c_str(), 700, 600);
    can->SetLeftMargin(0.15); can->SetRightMargin(0.18); can->SetBottomMargin(0.15);
    can->SetGridx(); can->SetGridy();
    DrawLogSmart(h, "HIST");
    h->GetXaxis()->CenterTitle();
    h->GetYaxis()->CenterTitle();
    h->GetYaxis()->SetTitle(("N_{events} / "+std::to_string(int(lumi))+" fb^{-1}").c_str());
    h->GetYaxis()->SetRangeUser(h->GetMinimum()*0.9, 1.1*h->GetMaximum());
    TLatex l; l.SetTextFont(42); l.SetNDC();
    std::string proc_title = title;
    if(proc_title.find("TChiWZ") != std::string::npos)
        proc_title = makeSMSChiTitle(ExtractProcName(proc_title));
    else
        proc_title = m_Title[ExtractProcName(proc_title)];
    l.SetTextSize(0.035); l.DrawLatex(0.57,0.943,proc_title.c_str());
    l.SetTextSize(0.04); l.DrawLatex(0.01,0.943,CMS_label.c_str());
    l.SetTextSize(0.045); l.DrawLatex(0.7,0.04,ExtractBinName(title).c_str());
    TString pdfName = Form("%spdfs/%s/%s.pdf", outputDir.c_str(), ExtractBinName(title).c_str(), title.c_str());
    gErrorIgnoreLevel = 1001;
    can->SaveAs(pdfName);
    gErrorIgnoreLevel = 0;
    if (outFile) { outFile->cd(); can->Write(0, TObject::kWriteDelete); }
    delete can;
}

// ----------------------
// Plot 2D
// ----------------------
void Plot_Hist2D(TH2* h) {
    if (!h) return;
    string title = h->GetName();
    TCanvas* can = new TCanvas(("can_"+title).c_str(), ("can_"+title).c_str(), 700, 600);
    can->SetLeftMargin(0.15); can->SetRightMargin(0.18); can->SetBottomMargin(0.15);
    can->SetGridx(); can->SetGridy();
    DrawLogSmart(h, "COLZ");
    h->SetMinimum(0.);
    h->Draw("COLZ");
    h->GetXaxis()->CenterTitle(); h->GetYaxis()->CenterTitle(); h->GetZaxis()->CenterTitle();
    h->GetZaxis()->SetTitle(("N_{events} / "+std::to_string(int(lumi))+" fb^{-1}").c_str());
    h->GetXaxis()->SetTitleOffset(1.05);
    h->GetZaxis()->SetTitleOffset(1.05);
    double t_size = 0.06;
    h->GetXaxis()->SetTitleSize(t_size); h->GetYaxis()->SetTitleSize(t_size); h->GetZaxis()->SetTitleSize(t_size);
    std::string proc_title = title;
    if(proc_title.find("TChiWZ") != std::string::npos)
        proc_title = makeSMSChiTitle(ExtractProcName(proc_title));
    else
        proc_title = m_Title[ExtractProcName(proc_title)];
    string bin_label = ExtractBinName(title);
    std::replace(bin_label.begin(), bin_label.end(), '_', ' ');
    double left   = gPad->GetLeftMargin();
    double right  = 1.0 - gPad->GetRightMargin();
    TLatex l;
    l.SetTextFont(42);
    l.SetNDC();
    l.SetTextSize(0.04);
    // --- Right-aligned (proc title) ---
    l.SetTextAlign(31);   // right-align horizontally, top-align vertically
    l.DrawLatex(right, 0.92, proc_title.c_str());
    // --- Left-aligned (CMS label) ---
    l.SetTextAlign(11);   // left-align horizontally, top-align vertically
    l.DrawLatex(left, 0.92, CMS_label.c_str());
    //l.SetTextSize(0.045); l.DrawLatex(0.7,0.04,bin_label.c_str());
    TString pdfName = Form("%spdfs/%s/%s.pdf", outputDir.c_str(), ExtractBinName(title).c_str(), title.c_str());
    gErrorIgnoreLevel = 1001;
    can->SaveAs(pdfName);
    gErrorIgnoreLevel = 0;
    if (outFile) { outFile->cd(); can->Write(0, TObject::kWriteDelete); }
    delete can;
}

// ----------------------
// Plot Ratio
// ----------------------
void Plot_Ratio(TH1* h, const std::string& outputDir, const RatioDef* rDef=nullptr){
    if(!h) return;

    std::string title = h->GetName();
    TCanvas* can = new TCanvas(("can_"+title).c_str(), ("can_"+title).c_str(), 700, 600);
    can->SetLeftMargin(0.15); can->SetRightMargin(0.18); can->SetBottomMargin(0.15);
    can->SetGridx(); can->SetGridy();

    // Determine if 1D or 2D
    if(h->InheritsFrom(TH2::Class())){
        TH2* h2 = dynamic_cast<TH2*>(h);
        // Set Z axis first
        if(rDef && rDef->z_range.has_value()) {
            h2->SetMinimum(rDef->z_range->first);
            h2->SetMaximum(rDef->z_range->second);
        } else {
            h2->SetMinimum(0.);
        }
        // Set Y axis (if provided)
        if(rDef && rDef->y_range.has_value()) {
            h2->GetYaxis()->SetRangeUser(rDef->y_range->first, rDef->y_range->second);
        }
        // Draw with style
        if(!rDef->normalize)
          DrawLogSmart(h2, "COLZ");
        else
          h2->Draw("COLZ");
        h2->GetXaxis()->CenterTitle();
        h2->GetYaxis()->CenterTitle();
        h2->GetZaxis()->CenterTitle();
        h2->GetZaxis()->SetTitle("Ratio");
    
    } else {
        // 1D histogram
        if(!rDef->normalize)
          DrawLogSmart(h, "HIST");
        else
          h->Draw("HIST");
        h->GetXaxis()->CenterTitle();
        h->GetYaxis()->CenterTitle();
        h->GetYaxis()->SetTitle("Ratio");
        if(rDef && rDef->y_range.has_value()) {
            h->GetYaxis()->SetRangeUser(rDef->y_range->first, rDef->y_range->second);
        } else {
            // fallback to reasonable auto-range
            h->GetYaxis()->SetRangeUser(h->GetMinimum() * 0.9, 1.1 * h->GetMaximum());
        }
    }

    // Label CMS / Process / Bin
    TLatex l; l.SetTextFont(42); l.SetNDC();
    std::string proc_title = title;
    if(proc_title.find("TChiWZ") != std::string::npos)
        proc_title = makeSMSChiTitle(ExtractProcName(proc_title));
    else
        proc_title = m_Title[ExtractProcName(proc_title)];
    //l.SetTextSize(0.035); l.DrawLatex(0.65,0.943,proc_title.c_str());
    l.SetTextSize(0.04);  l.DrawLatex(0.13,0.943,CMS_label.c_str());
    //l.SetTextSize(0.045); l.DrawLatex(0.7,0.04,ExtractBinName(title).c_str());

    // Output file
    TString pdfName = Form("%spdfs/%s.pdf", outputDir.c_str(), title.c_str());
    gErrorIgnoreLevel = 1001;
    can->SaveAs(pdfName);
    gErrorIgnoreLevel = 0;

    if(outFile) { outFile->cd(); can->Write(0, TObject::kWriteDelete); }
    delete can;
}

// ----------------------
// Plot Eff
// ----------------------
void Plot_Eff(TEfficiency* e){
    string title = e->GetName();
    TCanvas* can = (TCanvas*) new TCanvas(("can_eff_"+title).c_str(),("can_"+title).c_str(),700.,600);
    can->SetLeftMargin(0.15); can->SetRightMargin(0.18); can->SetBottomMargin(0.15);
    can->SetGridx(); can->SetGridy();
    can->Draw();
    can->cd();
    e->SetStatisticOption(TEfficiency::kFNormal); // assume effs were made from weighted hists
    e->Draw("AP");
    gPad->Update();
    e->GetPaintedGraph()->GetXaxis()->CenterTitle();
    e->GetPaintedGraph()->GetXaxis()->SetTitleFont(42);
    e->GetPaintedGraph()->GetXaxis()->SetTitleSize(0.06);
    e->GetPaintedGraph()->GetXaxis()->SetTitleOffset(1.06);
    e->GetPaintedGraph()->GetXaxis()->SetLabelFont(42);
    e->GetPaintedGraph()->GetXaxis()->SetLabelSize(0.05);
    double xmin = e->GetTotalHistogram()->GetXaxis()->GetXmin();
    double xmax = e->GetTotalHistogram()->GetXaxis()->GetXmax();
    if(xmin < 0) xmin = xmin*1.1;
    else xmin = xmin*0.9;
    if(xmax > 0) xmax = xmax*1.1;
    else xmax = xmax*0.9;
    e->GetPaintedGraph()->GetXaxis()->SetRangeUser(xmin,xmax);
    e->GetPaintedGraph()->GetYaxis()->CenterTitle();
    e->GetPaintedGraph()->GetYaxis()->SetTitleFont(42);
    e->GetPaintedGraph()->GetYaxis()->SetTitleSize(0.06);
    e->GetPaintedGraph()->GetYaxis()->SetTitleOffset(1.12);
    e->GetPaintedGraph()->GetYaxis()->SetLabelFont(42);
    e->GetPaintedGraph()->GetYaxis()->SetLabelSize(0.05);
    e->GetPaintedGraph()->GetYaxis()->SetRangeUser(0.,1.05);

    TLatex l; l.SetTextFont(42); l.SetNDC();
    std::string proc_title = title;
    if(proc_title.find("TChiWZ") != std::string::npos)
        proc_title = makeSMSChiTitle(ExtractProcName(proc_title));
    else
        proc_title = m_Title[ExtractProcName(proc_title)];
    l.SetTextSize(0.035); l.DrawLatex(0.65,0.943,proc_title.c_str());
    l.SetTextSize(0.04); l.DrawLatex(0.01,0.943,CMS_label.c_str());
    l.SetTextSize(0.045); l.DrawLatex(0.7,0.04,ExtractBinName(title).c_str());
    gSystem->mkdir(Form("%spdfs/%s/", outputDir.c_str(), ExtractBinName(title).c_str()));
    TString pdfName = Form("%spdfs/%s/%s.pdf", outputDir.c_str(), ExtractBinName(title).c_str(), title.c_str());
    gErrorIgnoreLevel = 1001;
    can->SaveAs(pdfName);
    gErrorIgnoreLevel = 0;
    if (outFile) { outFile->cd(); can->Write(0, TObject::kWriteDelete); }
    delete can;
}

// ----------------------
// Plot stack
// ----------------------
void Plot_Stack(const string& hname,
                vector<TH1*>& bkgHists,
                vector<TH1*>& sigHists,
                TH1* dataHist = nullptr,
                double signal_boost = 1.0,
                bool isFD_Stack = false,
                const std::string& groupTitle = "",
                const std::unordered_map<std::string, const YamlBinPattern*>& binLookup = {},
                const std::map<std::string, std::set<int>>& risrNeighborMap = {},
                const YamlBinPattern* groupPattern = {}
               )
{
    if (bkgHists.empty() && (sigHists.empty() || !dataHist)) return;
    if (!bkgHists.empty()) SortHistsByYield(bkgHists);
    vector<TH1*> allHists = bkgHists; allHists.insert(allHists.end(), sigHists.begin(), sigHists.end());
    if (dataHist) allHists.push_back(dataHist);
    double hmin, hmax; GetMinMaxIntegral(allHists, hmin, hmax);
    if (hmin <= 0.) hmin = 1.e-1;

    int stack_index = 0;
    TH1D* h_BKG = nullptr;
    for (auto* h : bkgHists) {
        if (!h) continue;
        SetMinimumBinContent(h, hmin); 
        if (!h_BKG) { 
            h_BKG = (TH1D*) h->Clone("TOT_BKG"); 
        } else {
            for(int k = 0; k < stack_index; k++){
              bkgHists[k]->Add(h);
            }
            h_BKG->Add(h);
        }
        stack_index++;
    }
    TH1D* h_DATA = nullptr;
    if (dataHist) h_DATA = (TH1D*) dataHist->Clone("TOT_DATA");
    // --- Build total background uncertainty band ---
    TH1D* h_BKG_ERR = nullptr;
    if (h_BKG) {
        h_BKG_ERR = (TH1D*) h_BKG->Clone("h_BKG_ERR");
        h_BKG_ERR->SetFillColor(kGray+2);
        h_BKG_ERR->SetFillStyle(3354);
        h_BKG_ERR->SetLineWidth(0);
        h_BKG_ERR->SetMarkerSize(0);
    }
    TH1D* h_ratio_err = nullptr;
    TH1D* h_ratio     = nullptr;

    double pad_ysplit = 0.5;
    string canvas_name = "can_stack_" + hname;
    TCanvas* can = new TCanvas(canvas_name.c_str(), canvas_name.c_str(), 1200, 700);
    can->SetLeftMargin(hlo);
    can->SetRightMargin(hhi);
    if(h_DATA) {
        can->SetLeftMargin(hlo-0.02);
        can->SetRightMargin(hhi-0.1);
    }
    can->SetBottomMargin(hbo);
    can->SetTopMargin(hto);
    //can->SetGridx();
    can->SetGridy();
    can->cd();
    TPad* pad_top = nullptr;
    if(h_DATA){
        pad_top = new TPad("pad_top", "pad_top", 0.0, pad_ysplit, 1.0, 1.0);
        pad_top->SetBottomMargin(0.01);
        pad_top->SetLeftMargin(can->GetLeftMargin());
        pad_top->SetRightMargin(can->GetRightMargin());
        //pad_top->SetGridx(true);
        pad_top->SetGridy(true);
        pad_top->Draw();
        pad_top->cd();
    }

    TH1* axisHist = !allHists.empty() ? allHists.front() : nullptr;
    if (!axisHist) return;
    DrawLogSmart(axisHist, "HIST");
    axisHist->GetYaxis()->SetRangeUser(max(0.9*hmin, 5.e-1), 1.5*hmax);
    axisHist->GetYaxis()->CenterTitle();
    axisHist->GetYaxis()->SetTitle(("N_{events}"));// / "+std::to_string(int(lumi))+" fb^{-1}").c_str());
    if(h_DATA){
        axisHist->GetXaxis()->SetLabelSize(0.0);
        axisHist->GetYaxis()->SetTitleSize(0.05);
        axisHist->GetYaxis()->SetTitleOffset(0.45);
        axisHist->GetYaxis()->SetLabelSize(0.05);
    }
    axisHist->GetYaxis()->SetNdivisions(303);

    for (size_t i = 0; i < bkgHists.size(); ++i) { TH1* h = bkgHists[i]; if (!h || h->GetEntries()==0) continue;
        h->SetLineColor(kBlack); h->SetLineWidth(1);
        int color = kBlack;
        auto it = m_Color.find(ExtractProcName(bkgHists[i]->GetName()));
        if (it != m_Color.end()) {
            color = it->second;
        } else {
            color = fallbackColors[fallbackIndex % fallbackColors.size()];
            fallbackIndex++;
            m_Color[it->first] = color;
        }
        h->SetMarkerColor(color);
        h->SetFillColor(color); h->SetFillStyle(1001);
        DrawLogSmart(h, "SAME HIST"); 
    }

    if (h_BKG) { h_BKG->SetLineWidth(3); h_BKG->SetLineColor(kRed); DrawLogSmart(h_BKG, "SAME HIST"); }

    for (size_t i = 0; i < sigHists.size(); ++i) { TH1* h = sigHists[i]; if (!h || h->GetEntries()==0) continue;
        SetMinimumBinContent(h, hmin);
        h->SetLineWidth(3); //h->SetLineStyle(7);
        int color = kBlack;
        auto it = m_Color.find(ExtractProcName(sigHists[i]->GetName()));
        if (it != m_Color.end()) {
            color = it->second;
        } else {
            color = fallbackColors[fallbackIndex % fallbackColors.size()];
            fallbackIndex++;
            m_Color[it->first] = color;
        }
        h->SetLineColor(color);
        h->SetMarkerColor(color);
        h->Scale(signal_boost); DrawLogSmart(h, "SAME HIST"); 
    }

    if (h_DATA) { h_DATA->SetMarkerStyle(20); h_DATA->SetMarkerSize(0.8); h_DATA->SetLineColor(kBlack); DrawLogSmart(h_DATA, "SAME E"); }
    // --- Draw uncertainty band on top pad ---
    if (h_DATA && h_BKG_ERR) {
        h_BKG_ERR->Draw("SAME E2");
    }

    TPad* pad_ratio = nullptr;

    if (h_BKG && h_DATA && h_BKG->GetEntries() > 0 && h_DATA->GetEntries() > 0) {

        can->cd();
        pad_ratio = new TPad("pad_ratio", "pad_ratio", 0.0, 0.05, 1.0, pad_ysplit);
        pad_ratio->SetTopMargin(0.01);
        pad_ratio->SetBottomMargin(0.35);
        pad_ratio->SetLeftMargin(can->GetLeftMargin());
        pad_ratio->SetRightMargin(can->GetRightMargin());
        //pad_ratio->SetGridx(true);
        pad_ratio->SetGridy(true);
        pad_ratio->Draw();
        pad_ratio->cd();

        h_ratio = (TH1D*) h_DATA->Clone("h_ratio");
        h_ratio->Divide(h_BKG);
        h_ratio->SetTitle("");
        float XLabelSize = 0.08;
        if (h_ratio->GetNbinsX() > 35) XLabelSize -= 0.001f * (h_ratio->GetNbinsX() - 5);
        if (XLabelSize < 0.015) XLabelSize = 0.015;
        h_ratio->GetXaxis()->SetLabelSize(XLabelSize);
        //h_ratio->GetXaxis()->LabelsOption("v");
        h_ratio->GetXaxis()->SetLabelOffset(0.01);
        h_ratio->GetYaxis()->SetTitle("#frac{data}{bkg model}");
        h_ratio->GetYaxis()->CenterTitle();
        h_ratio->GetYaxis()->SetNdivisions(505);
        h_ratio->GetYaxis()->SetTitleSize(0.05);
        h_ratio->GetYaxis()->SetTitleOffset(0.45);
        h_ratio->GetYaxis()->SetLabelSize(0.05);

        double rmin = 1e9;
        double rmax = -1e9;
        for (int i = 1; i <= h_ratio->GetNbinsX(); ++i) {
            double val = h_ratio->GetBinContent(i);
            double err = h_ratio->GetBinError(i);
            if (val == 0 && err == 0) continue; // skip empty bins
        
            double low  = val - err;
            double high = val + err;
        
            if (low  < rmin) rmin = low;
            if (high > rmax) rmax = high;
        }
        
        // Add a small padding
        double padding = 0.05 * (rmax - rmin);
        if (rmin - padding <= 0) rmin = 0.01; // avoid zero if using log scale
        else rmin -= padding;
        rmax += padding;
        
        h_ratio->GetYaxis()->SetRangeUser(rmin, rmax);

        h_ratio->SetMarkerStyle(20);
        h_ratio->SetMarkerSize(0.85);
        h_ratio->SetLineColor(kBlack);

        h_ratio->Draw("EP");
        // --- Background uncertainty band in ratio plot ---
        if (h_DATA && h_BKG_ERR) {
            h_ratio_err = (TH1D*) h_BKG_ERR->Clone("h_ratio_err");
            for (int i = 1; i <= h_ratio_err->GetNbinsX(); ++i) {
                double bc = h_BKG->GetBinContent(i);
                double be = h_BKG->GetBinError(i);
                if (bc > 0) {
                    h_ratio_err->SetBinContent(i, 1.0);
                    h_ratio_err->SetBinError(i, be / bc);
                } else {
                    h_ratio_err->SetBinContent(i, 0);
                    h_ratio_err->SetBinError(i, 0);
                }
            }
            h_ratio_err->SetFillColor(kGray+2);
            h_ratio_err->SetFillStyle(3354);
            h_ratio_err->SetLineWidth(0);
            h_ratio_err->SetMarkerSize(0);
            h_ratio_err->Draw("SAME E2");
        }

        pad_ratio->Modified();
    }

    if(h_DATA) { can->cd(); pad_top->cd(); }

    double legX1SHIFT = 0.;
    double legX2SHIFT = 0.;
    double legY1SHIFT = 0.;
    double legY2SHIFT = 0.;

    if(h_DATA){
        legX1SHIFT = 0.1;
        legX2SHIFT = 0.02;
        legY1SHIFT = -0.12;
        legY2SHIFT = 0.;
    }

    double textsize = 0.038;
    if(h_DATA) textsize = 0.05;
    TLegend* leg = new TLegend(1.-hhi+0.01+legX1SHIFT, 1.- (bkgHists.size()+sigHists.size()+2)*(1.-0.49)/9.+legY1SHIFT, 0.98+legX2SHIFT, 1.-hto-0.005+legY2SHIFT);
    leg->SetTextFont(132);
    leg->SetTextSize(textsize);
    leg->SetFillColor(kWhite);
    leg->SetLineColor(kWhite);
    leg->SetShadowColor(kWhite);

    if (h_BKG) leg->AddEntry(h_BKG,"SM total","L");
    for (size_t i=0;i<bkgHists.size();++i) if(bkgHists[i]) leg->AddEntry(bkgHists[i],m_Title[ExtractProcName(bkgHists[i]->GetName())].c_str(),"F");

    for (size_t i=0;i<sigHists.size();++i) {
        if(sigHists[i]) {
            std::string proc_title = sigHists[i]->GetName();
            if(proc_title.find("TChiWZ") != std::string::npos)
                proc_title = makeSMSChiTitle(ExtractProcName(proc_title));
            else
                proc_title = m_Title[ExtractProcName(proc_title)];
            std::string tmp_label = proc_title;
            if (signal_boost != 1.0) {
                std::ostringstream boost_str;
                boost_str << std::setprecision(3) << std::defaultfloat << signal_boost;
                tmp_label += " * " + boost_str.str();
            }
            leg->AddEntry(sigHists[i], tmp_label.c_str(), "L");
        }
    }
    if (h_DATA) leg->AddEntry(h_DATA,"Data","EP");
    if (h_DATA && h_BKG_ERR) leg->AddEntry(h_BKG_ERR, "Bkg unc.", "F");
    leg->Draw();

    TLatex l;
    l.SetNDC();
    l.SetTextSize(textsize);
    l.SetTextFont(42);
    
    // Dynamic positions based on margins
    double mleft  = can->GetLeftMargin();
    double mright = 1.0 - can->GetRightMargin();
    double mtop   = 1.0 - can->GetTopMargin();
    
    double xmin = mleft;
    double xmax = mright;
    double ytop = mtop + 0.01;
    
    l.SetTextAlign(11);
    l.DrawLatex(xmin, ytop, CMS_label.c_str());
    l.SetTextAlign(31);
    const std::string& plotTitle = (isFD_Stack && !groupTitle.empty())
                               ? groupTitle
                               : ExtractBinName(string(axisHist->GetName()));
    l.DrawLatex(xmax, ytop, plotTitle.c_str());

    // FitDiagnostics Labeling
    if(isFD_Stack) {
        // Recover bin labels from whichever histogram is actually drawn on
        // the label pad -> h_ratio when a ratio pad exists, axisHist otherwise.
        TH1* labelAxisHist = h_ratio ? h_ratio : axisHist;
        std::vector<std::string> currentSortedBins = RecoverBinLabels(labelAxisHist);
        TPad* labelPad = pad_ratio ? pad_ratio : (TPad*)can;
 
        // Enlarge bottom margin on the canvas to fit brackets.
        can->SetBottomMargin(hbo * 1.55);
        can->Modified();
        DrawBinAxisBrackets(labelPad, labelAxisHist, currentSortedBins, binLookup, risrNeighborMap, groupPattern);
    }

    if(outFile){ outFile->cd(); can->Write(0, TObject::kWriteDelete); }
    std::string BinName = SanitizeString(ExtractBinName(bkgHists[0]->GetName()));
    if(BinName.empty()) BinName = SanitizeString(hname);
    TString stackPdf = Form("%spdfs/%s/%s.pdf", outputDir.c_str(), BinName.c_str(), SanitizeString(canvas_name).c_str());
    gErrorIgnoreLevel = 1001; can->SaveAs(stackPdf); gErrorIgnoreLevel = 0;

    delete can;
    if(h_BKG)       delete h_BKG;
    if(h_DATA)      delete h_DATA;
    if(h_BKG_ERR)   delete h_BKG_ERR;
    if(h_ratio)     delete h_ratio;
    if(h_ratio_err) delete h_ratio_err;
}

void PlotMergedStack(const std::string&       mergedName,
                     const CombinedBinHists&  mergedHists,
                     const YamlBinPattern&    pattern,
                     const std::vector<std::string>& binNames,
                     const std::unordered_map<std::string, const YamlBinPattern*>& binLookup,
                     const std::map<std::string, std::set<int>>& risrNeighborMap,
                     const YamlBinPattern* groupPattern,
                     double signalBoost = 1.0)
{
    StackPlotInput stackInput = ConvertToStackInput(mergedHists);
    if (stackInput.bkgHists.empty() && stackInput.sigHists.empty() && !stackInput.dataHist) {
        std::cerr << "[warning] Nothing to plot for merged group: " << mergedName << std::endl;
        return;
    }

    // Build bracket tiers first so BuildGroupTitle can suppress what's already bracketed
    TH1* anyHist = !stackInput.bkgHists.empty() ? stackInput.bkgHists.front()
                 : stackInput.dataHist;
    std::vector<std::string> sortedBins = anyHist ? RecoverBinLabels(anyHist)
                                                   : binNames;
    BracketTierSet tiers = BuildBracketTiers(sortedBins, binLookup, risrNeighborMap, groupPattern);
    std::string groupTitle = BuildGroupTitle(pattern, tiers, binNames, binLookup);

    Plot_Stack(mergedName, stackInput.bkgHists, stackInput.sigHists,
               stackInput.dataHist, signalBoost, true, groupTitle, binLookup, risrNeighborMap, groupPattern);
}

void Plot_Overlay(const std::string& hname,
                  std::vector<TH1*>& bkgHists,
                  std::vector<TH1*>& sigHists,
                  TH1* dataHist = nullptr,
                  bool do_LogScale = false
                 )
{
    if (bkgHists.empty() && sigHists.empty() && !dataHist) return;

    // ------------------------
    // Normalize all histograms
    // ------------------------
    auto normalize = [](TH1* h) {
        if (!h) return;
        double integral = h->Integral();
        if (integral > 0) h->Scale(1.0 / integral);
    };

    for (auto* h : bkgHists) normalize(h);
    for (auto* h : sigHists) normalize(h);
    if (dataHist) normalize(dataHist);

    // Axis reference
    TH1* axisHist = nullptr;
    if (!bkgHists.empty()) axisHist = bkgHists.front();
    else if (!sigHists.empty()) axisHist = sigHists.front();
    else axisHist = dataHist;
    if (!axisHist) return;

    // ------------------------
    // Prepare canvas
    // ------------------------
    std::string canvas_name = "can_overlay_" + hname;
    TCanvas* can = new TCanvas(canvas_name.c_str(), canvas_name.c_str(), 1200, 700);
    can->SetGridx(); 
    can->SetGridy();
    can->SetLeftMargin(hlo);
    can->SetRightMargin(hhi);
    can->SetBottomMargin(hbo);
    can->SetTopMargin(hto);
    can->cd();

    // Draw empty axis
    //DrawLogSmart(axisHist, "HIST");
    axisHist->Draw("HIST");
    axisHist->SetLineWidth(2);
    axisHist->SetLineColor(kBlack);
    axisHist->GetYaxis()->SetTitle("Normalized events");
    axisHist->GetYaxis()->CenterTitle();
    axisHist->GetXaxis()->CenterTitle();
    axisHist->GetXaxis()->SetLabelSize(0.04);
    axisHist->GetYaxis()->SetLabelSize(0.04);
    axisHist->GetXaxis()->SetTitleSize(0.05);
    axisHist->GetYaxis()->SetTitleSize(0.05);
    axisHist->GetXaxis()->SetTitleOffset(1.1);
    axisHist->GetYaxis()->SetTitleOffset(0.94);

    // Compute Y max
    double ymax = axisHist->GetMaximum();
    for (auto* h : bkgHists) if (h) ymax = std::max(ymax, h->GetMaximum());
    for (auto* h : sigHists) if (h) ymax = std::max(ymax, h->GetMaximum());
    if (dataHist) ymax = std::max(ymax, dataHist->GetMaximum());
    axisHist->GetYaxis()->SetRangeUser(1e-4, 1.15 * ymax);

    // ------------------------
    // Draw backgrounds
    // ------------------------
    for (auto* h : bkgHists) {
        if (!h) continue;
        std::string proc = ExtractProcName(h->GetName());

        int color = kBlack;
        auto it = m_Color.find(proc);
        if (it != m_Color.end()) color = it->second;
        else {
            color = fallbackColors[fallbackIndex % fallbackColors.size()];
            m_Color[proc] = color;
            fallbackIndex++;
        }

        h->SetLineColor(color);
        h->SetLineWidth(3);
        h->SetFillColorAlpha(color, 0.4);
        if(!sigHists.empty()) h->SetLineStyle(7);
        else h->SetLineStyle(0);
        if(!sigHists.empty()) h->SetFillStyle(0);
        else h->SetFillStyle(3002);
        if(do_LogScale) DrawLogSmart(h, "SAME HIST");
        else h->Draw("SAME HIST");
    }

    // ------------------------
    // Draw signals
    // ------------------------
    for (auto* h : sigHists) {
        if (!h) continue;
        std::string proc = ExtractProcName(h->GetName());

        int color = kBlack;
        auto it = m_Color.find(proc);
        if (it != m_Color.end()) color = it->second;
        else {
            color = fallbackColors[fallbackIndex % fallbackColors.size()];
            m_Color[proc] = color;
            fallbackIndex++;
        }

        h->SetLineColor(color);
        h->SetLineWidth(3);
        h->SetFillColorAlpha(color, 0.4);
        h->SetFillStyle(3002);
        if(do_LogScale) DrawLogSmart(h, "SAME HIST");
        else h->Draw("SAME HIST");
    }

    // ------------------------
    // Draw data points
    // ------------------------
    if (dataHist) {
        dataHist->SetMarkerStyle(20);
        dataHist->SetMarkerSize(0.8);
        dataHist->SetLineColor(kBlack);
        if(do_LogScale) DrawLogSmart(dataHist, "SAME E");
        else dataHist->Draw("SAME HIST");
    }

    // ------------------------
    // Legend
    // ------------------------
    double textsize = 0.037;
    TLegend* leg = new TLegend(1.-hhi+0.01, 1.- (bkgHists.size()+sigHists.size()+2)*(1.-0.49)/9., 0.98, 1.-hto-0.005);
    leg->SetTextFont(132);
    leg->SetTextSize(textsize);
    leg->SetFillColor(kWhite);
    leg->SetLineColor(kWhite);

    for (auto* h : bkgHists)
        if (h && sigHists.empty())
            leg->AddEntry(h, m_Title[ExtractProcName(h->GetName())].c_str(), "LF");
        else
            leg->AddEntry(h, m_Title[ExtractProcName(h->GetName())].c_str(), "L");

    for (auto* h : sigHists)
        if (h) {
            std::string proc = ExtractProcName(h->GetName());
            if (proc.find("TChiWZ") != std::string::npos)
                proc = makeSMSChiTitle(proc);
            else
                proc = m_Title[proc];
            leg->AddEntry(h, proc.c_str(), "LF");
        }

    if (dataHist) leg->AddEntry(dataHist, "Data", "EP");
    leg->Draw();

    // CMS latex
    TLatex l;
    l.SetNDC();
    l.SetTextFont(42);
    l.SetTextSize(textsize);
    l.SetTextAlign(11);
    l.DrawLatex(can->GetLeftMargin(), 0.943, CMS_label.c_str());

    l.SetTextAlign(31);
    //l.DrawLatex(1.0 - can->GetRightMargin(), 0.943, ExtractBinName(axisHist->GetName()).c_str());

    // ------------------------
    // Save PDF
    // ------------------------
    if (outFile) { outFile->cd(); can->Write(0, TObject::kWriteDelete); }
    std::string BinName = SanitizeString(ExtractBinName(axisHist->GetName()));
    if(do_LogScale) canvas_name += "_log";
    TString pdf = Form("%spdfs/%s/%s.pdf",
                       outputDir.c_str(),
                       BinName.c_str(),
                       SanitizeString(canvas_name).c_str());
    gErrorIgnoreLevel = 1001;
    can->SaveAs(pdf);
    gErrorIgnoreLevel = 0;

    delete can;
}

// ----------------------
// Plot CutFlow
// ----------------------
void Plot_CutFlow(const std::string &hname,
                       const std::vector<TH1*> &bkgHists,
                       const std::vector<TH1*> &sigHists,
                       TH1* dataHist,
                       double signal_boost)
{
    if (bkgHists.empty() && (sigHists.empty() || !dataHist)) return;

    // Collect all hists for range finding
    vector<TH1*> allHists = bkgHists;
    allHists.insert(allHists.end(), sigHists.begin(), sigHists.end());
    if (dataHist) allHists.push_back(dataHist);

    double hmin, hmax;
    GetMinMaxIntegral(allHists, hmin, hmax);
    if (hmin <= 0.) hmin = 1.e-4;

    // Total background
    TH1D* h_BKG = nullptr;
    for (auto* h : bkgHists) {
        if (!h) continue;
        SetMinimumBinContent(h, 1.e-6);
        if (!h_BKG) h_BKG = (TH1D*) h->Clone("TOT_BKG");
        else h_BKG->Add(h);
    }

    // Clone data
    TH1D* h_DATA = nullptr;
    if (dataHist) h_DATA = (TH1D*) dataHist->Clone("TOT_DATA");

    // Canvas
    string canvas_name = "can_cutflow_" + hname;
    TCanvas* can = new TCanvas(canvas_name.c_str(), canvas_name.c_str(), 1200, 700);
    can->SetLeftMargin(hlo);
    can->SetRightMargin(hhi);
    can->SetBottomMargin(hbo);
    can->SetTopMargin(hto);
    can->SetGridx(); can->SetGridy();
    can->SetLogy();

    // Axis from first available hist
    TH1* axisHist = !allHists.empty() ? allHists.front() : nullptr;
    if (!axisHist) return;
    axisHist->Draw("");
    axisHist->GetYaxis()->SetRangeUser(max(0.8*hmin, 1.e-6), 1.2*hmax);
    axisHist->GetXaxis()->CenterTitle();
    axisHist->GetXaxis()->SetTitleFont(42);
    axisHist->GetXaxis()->SetTitleSize(0.05);
    axisHist->GetXaxis()->SetTitleOffset(1.0);
    axisHist->GetXaxis()->SetLabelFont(42);
    axisHist->GetXaxis()->SetLabelSize(0.04);
    axisHist->GetXaxis()->SetTickSize(0.);
    axisHist->GetYaxis()->CenterTitle();
    axisHist->GetYaxis()->SetTitleFont(42);
    axisHist->GetYaxis()->SetTitleSize(0.04);
    axisHist->GetYaxis()->SetTitleOffset(0.9);
    axisHist->GetYaxis()->SetLabelFont(42);
    axisHist->GetYaxis()->SetLabelSize(0.035);

    // Draw bkg
    for (size_t i = 0; i < bkgHists.size(); ++i) {
        TH1* h = bkgHists[i]; if (!h || h->GetEntries()==0) continue;
        h->SetLineColor(kBlack);
        h->SetLineWidth(2);
        int color = kBlack;
        auto it = m_Color.find(ExtractProcName(bkgHists[i]->GetName()));
        if (it != m_Color.end()) {
            color = it->second;
        } else {
            // Fallback to rotating palette
            color = fallbackColors[fallbackIndex % fallbackColors.size()];
            fallbackIndex++;
            m_Color[it->first] = color;
        }
        h->SetLineColor(color);
        h->SetMarkerColor(color);
        h->SetFillStyle(1001);
        h->Draw("SAME");
    }

    if (h_BKG) {
        h_BKG->SetLineWidth(3);
        h_BKG->SetLineColor(kRed);
        h_BKG->SetMarkerColor(kRed);
        h_BKG->Draw("SAME");
    }

    // Draw signals
    for (size_t i = 0; i < sigHists.size(); ++i) {
        TH1* h = sigHists[i]; if (!h || h->GetEntries()==0) continue;
        h->Scale(signal_boost);
        h->SetLineWidth(2);
        h->SetLineStyle(7);
        int color = kBlack;
        auto it = m_Color.find(ExtractProcName(sigHists[i]->GetName()));
        if (it != m_Color.end()) {
            color = it->second;
        } else {
            // Fallback to rotating palette
            color = fallbackColors[fallbackIndex % fallbackColors.size()];
            fallbackIndex++;
            m_Color[it->first] = color;
        }
        h->SetLineColor(color);
        h->SetMarkerColor(color);
        h->Draw("SAME");
    }

    // Data
    if (h_DATA) {
        h_DATA->SetMarkerStyle(20);
        h_DATA->SetMarkerSize(0.8);
        h_DATA->SetLineColor(kBlack);
        h_DATA->Draw("SAME E");
    }

    // Add Legend
    TLegend* leg = new TLegend(1.-hhi+0.01, 1.- (bkgHists.size()+sigHists.size()+2)*(1.-0.49)/9., 0.98, 1.-hto-0.005);
    leg->SetTextFont(132);
    leg->SetTextSize(0.039);
    leg->SetFillColor(kWhite);
    leg->SetLineColor(kWhite);
    leg->SetShadowColor(kWhite);
    if (h_BKG) leg->AddEntry(h_BKG,"SM total","L");
    for (size_t i=0;i<bkgHists.size();++i) if(bkgHists[i]) leg->AddEntry(bkgHists[i],m_Title[ExtractProcName(bkgHists[i]->GetName())].c_str(),"L");
    for (size_t i=0;i<sigHists.size();++i) {
        if(sigHists[i]) {
            std::string proc_title = sigHists[i]->GetName();
            if(proc_title.find("TChiWZ") != std::string::npos)
                proc_title = makeSMSChiTitle(ExtractProcName(proc_title));
            else
                proc_title = m_Title[ExtractProcName(proc_title)];
            std::string tmp_label = proc_title;
            if (signal_boost != 1.0) {
                std::ostringstream boost_str;
                boost_str << std::setprecision(3) << std::defaultfloat << signal_boost;
                tmp_label += " * " + boost_str.str();
            }
            leg->AddEntry(sigHists[i], tmp_label.c_str(), "L");
        }
    }
    if (h_DATA) leg->AddEntry(h_DATA,"Data","P");
    leg->Draw();

    TLatex l;
    l.SetNDC();
    l.SetTextSize(0.04);
    l.SetTextFont(42);
    l.DrawLatex(0.09,0.943,CMS_label.c_str());
    l.DrawLatex(0.69,0.943,ExtractBinName(string(axisHist->GetName())).c_str());

    // Save
    if(outFile){ outFile->cd(); can->Write(0, TObject::kWriteDelete); }
    TString pdfOut = Form("%spdfs/%s/%s.pdf", outputDir.c_str(), ExtractBinName(string(axisHist->GetName())).c_str(), hname.c_str());
    gErrorIgnoreLevel = 1001;
    can->SaveAs(pdfOut);
    gErrorIgnoreLevel = 0;

    delete can;
    if(h_BKG) delete h_BKG;
    if(h_DATA) delete h_DATA;
}

// ----------------------
// Plot TEff Multigraph
// groupType should be "Bin" or "Process"
// ----------------------
void Plot_Eff_Multi(const std::string& groupName,
                    const std::vector<TEfficiency*>& effs,
                    const std::string& groupType
                   )
{
    if(effs.empty()) return;

    // Canvas
    TCanvas* can = new TCanvas(("can_multi_"+groupName).c_str(), ("can_"+groupName).c_str(), 1200, 700);
    can->SetLeftMargin(hlo+0.025);
    can->SetRightMargin(hhi);
    can->SetBottomMargin(hbo);
    can->SetTopMargin(hto);
    can->SetGridx(); can->SetGridy();
    can->Draw();
    can->cd();

    double ymin = 1e6, ymax = -1e6;
    for(auto* e : effs){
        if(!e) continue;
    
        // check for effective points
        auto* htot = e->GetTotalHistogram();
        bool hasPoints = false;
        for(int i = 1; i <= htot->GetNbinsX(); ++i){
            if(htot->GetBinContent(i) > 0){ hasPoints = true; break; }
        }
        if(!hasPoints){
            std::cerr << "[Warning] Skipping TEff with no valid points: " << e->GetName() << "\n";
            continue;
        }
    
        // create TGraph without painting
        TGraphAsymmErrors* gr = e->CreateGraph();
        if(!gr) continue;
    
        const int n = gr->GetN();
        for(int i = 0; i < n; ++i){
            double x, y;
            gr->GetPoint(i, x, y);
            double ylow = y - gr->GetErrorYlow(i);
            double yhigh = y + gr->GetErrorYhigh(i);
            ymin = std::min(ymin, ylow);
            ymax = std::max(ymax, yhigh);
        }
    }
    if(ymin < 0.) ymin = 0.;
    ymax *= 1.05;

    // Add Legend
    TLegend* leg = new TLegend(1.-hhi+0.01, 1.- (effs.size()+2)*(1.-0.49)/9., 0.98, 1.-hto-0.005);
    leg->SetTextFont(132);
    leg->SetTextSize(0.039);
    leg->SetFillColor(kWhite);
    leg->SetLineColor(kWhite);
    leg->SetShadowColor(kWhite);

    TMultiGraph* mg = new TMultiGraph();

    for(size_t i = 0; i < effs.size(); ++i){
        TEfficiency* e = effs[i];
        if(!e) continue;
    
        // Create TGraph safely
        TGraphAsymmErrors* gr = nullptr;
        try {
            gr = (TGraphAsymmErrors*)e->CreateGraph(); // safer than Draw() + GetPaintedGraph()
        } catch (...) {
            std::cerr << "[Warning] Failed to create graph for TEff: " << e->GetName() << "\n";
            continue;
        }
    
        if(!gr || gr->GetN() == 0){
            continue;
        }
    
        // Parse name to get bin/proc
        HistId id = ParseHistName(e->GetName());
        std::string legendKey, legendTitle;
        int color = kBlack;
        
        if (groupType == "Bin") {
            legendKey = id.proc.empty() ? "unknown_proc" : id.proc;
            legendTitle = id.proc;
            if(legendTitle.find("TChiWZ") != std::string::npos)
                legendTitle = makeSMSChiTitle(legendTitle);
            else
                legendTitle = m_Title[legendTitle];
        } else {
            legendKey = id.bin.empty() ? "unknown_bin" : id.bin;
            legendTitle = m_Title.count(legendKey) ? m_Title[legendKey] : legendKey;
        }
        
        // Try to get color from map
        auto it = m_Color.find(legendKey);
        if (it != m_Color.end()) {
            color = it->second;
        } else {
            // Fallback to rotating palette
            color = fallbackColors[fallbackIndex % fallbackColors.size()];
            fallbackIndex++;
            m_Color[it->first] = color;
        }

        gr->SetMarkerStyle(20);
        gr->SetMarkerColor(color);
        gr->SetLineColor(color);
    
        mg->Add(gr);
        leg->AddEntry(gr, legendTitle.c_str(), "PL");
    }

    // Draw multi-graph and style axes
    mg->Draw("AP");
    mg->GetXaxis()->CenterTitle();
    mg->GetXaxis()->SetTitleFont(42);
    mg->GetXaxis()->SetTitleSize(0.06);
    mg->GetXaxis()->SetTitleOffset(1.05);
    mg->GetXaxis()->SetLabelFont(42);
    mg->GetXaxis()->SetLabelSize(0.05);
    mg->GetYaxis()->CenterTitle();
    mg->GetYaxis()->SetTitleFont(42);
    mg->GetYaxis()->SetTitleSize(0.06);
    mg->GetYaxis()->SetTitleOffset(1.01);
    mg->GetYaxis()->SetLabelFont(42);
    mg->GetYaxis()->SetLabelSize(0.05);
    mg->GetYaxis()->SetRangeUser(ymin, ymax);
    mg->GetXaxis()->SetTitle(effs[0]->GetPaintedGraph()->GetXaxis()->GetTitle());
    mg->GetYaxis()->SetTitle(effs[0]->GetPaintedGraph()->GetYaxis()->GetTitle());

    // Draw legend
    leg->Draw();

    // TLatex: CMS on top-left; group info on top-right
    TLatex l; l.SetNDC(); l.SetTextFont(42);
    l.SetTextSize(0.04);
    l.DrawLatex(0.12, 0.943, CMS_label.c_str());

    // top-right: show what this group is (Bin or Process)
    std::string topRight;
    if(groupType == "Bin") topRight = groupName;
    else topRight = m_Title[groupName];
    l.SetTextSize(0.045);
    l.DrawLatex(0.69, 0.943, topRight.c_str());
    string varName = ParseHistName(effs[0]->GetName()).var;

    // Save
    TString dirName = Form("%spdfs/%s", outputDir.c_str(), groupName.c_str());
    gSystem->MakeDirectory(dirName);
    TString pdfName = Form("%s/%s_%s.pdf", dirName.Data(), groupName.c_str(), varName.c_str());
    gErrorIgnoreLevel = 1001; can->SaveAs(pdfName); gErrorIgnoreLevel = 0;
    if(outFile){ outFile->cd(); can->Write(0, TObject::kWriteDelete); }

    // Cleanup
    delete mg;
    delete leg;
    delete can;
}

// ----------------------
// Plot 2D scatter overlay
// ----------------------
void Plot_Hist2DScatter(const std::string& hname,
                        const std::vector<TH2*>& bkgHists,
                        const std::vector<TH2*>& sigHists)
{
    if (bkgHists.empty() && sigHists.empty()) return;

    TH2* axisHist = !bkgHists.empty() ? bkgHists.front()
                   : !sigHists.empty() ? sigHists.front()
                   : nullptr;
    if (!axisHist) return;

    std::string canvas_name = "can_scatter_" + hname;
    TCanvas* can = new TCanvas(canvas_name.c_str(), canvas_name.c_str(), 1200, 700);
    can->SetLeftMargin(hlo);
    can->SetRightMargin(hhi);
    can->SetBottomMargin(hbo);
    can->SetTopMargin(hto);
    can->SetGridx();
    can->SetGridy();
    can->cd();

    // Axis frame
    axisHist->SetStats(0);
    axisHist->Draw("AXIS");
    axisHist->GetXaxis()->CenterTitle();
    axisHist->GetYaxis()->CenterTitle();
    axisHist->GetXaxis()->SetTitleOffset(1.05);
    axisHist->GetYaxis()->SetTitleOffset(1.05);

    // Infer a sensible range from all points
    double xmin = axisHist->GetXaxis()->GetXmin();
    double xmax = axisHist->GetXaxis()->GetXmax();
    double ymin = axisHist->GetYaxis()->GetXmin();
    double ymax = axisHist->GetYaxis()->GetXmax();

    auto updateRange = [&](const TH2* h) {
        if (!h) return;
        xmin = std::min(xmin, h->GetXaxis()->GetXmin());
        xmax = std::max(xmax, h->GetXaxis()->GetXmax());
        ymin = std::min(ymin, h->GetYaxis()->GetXmin());
        ymax = std::max(ymax, h->GetYaxis()->GetXmax());
    };
    for (auto* h : bkgHists) updateRange(h);
    for (auto* h : sigHists) updateRange(h);

    axisHist->GetXaxis()->SetRangeUser(xmin, xmax);
    axisHist->GetYaxis()->SetRangeUser(ymin, ymax);
    axisHist->GetZaxis()->SetTitle(("N_{events} / " + std::to_string(int(lumi)) + " fb^{-1}").c_str());

    TLegend* leg = new TLegend(1.-hhi+0.01, 1.- (bkgHists.size()+sigHists.size()+2)*(1.-0.49)/9., 0.98, 1.-hto-0.005);
    leg->SetTextFont(132);
    leg->SetTextSize(0.038);
    leg->SetFillColor(kWhite);
    leg->SetLineColor(kWhite);
    leg->SetShadowColor(kWhite);

    std::vector<TGraph*> ownedGraphs;

    auto drawOne = [&](TH2* h, bool isSignal) {
        if (!h) return;

        std::string proc = ExtractProcName(h->GetName());
        int color = kBlack;
        auto it = m_Color.find(proc);
        if (it != m_Color.end()) color = it->second;
        else {
            color = fallbackColors[fallbackIndex % fallbackColors.size()];
            m_Color[proc] = color;
            fallbackIndex++;
        }

        TGraph* g = TH2ToScatterGraph(h, std::string(h->GetName()) + "_gr");
        if (!g || g->GetN() == 0) {
            delete g;
            return;
        }

        g->SetMarkerColor(color);
        g->SetLineColor(color);
        g->SetMarkerSize(0.9);
        g->SetMarkerStyle(isSignal ? 20 : 22);  // dots for sig, squares for bkg

        g->Draw("P SAME");
        ownedGraphs.push_back(g);

        std::string label = m_Title.count(proc) ? m_Title[proc] : proc;
        if (proc.find("TChiWZ") != std::string::npos)
            label = makeSMSChiTitle(proc);

        leg->AddEntry(g, label.c_str(), "P");
    };

    for (auto* h : bkgHists) drawOne(h, false);
    for (auto* h : sigHists) drawOne(h, true);

    leg->Draw();

    TLatex l;
    l.SetNDC();
    l.SetTextFont(42);
    l.SetTextSize(0.04);
    l.DrawLatex(0.12, 0.943, CMS_label.c_str());

    l.SetTextSize(0.045);
    l.DrawLatex(0.69, 0.943, ExtractBinName(axisHist->GetName()).c_str());

    if (outFile) {
        outFile->cd();
        can->Write(0, TObject::kWriteDelete);
    }

    TString pdfName = Form("%spdfs/%s/%s.pdf",
                           outputDir.c_str(),
                           ExtractBinName(axisHist->GetName()).c_str(),
                           SanitizeString(canvas_name).c_str());
    gErrorIgnoreLevel = 1001;
    can->SaveAs(pdfName);
    gErrorIgnoreLevel = 0;

    for (auto* g : ownedGraphs) delete g;
    delete leg;
    delete can;
}

// ----------------------
// Plot 2D contour overlay
// ----------------------
void Plot_Hist2DContour(const std::string& hname,
                        const std::vector<TH2*>& bkgHists,
                        const std::vector<TH2*>& sigHists)
{
    if (bkgHists.empty() && sigHists.empty()) return;

    TH2* axisHist = !bkgHists.empty() ? bkgHists.front()
                   : !sigHists.empty() ? sigHists.front()
                   : nullptr;
    if (!axisHist) return;

    std::string canvas_name = "can_contour_" + hname;
    TCanvas* can = new TCanvas(canvas_name.c_str(), canvas_name.c_str(), 1200, 700);
    can->SetLeftMargin(hlo);
    can->SetRightMargin(hhi);
    can->SetBottomMargin(hbo);
    can->SetTopMargin(hto);
    can->SetGridx();
    can->SetGridy();
    can->cd();

    // Draw axis only
    axisHist->SetStats(0);
    axisHist->Draw("AXIS");
    axisHist->GetXaxis()->CenterTitle();
    axisHist->GetYaxis()->CenterTitle();
    axisHist->GetXaxis()->SetTitleSize(0.05);
    axisHist->GetYaxis()->SetTitleSize(0.05);
    axisHist->GetXaxis()->SetLabelSize(0.05);
    axisHist->GetYaxis()->SetLabelSize(0.05);
    axisHist->GetYaxis()->SetTitleOffset(0.92);

    // Legend
    TLegend* leg = new TLegend(1.-hhi+0.01, 1.- (bkgHists.size()+sigHists.size()+2)*(1.-0.49)/9., 0.98, 1.-hto-0.005);
    leg->SetTextFont(132);
    leg->SetTextSize(0.038);
    leg->SetFillColor(kWhite);
    leg->SetLineColor(kWhite);

    auto drawOne = [&](TH2* h, bool isSignal) {
        if (!h) return;
    
        std::string proc = ExtractProcName(h->GetName());
    
        int color = kBlack;
        auto it = m_Color.find(proc);
        if (it != m_Color.end()) color = it->second;
        else {
            color = fallbackColors[fallbackIndex % fallbackColors.size()];
            m_Color[proc] = color;
            fallbackIndex++;
        }
    
        // --- CLONE to avoid ROOT weirdness ---
        TH2* hc = (TH2*)h->Clone((std::string(h->GetName())+"_cont").c_str());
        hc->SetDirectory(0);
    
        // --- shape prep ---
        hc->Smooth(1);
        if (hc->Integral() > 0)
            hc->Scale(1.0 / hc->Integral());
    
        // --- contour definition AFTER scaling ---
        double level = 0.2175 * hc->GetMaximum();
        hc->SetLineWidth(3);
        hc->SetLineStyle(1);
        hc->SetLineColor(color);
        hc->SetContour(1, &level);
    
        // --- Filled contour ---
        hc->SetFillColorAlpha(color, 0.25);
        hc->Draw("CONT0 SAME");
        hc->Draw("CONT LIST SAME");
        
        gPad->Modified();
        gPad->Update();
        
        // Get contours
        TObjArray* contours = (TObjArray*)gROOT->GetListOfSpecials()->FindObject("contours");
        if (!contours || contours->GetSize() == 0) return;
        
        TList* contLevel = (TList*)contours->At(0);  // only 1 contour level
        if (!contLevel || contLevel->GetSize() == 0) return;

        std::vector<TGraph*> safeGraphs;
        for (int i = 0; i < contLevel->GetSize(); ++i) {
            TGraph* gr = (TGraph*)contLevel->At(i);
            if (!gr) continue;
            safeGraphs.push_back((TGraph*)gr->Clone());
        }
                
        for (auto* gr_clone : safeGraphs) {
            gr_clone->SetLineColor(color);
            gr_clone->SetLineWidth(3);
            gr_clone->Draw("L SAME");
        }
        gPad->Modified();
        gPad->Update();
    
        // legend
        std::string label = m_Title.count(proc) ? m_Title[proc] : proc;
        if (proc.find("TChiWZ") != std::string::npos)
            label = makeSMSChiTitle(proc);
    
        leg->AddEntry(hc, label.c_str(), "F");
    };

    // Draw backgrounds first
    for (auto* h : bkgHists) drawOne(h, false);

    // Then signals on top
    for (auto* h : sigHists) drawOne(h, true);

    leg->Draw();

    // CMS label
    TLatex l;
    l.SetNDC();
    l.SetTextFont(42);
    l.SetTextSize(0.04);
    l.DrawLatex(0.1, 0.943, CMS_label.c_str());

    l.SetTextSize(0.045);
    //l.DrawLatex(0.69, 0.943, ExtractBinName(axisHist->GetName()).c_str());

    // Save
    if (outFile) {
        outFile->cd();
        can->Write(0, TObject::kWriteDelete);
    }

    TString pdfName = Form("%spdfs/%s/%s.pdf",
                           outputDir.c_str(),
                           ExtractBinName(axisHist->GetName()).c_str(),
                           SanitizeString(canvas_name).c_str());

    gErrorIgnoreLevel = 1001;
    can->SaveAs(pdfName);
    gErrorIgnoreLevel = 0;

    delete leg;
    delete can;
}

// RunRatios: create TEfficiencies for efficiency-style ratios and queue generic ratios.
// This function fills effsByBin and effsByProcess with created TEff objects.
// For generic (non-efficiency) ratios it calls Plot_Ratio
void RunRatios(const std::vector<RatioDef>& ratioDefs,
               const std::map<std::string, std::map<std::string, TH1*>>& groups,
               const std::set<std::string>& uniqueBinNames,
               const std::string& outputDir,
               std::map<std::string, std::vector<TEfficiency*>> &effsByBin,
               std::map<std::string, std::vector<TEfficiency*>> &effsByProcess)
{
    for(const auto &r : ratioDefs) {
        // If explicit map mode
        if(!r.map.empty()){
            if(r.kind == RatioKind::Efficiency){
                // Try to create TEffs for each explicit map entry if possible
                for(const auto &p : r.map){
                    // find all matches for numerator and denominator
                    auto numMatches = FindMatchingHists(p.numerator, groups, uniqueBinNames, false);
                    auto denMatches = FindMatchingHists(p.denominator, groups, uniqueBinNames, false);

                    // only create TEff for any exact (bin,proc) pairs that match both numerator and denominator
                    for(const auto &nm : numMatches){
                        for(const auto &dm : denMatches){
                            if(nm.bin == dm.bin && nm.proc == dm.proc){
                                TH1* hnum = nm.hist;
                                TH1* hden = dm.hist;
                                if(!hnum || !hden) continue;
                                if(!HistsCompatible(hnum, hden)) {
                                    std::cerr << "[RunRatios] Hists incompatible for TEff: " << r.name 
                                              << " bin="<<nm.bin<<" proc="<<nm.proc<<std::endl;
                                    continue;
                                }
                                TEfficiency* eff = nullptr;
                                gErrorIgnoreLevel = 1001;
                                eff = new TEfficiency(*dynamic_cast<TH1*>(hnum), *dynamic_cast<TH1*>(hden));
                                gErrorIgnoreLevel = 0;
                                // Set a descriptive name
                                std::string effName = r.name + "__" + (nm.bin.empty() ? "inclusive" : nm.bin) + "__" + nm.proc;
                                eff->SetName(effName.c_str());
                                eff->SetTitle(effName.c_str());
                                // draw to make painted graph before adjusting titles
                                gErrorIgnoreLevel = 3001;
                                TCanvas* dum_canv = new TCanvas("dum_canv", "dum_canv", 750, 500);
                                dum_canv->cd();
                                eff->Draw();
                                dum_canv->Update();
                                // Copy y-axis title from numerator if available
                                eff->GetPaintedGraph()->GetYaxis()->SetTitle(hnum->GetYaxis()->GetTitle());
                                delete dum_canv;
                                gErrorIgnoreLevel = 0;
                                effsByBin[nm.bin].push_back(eff);
                                effsByProcess[nm.proc].push_back(eff);
                                // Plot it immediately using helper
                                Plot_Eff(eff);
                            }
                        }
                    }
                }
            } else {
                for(const auto &p : r.map){
                    auto numMatches = FindMatchingHists(p.numerator, groups, uniqueBinNames);
                    auto denMatches = FindMatchingHists(p.denominator, groups, uniqueBinNames);
                    for(const auto &nm : numMatches){
                        for(const auto &dm : denMatches){
                            TH1* hratio = MakeRatioHist(nm.hist, dm.hist, r.normalize);
                            if(!hratio) continue;
                            std::string hname = r.name;
                            hratio->SetName(hname.c_str());
                            hratio->SetTitle(hname.c_str());
                            Plot_Ratio(hratio, outputDir, &r);
                            delete hratio; // avoid memory leak
                        }
                    }
                }
            }
            continue; // process next RatioDef
        }

        // --- Implicit efficiency mode ---
        if(!r.numerator_var.empty() && !r.denominator_var.empty() && r.kind == RatioKind::Efficiency){
        
            // Determine bins to loop — empty bins list means use all discovered bins
            std::vector<std::string> binsToUse;
            if(!r.bins.empty()){
                binsToUse = r.bins;
            } else {
                binsToUse.assign(uniqueBinNames.begin(), uniqueBinNames.end());
                if(binsToUse.empty()) binsToUse.push_back("");
            }
        
            for(const auto& bin : binsToUse){
        
                // Find the numerator group for this bin
                std::string gk_num = MakeGroupKeyForVar(bin, r.numerator_var);
                std::string gk_den = MakeGroupKeyForVar(bin, r.denominator_var);
        
                auto it_num = groups.find(gk_num);
                auto it_den = groups.find(gk_den);
        
                if(it_num == groups.end()){
                    std::cerr << "[RunRatios] No numerator group found for bin='"
                              << bin << "' var='" << r.numerator_var << "'" << std::endl;
                    continue;
                }
                if(it_den == groups.end()){
                    std::cerr << "[RunRatios] No denominator group found for bin='"
                              << bin << "' var='" << r.denominator_var << "'" << std::endl;
                    continue;
                }
        
                // Determine processes — empty means all processes present in numerator group
                std::vector<std::string> procsToUse;
                if(!r.processes.empty()){
                    procsToUse = r.processes;
                } else {
                    for(const auto& pp : it_num->second)
                        procsToUse.push_back(pp.first);
                }
        
                for(const auto& proc : procsToUse){
        
                    // Strict same-bin same-process lookup — no cross matching
                    auto it_hnum = it_num->second.find(proc);
                    auto it_hden = it_den->second.find(proc);
        
                    if(it_hnum == it_num->second.end()){
                        std::cerr << "[RunRatios] Numerator missing for efficiency '" << r.name
                                  << "' bin='" << bin << "' proc='" << proc << "'" << std::endl;
                        continue;
                    }
                    if(it_hden == it_den->second.end()){
                        std::cerr << "[RunRatios] Denominator missing for efficiency '" << r.name
                                  << "' bin='" << bin << "' proc='" << proc << "'" << std::endl;
                        continue;
                    }
        
                    TH1* hnum = it_hnum->second;
                    TH1* hden = it_hden->second;
        
                    if(!hnum || !hden) continue;
        
                    if(!HistsCompatible(hnum, hden)){
                        std::cerr << "[RunRatios] Incompatible hists for efficiency '" << r.name
                                  << "' bin='" << bin << "' proc='" << proc << "'" << std::endl;
                        continue;
                    }
        
                    // Build TEfficiency — numerator must be subset of denominator,
                    // ROOT will warn but we suppress and check manually
                    gErrorIgnoreLevel = kWarning;
                    TEfficiency* eff = new TEfficiency(
                        *dynamic_cast<TH1*>(hnum),
                        *dynamic_cast<TH1*>(hden)
                    );
                    gErrorIgnoreLevel = kPrint;
        
                    if(!eff){
                        std::cerr << "[RunRatios] TEfficiency construction failed for '"
                                  << r.name << "' bin='" << bin << "' proc='" << proc << "'" << std::endl;
                        continue;
                    }
        
                    // Name mirrors the canvas naming convention used by Trigger_SFs
                    // so downstream code can discover these by scanning the root file:
                    // {ratio_name}__{bin}__{proc}
                    std::string effName = r.name
                                        + "__" + (bin.empty() ? "inclusive" : bin)
                                        + "__" + proc;
                    eff->SetName(effName.c_str());
                    eff->SetTitle(effName.c_str());
        
                    // Paint once into a temporary canvas to populate GetPaintedGraph()
                    // and set axis titles from the numerator histogram
                    {
                        gErrorIgnoreLevel = kError;
                        TCanvas* tmp = new TCanvas("tmp_eff_paint","tmp_eff_paint",750,500);
                        tmp->cd();
                        eff->Draw("AP");
                        tmp->Update();
                        if(eff->GetPaintedGraph()){
                            eff->GetPaintedGraph()->GetXaxis()->SetTitle(
                                hnum->GetXaxis()->GetTitle());
                            eff->GetPaintedGraph()->GetYaxis()->SetTitle(
                                hnum->GetYaxis()->GetTitle());
                        }
                        delete tmp;
                        gErrorIgnoreLevel = kPrint;
                    }
        
                    effsByBin[bin].push_back(eff);
                    effsByProcess[proc].push_back(eff);
                    Plot_Eff(eff);
        
                } // end proc loop
            } // end bin loop
            continue;
        } // end implicit efficiency mode

        // Non-efficiency implicit mode
        if(!r.numerator_var.empty() && !r.denominator_var.empty()){
            // loop over bins
            std::vector<std::string> binsToUse;
            if(!r.bins.empty()) binsToUse = r.bins;
            else binsToUse.assign(uniqueBinNames.begin(), uniqueBinNames.end());
            if(binsToUse.empty()) binsToUse.push_back("");
            
            for(const auto &bin : binsToUse){
                std::vector<std::string> procsToUse;
                if(!r.processes.empty()) procsToUse = r.processes;
                else {
                    // all processes from numerator group
                    std::string gknum = MakeGroupKeyForVar(bin, r.numerator_var);
                    auto itg = groups.find(gknum);
                    if(itg!=groups.end())
                        for(const auto &pp : itg->second) procsToUse.push_back(pp.first);
                }
            
                for(const auto &proc : procsToUse){
                    std::string gknum = MakeGroupKeyForVar(bin, r.numerator_var);
                    std::string gkden = MakeGroupKeyForVar(bin, r.denominator_var);
                    TH1* hnum = nullptr;
                    TH1* hden = nullptr;
                    auto itgnum = groups.find(gknum);
                    if(itgnum != groups.end()){
                        auto itp = itgnum->second.find(proc);
                        if(itp!=itgnum->second.end()) hnum = itp->second;
                    }
                    auto itgden = groups.find(gkden);
                    if(itgden != groups.end()){
                        auto itp = itgden->second.find(proc);
                        if(itp!=itgden->second.end()) hden = itp->second;
                    }
                    if(!hnum || !hden) continue;
                    TH1* hratio = MakeRatioHist(hnum,hden,r.normalize);
                    if(!hratio) continue;
                    std::string hname = r.name + "__" + (bin.empty()?"inclusive":bin) + "__" + proc;
                    hratio->SetName(hname.c_str());
                    hratio->SetTitle(hname.c_str());
                    Plot_Ratio(hratio, outputDir, &r);
                    delete hratio;
                }
            }
        }
    } // end ratioDefs loop
}

void Plot_EventCount2D(TH2* h, const std::string &mode,
                       double zmin_override = std::numeric_limits<double>::quiet_NaN(),
                       double zmax_override = std::numeric_limits<double>::quiet_NaN()) {
    if(!h) return;
    bool include_err = false;

    // Locate Total Bkg row
    int totalRow = -1;
    for(int j=1; j<=h->GetNbinsY(); ++j){
      TString lab = h->GetYaxis()->GetBinLabel(j);
      if(lab.Contains("Total Bkg")) { totalRow = j; break; }
    }

    // Determine Z-axis range
    double use_zmin = std::numeric_limits<double>::quiet_NaN();
    double use_zmax = std::numeric_limits<double>::quiet_NaN();
    if(std::isfinite(zmin_override) && std::isfinite(zmax_override) && zmax_override > zmin_override){
      use_zmin = zmin_override;
      use_zmax = zmax_override;
    } else if(mode=="Zbi"){
      double zmin=1e300, zmax=-1e300;
      for(int iy=1; iy<=h->GetNbinsY(); ++iy){
        if(iy==totalRow) continue; // ignore Total Bkg
        for(int ix=1; ix<=h->GetNbinsX(); ++ix){
          double v = h->GetBinContent(ix, iy);
          if(!std::isfinite(v) || v<=0) continue;
          if(v<zmin) zmin=v;
          if(v>zmax) zmax=v;
        }
      }
      if(zmax>=zmin && zmin<1e299){
        use_zmin = std::max(0.0, 0.9*zmin);
        use_zmax = 1.1*zmax;
        if(use_zmax<=use_zmin) use_zmax = use_zmin + 1e-6;
      }
    }

    if(std::isfinite(use_zmin) && std::isfinite(use_zmax))
      h->GetZaxis()->SetRangeUser(use_zmin, use_zmax);

    // Canvas setup
    TCanvas* can = new TCanvas(("can_"+std::string(h->GetName())).c_str(), "", 1200, 700);
    can->SetLeftMargin(0.13);
    can->SetRightMargin(0.13);
    can->SetBottomMargin(0.17);
    can->SetTopMargin(0.06);
    can->SetGridx(); can->SetGridy();
    if(mode=="raw" || mode=="yield" || mode=="effective") can->SetLogz(); // <--- enable log for effective as well

    // Draw histogram COLZ
    h->Draw("COLZ");
    gPad->Update();

    // Prepare TLatex for numbers
    TLatex tex;
    tex.SetTextFont(42);
    tex.SetTextAlign(22);
    float textSize = 0.04;
    if (h->GetNbinsX() > 10) textSize = textSize - 0.0016f * (h->GetNbinsX()- 10);
    if (textSize < 0.01) textSize = 0.01;
    tex.SetTextSize(textSize);

    // Draw numbers
    for(int iy=1; iy<=h->GetNbinsY(); ++iy){
      for(int ix=1; ix<=h->GetNbinsX(); ++ix){
        double val = h->GetBinContent(ix, iy);
        double val_err = h->GetBinError(ix, iy);
        double xlow = h->GetXaxis()->GetBinLowEdge(ix);
        double xup  = h->GetXaxis()->GetBinUpEdge(ix);
        double ylow = h->GetYaxis()->GetBinLowEdge(iy);
        double yup  = h->GetYaxis()->GetBinUpEdge(iy);
        double xc = 0.5*(xlow+xup);
        double yc = 0.5*(ylow+yup);
        if(val < 1.e-5 && (mode != "yield" || mode != "raw"))
            val = 0.;
        TString label = "";
        if(val < 1)
            label = Form("%.2g", val);
        else
            label = Form("%.3g", val);
        if(mode == "yield" && include_err){
            if(val < 1)
                label += Form(" #pm %.2g", val_err);
            else
                label += Form(" #pm %.3g", val_err);
        }

        if(mode=="Zbi" && iy==totalRow){
          // Draw white box behind Total Bkg text
          double xpad = 0.003*(xup-xlow);
          double ypad = 0.006*(yup-ylow);
          TBox* box = new TBox(xlow+xpad, ylow+ypad, xup-xpad, yup-ypad);
          box->SetFillColor(kWhite);
          box->SetLineColor(kBlack);
          box->SetFillStyle(1001);
          box->Draw("F same");
          tex.SetTextColor(kBlack);
        } else {
          tex.SetTextColor(kRed);
        }

        tex.DrawLatex(xc, yc, label);
      }
    }

    gPad->Update(); // force redraw

    // Re-apply axis labels & formatting
    h->GetXaxis()->CenterTitle();
    h->GetXaxis()->SetTitleFont(42); h->GetXaxis()->SetTitleSize(0.06); h->GetXaxis()->SetTitleOffset(1.06);
    h->GetXaxis()->SetLabelFont(42);

    float XLabelSize = 0.039;
    if (h->GetNbinsX() > 5) XLabelSize = XLabelSize - 0.001f * (h->GetNbinsX() - 5);
    if (XLabelSize < 0.015) XLabelSize = 0.015;
    h->GetXaxis()->SetLabelSize(XLabelSize);
    h->GetYaxis()->CenterTitle();
    h->GetYaxis()->SetTitleFont(42); h->GetYaxis()->SetTitleSize(0.06); h->GetYaxis()->SetTitleOffset(1.1);
    h->GetYaxis()->SetLabelFont(42); h->GetYaxis()->SetLabelSize(0.035);
    h->GetZaxis()->CenterTitle();
    h->GetZaxis()->SetTitleFont(42); h->GetZaxis()->SetTitleSize(0.03); h->GetZaxis()->SetTitleOffset(1.03);
    h->GetZaxis()->SetLabelFont(42); h->GetZaxis()->SetLabelSize(0.03);

    // Z-axis title
    if(mode == "yield")
      h->GetZaxis()->SetTitle(("N_{events} passing category scaled to "+std::to_string(lumi)+" fb^{-1}").c_str());
    else if(mode == "raw")
      h->GetZaxis()->SetTitle("N_{events} passing category");
    else if(mode == "SoB")
      h->GetZaxis()->SetTitle(("#frac{N_{events}}{N_{TOT BKG}} for process in category scaled to "+std::to_string(lumi)+" fb^{-1}").c_str());
    else if(mode == "SoverSqrtB")
      h->GetZaxis()->SetTitle(("#frac{N_{events}}{#sqrt{N_{TOT BKG}}} for process in category scaled to "+std::to_string(lumi)+" fb^{-1}").c_str());
    else if(mode == "Zbi")
      h->GetZaxis()->SetTitle("Z_{bi} for signal in category");
    else if(mode == "effective")
      h->GetZaxis()->SetTitle(("Effective yield = yield^{2}/err^{2} for process in category scaled to "+std::to_string(lumi)+" fb^{-1}").c_str());
    else
      h->GetZaxis()->SetTitle("Yield");

    TLatex l; l.SetNDC(); l.SetTextFont(42);
    l.SetTextSize(0.04);
    l.DrawLatex(0.13, 0.947, CMS_label.c_str());

    // Save canvas
    TString pdfName = Form("%s/pdfs/%s.pdf", outputDir.c_str(), h->GetName());
    can->Write();
    gErrorIgnoreLevel = 1001; can->SaveAs(pdfName); gErrorIgnoreLevel = 0;

    delete can;
    delete h;
}

void MakeAndPlotCutflow2D(
    const std::map<std::string, std::map<std::string, TH1*>> &cutflowMap,
    const std::string &groupKey,
    const std::string &mode = "yield",
    double Zbi_unc = 0.0)
{
    if(cutflowMap.empty()) return;

    // --- 1) collect list of bins (X axis) ---
    std::vector<std::string> bins;
    bins.reserve(cutflowMap.size());
    for (const auto &bp : cutflowMap) bins.push_back(bp.first);

    int nx = (int)bins.size();

    // --- 2) collect set of all processes across bins ---
    std::set<std::string> procSet;
    for (const auto &bp : cutflowMap) {
        for (const auto &pp : bp.second) {
            const std::string &pname = pp.first;
            procSet.insert(pname);
        }
    }

    // --- 3) classify processes into bkg / sig (use tool) ---
    std::vector<std::string> allBkgs, allSigs, allData;
    for (const auto &p : procSet) {
        if (tool.BkgDict.count(p)) allBkgs.push_back(p);
        else if (std::find(tool.SignalKeys.begin(), tool.SignalKeys.end(), p) != tool.SignalKeys.end()
                 || p.find("SMS") != std::string::npos || p.find("Cascades") != std::string::npos)
            allSigs.push_back(p);
        else if (p.find("data") != std::string::npos || p.find("Data") != std::string::npos)
            allData.push_back(p);
        else
            allBkgs.push_back(p); // default unknown -> background
    }

    // Clear sigs if data is in json to prevent accidental unblinding
    // Can remove this safety check after approval
    if (!allData.empty()) allSigs.clear();

    // --- 4) build yields[proc][binIndex] and yields_err[proc][binIndex] (binIndex: 0..nx-1) ---
    std::map<std::string, std::vector<double>> yields;
    std::map<std::string, std::vector<double>> yields_err;
    for (const auto &p : procSet) {
        yields[p] = std::vector<double>(nx, 0.0);
        yields_err[p] = std::vector<double>(nx, 0.0);
    }

    auto BinContentSafe = [&](TH1* h, int bin)->double {
        if(!h) return 0.0;
        int nb = h->GetNbinsX();
        if(bin < 1 || bin > nb) return 0.0;
        return h->GetBinContent(bin);
    };
    auto BinErrorSafe = [&](TH1* h, int bin)->double {
        if(!h) return 0.0;
        int nb = h->GetNbinsX();
        if(bin < 1 || bin > nb) return 0.0;
        return h->GetBinError(bin);
    };

    for (int ib = 0; ib < nx; ++ib) {
        const auto &binMap = cutflowMap.at(bins[ib]);
        for (const auto &pp : binMap) {
            const std::string &proc = pp.first;
            TH1* h = pp.second;
            if(!h) continue;

            int nbins = h->GetNbinsX(); // number of regular bins
            double last = 0.0;
            double last_err = 0.0;
            if(nbins >= 1) {
                // safely get last regular bin using BinContentSafe and BinErrorSafe
                last = BinContentSafe(h, nbins);
                last_err = BinErrorSafe(h, nbins);
            } else {
                // defensive: histogram has no regular bins
                last = 0.0;
                last_err = 0.0;
                std::cerr << "[Warning] Proc '" << proc << "' in bin '" << bins[ib]
                          << "' has zero bins; setting yield and error to 0\n";
            }
            yields[proc][ib] = last;
            yields_err[proc][ib] = last_err;
        }
    }

    // Ensure all bkgs/sigs exist in yields map (may be empty vectors already)
    for (const auto &b : allBkgs) if(!yields.count(b)) { yields[b] = std::vector<double>(nx,0.0); yields_err[b] = std::vector<double>(nx,0.0); }
    for (const auto &s : allSigs) if(!yields.count(s)) { yields[s] = std::vector<double>(nx,0.0); yields_err[s] = std::vector<double>(nx,0.0); }
    for (const auto &s : allData) if(!yields.count(s)) { yields[s] = std::vector<double>(nx,0.0); yields_err[s] = std::vector<double>(nx,0.0); }

    // --- 5) compute total background per bin and its error in quadrature ---
    std::vector<double> totalBkg(nx, 0.0);
    std::vector<double> totalBkgErr(nx, 0.0);
    for (int ib=0; ib<nx; ++ib) {
        double sum = 0.0;
        double sumsqerr = 0.0;
        for (const auto &b : allBkgs) {
            auto it = yields.find(b);
            if (it != yields.end()) sum += it->second[ib];
            auto iterr = yields_err.find(b);
            if (iterr != yields_err.end()) {
                double e = iterr->second[ib];
                if(std::isfinite(e)) sumsqerr += e*e;
            }
        }
        totalBkg[ib] = sum;
        totalBkgErr[ib] = (sumsqerr > 0.0 ? std::sqrt(sumsqerr) : 0.0);
    }

    // --- 6) ordering: signals by last-bin yield (descending),
    //  backgrounds ascending by last-bin (so largest ends up last,
    //  i.e. adjacent to "Total Bkg"), bins by totalBkg (descending) ---
    if (nx <= 0) return;

    std::vector<int> binOrder(nx);
    for (int i = 0; i < nx; ++i) binOrder[i] = i;

    // --- 7) build Y-order depending on mode ---
    std::vector<std::string> yOrder;
    if(mode == "Zbi"){
        // Total Bkg at top, then signals (no backgrounds)
        yOrder.push_back("Total Bkg");
        for (const auto &s : allSigs) yOrder.push_back(s);
    } else {
        // signals, data, Total Bkg, backgrounds
        for (const auto &s : allSigs) yOrder.push_back(s);
        for (const auto &d : allData) yOrder.push_back(d);
        yOrder.push_back("Total Bkg");
        for (const auto &b : allBkgs) yOrder.push_back(b);
    }

    int ny = (int)yOrder.size();

    // --- 8) create TH2D with appropriate binning & labels ---
    std::string histName = groupKey + "_Cutflow2D_" + mode;
    for (auto &c : histName) if(c==' '||c=='/') c='_';
    TH2D *h2 = new TH2D(histName.c_str(), histName.c_str(), nx, 0.5, nx+0.5, ny, 0.5, ny+0.5);

    // X labels = bin names in the sorted order
    for (int ix=0; ix<nx; ++ix) {
        int old = binOrder[ix];
        h2->GetXaxis()->SetBinLabel(ix+1, bins[old].c_str());
    }
    // Y labels
    for (int iy=0; iy<ny; ++iy) {
        std::string proc_title = yOrder[iy];
        if(proc_title.find("TChiWZ") != std::string::npos)
            proc_title = makeSMSChiTitle(proc_title);
        else {
          auto it = m_Title.find(proc_title);
          if (it != m_Title.end() && !it->second.empty())
              proc_title = it->second;
        }
        h2->GetYaxis()->SetBinLabel(iy+1, proc_title.c_str());
    }

    // --- 9) fill contents. For Zbi collect signal Zbi values for z-range override ---
    std::vector<double> zbi_values_for_range;
    for (int ix=0; ix<nx; ++ix) {
        int oldBin = binOrder[ix];
        double B = totalBkg[oldBin];
        double sqrtB = (B>0.0 ? std::sqrt(B) : 0.0);
        double Berr = totalBkgErr[oldBin];
        for (int iy=0; iy<ny; ++iy) {
            const std::string &procName = yOrder[iy];
            double val = 0.0;
            if(mode == "Zbi") {
                if(procName == "Total Bkg") {
                    val = B;
                } else {
                    // signal Zbi for this bin
                    double S = (yields.count(procName) ? yields[procName][oldBin] : 0.0);
                    if(S > 0.0 && B >= 0.0) {
                        val = CalculateZbi(S, B, Zbi_unc);
                        if(val > 0.0) zbi_values_for_range.push_back(val);
                    } else {
                        val = 0.0;
                    }
                }
            } else {
                double pY = (yields.count(procName) ? yields[procName][oldBin] : 0.0);
                double pErr = (yields_err.count(procName) ? yields_err[procName][oldBin] : 0.0);
                if(procName == "Total Bkg") {
                    if(mode == "yield" || mode == "raw") val = B;
                    else if(mode == "SoB") val = (B > 0.0 ? 1.0 : 0.0);
                    else if(mode == "SoverSqrtB") val = (sqrtB > 0.0 ? sqrtB : 0.0);
                    else if(mode == "effective") {
                        if (Berr > 0.0 && std::isfinite(Berr))
                            val = (B*B) / (Berr * Berr);
                        else
                            val = 0.0;
                    } else val = B;
                } else if((procName.find("data") != std::string::npos || procName.find("Data") != std::string::npos)) {
                    if(mode == "SoB") val = (B > 0.0 ? pY / B : 0.0);
                    else if(mode == "effective") {
                        if (pErr > 0.0 && std::isfinite(pErr))
                            val = (pY*pY) / (pErr * pErr);
                        else
                            val = 0.0;
                    } else val = pY;
                } else {
                    if(mode == "yield" || mode == "raw") val = pY;
                    else if(mode == "SoB") val = (B > 0.0 ? pY / B : 0.0);
                    else if(mode == "SoverSqrtB") val = (sqrtB > 0.0 ? pY / sqrtB : 0.0);
                    else if(mode == "effective") {
                        if (pErr > 0.0 && std::isfinite(pErr))
                            val = (pY*pY) / (pErr * pErr);
                        else
                            val = 0.0;
                    } else val = pY;
                }
            }
            h2->SetBinContent(ix+1, iy+1, val);
        }
    }

    // --- 10) determine z-range override for Zbi (based only on signal Zbi values) ---
    double zmin_override = std::numeric_limits<double>::quiet_NaN();
    double zmax_override = std::numeric_limits<double>::quiet_NaN();
    if(mode == "Zbi" && !zbi_values_for_range.empty()) {
        double zmin = 1e300, zmax = -1e300;
        for(double v : zbi_values_for_range){
            if(v > 0.0) { if(v < zmin) zmin = v; if(v > zmax) zmax = v; }
        }
        if(zmax >= zmin && zmin < 1e299){
            zmin_override = std::max(0.0, 0.9 * zmin);
            zmax_override = 1.1 * zmax;
            if(zmax_override <= zmin_override) zmax_override = zmin_override + 1e-6;
        }
    }

    h2->SetMinimum(0.01);
    // --- 11) call plotting routine which handles drawing + special Zbi total-row masking ---
    Plot_EventCount2D(h2, mode, zmin_override, zmax_override);
}
