// Helpers for Plotting Tools
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <set>
#include <iomanip>
#include <sstream>
#include <limits>
#include <cmath>
#include <fstream>
#include <regex>
#include <fnmatch.h>
#include <unordered_set>
#include <optional>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TKey.h>
#include <TCanvas.h>
#include <TSystem.h>
#include <TString.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TCollection.h>
#include <TPad.h>
#include <TStyle.h>
#include <TEfficiency.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TGraphAsymmErrors.h>
#include <TPaveText.h>
#include <TDirectory.h>
#include <TInterpreter.h>
#include <TLine.h>
#include <TBox.h>
#include <TMathText.h>

#include "SampleTool.h"

using namespace std;

// globals

TFile* outFile = nullptr;
int lumi = 1;
string outputDir = "plots/";
map<string,string> m_Title;
map<string,int>    m_Color;
SampleTool tool;

std::string CMS_label = "#bf{CMS} Preliminary";

double hlo = 0.09;
double hhi = 0.22;
double hbo = 0.15;
double hto = 0.07;

// ----------------------
// Helpers
// ----------------------

std::string SanitizeString(const std::string& name) {
    std::string out;
    for (char c : name) {
        if (std::isalnum(c) || c=='_' || c=='-') out += c;
        //else out += '_';
    }
    return out;
}

void SetMinimumBinContent(TH1* h, double minVal) {
    if (!h) return;
    int nb = h->GetNbinsX();
    for (int i = 1; i <= nb; ++i) {
        double c = h->GetBinContent(i);
        if (c < minVal) h->SetBinContent(i, minVal);
    }
}

void GetMinMaxIntegral(const vector<TH1*>& vect, double &hmin, double &hmax) {
    hmin = 1e99; hmax = 0.;
    for (auto h : vect) {
        if (!h) continue;
        int nb = h->GetNbinsX();
        double localmin = 1e99;
        for (int i = 1; i <= nb; ++i) {
            double c = h->GetBinContent(i);
            if (c > 0. && c < localmin) localmin = c;
            if (c > hmax) hmax = c;
        }
        if (localmin < hmin) hmin = localmin;
    }
    if (hmin > 1e98) hmin = 0.;
}

// helper: get last regular bin content (not overflow)
double LastBinContent(TH1* h){
    if(!h) return 0.0;
    int nb = h->GetNbinsX();
    return h->GetBinContent(nb);
}

// helper: get bin content at given 1-based bin index (safe)
double BinContentSafe(TH1* h, int bin){
    if(!h) return 0.0;
    int nb = h->GetNbinsX();
    if(bin < 1 || bin > nb) return 0.0;
    return h->GetBinContent(bin);
}

double CalculateZbi(double Nsig, double Nbkg, double deltaNbkg){
  if(deltaNbkg >= 1.) deltaNbkg /= 100; // assume user passed in percent rather than fraction
  double Nobs = Nsig+Nbkg;
  double tau = 1./Nbkg/(deltaNbkg*deltaNbkg);
  double aux = Nbkg*tau;
  double Pvalue = TMath::BetaIncomplete(1./(1.+tau),Nobs,aux+1.);
  double sigma = sqrt(2.)*TMath::ErfcInverse(Pvalue*2);

  return (isnan(sigma))?0:sigma;
}

void SortHistsByYield(std::vector<TH1*> &hists) {
    std::sort(hists.begin(), hists.end(),
              [](TH1* a, TH1* b) {
                  // Handle nulls (treat them as zero yield)
                  double ya = (a ? a->Integral() : 0.0);
                  double yb = (b ? b->Integral() : 0.0);
                  return ya > yb;   // descending
              });
}

const TColor rf_blue0(7000,0.749,0.78,0.933);
const TColor rf_blue1(7001,0.424,0.467,0.651);
const TColor rf_blue2(7002,0.255,0.302,0.522);
const TColor rf_blue3(7003,0.114,0.165,0.396);
const TColor rf_blue4(7004,0.024,0.063,0.251);
const TColor rf_green0(7010,0.737,0.949,0.784);
const TColor rf_green1(7011,0.435,0.722,0.498);
const TColor rf_green2(7012,0.239,0.576,0.314);
const TColor rf_green3(7013,0.082,0.439,0.161);
const TColor rf_green4(7014,0,0.275,0.063);
const TColor rf_red0(7020,1,0.796,0.776);
const TColor rf_red1(7021,0.957,0.612,0.576);
const TColor rf_red2(7022,0.765,0.361,0.318);
const TColor rf_red3(7023,0.58,0.157,0.11);
const TColor rf_red4(7024,0.365,0.035,0);
const TColor rf_yellow0(7030,1,0.933,0.776);
const TColor rf_yellow1(7031,0.957,0.843,0.576);
const TColor rf_yellow2(7032,0.765,0.631,0.318);
const TColor rf_yellow3(7033,0.58,0.443,0.11);
const TColor rf_yellow4(7034,0.365,0.259,0);
const TColor rf_purple0(7040,0.937,0.729,0.898);
const TColor rf_purple1(7041,0.753,0.478,0.702);
const TColor rf_purple2(7042,0.6,0.286,0.541);
const TColor rf_purple3(7043,0.42,0.075,0.353);
const TColor rf_purple4(7044,0.196,0,0.161);
const TColor rf_cyan0(7050,0.714,0.898,0.918);
const TColor rf_cyan1(7051,0.424,0.639,0.659);
const TColor rf_cyan2(7052,0.247,0.49,0.51);
const TColor rf_cyan3(7053,0.067,0.329,0.357);
const TColor rf_cyan4(7054,0,0.153,0.169);
const TColor rf_orange0(7060,1,0.882,0.776);
const TColor rf_orange1(7061,1,0.808,0.639);
const TColor rf_orange2(7062,0.839,0.608,0.4);
const TColor rf_orange3(7063,0.584,0.329,0.106);
const TColor rf_orange4(7064,0.275,0.129,0);
const TColor rf_lime0(7070,0.941,0.992,0.769);
const TColor rf_lime1(7071,0.882,0.961,0.612);
const TColor rf_lime2(7072,0.706,0.8,0.38);
const TColor rf_lime3(7073,0.455,0.557,0.098);
const TColor rf_lime4(7074,0.204,0.263,0);

const TColor cms1(8001, 0.24705882352941178, 0.5647058823529412, 0.8549019607843137);
const TColor cms2(8002, 1., 0.6627450980392157, 0.054901960784313725);
const TColor cms3(8004, 0.7411764705882353, 0.12156862745098039, 0.00392156862745098);
const TColor cms4(8005, 0.5803921568627451, 0.6431372549019608, 0.6352941176470588);
const TColor cms5(8006, 0.5137254901960784, 0.17647058823529413, 0.7137254901960784);
const TColor cms6(8007, 0.6627450980392157, 0.4196078431372549, 0.34901960784313724);
const TColor cms7(8008, 0.9058823529411765, 0.38823529411764707, 0.);
const TColor cms8(8009, 0.7254901960784313, 0.6745098039215687, 0.4392156862745098);
const TColor cms9(8010, 0.44313725490196076, 0.4588235294117647, 0.5058823529411764);
const TColor cms10(8011, 0.5725490196078431, 0.8549019607843137, 0.8666666666666667);

std::vector<int> fallbackColors = {
    // CMS First
    8001, 8002, 8011, 8005, 8006, 8007, 8004, 8009, 8008, 8011,
    8001, 8002, 8011, 8005, 8006, 8007, 8004, 8002, 8007, 8001,

    // RestFrames
    7000, 7010, 7020, 7030, 7040, 7050, 7060, 7070,
    7001, 7011, 7021, 7031, 7041, 7051, 7061, 7071,
    7002, 7012, 7022, 7032, 7042, 7052, 7062, 7072,
    7003, 7013, 7023, 7033, 7043, 7053, 7063, 7073,
    7004, 7014, 7024, 7034, 7044, 7054, 7064, 7074 
};
static size_t fallbackIndex = 0;

std::string makeSMSChiTitle(const std::string& key) {
    int color = kBlack;
    auto it = m_Color.find(key);
    if (it == m_Color.end()) {
        // Fallback to rotating palette
        color = fallbackColors[fallbackIndex % fallbackColors.size()];
        fallbackIndex++;
        m_Color[it->first] = color;
    }
    // Determine model type
    std::string model;
    if (key.find("TChiWZ") != std::string::npos) model = "TChiWZ";
    else if (key.find("TChiHZ") != std::string::npos) model = "TChiHZ";
    else if (key.find("TChipmWW") != std::string::npos || key.find("TChiWW") != std::string::npos) model = "TChipmWW";
    else model = "Unknown";

    // Determine particle text for the model
    std::string particles;
    if (model == "TChiWZ" || model == "TChiHZ")
        particles = "#tilde{#chi}^{0}_{2} #tilde{#chi}^{#pm}_{1}";
    else if (model == "TChipmWW")
        particles = "#tilde{#chi}^{#pm}_{1} #tilde{#chi}^{#mp}_{1}";
    else
        particles = "#tilde{#chi} #tilde{#chi}"; // fallback

    // Extract mass numbers from key
    int m1 = 0, m3 = 0;
    size_t lastUnderscore = key.find_last_of('_');
    size_t secondLastUnderscore = key.find_last_of('_', lastUnderscore - 1);
    if (secondLastUnderscore == std::string::npos || lastUnderscore == std::string::npos)
        return "Invalid";

    try {
        m1 = std::stoi(key.substr(secondLastUnderscore + 1, lastUnderscore - secondLastUnderscore - 1));
        m3 = std::stoi(key.substr(lastUnderscore + 1));
    } catch (...) {
        return "Invalid";
    }

    // Handle Sandwich models
    bool isSandwich = key.find("Sandwich") != std::string::npos;
    int m2 = (isSandwich && model != "TChipmWW") ? (m1 + m3) / 2 : m1;
    std::string extra = "";
    if(key.find("preUL") != std::string::npos) extra += "preUL ";
    if(key.find("TEST") != std::string::npos) extra += "TEST ";

    // Compose final title
    std::ostringstream title;
    title << extra << particles << " " << m1 << ", " << m2 << ", " << m3;
    return title.str();
}

void loadFormatMaps(){

  m_Title["data_obs"] = "Data";
  m_Color["data_obs"] = kBlack;

  m_Title["data"] = "Data";
  m_Color["data"] = kBlack;
  
  m_Title["top"] = "top";
  m_Color["top"] = 7001;

  m_Title["boson"] = "boson";
  m_Color["boson"] = 7011;
  //m_Color["boson"] = 8006;

  m_Title["diboson"] = "di-bosons";
  m_Color["diboson"] = 7042;

  m_Title["triboson"] = "tri-bosons";
  m_Color["triboson"] = 7062;

  m_Title["DB"] = "dibosons";
  m_Color["DB"] = 7041;
  //m_Color["DB"] = 8002;

  m_Title["TB"] = "tribosons";
  m_Color["TB"] = 7033;
  //m_Color["TB"] = 8006;

  m_Title["ttbar"] = "t #bar{t} + X";
  m_Color["ttbar"] = 7011;
  //m_Color["ttbar"] = 8003;

  m_Title["Vfakeleps"] = "fake enriched";
  m_Color["Vfakeleps"] = 7001;
  //m_Color["Vfakeleps"] = 8001;

  m_Title["top_2018"] = "top";
  m_Color["top_2018"] = 7011;
  //m_Color["top_2018"] = 8003;

  m_Title["Vfakeleps_2018"] = "fake enriched";
  m_Color["Vfakeleps_2018"] = 7001;
  //m_Color["Vfakeleps_2018"] = 8001;

  m_Title["boson_2018"] = "boson";
  m_Color["boson_2018"] = 7050;
  //m_Color["boson_2018"] = 8006;

  m_Title["ST"] = "single top";
  m_Color["ST"] = 7010;
  //m_Color["ST"] = 8004;
 
  m_Title["DBTB"] = "di & tri-bosons";
  m_Color["DBTB"] = 7050;
  //m_Color["DBTB"] = 8006;

  m_Title["ZDY"] = "Z / #gamma* + jets";
  m_Color["ZDY"] = 7000;
  //m_Color["ZDY"] = 8005;

  m_Title["Wjets"] = "W + jets";
  m_Color["Wjets"] = 7001;
  //m_Color["Wjets"] = 8001; 
  
  m_Title["Gjets"] = "#gamma + jets";
  m_Color["Gjets"] = 7051;
  //m_Color["Gjets"] = 8002; 

  m_Title["QCD"] = "QCD multijets";
  m_Color["QCD"] = 7023;
  //m_Color["QCD"] = 8007;

  m_Title["ZInv"] = "Z Inv";
  m_Color["ZInv"] = 7022;
  //m_Color["ZInv"] = 8005;
  
  m_Title["DY"] = "DY";
  m_Color["DY"] = 7021;
  //m_Color["DY"] = 8006;

  m_Title["top_Run2"] = "top";
  m_Title["top_Run3"] = "top";
  m_Title["boson_Run2"] = "boson";
  m_Title["boson_Run3"] = "boson";
  m_Title["diboson_Run2"] = "diboson";
  m_Title["diboson_Run3"] = "diboson";
  m_Title["triboson_Run2"] = "triboson";
  m_Title["triboson_Run3"] = "triboson";

  m_Color["top_Run2"] = 7011;
  m_Color["top_Run3"] = 7011;
  m_Color["boson_Run2"] = 7001;
  m_Color["boson_Run3"] = 7001;
  m_Color["diboson_Run2"] = 7051;
  m_Color["diboson_Run3"] = 7051;
  m_Color["triboson_Run2"] = 7050;
  m_Color["triboson_Run3"] = 7050;

  m_Title["HF_FAKES"] = "HF leptons";
  m_Color["HF_FAKES"] = 7022;
  //m_Color["HF_FAKES"] = 8008;

  m_Title["LF_FAKES"] = "LF/fake leptons";
  m_Color["LF_FAKES"] = 7021;
  //m_Color["LF_FAKES"] = 8009;
  
  m_Title["FAKES"] = "fake leptons";
  m_Color["FAKES"] = 7021;
  //m_Color["FAKES"] = 8010;
  
  m_Title["Fakes_elHF_Run2"] = "HF fake e^{#pm}";
  m_Color["Fakes_elHF_Run2"] = 7021;

  m_Title["Fakes_elLF_Run2"] = "LF fake e^{#pm}";
  m_Color["Fakes_elLF_Run2"] = 7022;

  m_Title["Fakes_muHF_Run2"] = "HF fake #mu^{#pm}";
  m_Color["Fakes_muHF_Run2"] = 7023;

  m_Title["Fakes_muLF_Run2"] = "LF fake #mu^{#pm}";
  m_Color["Fakes_muLF_Run2"] = 7024;

  m_Title["Fakes_elHF_Run3"] = "HF fake e^{#pm}";
  m_Color["Fakes_elHF_Run3"] = 7021;

  m_Title["Fakes_elLF_Run3"] = "LF fake e^{#pm}";
  m_Color["Fakes_elLF_Run3"] = 7022;

  m_Title["Fakes_muHF_Run3"] = "HF fake #mu^{#pm}";
  m_Color["Fakes_muHF_Run3"] = 7023;

  m_Title["Fakes_muLF_Run3"] = "LF fake #mu^{#pm}";
  m_Color["Fakes_muLF_Run3"] = 7024;

  m_Title["Fakes_elHF"] = "HF fake e^{#pm}";
  m_Color["Fakes_elHF"] = 7021;

  m_Title["Fakes_elLF"] = "LF fake e^{#pm}";
  m_Color["Fakes_elLF"] = 7022;

  m_Title["Fakes_muHF"] = "HF fake #mu^{#pm}";
  m_Color["Fakes_muHF"] = 7023;

  m_Title["Fakes_muLF"] = "LF fake #mu^{#pm}";
  m_Color["Fakes_muLF"] = 7024;

  m_Title["HF"] = "heavy flavor";
  m_Color["HF"] = 7022;

  m_Title["LF"] = "light flavor";
  m_Color["LF"] = 7021;

  m_Title["ttbar_FAKES"] = "t #bar{t} fakes";
  m_Color["ttbar_FAKES"] = 7020;

  m_Title["top_FAKES"] = "top fakes";
  m_Color["top_FAKES"] = 7020;

  m_Title["V_FAKES"] = "Nonprompt";
  m_Color["V_FAKES"] = 7001;

  m_Title["boson_FAKES"] = "boson fakes";
  m_Color["boson_FAKES"] = 7012;

  m_Title["Wjets_FAKES"] = "W+jets fakes";
  m_Color["Wjets_FAKES"] = 7023;

  m_Title["ST_FAKES"] = "single top fakes";
  m_Color["ST_FAKES"] = 7024;

  m_Title["DB_FAKES"] = "di-boson fakes";
  m_Color["DB_FAKES"] = 7012;

  m_Title["TB_FAKES"] = "tri-boson fakes";
  m_Color["TB_FAKES"] = 7013;

  m_Title["ZDY_FAKES"] = "Z / #gamma* + jets fakes";
  m_Color["ZDY_FAKES"] = 7014;

  m_Title["Fakes_el"] = "e^{#pm} fakes";
  m_Color["Fakes_el"] = 7020;

  m_Title["Fakes_mu"] = "#mu^{#pm} fakes";
  m_Color["Fakes_mu"] = 7023;

  m_Title["Fakes_emu"] = "e^{#pm} #mu^{#pm} fakes";
  m_Color["Fakes_emu"] = 7012;

  m_Title["ttbar_all"] = "t #bar{t} + jets";
  m_Color["ttbar_all"] = 7011;

  m_Title["ST_all"] = "single top";
  m_Color["ST_all"] = 7010;

  m_Title["ZDY_all"] = "Z / #gamma* + jets";
  m_Color["ZDY_all"] = 7000;

  m_Title["Total"] = "Total Bkg";
  m_Color["Total"] = 7000;
  m_Title["Total Bkg"] = "Total Bkg";
  m_Color["Total Bkg"] = 7000;

  m_Title["bkg"] = "Background";
  m_Color["bkg"] = 7001;

  m_Title["bkg_Run2"] = "Background";
  m_Color["bkg_Run2"] = 7001;

  m_Title["bkg_Run3"] = "Background";
  m_Color["bkg_Run3"] = 7001;

  m_Title["Data_2016"] = "Data";
  m_Color["Data_2016"] = 7001;

  m_Title["Cascades_300_300_289_260_240_220_220_209_200_190_180"] = "Cascades 180";
  m_Color["Cascades_300_300_289_260_240_220_220_209_200_190_180"] = 7040; // 7072 might look better...?
  m_Title["Cascades_209_220_209_200_190_180"] = "Cascades 180";
  m_Color["Cascades_209_220_209_200_190_180"] = 7040; // 7072 might look better...?
  m_Title["Cascades_220_220_209_200_190_180"] = "Cascades 180";
  m_Color["Cascades_220_220_209_200_190_180"] = 7040; // 7072 might look better...?

  m_Title["Cascades_300_300_289_260_240_220"] = "Cascades 220";
  m_Color["Cascades_300_300_289_260_240_220"] = 7071;
  m_Title["Cascades_300_300_289_260_240_220"] = "Cascades 220";
  m_Color["Cascades_300_300_289_260_240_220"] = 7071;
  m_Title["Cascades_300_300_289_260_240_220_SMS"] = "Cascades 220 SMS";
  m_Color["Cascades_300_300_289_260_240_220_SMS"] = 7071;
  //m_Color["Cascades_300_300_289_260_240_220"] = 8007;

  m_Title["Cascades_300_300_289_280_270_260"] = "Cascades 260";
  m_Color["Cascades_300_300_289_280_270_260"] = 7041;
  m_Title["Cascades_289_300_289_280_270_260"] = "Cascades 260";
  m_Color["Cascades_289_300_289_280_270_260"] = 7041;
  m_Title["Cascades_300_300_289_280_270_260_SMS"] = "Cascades 260 SMS";
  m_Color["Cascades_300_300_289_280_270_260_SMS"] = 7041;
  //m_Color["Cascades_300_300_289_280_270_260"] = 8008;

  m_Title["Cascades_300_300_289_280_275_270"] = "Cascades 270";
  m_Color["Cascades_300_300_289_280_275_270"] = 7061;
  m_Title["Cascades_300_300_289_280_275_270"] = "Cascades 270";
  m_Color["Cascades_300_300_289_280_275_270"] = 7061;
  m_Title["Cascades_300_300_289_280_275_270_SMS"] = "Cascades 270 SMS";
  m_Color["Cascades_300_300_289_280_275_270_SMS"] = 7061;
  //m_Color["Cascades_300_300_289_280_275_270"] = 8009;

  m_Title["SMS_Gluinos_SMS_1000_900"] = "T1qqqq 1000 900";
  m_Color["SMS_Gluinos_SMS_1000_900"] = 7071;
  m_Title["SMS_Gluinos_SMS_1100_1000"] = "T1qqqq 1100 1000";
  m_Color["SMS_Gluinos_SMS_1100_1000"] = 7041;
  m_Title["SMS_Gluinos_SMS_1200_1100"] = "T1qqqq 1200 1100";
  m_Color["SMS_Gluinos_SMS_1200_1100"] = 7061;
  m_Title["SMS_Gluinos_SMS_1200_1176"] = "T1qqqq 1200 1175";
  m_Color["SMS_Gluinos_SMS_1200_1176"] = 7071;
  m_Title["SMS_Gluinos_SMS_2000_1900"] = "T1qqqq 2000 1900";
  m_Color["SMS_Gluinos_SMS_2000_1900"] = 7041;
  m_Title["SMS_Gluinos_SMS_1500_1400"] = "T1qqqq 1500 1400";
  m_Color["SMS_Gluinos_SMS_1500_1400"] = 7041;

}

TH1D* TGraphToTH1(TGraphAsymmErrors* g, const std::string& name) {
    if (!g) return nullptr;
    int nbins = g->GetN();
    double xmin = g->GetX()[0] - g->GetEXlow()[0];
    double xmax = g->GetX()[nbins-1] + g->GetEXhigh()[nbins-1];

    TH1D* h = new TH1D(name.c_str(), name.c_str(), nbins, xmin, xmax);
    for (int i=0; i<nbins; ++i) {
        h->SetBinContent(i+1, g->GetY()[i]);
        h->SetBinError(i+1, 0.5*(g->GetEYlow()[i]+g->GetEYhigh()[i]));
    }
    return h;
}

// ----------------------
// TH2 -> scatter graph
// one point per occupied bin
// ----------------------
TGraph* TH2ToScatterGraph(const TH2* h, const std::string& name)
{
    if (!h) return nullptr;

    auto* g = new TGraph();
    g->SetName(name.c_str());

    int ip = 0;
    for (int ix = 1; ix <= h->GetNbinsX(); ++ix) {
        for (int iy = 1; iy <= h->GetNbinsY(); ++iy) {
            const double c = h->GetBinContent(ix, iy);
            if (c <= 0.) continue;

            const double x = h->GetXaxis()->GetBinCenter(ix);
            const double y = h->GetYaxis()->GetBinCenter(iy);
            g->SetPoint(ip++, x, y);
        }
    }

    return g;
}

struct YamlBinPattern {
    std::string name;
    std::vector<std::string> include;
    std::vector<std::string> exclude;
};

struct MergedBinGroup {
    std::string group_name;
    std::vector<std::string> bin_names;
    YamlBinPattern pattern;
};

struct YamlProcessPattern {
    std::string name;
    std::vector<std::string> include;
    std::vector<std::string> exclude;
};

struct YamlConfig {
    std::vector<YamlBinPattern> bins;
    std::vector<YamlProcessPattern> process_merges;
};

YamlConfig LoadYamlConfig(const std::string& yamlFile) {

    YamlConfig cfg;
    if (yamlFile.empty()) return cfg;

    YAML::Node root;
    try {
        root = YAML::LoadFile(yamlFile);
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to parse YAML file: " << yamlFile << "\n"
                  << "Exception: " << e.what() << "\n";
        return cfg;
    }

    // -------------------------
    // Parse bins
    // -------------------------
    if (root["bins"]) {
        for (const auto& n : root["bins"]) {
            YamlBinPattern b;
            b.name = n["name"] ? n["name"].as<std::string>() : "";

            // Include: scalar or sequence
            if (n["include"]) {
                if (n["include"].IsSequence()) {
                    for (const auto& inc : n["include"])
                        b.include.push_back(inc.as<std::string>());
                }
                else if (n["include"].IsScalar()) {
                    b.include.push_back(n["include"].as<std::string>());
                }
            }
            else {
                b.include.push_back("*");
            }

            // Exclude: scalar or sequence
            if (n["exclude"]) {
                if (n["exclude"].IsSequence()) {
                    for (const auto& ex : n["exclude"])
                        b.exclude.push_back(ex.as<std::string>());
                }
                else if (n["exclude"].IsScalar()) {
                    b.exclude.push_back(n["exclude"].as<std::string>());
                }
            }

            cfg.bins.push_back(std::move(b));
        }
    }

    // -------------------------
    // Parse process merges
    // -------------------------
    if (root["processes"] && root["processes"]["merge"]) {
        for (const auto& n : root["processes"]["merge"]) {
            YamlProcessPattern p;
            p.name = n["name"].as<std::string>();

            if (n["include"]) {
                if (n["include"].IsSequence()) {
                    for (const auto& inc : n["include"])
                        p.include.push_back(inc.as<std::string>());
                }
                else if (n["include"].IsScalar()) {
                    p.include.push_back(n["include"].as<std::string>());
                }
            }

            if (n["exclude"]) {
                if (n["exclude"].IsSequence()) {
                    for (const auto& ex : n["exclude"])
                        p.exclude.push_back(ex.as<std::string>());
                }
                else if (n["exclude"].IsScalar()) {
                    p.exclude.push_back(n["exclude"].as<std::string>());
                }
            }

            cfg.process_merges.push_back(std::move(p));
        }
    }

    return cfg;
}

static std::string WildcardToRegex(const std::string& pat)
{
    std::string r;
    r.reserve(pat.size() * 2 + 4);
    for (char c : pat) {
        if (c == '*')      r += ".*";
        else if (c == '?') r += ".";
        else if (std::isalnum(static_cast<unsigned char>(c))) r.push_back(c);
        else {
            // escape any other character for regex safety
            r.push_back('\\');
            r.push_back(c);
        }
    }
    return r;
}

namespace BinTokens {

    // --- Run: Run2=0, Run3=1 ------------------------------------
    inline int ExtractRun(const std::string& bin) {
        if (bin.find("Run3") != std::string::npos) return 1;
        if (bin.find("Run2") != std::string::npos) return 0;
        return 99;
    }
    
    // --- Lepton multiplicity: 2L=0, 3L=1, 4L=2 -----------------
    inline int ExtractLepMult(const std::string& bin) {
        std::smatch m;
        if (std::regex_search(bin, m, std::regex("_(\\d)L_")))
            return std::stoi(m[1]) - 2;
        return 99;
    }
    
    // --- Quality: Bronze=0, Silver=1, Gold=2 ----------------------
    inline int ExtractQuality(const std::string& bin) {
        if (bin.find("Bronze") != std::string::npos) return 0;
        if (bin.find("Silver") != std::string::npos) return 1;
        if (bin.find("Gold")   != std::string::npos) return 2;
        return 99;
    }
    
    // --- Jet multiplicity: 0J=0, 1J=1 -----------------
    inline int ExtractJets(const std::string& bin) {
        if (bin.find("0J")    != std::string::npos) return 0;
        if (bin.find("1J")    != std::string::npos) return 1;
        return 2;   // Incl
    }
    
    // --- PTISR: numeric value after 'P', e.g. P250 ----------
    inline int ExtractPTISR(const std::string& bin) {
        std::smatch m;
        if (std::regex_search(bin, m, std::regex("_P(\\d+)")))
            return std::stoi(m[1]);
        return 9999;
    }
    
    // --- RISR: e.g. R7->0.70, R75->0.75, R8->0.80, R9->0.90 -------
    //     Rule: two-digit token (R75) -> divide by 100
    //           one-digit token (R7)  -> divide by 10
    inline int ExtractRISR(const std::string& bin)
    {
        std::smatch m;
        // R75
        if (std::regex_search(bin, m, std::regex("_R(\\d{2})(?:[^\\d]|$)")))
            return std::stoi(m[1]);
        // R7
        if (std::regex_search(bin, m, std::regex("_R(\\d)(?:[^\\d]|$)")))
            return std::stoi(m[1]) * 10;
        return 999;
    }
    // --- Mperp sort key -----------------------------------------
    //  Sort signal-like bins to the right:
    //    M<N>  (lower bound only, e.g. M30)  -> use N as key, large first
    //    M<lo>_<hi> (range)                  -> use lo as key
    //    Mlt<N>                              -> key = 0 (lowest)
    //    Btag (no Mperp)                     -> key = -1 (after all Mperp)
    //
    //  We want descending lower-edge -> ascending sort key = -lower_edge
    inline double ExtractMperpSortKey(const std::string& bin) {
        if (bin.find("Btag") != std::string::npos) return 1e6;  // Btag last
    
        std::smatch m;
        // Mlt<N>  e.g. Mlt15 -> lower edge 0, sort key = -(0) = 0 -> smallest -> leftmost
        if (std::regex_search(bin, m, std::regex("_Mlt(\\d+)")))
            return 0.0;
    
        // M<lo>_<hi>  e.g. M20_30 -> lower edge 20
        if (std::regex_search(bin, m, std::regex("_M(\\d+)_(\\d+)")))
            return std::stod(m[1]);    // higher lower-edge -> larger key -> sorted last (right)
      
        // M<N>  lower-bound only e.g. M30
        if (std::regex_search(bin, m, std::regex("_M(\\d+)(?=_|$)")))
            return std::stod(m[1]); 

        return 500.0;   // unknown Mperp token, before Btag
    }
    
    // --- Flavor/Charge: a simple lexicographic fallback ----------
    //  OS_ee < OS_emu < OS_mumu < OS_SF < OS_OF < OS_* < SS < Btag
    inline int ExtractFlavorRank(const std::string& bin) {
        if (bin.find("OS_ee")   != std::string::npos) return 0;
        if (bin.find("OS_emu")  != std::string::npos) return 1;
        if (bin.find("OS_mumu") != std::string::npos) return 2;
        if (bin.find("OSOFa")   != std::string::npos) return 3;
        if (bin.find("OSO")     != std::string::npos) return 3;  // other OS_OF variants
        if (bin.find("OSSFa")   != std::string::npos) return 4;
        if (bin.find("OSSF")    != std::string::npos) return 4;
        if (bin.find("OS_")     != std::string::npos) return 5;  // other OS
        if (bin.find("SSa")     != std::string::npos) return 6;
        if (bin.find("SS")      != std::string::npos) return 6;
        if (bin.find("Btag")    != std::string::npos) return 7;
        return 8;
    }
    
    // --- Composite sort key tuple --------------------------------
    struct BinKey {
        int    run;
        int    lep;
        int    quality;
        int    jets;
        int    ptisr;
        int risr;
        double mperp;
        int    flavor;
        std::string raw;   // tiebreak
    };
    
    inline BinKey MakeKey(const std::string& bin) {
        BinKey k;
        k.run    = ExtractRun(bin);
        k.lep    = ExtractLepMult(bin);
        k.quality  = ExtractQuality(bin);
        k.jets   = ExtractJets(bin);
        k.ptisr  = ExtractPTISR(bin);
        k.risr   = ExtractRISR(bin);
        k.mperp  = ExtractMperpSortKey(bin);
        k.flavor = ExtractFlavorRank(bin);
        k.raw    = bin;
        return k;
    }
    
    inline bool KeyLess(const BinKey& a, const BinKey& b) {
        if (a.run    != b.run)    return a.run    < b.run;
        if (a.lep    != b.lep)    return a.lep    < b.lep;
        if (a.quality  != b.quality)  return a.quality  < b.quality;
        if (a.jets   != b.jets)   return a.jets   < b.jets;
        if (a.ptisr  != b.ptisr)  return a.ptisr  < b.ptisr;
        if (a.risr   != b.risr)   return a.risr   < b.risr;
        if (a.mperp  != b.mperp)  return a.mperp  < b.mperp;
        if (a.flavor != b.flavor) return a.flavor < b.flavor;
        return a.raw < b.raw;
    }

} // namespace BinTokens

inline bool SameRISRParent(const BinTokens::BinKey& a,
                           const BinTokens::BinKey& b)
{
    return
        a.run     == b.run     &&
        a.lep     == b.lep     &&
        a.quality == b.quality &&
        a.jets    == b.jets    &&
        a.ptisr   == b.ptisr;
}

inline bool SameMperpParent(const BinTokens::BinKey& a,
                            const BinTokens::BinKey& b)
{
    return
        a.run     == b.run     &&
        a.lep     == b.lep     &&
        a.quality == b.quality &&
        a.jets    == b.jets    &&
        a.ptisr   == b.ptisr   &&
        a.risr    == b.risr;
}

/// Call this after BuildMergedBinGroupsFromYaml to re-sort bin_names.
bool BinSortFull(const std::string& a, const std::string& b) {
    return BinTokens::KeyLess(BinTokens::MakeKey(a), BinTokens::MakeKey(b));
}

/// Sort a vector of bin name strings in-place.
void SortBinNames(std::vector<std::string>& bins) {
    std::sort(bins.begin(), bins.end(), BinSortFull);
}

/// Convenience: sort all groups produced by BuildMergedBinGroupsFromYaml.
void SortAllGroups(std::vector<MergedBinGroup>& groups) {
    for (auto& g : groups)
        SortBinNames(g.bin_names);
}

std::string SwapPTISRTag(const std::string& name)
{
    if (name.find("Low_PTISR") != std::string::npos)
        return std::regex_replace(name, std::regex("Low_PTISR"), "High_PTISR");

    if (name.find("High_PTISR") != std::string::npos)
        return std::regex_replace(name, std::regex("High_PTISR"), "Low_PTISR");

    return "";
}

bool ExtractPTISRBoundary(const YamlBinPattern& p, int& out)
{
    std::regex re("_P(\\d+)");
    std::smatch m;

    for (const auto& inc : p.include) {
        if (std::regex_search(inc, m, re)) {
            out = std::stoi(m[1].str());
            return true;
        }
    }
    return false;
}

inline bool ExtractRISRBoundary(const YamlBinPattern& p, double& out)
{
    std::regex re("_R(\\d+)");
    std::smatch m;

    for (const auto& inc : p.include) {
        if (std::regex_search(inc, m, re)) {

            int v = std::stoi(m[1].str());

            out = (v >= 10)
                ? v / 100.0
                : v / 10.0;

            return true;
        }
    }

    return false;
}

inline double FindNextRISRBoundary(
    int currentIndex,
    const std::vector<BinTokens::BinKey>& keys)
{
    const auto& current = keys[currentIndex];

    double best = 999;

    for (size_t i = 0; i < keys.size(); ++i) {

        if ((int)i == currentIndex)
            continue;

        const auto& other = keys[i];

        if (!SameRISRParent(current, other))
            continue;

        if (other.risr > current.risr &&
            other.risr < best)
        {
            best = other.risr;
        }
    }

    return best;
}

inline bool ExtractMperpBoundary(const YamlBinPattern& p, int& out)
{
    std::smatch m;

    for (const auto& inc : p.include) {

        // Mlt15 -> lower edge = 0
        if (std::regex_search(inc, m, std::regex("_Mlt(\\d+)"))) {
            out = 0;
            return true;
        }

        // M20_30 -> lower edge = 20
        if (std::regex_search(inc, m, std::regex("_M(\\d+)_(\\d+)"))) {
            out = std::stoi(m[1].str());
            return true;
        }

        // M30 -> lower edge = 30
        if (std::regex_search(inc, m, std::regex("_M(\\d+)(?:_|$)"))) {
            out = std::stoi(m[1].str());
            return true;
        }
    }

    return false;
}

inline int FindNextMperpBoundary(
    int currentIndex,
    const std::vector<BinTokens::BinKey>& keys)
{
    const auto& current = keys[currentIndex];

    int best = 999999;

    for (size_t i = 0; i < keys.size(); ++i) {

        if ((int)i == currentIndex)
            continue;

        const auto& other = keys[i];

        if (!SameMperpParent(current, other))
            continue;

        if (other.mperp > current.mperp &&
            other.mperp < best)
        {
            best = static_cast<int>(other.mperp);
        }
    }

    return best;
}

// Build once after LoadYamlConfig, pass into BuildBracketTiers
// Key: include pattern with R-token replaced by placeholder
// Value: sorted set of all R integer values seen across all YAML entries sharing that pattern shape
std::map<std::string, std::set<int>> BuildRISRNeighborMap(const YamlConfig& cfg)
{
    std::regex rToken("_R(\\d+)(?=[^\\d]|$)");
    std::map<std::string, std::set<int>> out;

    for (const auto& b : cfg.bins) {
        for (const auto& inc : b.include) {
            std::smatch m;
            if (std::regex_search(inc, m, rToken)) {
                int raw = std::stoi(m[1].str());
                // Normalize to the same scale as ExtractRISR:
                // single-digit (R8) -> multiply by 10; two-digit (R85) -> keep as-is
                int rval = (raw < 10) ? raw * 10 : raw;
                std::string key = std::regex_replace(inc, rToken, "_R{X}");
                out[key].insert(rval);
            }
        }
    }
    return out;
}

namespace BinLabels {

    // Convert a full bin name to a short per-bin x-axis tick label.
    inline std::string ShortBinLabel(const std::string& bin) {
        // Btag bins
        if (bin.find("Btag") != std::string::npos) return "b-tag";
    
        std::string label;
    
        // Flavor/charge part
        std::string flav;
        if      (bin.find("OS_ee")   != std::string::npos) flav = "e^{+}e^{-}";
        else if (bin.find("OS_emu")  != std::string::npos) flav = "e^{#pm}#mu^{#mp}";
        else if (bin.find("OS_mumu") != std::string::npos) flav = "#mu^{+}#mu^{-}";
        else if (bin.find("OSOFa")   != std::string::npos) flav = "e^{#pm}#mu^{#mp}";
        else if (bin.find("OSSFa")   != std::string::npos) flav = "#it{l}^{+}#it{l}^{-}";
        else if (bin.find("OS_")     != std::string::npos) flav = "#it{l}^{+}#it{l}^{-}";
        else if (bin.find("SSa")     != std::string::npos) flav = "#it{l}^{#pm}#it{l}^{#pm}";
        else if (bin.find("SS")      != std::string::npos) flav = "#it{l}^{#pm}#it{l}^{#pm}";
        else if (bin.find("_31")     != std::string::npos) flav = "Lep Split 3+1";
        else if (bin.find("_22")     != std::string::npos) flav = "Lep Split 2+2";
    
        if (!label.empty() && !flav.empty()) return label + " " + flav;
        if (!label.empty()) return label;
        if (!flav.empty())  return flav;
        return bin;   // fallback: show raw name
    }
    
    /// Human-readable tier label strings
    inline std::string RunLabel(int run) {
        if (run == 0) return "Run 2";
        if (run == 1) return "Run 3";
        return "";
    }
    inline std::string LepLabel(int lep) {
        return std::to_string(lep + 2) + "L";
    }
    inline std::string QualityLabel(int quality) {
        if (quality == 0) return "Bronze";
        if (quality == 1) return "Silver";
        if (quality == 2) return "Gold";
        return "";
    }
    inline std::string JetsLabel(int jets) {
        if (jets == 0) return "0J";
        if (jets == 1) return "1J";
        if (jets == 2) return "J-incl";
        return "";
    }
    inline std::string PTISRLabel(int ptisr) {
        if (ptisr >= 9999) return "";
        return "p_{T}^{ISR} > " + std::to_string(ptisr);
    }
    inline std::string RISRLabel(int risr) {
        if (risr >= 999) return "";
        std::ostringstream ss;
        ss << "R_{ISR} > " << (risr / 100.0);
        return ss.str();
    }
} // namespace BinLabels

/// Describes one contiguous span of bins sharing a label at one tier.
struct BracketSpan {
    int    binFirst;   // 1-based bin index, inclusive
    int    binLast;    // 1-based bin index, inclusive
    std::string label;
    std::string sideLabel; // label drawn on side of bracket
};

/// All the bracket tiers for one plot.
/// tiers[0] = innermost (Mperp), tiers.back() = outermost (Run)
/// Only tiers with >1 unique value are included.
struct BracketTierSet {
    std::vector<std::vector<BracketSpan>> tiers;
    std::vector<std::string>              tierNames;  // for debugging
};

/// Build bracket tiers from the sorted bin list.
/// Tiers are added only when they have more than one distinct value
/// across the bins, so a single-Run plot won't waste a row on "Run 2".
inline BracketTierSet BuildBracketTiers(
    const std::vector<std::string>& sortedBins,
    const std::unordered_map<std::string,
    const YamlBinPattern*>& binLookup,
    const std::map<std::string, std::set<int>>& risrNeighborMap,
    const YamlBinPattern* groupPattern 
   ){

    int n = (int)sortedBins.size();
    if (n == 0) return {};

    // Pre-extract keys
    std::vector<BinTokens::BinKey> keys;
    keys.reserve(n);
    for (auto& b : sortedBins) keys.push_back(BinTokens::MakeKey(b));

    // Helper: build spans for a given tier-value function
    auto BuildSpans = [&](std::function<std::string(int)> labelFn,
                          std::function<bool(int,int)>   sameGroup) -> std::vector<BracketSpan>
    {
        std::vector<BracketSpan> spans;
        int start = 0;
        for (int i = 1; i <= n; ++i) {
            if (i == n || !sameGroup(i-1, i)) {
                BracketSpan sp;
                sp.binFirst = start + 1;
                sp.binLast  = i;
                sp.label    = labelFn(start);
                if (!sp.label.empty())
                    spans.push_back(sp);
                start = i;
            }
        }
        return spans;
    };

    // Helper: skip tier if all spans have same label (only 1 unique value)
    auto AllSame = [](const std::vector<BracketSpan>& spans) {
        if (spans.size() <= 1) return true;
        for (size_t i = 1; i < spans.size(); ++i)
            if (spans[i].label != spans[0].label) return false;
        return true;
    };

    BracketTierSet out;

    // --- Tier: Mperp (innermost) ---
    {
        // First pass: collect spans with their raw mperp keys
        struct MperpSpanRaw { int binFirst; int binLast; double mkey; };
        std::vector<MperpSpanRaw> rawSpans;
        int start = 0;
        for (int i = 1; i <= n; ++i) {
            bool newGroup = (i == n) || !(
                keys[i].run   == keys[i-1].run   &&
                keys[i].lep   == keys[i-1].lep   &&
                keys[i].quality == keys[i-1].quality  &&
                keys[i].jets  == keys[i-1].jets   &&
                keys[i].ptisr == keys[i-1].ptisr  &&
                keys[i].risr  == keys[i-1].risr   &&
                keys[i].mperp == keys[i-1].mperp
            );
            if (newGroup) {
                rawSpans.push_back({start + 1, i, keys[start].mperp});
                start = i;
            }
        }
        // Second pass: build labels using the RIGHT neighbour as the upper bound.
        // After the sort reversal, spans go low-mperp (left) -> high-mperp (right),
        // so span[s+1].mkey gives the lower edge of the next bin = upper bound of span[s].
        std::vector<BracketSpan> spans;
        for (int s = 0; s < (int)rawSpans.size(); ++s) {
            double mkey     = rawSpans[s].mkey;
            // nextMkey: lower edge of the span to the RIGHT
            int nextMkey = FindNextMperpBoundary(
                           rawSpans[s].binFirst - 1,
                           keys);
            BracketSpan sp;
            sp.binFirst = rawSpans[s].binFirst;
            sp.binLast  = rawSpans[s].binLast;
            if (s == 0) sp.sideLabel = "M_{#perp}";
 
            if (mkey >= 1e5) {
                sp.label = "b-tag";
            } else if (mkey == 0.0) {
                // Mlt bin: upper bound is the lower edge of the next (right) span
                if (nextMkey > 0.0 && nextMkey < 1e5) {
                    std::ostringstream ss;
                    ss << "[0," << static_cast<int>(nextMkey) << "]";
                    sp.label = ss.str();
                } else {
                    sp.label = "M_{#perp}  low";
                }
            } else {
                int lo = static_cast<int>(mkey);
                if (nextMkey > 0.0 && nextMkey < 1e5) {
                    int hi = static_cast<int>(nextMkey);
                    std::ostringstream ss;
                    ss << "[" << lo << "," << hi << "]";
                    sp.label = ss.str();
                } else {
                    // Rightmost span -> no upper bound known
                    std::ostringstream ss;
                    ss << " > " << lo;
                    sp.label = ss.str();
                }
            }
 
            if (!sp.label.empty()) spans.push_back(sp);
        } 
        if (!AllSame(spans)) { out.tiers.push_back(spans); out.tierNames.push_back("Mperp"); }
    }

    // --- Tier: RISR ---
    {
        // First pass: collect raw spans with their risr value (same grouping as before)
        struct RisrSpanRaw { int binFirst; int binLast; int risr; };
        std::vector<RisrSpanRaw> rawRisr;
        {
            int start = 0;
            for (int i = 1; i <= n; ++i) {
                bool newGroup = (i == n) || !(
                    keys[i].run    == keys[i-1].run    &&
                    keys[i].lep    == keys[i-1].lep    &&
                    keys[i].quality == keys[i-1].quality &&
                    keys[i].jets   == keys[i-1].jets   &&
                    keys[i].ptisr  == keys[i-1].ptisr  &&
                    keys[i].risr   == keys[i-1].risr
                );
                if (newGroup) {
                    rawRisr.push_back({start + 1, i, keys[start].risr});
                    start = i;
                }
            }
        }
 
        // Second pass: build range labels.
        // Bins are sorted low-RISR -> high-RISR (ascending), so the upper bound
        // of span[s] is the lower bound of span[s+1]; the last span caps at 1.0.
        // Pre-compute: for this group's include patterns, what is the next R value
        // in the global YAML neighbor map?
        auto FindYamlNextRISR = [&](int maxRisr) -> int {
            std::regex rToken("_R(\\d+)(?=[^\\d]|$)");
            
            // Collect all R values from ALL shape keys that match any include 
            // pattern of this group (with R token wildcarded)
            std::set<int> allRvals;
            
            for (const auto& inc : groupPattern->include) {
                std::smatch m;
                if (std::regex_search(inc, m, rToken)) {
                    std::string key = std::regex_replace(inc, rToken, "_R{X}");
                    auto it = risrNeighborMap.find(key);
                    if (it != risrNeighborMap.end()) {
                        allRvals.insert(it->second.begin(), it->second.end());
                    }
                }
            }
            
            // Find the first value greater than maxRisr
            auto pos = allRvals.upper_bound(maxRisr);
            if (pos != allRvals.end())
                return *pos;
            
            return -1; // no successor -> caller uses 1.0
        }; 
        std::vector<BracketSpan> spans;
        for (int s = 0; s < (int)rawRisr.size(); ++s) {
            int lo = rawRisr[s].risr;
            if (lo >= 999) continue;
        
            double nextRisr = FindNextRISRBoundary(rawRisr[s].binFirst - 1, keys);
        
            double hi;
            if (nextRisr < 999) {
                // Neighbor exists within this group's bins
                hi = nextRisr / 100.0;
            } else {
                // No in-group neighbor: ask the YAML map what comes after the max R token
                int yamlSucc = FindYamlNextRISR(lo);
                hi = (yamlSucc > 0) ? yamlSucc / 100.0 : 1.0;
            }
        
            std::ostringstream ss;
            ss << "[" << (lo / 100.0) << "," << hi << "]";
            BracketSpan sp;
            sp.binFirst  = rawRisr[s].binFirst;
            sp.binLast   = rawRisr[s].binLast;
            sp.label     = ss.str();
            if (s == 0) sp.sideLabel = "R_{ISR}";
            spans.push_back(sp);
        }
        if (!AllSame(spans)) { out.tiers.push_back(spans); out.tierNames.push_back("RISR"); }
    }
    // --- Tier: PTISR ---
    {
        // First pass: collect raw spans with their ptisr value
        struct PTISRSpanRaw { int binFirst; int binLast; int ptisr; };
        std::vector<PTISRSpanRaw> rawPTISR;

        {
            int start = 0;
            for (int i = 1; i <= n; ++i) {
                bool newGroup = (i == n) || !(
                    keys[i].run    == keys[i-1].run    &&
                    keys[i].lep    == keys[i-1].lep    &&
                    keys[i].quality == keys[i-1].quality &&
                    keys[i].jets   == keys[i-1].jets   &&
                    keys[i].ptisr  == keys[i-1].ptisr
                );

                if (newGroup) {
                    rawPTISR.push_back({start + 1, i, keys[start].ptisr});
                    start = i;
                }
            }
        }

        // Second pass: build labels using next PTISR threshold
        std::vector<BracketSpan> spans;

        for (int s = 0; s < (int)rawPTISR.size(); ++s) {
        
            int lo = rawPTISR[s].ptisr;
            if (lo >= 9999) continue;
        
            BracketSpan sp;
            sp.binFirst = rawPTISR[s].binFirst;
            sp.binLast  = rawPTISR[s].binLast;
        
            std::ostringstream ss;
        
            bool hasNext =
                (s + 1 < (int)rawPTISR.size()) &&
                (rawPTISR[s + 1].ptisr < 9999);
        
            bool sameParent = false;
        
            if (hasNext) {
                sameParent =
                    keys[rawPTISR[s].binFirst - 1].run     ==
                    keys[rawPTISR[s+1].binFirst - 1].run &&
        
                    keys[rawPTISR[s].binFirst - 1].lep     ==
                    keys[rawPTISR[s+1].binFirst - 1].lep &&
        
                    keys[rawPTISR[s].binFirst - 1].quality ==
                    keys[rawPTISR[s+1].binFirst - 1].quality &&
        
                    keys[rawPTISR[s].binFirst - 1].jets    ==
                    keys[rawPTISR[s+1].binFirst - 1].jets;
            }
        
            if (hasNext && sameParent) {
        
                int hi = rawPTISR[s + 1].ptisr;
                ss << "[" << lo << "," << hi << "]";
        
            } else {
        
                // Terminal PTISR bin within this parent grouping
                ss << " > " << lo;
            }
        
            sp.label = ss.str();
            if (s == 0) sp.sideLabel = "p_{T}^{ISR}";
            spans.push_back(sp);
        }

        if (!AllSame(spans)) {
            out.tiers.push_back(spans);
            out.tierNames.push_back("PTISR");
        }
    }
    // --- Tier: Jets ---
    {
        auto spans = BuildSpans(
            [&](int i){ return BinLabels::JetsLabel(keys[i].jets); },
            [&](int i, int j){
                return keys[i].run == keys[j].run &&
                       keys[i].lep == keys[j].lep &&
                       keys[i].quality == keys[j].quality &&
                       keys[i].jets == keys[j].jets;
            });
        if (!AllSame(spans)) { out.tiers.push_back(spans); out.tierNames.push_back("Jets"); }
    }

    // --- Tier: Quality ---
    {
        auto spans = BuildSpans(
            [&](int i){ return BinLabels::QualityLabel(keys[i].quality); },
            [&](int i, int j){
                return keys[i].run == keys[j].run &&
                       keys[i].lep == keys[j].lep &&
                       keys[i].quality == keys[j].quality;
            });
        if (!AllSame(spans)) { out.tiers.push_back(spans); out.tierNames.push_back("Quality"); }
    }

    // --- Tier: Lepton multiplicity ---
    {
        auto spans = BuildSpans(
            [&](int i){ return BinLabels::LepLabel(keys[i].lep); },
            [&](int i, int j){
                return keys[i].run == keys[j].run &&
                       keys[i].lep == keys[j].lep;
            });
        if (!AllSame(spans)) { out.tiers.push_back(spans); out.tierNames.push_back("Lep"); }
    }

    // --- Tier: Run (outermost) ---
    {
        auto spans = BuildSpans(
            [&](int i){ return BinLabels::RunLabel(keys[i].run); },
            [&](int i, int j){ return keys[i].run == keys[j].run; });
        if (!AllSame(spans)) { out.tiers.push_back(spans); out.tierNames.push_back("Run"); }
    }

    return out;
}

inline void DrawBinAxisBrackets(TPad* pad,
                                TH1* axisHist,
                                const std::vector<std::string>& sortedBins,
                                const std::unordered_map<std::string,
                                const YamlBinPattern*>& binLookup,
                                const std::map<std::string, std::set<int>>& risrNeighborMap,
                                const YamlBinPattern* groupPattern,
                                double bottomFrac = 0.95, double tickClearance = -1.0
)
{
    if (!pad || !axisHist || sortedBins.empty()) return;
    int n = (int)sortedBins.size();

    // ---- 1. Build short labels and detect if they are all identical ----
    std::vector<std::string> shortLabels;
    shortLabels.reserve(n);
    for (auto& b : sortedBins)
        shortLabels.push_back(BinLabels::ShortBinLabel(b));

    bool allSameTickLabel = true;
    for (int i = 1; i < n; ++i)
        if (shortLabels[i] != shortLabels[0]) { allSameTickLabel = false; break; }

    // ---- 2. Set per-bin tick labels ----
    for (int i = 0; i < n; ++i)
        axisHist->GetXaxis()->SetBinLabel(i + 1, allSameTickLabel ? "" : shortLabels[i].c_str());

    double padLeft  = pad->GetLeftMargin();
    double padRight = 1.0 - pad->GetRightMargin();
    double padBot   = pad->GetBottomMargin();

    bool verticalLabels = false;
    double labelSize = 0.0;
    double binWidth = (padRight - padLeft) / n;

    if (allSameTickLabel) {
        axisHist->GetXaxis()->SetLabelSize(0.0001);
        axisHist->GetXaxis()->SetLabelOffset(999);
    } else {
        verticalLabels = (n > 30);
        //labelSize = (n > 40) ? 0.055 : 0.085;
        // Scale label size with bin width; the 0.75 aspect factor converts
        // NDC width -> NDC height. Clamp so it never gets unreadably large
        // or small regardless of bin count.
        labelSize = std::min(0.090, std::max(0.085, binWidth * 4.0));
        if (verticalLabels) axisHist->GetXaxis()->LabelsOption("v");
        axisHist->GetXaxis()->SetLabelSize(labelSize);
    }

    pad->cd();
    pad->Update();

    // ---- 3. Build bracket tiers ----
    BracketTierSet tiers = BuildBracketTiers(sortedBins, binLookup, risrNeighborMap, groupPattern);
    int nTiers = (int)tiers.tiers.size();
    if (nTiers == 0) return;

    // ---- 4. Geometry ----

    if (allSameTickLabel) {
        // Tick labels are invisible: reclaim almost all of their space.
        // Just leave a tiny gap so we don't clip the axis line itself.
        tickClearance = 0.005;
    } else if (tickClearance < 0.0) {
        double labelOffset = axisHist->GetXaxis()->GetLabelOffset();
        if (labelOffset <= 0.0) labelOffset = 0.005;
        if (verticalLabels) {
            size_t maxChars = 1;
            for (auto& lbl : shortLabels)
                if (lbl.size() > maxChars) maxChars = lbl.size();
            tickClearance = labelOffset + labelSize * maxChars * 0.55 + 0.01;
        } else {
            tickClearance = labelOffset + labelSize * 1.01 + 0.001;
        }
        tickClearance = std::min(tickClearance, 0.5 * padBot);
    }

    double bracketZoneTop = padBot - tickClearance;
    double tierHeight = (bottomFrac * bracketZoneTop) / nTiers;
    if (bracketZoneTop - nTiers * tierHeight < 0.0)
        tierHeight = bracketZoneTop / nTiers;

    auto BinCenterNDC = [&](int bin1) -> double {
        return padLeft + (bin1 - 0.5) * binWidth;
    };

    // ---- 5. Draw bracket tiers ----
    // tiers[0] = innermost, tiers.back() = outermost.
    // When all tick labels are identical, the outermost tier's top horizontal
    // line is redundant (nothing to group above it), so we skip just that line
    // while still drawing the label. Everything shifts up naturally because
    // bracketZoneTop is already higher due to the collapsed tickClearance.
    for (int t = 0; t < nTiers; ++t) {
        double yTop  = bracketZoneTop - t * tierHeight;
        double yBot  = yTop - tierHeight;
        double yLine = yTop - 0.12 * tierHeight;
        double yTick = yLine + 0.30 * tierHeight;

        bool isTopMost = (t == 0);
        bool suppressTopLine = allSameTickLabel && isTopMost;

        for (const auto& span : tiers.tiers[t]) {
            double xL   = BinCenterNDC(span.binFirst) - 0.5 * binWidth + 0.004;
            double xR   = BinCenterNDC(span.binLast)  + 0.5 * binWidth - 0.004;
            double xMid = 0.5 * (xL + xR);

            if (!suppressTopLine) {
                TLine* hline = new TLine(xL, yLine, xR, yLine);
                hline->SetNDC(); hline->SetLineColor(kGray+2); hline->SetLineWidth(1); hline->Draw();

                TLine* ltick = new TLine(xL, yLine, xL, yTick);
                ltick->SetNDC(); ltick->SetLineColor(kGray+2); ltick->SetLineWidth(1); ltick->Draw();

                TLine* rtick = new TLine(xR, yLine, xR, yTick);
                rtick->SetNDC(); rtick->SetLineColor(kGray+2); rtick->SetLineWidth(1); rtick->Draw();
            }
            double spanFrac = xR - xL;
            double labelPadScale = 0.08;
            if (span.label.find("^") != std::string::npos
            ||  span.label.find("_") != std::string::npos)
                labelPadScale = 0.13;
            double labelPad = labelPadScale * tierHeight;
            double minSize = 0.1;
            double tsize;
            if (suppressTopLine) {
                tsize = std::min(minSize, std::max(0.030, binWidth * 0.8));
            } else {
                tsize = std::max(0.02, std::min(minSize, spanFrac * 0.65));
                tsize += 0.004 * t;
                // Floor: never smaller than a reasonable readable size regardless of span width
                double minByBinWidth = std::min(minSize, std::max(0.040, binWidth * 0.8));
                tsize = std::max(tsize, minByBinWidth);
                double maxByHeight = (yLine - yBot - 2.0 * labelPad);
                tsize = std::min(tsize, std::max(0.02, maxByHeight));
                tsize = std::min(tsize, minSize);
            } 
            double yText = suppressTopLine
                ? 0.5 * (yTop + yBot)
                : 0.5 * (yLine + yBot) - 0.001;
            
            TLatex* tex = new TLatex(xMid, yText, span.label.c_str());
            tex->SetNDC(); tex->SetTextFont(42); tex->SetTextSize(tsize);
            tex->SetTextAlign(22); tex->Draw();
            if (!span.sideLabel.empty()) {
                double xSide = padLeft - 0.01;
                TLatex* side = new TLatex(xSide, yText, span.sideLabel.c_str());
                side->SetNDC();
                side->SetTextFont(42);
                side->SetTextSize(tsize);
                side->SetTextAlign(32);
                side->Draw();
            }
        }
    }

    pad->Modified();
    pad->Update();
}

std::vector<std::string> RecoverBinLabels(TH1* h) {
    std::vector<std::string> out;
    if (!h) return out;
    for (int i = 1; i <= h->GetNbinsX(); ++i)
        out.push_back(h->GetXaxis()->GetBinLabel(i));
    return out;
}

std::vector<MergedBinGroup>
BuildMergedBinGroupsFromYaml(const std::vector<std::string>& allBins,
                             const YamlConfig& cfg)
{
    std::vector<MergedBinGroup> out;
    for (const auto& b : cfg.bins) {
        MergedBinGroup g;
        g.group_name = b.name;
        g.pattern    = b;
        for (const auto& bin : allBins) {
            bool included = false;
            for (const auto& inc : b.include) {
                std::regex incRegex(WildcardToRegex(inc));
                if (std::regex_match(bin, incRegex)) {
                    included = true;
                    break;
                }
            }
            if (!included) continue;
            bool excluded = false;
            for (const auto& ex : b.exclude) {
                std::regex exRegex(WildcardToRegex(ex));
                if (std::regex_search(bin, exRegex)) {
                    excluded = true;
                    break;
                }
            }
            if (!excluded)
                g.bin_names.push_back(bin);
        }
        if (!g.bin_names.empty()){
            SortBinNames(g.bin_names);
            out.push_back(g);
        }
    }
    return out;
}

struct CombinedBinHists {
    map<string, unique_ptr<TH1>> bkg;      // backgrounds
    map<string, unique_ptr<TH1>> signal;   // signals
    map<string, unique_ptr<TH1>> data;     // data
};

struct BinContent { double content; double error; };

template <typename T>
void ApplyMergingRules(
    std::map<std::string, std::vector<T>>& contents,
    const std::vector<YamlProcessPattern>& rules)
{
    if (contents.empty()) return;

    std::map<std::string, std::vector<T>> result;
    std::unordered_set<std::string> consumed;

    // Loop over YAML-defined merge rules
    for (const auto& rule : rules) {

        const std::string& target = rule.name;
        const auto& includes       = rule.include;
        const auto& excludes       = rule.exclude;

        std::vector<std::string> matches;

        // Find matching process keys
        for (const auto& kv : contents) {
            const std::string& key = kv.first;
            if (consumed.count(key)) continue;

            bool match = false;

            // include patterns
            for (const auto& inc : includes) {
                if (fnmatch(inc.c_str(), key.c_str(), 0) == 0) {
                    match = true;
                    break;
                }
            }
            if (!match) continue;

            // exclude patterns
            for (const auto& ex : excludes) {
                if (fnmatch(ex.c_str(), key.c_str(), 0) == 0) {
                    match = false;
                    break;
                }
            }

            if (match)
                matches.push_back(key);
        }

        if (matches.empty()) continue;

        // Consistency check
        size_t n = contents.at(matches.front()).size();
        for (const auto& m : matches) {
            if (contents.at(m).size() != n) {
                std::cerr << "[ApplyMergingRules] Inconsistent bin sizes for '"
                          << target << "'\n";
                matches.clear();
                break;
            }
        }
        if (matches.empty()) continue;

        // Build merged vector
        std::vector<T> mergedVals(n);
        for (size_t i = 0; i < n; ++i) {
            mergedVals[i].content = 0.0;
            mergedVals[i].error   = 0.0;
        }

        // Combine content + errors
        for (const auto& m : matches) {
            const auto& vec = contents.at(m);
            for (size_t i = 0; i < n; ++i) {
                mergedVals[i].content += vec[i].content;
                mergedVals[i].error    = std::hypot(mergedVals[i].error, vec[i].error);
            }
        }

        // Save and mark consumed
        result[target] = std::move(mergedVals);
        for (const auto& m : matches) consumed.insert(m);
    }

    // Copy unmerged keys
    for (const auto& kv : contents) {
        if (!consumed.count(kv.first))
            result[kv.first] = kv.second;
    }

    contents.swap(result);
}

CombinedBinHists LoadAndCombineBinHists(TDirectory* treeDir,
                                        const std::string& binTag,
                                        const std::vector<std::string>& binsToCombine,
                                        const std::vector<YamlProcessPattern>& processMerges
){
    CombinedBinHists out;

    if (!treeDir) {
        std::cerr << "[ERROR] Invalid TDirectory pointer passed to LoadAndCombineBinHists\n";
        return out;
    }

    auto MakeHistName = [](const std::string& bin, const std::string& proc) { return SanitizeString(bin)+"__"+proc+"__FD"; };

    // Map from process name to vector of bin contents/errors
    std::map<std::string, std::vector<BinContent>> bkgContents;
    std::map<std::string, std::vector<BinContent>> sigContents;
    std::map<std::string, std::vector<BinContent>> dataContents;
    std::vector<std::string> finalBinLabels;

    for (size_t binIndex = 0; binIndex < binsToCombine.size(); ++binIndex) {
        const auto& bin = binsToCombine[binIndex];
        TDirectory* binDir = dynamic_cast<TDirectory*>(treeDir->Get(bin.c_str()));
        if (!binDir) {
            std::cerr << "[warning] Bin directory '" << bin << "' not found, skipping.\n";
            continue;
        }
        finalBinLabels.push_back(bin);
    
        TIter next(binDir->GetListOfKeys());
        TKey* key;
        while ((key = (TKey*)next())) {
            TObject* obj = key->ReadObj();
            if (!obj) continue;
    
            std::string procName = obj->GetName();
    
            // Handle data_obs TGraphAsymmErrors
            if (obj->InheritsFrom(TGraphAsymmErrors::Class())) {
                auto g = dynamic_cast<TGraphAsymmErrors*>(obj);
                if (!g) continue;
    
                double y = g->GetY()[0];
                double e = 0.5*(g->GetEYlow()[0] + g->GetEYhigh()[0]);
    
                auto& vec = dataContents[procName];
                if (vec.size() < binIndex + 1) vec.resize(binIndex + 1, {0.0,0.0});
                vec[binIndex] = {y, e};
                continue;
            }
    
            // Only process TH1 objects
            if (!obj->InheritsFrom(TH1::Class())) continue;
            TH1* h = dynamic_cast<TH1*>(obj);
            if (!h) continue;
            if (h->GetBinContent(1) < 1.e-7) continue; // skip procs with small (~0) yields
    
            // Skip totals/covariance
            if (procName.find("total") != std::string::npos) continue;
    
            bool isSignal = (procName.find("SMS") != std::string::npos || procName.find("Cascade") != std::string::npos);
    
            auto& targetContents = isSignal ? sigContents : bkgContents;
            double content = h->GetBinContent(1);
            double error = h->GetBinError(1);
    
            auto& vec = targetContents[procName];
            if (vec.size() < binIndex + 1) vec.resize(binIndex + 1, {0.0,0.0});
            vec[binIndex] = {content, error};
        }
        // for any procs that did not appear, pad with hist with 0 entries
        auto PadToBin = [](auto& contentsMap, size_t binIndex) {
            for (auto& kv : contentsMap) {
                auto& vec = kv.second;
                if (vec.size() < binIndex + 1)
                    vec.resize(binIndex + 1, {0.0, 0.0});
            }
        };
        PadToBin(bkgContents, binIndex);
        PadToBin(sigContents, binIndex);
        PadToBin(dataContents, binIndex);
    }

    ApplyMergingRules(bkgContents, processMerges);

    // Now build concatenated histograms with bin labels
    auto BuildConcatHist = [&](const std::string& name,
                                const std::vector<BinContent>& vals,
                                const std::vector<std::string>& labels) -> std::unique_ptr<TH1>
    {
        int nbins = vals.size();
        TH1D* h = new TH1D(name.c_str(), name.c_str(), nbins, 0.5, nbins + 0.5);
    
        for (int i = 0; i < nbins; ++i) {
            h->SetBinContent(i + 1, vals[i].content);
            h->SetBinError(i + 1, vals[i].error);
            if (i < (int)labels.size())
                h->GetXaxis()->SetBinLabel(i + 1, labels[i].c_str());
        }
    
        h->SetDirectory(nullptr);
        return std::unique_ptr<TH1>(h);
    };

    for (auto& [proc, vals] : bkgContents) out.bkg[proc] = BuildConcatHist(MakeHistName(binTag, proc), vals, finalBinLabels);
    for (auto& [proc, vals] : sigContents) out.signal[proc] = BuildConcatHist(MakeHistName(binTag, proc), vals, finalBinLabels);
    for (auto& [proc, vals] : dataContents) out.data[proc] = BuildConcatHist(MakeHistName(binTag, proc), vals, finalBinLabels);
    return out;
}

std::map<std::string, std::map<std::string, std::map<std::string, TH1*>>>
BuildMergedJsonCutflow(
    const std::map<std::string, std::map<std::string, TH1*>> &cutflowMap,
    const std::vector<MergedBinGroup> &groups,
    const YamlConfig &cfg
){
    // return: groupName -> ( binName -> ( procName -> TH1* ) )
    std::map<std::string, std::map<std::string, std::map<std::string, TH1*>>> out;

    // Helper: merge processes inside a single bin according to YAML rules
    auto MergeProcessesInBin = [&](const std::map<std::string, BinContent> &binContents)
        -> std::map<std::string, BinContent>
    {
        std::map<std::string, BinContent> result;
        std::unordered_set<std::string> consumed;

        for (const auto &rule : cfg.process_merges) {
            const std::string &target = rule.name;
            const auto &includes = rule.include;
            const auto &excludes = rule.exclude;

            std::vector<std::string> matches;
            for (const auto &kv : binContents) {
                const std::string &proc = kv.first;
                if (consumed.count(proc)) continue;

                bool ok = false;
                // include patterns
                for (const auto &inc : includes) {
                    if (fnmatch(inc.c_str(), proc.c_str(), 0) == 0) { ok = true; break; }
                }
                if (!ok) continue;

                // exclude patterns
                for (const auto &ex : excludes) {
                    if (fnmatch(ex.c_str(), proc.c_str(), 0) == 0) { ok = false; break; }
                }
                if (ok) matches.push_back(proc);
            }

            if (matches.empty()) continue;

            BinContent merged{0.0, 0.0};
            for (const auto &m : matches) {
                const auto &bc = binContents.at(m);
                merged.content += bc.content;
                merged.error = std::hypot(merged.error, bc.error);
            }
            result[target] = merged;
            for (const auto &m : matches) consumed.insert(m);
        }

        // copy any unmerged procs
        for (const auto &kv : binContents) {
            if (!consumed.count(kv.first)) result[kv.first] = kv.second;
        }

        return result;
    };

    // Loop over groups
    for (const auto &grp : groups) {
        const std::string groupName = grp.group_name;
        const auto &bins = grp.bin_names;

        // 1) Collect only existing bins (preserve order), warn about missing ones
        std::vector<std::string> existingBins;
        existingBins.reserve(bins.size());
        for (const auto &b : bins) {
            auto it = cutflowMap.find(b);
            if (it != cutflowMap.end()) existingBins.push_back(b);
            else {
                std::cerr << "[BuildMergedJsonCutflow] Warning: bin '" << b
                          << "' listed in group '" << groupName << "' not found in input; skipping\n";
            }
        }
        if (existingBins.empty()) {
            std::cerr << "[BuildMergedJsonCutflow] Warning: no bins found for group '" << groupName << "'; skipping\n";
            continue;
        }

        // 2) For each bin, compute merged per-bin yields (proc -> BinContent)
        //    and also collect the union of all merged proc names across the group.
        std::map<std::string, std::map<std::string, BinContent>> perBinMerged; // binName -> (proc -> BinContent)
        std::unordered_set<std::string> allMergedProcs;

        for (const auto &binName : existingBins) {
            std::map<std::string, BinContent> binContents;

            // read raw per-process values from cutflowMap[binName]
            const auto &procMap = cutflowMap.at(binName);
            for (const auto &kv : procMap) {
                const std::string proc = kv.first;
                TH1* h = kv.second;
                double c = 0.0, e = 0.0;
                if (h) {
                    // each input TH1 is 1-bin with yield in bin 1
                    c = h->GetBinContent(1);
                    e = h->GetBinError(1);
                }
                binContents[proc] = {c, e};
            }

            // Merge processes inside this bin only
            auto mergedBin = MergeProcessesInBin(binContents);

            // Save and record process names
            for (const auto &kv : mergedBin) {
                perBinMerged[binName][kv.first] = kv.second;
                allMergedProcs.insert(kv.first);
            }
        }

        // 3) Build groupCutflowMap: for each bin and for every proc in allMergedProcs,
        //    produce a single-bin TH1* (0 if proc missing in that bin).
        std::map<std::string, std::map<std::string, TH1*>> groupCutflowMap; // binName -> (proc -> TH1*)

        for (const auto &binName : existingBins) {
            for (const auto &proc : allMergedProcs) {
                double c = 0.0, e = 0.0;
                auto itb = perBinMerged.find(binName);
                if (itb != perBinMerged.end()) {
                    auto itp = itb->second.find(proc);
                    if (itp != itb->second.end()) {
                        c = itp->second.content;
                        e = itp->second.error;
                    }
                }
                std::string histName = Form("%s__%s__%s", groupName.c_str(), binName.c_str(), proc.c_str());
                TH1F* h = new TH1F(histName.c_str(), histName.c_str(), 1, 0, 1);
                h->SetBinContent(1, c);
                h->SetBinError(1, e);
                h->SetDirectory(nullptr);
                groupCutflowMap[binName][proc] = h;
            }
        }

        // 4) store into out
        out[groupName] = std::move(groupCutflowMap);
    } // end groups loop

    return out;
}

std::unordered_map<std::string, const YamlBinPattern*> BuildBinLookup(const YamlConfig& cfg)
{
    std::unordered_map<std::string, const YamlBinPattern*> map;

    for (const auto& b : cfg.bins) {
        map[b.name] = &b;
    }

    return map;
}

inline std::string BuildGroupTitle(const YamlBinPattern&           pattern,
                                   const BracketTierSet&           tiers,
                                   const std::vector<std::string>& binNames,
                                   const std::unordered_map<std::string, const YamlBinPattern*>& binLookup
                                  )
{
    std::unordered_set<std::string> bracketted(
        tiers.tierNames.begin(), tiers.tierNames.end());

    const std::string& nm = pattern.name;
    std::vector<std::string> parts;

    // --- Run ---
    if (!bracketted.count("Run")) {
        if      (nm.find("Run3") != std::string::npos) parts.push_back("Run-3");
        else if (nm.find("Run2") != std::string::npos) parts.push_back("Run-2");
    }

    // --- Lepton multiplicity ---
    if (!bracketted.count("Lep")) {
        std::smatch m;
        if (std::regex_search(nm, m, std::regex("_(\\d)L(?:_|$)")))
            parts.push_back(m[1].str() + "L");
    }

    // --- Grade ---
    if (!bracketted.count("Quality")) {
        if      (nm.find("Gold")   != std::string::npos) parts.push_back("Gold");
        else if (nm.find("Silver") != std::string::npos) parts.push_back("Silver");
        else if (nm.find("Bronze") != std::string::npos) parts.push_back("Bronze");
    }

    // --- Jets: check bin_names since some YAML names omit the jet token ---
    if (!bracketted.count("Jets")) {
        // Collect jet tokens actually present in the bins
        std::set<std::string> jetsSeen;
        for (const auto& bin : binNames) {
            if      (bin.find("0J")    != std::string::npos) jetsSeen.insert("0J");
            else if (bin.find("1J")    != std::string::npos) jetsSeen.insert("1J");
            else if (bin.find("Jincl") != std::string::npos) jetsSeen.insert("J-incl");
        }
        // Only emit if all bins agree on jet multiplicity (not a bracket variable)
        if (jetsSeen.size() == 1)
            parts.push_back(*jetsSeen.begin());
        // if size > 1, jets vary across bins bracket handles it (or suppress)
    }

    // --- PTISR ---
    if (!bracketted.count("PTISR")) {
        std::smatch m;
        std::regex re("_P(\\d+)");
        int selfBoundary = -1;
        auto itSelf = binLookup.find(nm);
        if (itSelf != binLookup.end()) {
            ExtractPTISRBoundary(*itSelf->second, selfBoundary);
        }
        std::string partnerName = SwapPTISRTag(nm);
        int partnerBoundary = -1;
        auto it = binLookup.find(partnerName);
        if (it != binLookup.end()) {
            ExtractPTISRBoundary(*it->second, partnerBoundary);
        }
        std::ostringstream ss;
        if (selfBoundary > 0 && partnerBoundary > 0 && selfBoundary < partnerBoundary) {
            int low  = std::min(selfBoundary, partnerBoundary);
            int high = std::max(selfBoundary, partnerBoundary);
            ss << "p_{T}^{ISR}: [" << low << ", " << high << "]";
            parts.push_back(ss.str());
        }
        else if (selfBoundary > 0) {
            ss << "p_{T}^{ISR} > " << selfBoundary;
            parts.push_back(ss.str());
        }
    }

    // --- RISR: extract from include patterns ---
    if (!bracketted.count("RISR")) {
        std::set<int> risrSeen;
        std::regex re("_R(\\d+)(?=[^\\d]|$)");
        for (const auto& inc : pattern.include) {
            std::smatch m;
            std::string s = inc;
            // collect all R tokens in this include string
            while (std::regex_search(s, m, re)) {
                risrSeen.insert(std::stoi(m[1].str()));
                s = m.suffix().str();
            }
        }
        if (risrSeen.size() == 1) {
            int v = *risrSeen.begin();
            double rval = (v >= 10) ? v / 100.0 : v / 10.0;
            std::ostringstream ss;
            ss << "R_{ISR} > " << rval;
            parts.push_back(ss.str());
        }
        // if size > 1, RISR varies bracket tier handles it
    }

    // --- Btag special case ---
    // Groups like Run2_Bronze_Btag have no PTISR/RISR tokens at all
    if (parts.empty() || nm.find("Btag") != std::string::npos) {
        if (nm.find("Btag") != std::string::npos)
            parts.push_back("b-tag");
    }

    std::string title;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) title += ", ";
        title += parts[i];
    }
    return title;
}

struct StackPlotInput {
    std::vector<TH1*> bkgHists;
    std::vector<TH1*> sigHists;
    TH1* dataHist = nullptr;
};

StackPlotInput ConvertToStackInput(const CombinedBinHists& mergedHists) {
    StackPlotInput out;

    // Background
    for (const auto& [name, hptr] : mergedHists.bkg)
        if (hptr) out.bkgHists.push_back(hptr.get());

    // Signal
    for (const auto& [name, hptr] : mergedHists.signal)
        if (hptr) out.sigHists.push_back(hptr.get());

    // Data
    if (!mergedHists.data.empty()) {
        auto it = mergedHists.data.begin();
        if (it->second) {
            out.dataHist = it->second.get();
        }
        out.sigHists.clear(); // remove signal if data exists to prevent accidental unblinding
    }
    return out;
}

struct HistId {
    string bin; string proc; string var; 
    bool operator<(const HistId& other) const {
        return std::tie(bin, proc, var) < std::tie(other.bin, other.proc, other.var);
    }
};

HistId ParseHistName(const std::string &name) {
    std::string s = name;
    // strip ";..." suffix
    size_t sem = s.find(';');
    if (sem != std::string::npos)
        s = s.substr(0, sem);
    // strip leading "can_" or "c_"
    if (s.rfind("can_", 0) == 0) s = s.substr(4);
    else if (s.rfind("c_", 0) == 0) s = s.substr(2);
    HistId out{"", "", ""};
    // split by "__"
    size_t first = s.find("__");
    if (first == std::string::npos) {
        out.var = s;
        return out;
    }
    size_t second = s.find("__", first + 2);
    if (second == std::string::npos) {
        // only one "__" -> treat as bin + var
        out.bin = s.substr(0, first);
        out.var = s.substr(first + 2);
        return out;
    }
    out.bin  = s.substr(0, first);
    out.proc = s.substr(first + 2, second - (first + 2));
    out.var  = s.substr(second + 2); 
    return out;
}

// Return just the bin name from the hist title
std::string ExtractBinName(const std::string &histName) {
    HistId id = ParseHistName(histName);
    return id.bin;
}

// Return just the process name from the hist title
std::string ExtractProcName(const std::string &histName) {
    HistId id = ParseHistName(histName);
    return id.proc;
}

// Return just the variable name from the hist title
std::string ExtractVarName(const std::string &histName) {
    HistId id = ParseHistName(histName);
    return id.var;
}

// Return true if the histogram belongs to a signal sample
bool IsSignalHist(const std::string &histName, const SampleTool &tool) {
    HistId id = ParseHistName(histName);
    for (const auto &sigKey : tool.SignalKeys) {
        if (id.proc.find(sigKey) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// Return true if the histogram belongs to a background sample
bool IsBkgHist(const std::string &histName, const SampleTool &tool) {
    HistId id = ParseHistName(histName);
    return tool.BkgDict.count(id.proc) > 0;
}

struct RatioPair {
    HistId numerator;
    HistId denominator;
};

enum class RatioKind {
    Generic,
    Efficiency
};

struct RatioDef {
    std::string name;
    std::string type;              // "1D", "2D"
    RatioKind kind = RatioKind::Generic;

    bool normalize = false;

    std::optional<std::pair<double,double>> y_range;
    std::optional<std::pair<double,double>> z_range;

    // Implicit expansion mode
    std::string numerator_var;
    std::string denominator_var;
    std::vector<std::string> processes; // empty = all
    std::vector<std::string> bins;      // empty = all

    // Explicit mode
    std::vector<RatioPair> map;
};

// ----------------------
// Ratio YAML loader
// ----------------------
std::vector<RatioDef> LoadRatioYAML(const std::string &filename) {
    std::vector<RatioDef> ratios;

    YAML::Node doc = YAML::LoadFile(filename);
    if(!doc["ratios"]) {
        std::cerr << "[LoadRatioYAML] No 'ratios:' key found in " << filename << std::endl;
        return ratios;
    }

    for(const auto &node : doc["ratios"]) {
        RatioDef r;

        if(node["name"]) r.name = node["name"].as<std::string>();
        if(node["type"]) r.type = node["type"].as<std::string>();
        if(node["kind"]) {
            std::string k = node["kind"].as<std::string>();
            if(k == "efficiency") r.kind = RatioKind::Efficiency;
        }

        if(node["normalize"]) r.normalize = node["normalize"].as<bool>();

        if(node["y_range"]) {
            auto yr = node["y_range"];
            if(yr.size() == 2) r.y_range = std::make_pair(yr[0].as<double>(), yr[1].as<double>());
        }
        if(node["z_range"]) {
            auto zr = node["z_range"];
            if(zr.size() == 2) r.z_range = std::make_pair(zr[0].as<double>(), zr[1].as<double>());
        }

        // --- Explicit map mode ---
        if(node["map"]) {
            for(const auto &m : node["map"]) {
                RatioPair p;
                if(m["numerator"]) {
                    auto num = m["numerator"];
                    p.numerator.var  = num["hist"] ? num["hist"].as<std::string>() : "";
                    p.numerator.proc = num["process"] ? num["process"].as<std::string>() : "";
                    p.numerator.bin  = num["bin"] ? num["bin"].as<std::string>() : "";
                }
                if(m["denominator"]) {
                    auto den = m["denominator"];
                    p.denominator.var  = den["hist"] ? den["hist"].as<std::string>() : "";
                    p.denominator.proc = den["process"] ? den["process"].as<std::string>() : "";
                    p.denominator.bin  = den["bin"] ? den["bin"].as<std::string>() : "";
                }
                r.map.push_back(p);
            }
        }
        // --- Implicit mode (efficiency style) ---
        else if(node["numerator"] && node["denominator"]) {
            r.numerator_var   = node["numerator"].as<std::string>();
            r.denominator_var = node["denominator"].as<std::string>();

            if(node["processes"]) {
                for(const auto &p : node["processes"])
                    r.processes.push_back(p.as<std::string>());
            }
            if(node["bins"]) {
                for(const auto &b : node["bins"])
                    r.bins.push_back(b.as<std::string>());
            }
        }

        ratios.push_back(r);
    }

    return ratios;
}

static std::string MakeGroupKeyForVar(const std::string &bin, const std::string &var) {
    if(!bin.empty()) return bin + "__" + var;
    return var;
}

// Helper: find histogram pointer(s) in groups that match a HistId
// If HistId.proc or HistId.bin is empty, these act as wildcards and this function
// returns all matching (bin,proc,TH1*) tuples as a vector of tuples.
struct HistMatch { std::string bin, proc; TH1* hist; };
std::vector<HistMatch> FindMatchingHists(
    const HistId &hid,
    const std::map<std::string, std::map<std::string, TH1*>> &groups,
    const std::set<std::string> &allBins,
    bool exact = false 
) {
    std::vector<HistMatch> out;

    // If bin specified -> single groupKey
    if(!hid.bin.empty()){
        std::string gk = MakeGroupKeyForVar(hid.bin, hid.var);
        auto it = groups.find(gk);
        if(it == groups.end()) return out;
        // if proc specified -> narrow
        if(!hid.proc.empty()){
            auto itp = it->second.find(hid.proc);
            if(itp != it->second.end()){
                out.push_back({hid.bin, hid.proc, itp->second});
            }
        } else {
            // wildcard proc -> return all procs in this group
            for(const auto &pp : it->second){
                out.push_back({hid.bin, pp.first, pp.second});
            }
        }
        return out;
    }

    // If bin not specified, search across allBins
    for(const auto &bin : allBins){
        std::string gk = MakeGroupKeyForVar(bin, hid.var);
        auto it = groups.find(gk);
        if(it == groups.end()) continue;
        if(!hid.proc.empty()){
            auto itp = it->second.find(hid.proc);
            if(itp != it->second.end()){
                out.push_back({bin, hid.proc, itp->second});
            }
        } else {
            for(const auto &pp : it->second){
                out.push_back({bin, pp.first, pp.second});
            }
        }
    }

    // --- ONLY fallback to var-only groups if exact==false ---
    if(!exact){
        auto it_varonly = groups.find(hid.var);
        if(it_varonly != groups.end()){
            if(!hid.proc.empty()){
                auto itp = it_varonly->second.find(hid.proc);
                if(itp != it_varonly->second.end()){
                    out.push_back({"", hid.proc, itp->second});
                }
            } else {
                for(const auto &pp : it_varonly->second){
                    out.push_back({"", pp.first, pp.second});
                }
            }
        }
    }
    return out;
}

template<typename T>
T* GetHistClone(TFile *f, const string &name) {
    T* h = dynamic_cast<T*>(f->Get(name.c_str()));
    if (!h) return nullptr;
    T* clone = dynamic_cast<T*>(h->Clone());
    clone->SetDirectory(nullptr);
    return clone;
}

bool HistsCompatible(TH1* num, const TH1* den, double tol=1) {
    if (!num || !den) return false;
    int nbins = num->GetNbinsX();
    if (nbins != den->GetNbinsX()) return false;
    // Check bin edges match exactly
    for (int i = 1; i <= nbins; ++i) {
        if (num->GetBinLowEdge(i) != den->GetBinLowEdge(i)) return false;
        if (num->GetBinWidth(i) != den->GetBinWidth(i)) return false;
    }
    bool hasNonZeroBin = false;
    for (int i = 1; i <= nbins; ++i) {
        double n = num->GetBinContent(i);
        double d = den->GetBinContent(i);
        // Catch invalid numbers
        if (!std::isfinite(n) || !std::isfinite(d)) return false;
        // TEff requires numerator <= denominator
        // Allow small tolerance and clamp numerator
        if (n > d) {
            if ((n - d) > tol) {
                // Too large an overshoot -> incompatible
                std::cerr << "[FAIL] bin " << i << " num=" << n << " den=" << d << std::endl;
                return false;
            }
            // Otherwise, clamp numerator to denominator
            num->SetBinContent(i, d);
            num->SetBinError(i, den->GetBinError(i));
        }
        if (d > 0) hasNonZeroBin = true;
    }
    // If all denominator bins are zero, skip
    return hasNonZeroBin;
}

// Divide two histograms bin-by-bin, optionally normalize to 1, returns new TH1/TH2
TH1* MakeRatioHist(TH1* hnum, TH1* hden, bool normalize = false) {
    if(!hnum || !hden) return nullptr;

    TH1* hratio = (TH1*)hnum->Clone();
    hratio->SetName(Form("%s_ratio", hnum->GetName()));
    hratio->SetTitle(hratio->GetName());
    hratio->Reset();
    if(normalize){
        if(hnum->Integral()>0) hnum->Scale(1./hnum->Integral());
        if(hden->Integral()>0) hden->Scale(1./hden->Integral());
    }

    if(hnum->InheritsFrom(TH2::Class())){
        TH2* h2num = dynamic_cast<TH2*>(hnum);
        TH2* h2den = dynamic_cast<TH2*>(hden);
        TH2* h2ratio = dynamic_cast<TH2*>(hratio);

        int nx = h2num->GetNbinsX();
        int ny = h2num->GetNbinsY();
        for(int ix=1; ix<=nx; ++ix){
            for(int iy=1; iy<=ny; ++iy){
                double n = h2num->GetBinContent(ix,iy);
                double d = h2den->GetBinContent(ix,iy);
                double val = (d!=0) ? n/d : 0.;
                h2ratio->SetBinContent(ix,iy,val);
                h2ratio->SetBinError(ix,iy,0.); // optional: could compute errors
            }
        }
    } else {
        int nb = hnum->GetNbinsX();
        for(int i=1;i<=nb;++i){
            double n = hnum->GetBinContent(i);
            double d = hden->GetBinContent(i);
            double val = (d!=0) ? n/d : 0.;
            hratio->SetBinContent(i,val);
            hratio->SetBinError(i,0.); // optional
        }
        //if(normalize){
        //    double s = hratio->Integral();
        //    if(s>0) hratio->Scale(1./s);
	//}
    }
    return hratio;
}

// --------------------------------------------------
// Sort background histograms and process names by total yield (descending)
// --------------------------------------------------
void SortByYield(
    std::vector<TH1*>& Hists,
    std::vector<std::string>& Procs)
{
    if(Hists.empty() || Hists.size() != Procs.size()) return;

    // Create vector of pairs {integral, index}
    std::vector<std::pair<double,int>> yields_idx;
    for(size_t i=0;i<Hists.size();++i)
        yields_idx.push_back({Hists[i]->Integral(), (int)i});

    // Sort descending
    std::sort(yields_idx.rbegin(), yields_idx.rend());

    // Reorder histograms and process names
    std::vector<TH1*> sortedHists;
    std::vector<std::string> sortedProcs;
    for(auto &p : yields_idx){
        sortedHists.push_back(Hists[p.second]);
        sortedProcs.push_back(Procs[p.second]);
    }

    Hists.swap(sortedHists);
    Procs.swap(sortedProcs);
}

// --------------------------------------------------
// Sort cutflow histograms and process names by last-bin yield (descending)
// --------------------------------------------------
void SortCutFlowsByLastBin(
    std::vector<TH1*>& cutflowHists,
    std::vector<std::string>& cutflowProcs)
{
    if(cutflowHists.empty() || cutflowHists.size() != cutflowProcs.size()) return;

    // Create vector of pairs {last-bin content, index}
    std::vector<std::pair<double,int>> yields_idx;
    for(size_t i=0;i<cutflowHists.size();++i){
        int lastBin = cutflowHists[i]->GetNbinsX();
        yields_idx.push_back({cutflowHists[i]->GetBinContent(lastBin), (int)i});
    }

    // Sort descending
    std::sort(yields_idx.rbegin(), yields_idx.rend());

    // Reorder histograms and process names
    std::vector<TH1*> sortedHists;
    std::vector<std::string> sortedProcs;
    for(auto &p : yields_idx){
        sortedHists.push_back(cutflowHists[p.second]);
        sortedProcs.push_back(cutflowProcs[p.second]);
    }

    cutflowHists.swap(sortedHists);
    cutflowProcs.swap(sortedProcs);
}

// ------------------ TH1 ------------------
void DrawLog(TH1* h, const char* opt = "",
             double fallbackMin = 1e-1, double rangeFactor = 1.2) {
    if (!h) return;
    double max = h->GetMaximum();
    if (max <= 0) {
        // std::cerr << "TH1: " << h->GetName() << " has no positive entries\n";
        h->Draw(opt);
        return;
    }
    if (h->GetMinimum() <= 0) h->SetMinimum(fallbackMin);
    h->Draw(opt);
    gPad->SetLogy(1);
    h->SetMaximum(max * rangeFactor);
    gPad->Update();
}

// ------------------ TH2 ------------------
void DrawLog(TH2* h, const char* opt = "",
             double fallbackMin = 1e-1, double rangeFactor = 1.2) {
    if (!h) return;
    double max = h->GetMaximum();
    if (max <= 0) {
        // std::cerr << "TH2: " << h->GetName() << " has no positive entries\n";
        h->Draw(opt);
        return;
    }
    if (h->GetMinimum() <= 0) h->SetMinimum(fallbackMin);
    h->Draw(opt);
    gPad->SetLogz(1);
    h->SetMaximum(max * rangeFactor);
    gPad->Update();
}

// ------------------ TEfficiency ------------------
void DrawLog(TEfficiency* e, const char* opt = "",
             double fallbackMin = 1e-1, double rangeFactor = 1.2) {
    if (!e) return;
    double ymin = 1e30, ymax = -1e30;
    for (int i = 1; i <= e->GetTotalHistogram()->GetNbinsX(); ++i) {
        double val = e->GetEfficiency(i);
        if (val > 0) {
            ymin = std::min(ymin, val);
            ymax = std::max(ymax, val);
        }
    }
    if (ymax <= 0 || ymin <= 0) {
        // std::cerr << "TEfficiency: " << e->GetName() << " has no positive values\n";
        e->Draw(opt);
        return;
    }
    e->Draw(opt);
    gPad->SetLogy(1);
    gPad->Update();
    e->GetPaintedGraph()->GetYaxis()->SetRangeUser(
        std::max(ymin / rangeFactor, fallbackMin), ymax * rangeFactor);
}

// ------------------ TGraph / TGraphAsymmErrors ------------------
void DrawLog(TGraph* g, const char* opt = "AP",
             double fallbackMin = 1e-1, double rangeFactor = 1.2) {
    if (!g) return;
    double ymin = 1e30, ymax = -1e30;
    for (int i = 0; i < g->GetN(); ++i) {
        double x, y;
        g->GetPoint(i, x, y);
        if (y > 0) {
            ymin = std::min(ymin, y);
            ymax = std::max(ymax, y);
        }
    }
    if (ymax <= 0 || ymin <= 0 || ymin > ymax) {
        // std::cerr << "TGraph: " << g->GetName() << " has no positive points\n";
        g->Draw(opt);
        return;
    }
    g->Draw(opt);
    gPad->SetLogy(1);
    gPad->Update();
    g->GetYaxis()->SetRangeUser(
        std::max(ymin / rangeFactor, fallbackMin), ymax * rangeFactor);
}

// ------------------ Smart Dispatcher ------------------
template <typename T>
void DrawLogSmart(T* obj, const char* opt = "",
                  double fallbackMin = 1e-1, double rangeFactor = 1.2) {
    DrawLog(obj, opt, fallbackMin, rangeFactor);
}

