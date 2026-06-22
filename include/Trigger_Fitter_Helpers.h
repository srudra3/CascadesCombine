// Trigger_Fitter_Helpers.h
// Fitting efficiency curves for MET trigger scale factors

#pragma once

#include <TMinuit.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <filesystem>
#include <algorithm>
#include <functional>
#include <TMatrixDSym.h>
#include <TGraphErrors.h>
#include <TGraphAsymmErrors.h>
#include <TPaveStats.h>
#include <TLine.h>
#include <TFitResult.h>
#include <TEfficiency.h>
#include <TMultiGraph.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TLatex.h>
#include <TStyle.h>
#include <TFile.h>
#include <TKey.h>
#include <TF1.h>
#include <Math/ProbFunc.h>
#include "nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

std::string CMS_label = "#bf{CMS} Preliminary";

// -----------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------
void Get_Fit(TGraphAsymmErrors*& gr, vector<TF1*> funcs, vector<int> colors,
             string outFile, string name, const string& outdir);
void Fit_Graph_With_Funcs(TCanvas*& canv, TGraphAsymmErrors*& gr,
                           vector<TF1*> funcs, const vector<int>& colors,
                           string name, const string& outdir);
TGraph*        Get_Fit_Ratio(double x_min, double x_max, TF1* Bkg_Nominal, TF1* Data_Nominal);
TGraphErrors*  Get_Bands(double x_min, double x_max, TF1* Data_Nominal, int N,
                          double& a1, double& a2, double& b1, double& b2,
                          double& c1, double& c2);
TGraphErrors*  Get_Bands_Ratio(double x_min, double x_max, TGraphAsymmErrors* gr,
                                TF1* Bkg_Nominal, TF1* Data_Nominal,
                                double& a1, double& a2, double& b1, double& b2,
                                double& c1, double& c2);
TGraphAsymmErrors* TGAE_Ratio(TGraphAsymmErrors* gr_bkg, TGraphAsymmErrors* gr_data);

// -----------------------------------------------------------------------
// Global flags
// -----------------------------------------------------------------------
bool invert_colors = false;

// -----------------------------------------------------------------------
// Filesystem helpers
// -----------------------------------------------------------------------
bool fileExists(const std::string& filename)
{
    struct stat buf;
    return (stat(filename.c_str(), &buf) != -1);
}

string Derive_Output_Dir(const string& fname)
{
    filesystem::path input(fname);
    filesystem::path outdir = input.parent_path().parent_path() / "trigger_sf";
    if (!filesystem::exists(outdir)) {
        filesystem::create_directories(outdir);
        cout << "Created output directory: " << outdir << endl;
    } else {
        cout << "Output directory: " << outdir << endl;
    }
    return outdir.string() + "/";
}

// -----------------------------------------------------------------------
// JSON parameter I/O
// -----------------------------------------------------------------------
const string JSON_NAME = "Fit_Parameters.json";
json Load_JSON(const string& path)
{
    json j;
    if (!fileExists(path)) return j;
    ifstream f(path);
    f >> j;
    return j;
}

void Save_JSON(const json& j, const string& path)
{
    ofstream f(path);
    f << j.dump(4) << endl;
}

void Write_Fit_Params_JSON(const string& key, const string& json_path,
                            double norm, double mean, double sigma,
                            double scale, double weight)
{
    json j = Load_JSON(json_path);
    j[key]["fit"]["norm"]   = norm;
    j[key]["fit"]["mean"]   = mean;
    j[key]["fit"]["sigma"]  = sigma;
    j[key]["fit"]["scale"]  = scale;
    j[key]["fit"]["weight"] = weight;
    Save_JSON(j, json_path);
}

void Write_Band_Params_JSON(const string& key, const string& json_path,
                             double a1, double a2,
                             double b1, double b2,
                             double c1, double c2)
{
    json j = Load_JSON(json_path);
    j[key]["bands"]["a1"] = a1;
    j[key]["bands"]["a2"] = a2;
    j[key]["bands"]["b1"] = b1;
    j[key]["bands"]["b2"] = b2;
    j[key]["bands"]["c1"] = c1;
    j[key]["bands"]["c2"] = c2;
    Save_JSON(j, json_path);
}

bool Read_Fit_Params_JSON(const string& key, const string& json_path,
                           double& norm, double& mean, double& sigma,
                           double& scale, double& weight)
{
    json j = Load_JSON(json_path);
    if (!j.contains(key) || !j[key].contains("fit")) {
        cout << "WARNING: No fit params in JSON for key: " << key << endl;
        return false;
    }
    auto& f = j[key]["fit"];
    norm   = f.value("norm",   1.0);
    mean   = f.value("mean",   200.0);
    sigma  = f.value("sigma",  40.0);
    scale  = f.value("scale",  0.0);
    weight = f.value("weight", 0.0);
    return true;
}

bool Read_Band_Params_JSON(const string& key, const string& json_path,
                            double& a1, double& a2,
                            double& b1, double& b2,
                            double& c1, double& c2)
{
    json j = Load_JSON(json_path);
    if (!j.contains(key) || !j[key].contains("bands")) {
        cout << "WARNING: No band params in JSON for key: " << key << endl;
        return false;
    }
    auto& b = j[key]["bands"];
    a1 = b.value("a1",  .4e-5);
    a2 = b.value("a2", -.6e-5);
    b1 = b.value("b1", 220.0);
    b2 = b.value("b2", 220.0);
    c1 = b.value("c1", 1.004);
    c2 = b.value("c2", 0.996);
    return true;
}

struct BandParams { double a1,a2,b1,b2,c1,c2; };
BandParams Auto_Scale_Bands(TGraphAsymmErrors* gr,
                             TF1* fit,
                             double x_min, double x_max,
                             double quantile  = 0.80,   // coverage fraction
                             double max_width = 0.10,   // hard cap: ±10% max
                             double min_width = 0.005)  // floor so band is always visible
{
    // ---- 1. Collect fractional residuals in the turn-on region ----
    // Only use points where the fit is between 5% and 95% efficiency
    // (plateau points add no info about the turn-on shape)
    struct Point { double x, resid_up, resid_dn; double w; };
    vector<Point> pts;

    int N = gr->GetN();
    for (int i = 0; i < N; i++) {
        double x, y;
        gr->GetPoint(i, x, y);
        if (x < x_min || x > x_max) continue;
        double fval = fit->Eval(x);
        if (fval < 0.05 || fval > 0.95) continue;

        // Fractional residual -> use error bar extents for the bound direction
        double err_hi = gr->GetErrorYhigh(i);
        double err_lo = gr->GetErrorYlow(i);
        // Upward deviation: point center + upper error bar
        double resid_up = (fval > 0.) ? ((y + err_hi) / fval - 1.0) : 0.;
        // Downward deviation: point center - lower error bar  
        double resid_dn = (fval > 0.) ? ((y - err_lo) / fval - 1.0) : 0.;

        // Weight by 1/fval so turn-on points (low efficiency, high scatter)
        // don't dominate over the shoulder -> want even coverage, not
        // a band that only chases the noisiest low-MET points
        double w = 1.0 / (fval * (1.0 - fval) + 1e-6);  // variance-like weight
        pts.push_back({x, resid_up, resid_dn, w});
    }

    if (pts.empty()) {
        cout << "  Auto_Scale_Bands: no turn-on points found, using defaults" << endl;
        return {4e-6, -6e-6, 220., 220., 1.01, 0.99};
    }

    // ---- 2. Weighted quantile of the residual magnitudes ----
    // Sort by upward residual, find the quantile-th weighted percentile
    // This is the "how wide does the band need to be" answer, ignoring outliers
    auto weighted_quantile = [](vector<pair<double,double>> vals_weights,
                                 double q) -> double {
        sort(vals_weights.begin(), vals_weights.end());
        double total_w = 0.;
        for (auto& [v,w] : vals_weights) total_w += w;
        double cumul = 0.;
        for (auto& [v,w] : vals_weights) {
            cumul += w;
            if (cumul >= q * total_w) return v;
        }
        return vals_weights.back().first;
    };

    vector<pair<double,double>> up_vals, dn_vals;
    for (auto& p : pts) {
        up_vals.push_back({ p.resid_up,  p.w});
        dn_vals.push_back({-p.resid_dn,  p.w});  // flip sign so both are positive magnitudes
    }

    double target_up = weighted_quantile(up_vals, quantile);
    double target_dn = weighted_quantile(dn_vals, quantile);

    // ---- 3. Hard caps ----
    target_up = std::clamp(target_up, min_width, max_width);
    target_dn = std::clamp(target_dn, min_width, max_width);

    // ---- 4. Vertex position: weighted mean x of turn-on points ----
    // Better than using fit Mean directly -> the Mean is where the CDF
    // is at 50% which can be outside the plot range. The weighted mean
    // of the actual turn-on points is where we have data and where
    // the band needs to be most accurate.
    double sum_wx = 0., sum_w = 0.;
    for (auto& p : pts) { sum_wx += p.w * p.x; sum_w += p.w; }
    double b = (sum_w > 0.) ? sum_wx / sum_w : fit->GetParameter(1);
    b = std::clamp(b, x_min, x_max);

    // ---- 5. Curvature: parabola must reach 0 residual at x_max ----
    // i.e. the band converges back to the fit curve in the plateau.
    // Solve: 0 = a*(x_max - b)^2 + target  =>  a = -target/(x_max-b)^2
    double dx = x_max - b;
    double a1 = (dx > 1.) ?  target_up / (dx * dx) : 4e-6;   // upper arm opens upward left of b
    double a2 = (dx > 1.) ? -target_dn / (dx * dx) : -6e-6;  // lower arm opens downward

    double c1 = 1.0 + target_up;  // vertex value (widest point of upper band)
    double c2 = 1.0 - target_dn;

    cout << "  Auto-scaled bands (" << pts.size() << " turn-on pts):"
         << " target_up=" << target_up << " target_dn=" << target_dn
         << " b=" << b << " a1=" << a1 << " a2=" << a2
         << " c1=" << c1 << " c2=" << c2 << endl;

    return {a1, a2, b, b, c1, c2};
}

// -----------------------------------------------------------------------
// Fit functions
// -----------------------------------------------------------------------
Double_t Gaussian_CDF_Func(Double_t *x, Double_t *par)
{
    return par[0]*ROOT::Math::normal_cdf(x[0], par[2], par[1]);
}

Double_t Double_Gaussian_CDF_Func_Add(Double_t *x, Double_t *par)
{
    return par[0]*( (TMath::Cos(par[4])*TMath::Cos(par[4]))*ROOT::Math::normal_cdf(x[0], par[2],        par[1])
                  + (TMath::Sin(par[4])*TMath::Sin(par[4]))*ROOT::Math::normal_cdf(x[0], par[2]+par[3], par[1]) );
}

Double_t Double_Gaussian_CDF_Func_Multi(Double_t *x, Double_t *par)
{
    return par[0]*( (TMath::Cos(par[4])*TMath::Cos(par[4]))*ROOT::Math::normal_cdf(x[0], par[2],        par[1])
                  + (TMath::Sin(par[4])*TMath::Sin(par[4]))*ROOT::Math::normal_cdf(x[0], par[2]*par[3], par[1]) );
}

// -----------------------------------------------------------------------
// Number formatting (used when writing fit results to JSON)
// -----------------------------------------------------------------------
double toPrecision(double num, int n)
{
    if (num == 0) return 0.0;
    double d = std::ceil(std::log10(num < 0 ? -num : num));
    int power = n - (int)d;
    double magnitude = std::pow(10., power);
    long shifted = std::round(num * magnitude);
    std::ostringstream oss;
    oss << shifted / magnitude;
    return atof(oss.str().c_str());
}

double Round(const double& num) { return toPrecision(num, 3); }

// -----------------------------------------------------------------------
// Write fit result to JSON  (replaces Output_Fit_ToCSV)
// -----------------------------------------------------------------------
void Output_Fit_ToJSON(TF1* func, const string& name, const string& status,
                        TFitResultPtr result, const string& json_path)
{
    json j = Load_JSON(json_path);
    json entry;
    entry["status"] = status;
    entry["chi2"]   = Round(func->GetChisquare());
    entry["ndf"]    = func->GetNDF();

    for (int k = 0; k < func->GetNpar(); k++) {
        string parName = func->GetParName(k);
        double val = 0., err = 0.;

        if (parName.find("Weight") != string::npos) {
            double p = func->GetParameter(k);
            double e = func->GetParError(k);
            val = Round(TMath::Cos(p)*TMath::Cos(p));
            err = Round(fabs(-TMath::Sin(2.*p)*e - TMath::Cos(2.*p)*e*e));
        } else if (parName.find("Scale") != string::npos) {
            double scale     = func->GetParameter(k);
            double sigma     = func->GetParameter(k-1);
            double scale_err = func->GetParError(k);
            double sigma_err = func->GetParError(k-1);
            TMatrixDSym cov  = result->GetCovarianceMatrix();
            double cov_ss    = cov(k, k-1);
            string fname_str = func->GetName();
            if (fname_str.find("Multi") != string::npos) {
                val = Round(scale * sigma);
                err = Round(sqrt(sigma*scale_err*sigma*scale_err
                               + sigma_err*scale*sigma_err*scale
                               + 2.*sigma*scale*cov_ss*cov_ss));
            } else if (fname_str.find("Add") != string::npos) {
                val = Round(scale + sigma);
                err = Round(sqrt(scale_err*scale_err + sigma_err*sigma_err));
            }
        } else {
            val = Round(func->GetParameter(k));
            err = Round(func->GetParError(k));
        }

        entry["parameters"][parName]["value"] = val;
        entry["parameters"][parName]["error"] = err;
    }

    // Store under name -> func_name so multiple functions per graph coexist
    j[name][func->GetName()] = entry;
    Save_JSON(j, json_path);
}

// -----------------------------------------------------------------------
// Graph formatting
// -----------------------------------------------------------------------
void Format_Graph(TMultiGraph*& gr)
{
    gr->GetXaxis()->CenterTitle(true);
    gr->GetXaxis()->SetTitleFont(132);
    gr->GetXaxis()->SetTitleSize(0.06);
    gr->GetXaxis()->SetTitleOffset(1.06);
    gr->GetXaxis()->SetLabelFont(132);
    gr->GetXaxis()->SetLabelSize(0.0);
    gr->GetYaxis()->CenterTitle(true);
    gr->GetYaxis()->SetTitleFont(132);
    gr->GetYaxis()->SetTitleSize(0.06);
    gr->GetYaxis()->SetTitleOffset(0.6);
    gr->GetYaxis()->SetLabelFont(132);
    gr->GetYaxis()->SetLabelSize(0.05);
    if (invert_colors) {
        gr->GetXaxis()->SetAxisColor(kWhite);  gr->GetYaxis()->SetAxisColor(kWhite);
        gr->GetXaxis()->SetTitleColor(kWhite); gr->GetYaxis()->SetTitleColor(kWhite);
        gr->GetXaxis()->SetLabelColor(kWhite); gr->GetYaxis()->SetLabelColor(kWhite);
    }
}

void Format_Graph_res(TMultiGraph*& gr)
{
    gr->GetXaxis()->CenterTitle(true);
    gr->GetXaxis()->SetTitleFont(132);
    gr->GetXaxis()->SetTitleSize(0.12);
    gr->GetXaxis()->SetTitleOffset(0.71);
    gr->GetXaxis()->SetLabelFont(132);
    gr->GetXaxis()->SetLabelSize(0.1);
    gr->GetYaxis()->CenterTitle(true);
    gr->GetYaxis()->SetTitleFont(132);
    gr->GetYaxis()->SetTitleSize(0.12);
    gr->GetYaxis()->SetTitleOffset(0.3);
    gr->GetYaxis()->SetLabelFont(132);
    gr->GetYaxis()->SetLabelSize(0.1);
    if (invert_colors) {
        gr->GetXaxis()->SetAxisColor(kWhite);  gr->GetYaxis()->SetAxisColor(kWhite);
        gr->GetXaxis()->SetTitleColor(kWhite); gr->GetYaxis()->SetTitleColor(kWhite);
        gr->GetXaxis()->SetLabelColor(kWhite); gr->GetYaxis()->SetLabelColor(kWhite);
    }
}

void Format_Graph_res(TGraphErrors*& gr)
{
    gr->GetXaxis()->CenterTitle(true);
    gr->GetXaxis()->SetTitleFont(132);
    gr->GetXaxis()->SetTitleSize(0.115);
    gr->GetXaxis()->SetTitleOffset(0.75);
    gr->GetXaxis()->SetLabelFont(132);
    gr->GetXaxis()->SetLabelSize(0.09);
    gr->GetYaxis()->CenterTitle(true);
    gr->GetYaxis()->SetTitleFont(132);
    gr->GetYaxis()->SetTitleSize(0.115);
    gr->GetYaxis()->SetTitleOffset(0.33);
    gr->GetYaxis()->SetLabelFont(132);
    gr->GetYaxis()->SetLabelSize(0.09);
    if (invert_colors) {
        gr->GetXaxis()->SetAxisColor(kWhite);  gr->GetYaxis()->SetAxisColor(kWhite);
        gr->GetXaxis()->SetTitleColor(kWhite); gr->GetYaxis()->SetTitleColor(kWhite);
        gr->GetXaxis()->SetLabelColor(kWhite); gr->GetYaxis()->SetLabelColor(kWhite);
    }
}

void Format_Graph(TGraphAsymmErrors*& gr)
{
    gr->GetXaxis()->CenterTitle(true);
    gr->GetXaxis()->SetTitleFont(132);
    gr->GetXaxis()->SetTitleSize(0.06);
    gr->GetXaxis()->SetTitleOffset(1.06);
    gr->GetXaxis()->SetLabelFont(132);
    gr->GetXaxis()->SetLabelSize(0.0);
    gr->GetYaxis()->CenterTitle(true);
    gr->GetYaxis()->SetTitleFont(132);
    gr->GetYaxis()->SetTitleSize(0.06);
    gr->GetYaxis()->SetTitleOffset(0.6);
    gr->GetYaxis()->SetLabelFont(132);
    gr->GetYaxis()->SetLabelSize(0.05);
    if (invert_colors) {
        gr->GetXaxis()->SetAxisColor(kWhite);  gr->GetYaxis()->SetAxisColor(kWhite);
        gr->GetXaxis()->SetTitleColor(kWhite); gr->GetYaxis()->SetTitleColor(kWhite);
        gr->GetXaxis()->SetLabelColor(kWhite); gr->GetYaxis()->SetLabelColor(kWhite);
    }
}

void Format_Graph(TGraphErrors*& gr)
{
    gr->GetXaxis()->CenterTitle(true);
    gr->GetXaxis()->SetTitleFont(132);
    gr->GetXaxis()->SetTitleSize(0.06);
    gr->GetXaxis()->SetTitleOffset(1.06);
    gr->GetXaxis()->SetLabelFont(132);
    gr->GetXaxis()->SetLabelSize(0.0);
    gr->GetYaxis()->CenterTitle(true);
    gr->GetYaxis()->SetTitleFont(132);
    gr->GetYaxis()->SetTitleSize(0.06);
    gr->GetYaxis()->SetTitleOffset(0.6);
    gr->GetYaxis()->SetLabelFont(132);
    gr->GetYaxis()->SetLabelSize(0.05);
    if (invert_colors) {
        gr->GetXaxis()->SetAxisColor(kWhite);  gr->GetYaxis()->SetAxisColor(kWhite);
        gr->GetXaxis()->SetTitleColor(kWhite); gr->GetYaxis()->SetTitleColor(kWhite);
        gr->GetXaxis()->SetLabelColor(kWhite); gr->GetYaxis()->SetLabelColor(kWhite);
    }
}

// -----------------------------------------------------------------------
// Residual graph: data points minus fit curve
// -----------------------------------------------------------------------
TGraphAsymmErrors* TGAE_TF1(TGraphAsymmErrors* gr, TF1* fit_func)
{
    int N = gr->GetN();
    double xnew[N], ynew[N];
    for (int i = 0; i < N; i++) {
        double x, y;
        gr->GetPoint(i, x, y);
        xnew[i] = x;
        ynew[i] = y - fit_func->Eval(x);
    }
    TGraphAsymmErrors* res_gr = new TGraphAsymmErrors(N, xnew, ynew);
    for (int i = 0; i < N; i++)
        res_gr->SetPointError(i, gr->GetErrorXlow(i), gr->GetErrorXhigh(i),
                                 gr->GetErrorYlow(i), gr->GetErrorYhigh(i));
    res_gr->SetTitle("");
    return res_gr;
}

// -----------------------------------------------------------------------
// Ratio of two efficiency graphs: data / bkg
// -----------------------------------------------------------------------
TGraphAsymmErrors* TGAE_Ratio(TGraphAsymmErrors* gr_bkg, TGraphAsymmErrors* gr_data)
{
    int N = gr_bkg->GetN();
    if (N != gr_data->GetN()) return nullptr;

    double xnew[N], ynew[N];
    for (int i = 0; i < N; i++) {
        double x_bkg, y_bkg, x_data, y_data;
        gr_bkg->GetPoint(i,  x_bkg,  y_bkg);
        gr_data->GetPoint(i, x_data, y_data);
        if (x_bkg != x_data) return nullptr;
        xnew[i] = x_bkg;
        ynew[i] = (y_bkg > 0.) ? y_data / y_bkg : 0.;
    }

    TGraphAsymmErrors* mg = new TGraphAsymmErrors(N, xnew, ynew);
    for (int i = 0; i < N; i++) {
        double x_bkg, y_bkg, x_data, y_data;
        gr_bkg->GetPoint(i,  x_bkg,  y_bkg);
        gr_data->GetPoint(i, x_data, y_data);
        double ratio   = (y_bkg > 0.) ? y_data / y_bkg : 0.;
        double bkg_h   = gr_bkg->GetErrorYhigh(i);
        double bkg_l   = gr_bkg->GetErrorYlow(i);
        double data_h  = gr_data->GetErrorYhigh(i);
        double data_l  = gr_data->GetErrorYlow(i);
        double err_l   = (y_data > 0. && y_bkg > 0.)
                         ? sqrt(data_l*data_l*ratio*ratio/(y_data*y_data)
                              + bkg_l*bkg_l*ratio*ratio/(y_bkg*y_bkg)) : 0.;
        double err_h   = (y_data > 0. && y_bkg > 0.)
                         ? sqrt(data_h*data_h*ratio*ratio/(y_data*y_data)
                              + bkg_h*bkg_h*ratio*ratio/(y_bkg*y_bkg)) : 0.;
        mg->SetPointError(i, gr_bkg->GetErrorXlow(i), gr_data->GetErrorXhigh(i), err_l, err_h);
    }
    return mg;
}

// -----------------------------------------------------------------------
// Systematic uncertainty bands
// -----------------------------------------------------------------------
TGraphErrors* Get_Bands_Ratio(double x_min, double x_max, TGraphAsymmErrors* /*gr*/,
                               TF1* Bkg_Nominal, TF1* Data_Nominal,
                               double& a1, double& a2,
                               double& b1, double& b2,
                               double& c1, double& c2)
{
    const int N = 1000;
    TGraphErrors* gr_bands_ratio = new TGraphErrors(N);
    double x_err = (x_max - x_min) / N;

    for (int i = 0; i < N; i++) {
        double x       = x_min + x_err * i;
        double y_upper = (x > b1) ? 1.015 : (a1*(x-b1)*(x-b1) + c1);
        double y_lower = (x > b2) ? 0.99  : (a2*(x-b2)*(x-b2) + c2);
        double ratio   = (Bkg_Nominal->Eval(x) > 0.)
                         ? Data_Nominal->Eval(x) / Bkg_Nominal->Eval(x) : 1.;
        y_upper *= ratio;
        y_lower *= ratio;
        double y     = (y_upper + y_lower) / 2.;
        double y_err = (y_upper - y_lower) / 2.;
        gr_bands_ratio->SetPoint(i, x, y);
        gr_bands_ratio->SetPointError(i, x_err, y_err);
    }
    return gr_bands_ratio;
}

TGraphErrors* Get_Bands(double x_min, double x_max, TF1* Data_Nominal, int N,
                         double& a1, double& a2,
                         double& b1, double& b2,
                         double& c1, double& c2)
{
    TGraphErrors* gr_bands = new TGraphErrors(N);
    double x_err = (x_max - x_min) / N;

    for (int i = 0; i < N; i++) {
        double x       = x_min + x_err * i;
        double y_upper = (x > b1) ? 1.01 : (a1*(x-b1)*(x-b1) + c1);
        double y_lower = (x > b2) ? 0.99 : (a2*(x-b2)*(x-b2) + c2);
        y_upper = std::min(1., Data_Nominal->Eval(x) * y_upper);
        y_lower = Data_Nominal->Eval(x) * y_lower;
        double y     = (y_upper + y_lower) / 2.;
        double y_err = (y_upper - y_lower) / 2.;
        if (y     > 1.) y     = 1.;
        if (y_err > 1.) y_err = 1.;
        gr_bands->SetPoint(i, x, y);
        gr_bands->SetPointError(i, x_err, y_err);
    }
    return gr_bands;
}

TGraph* Get_Fit_Ratio(double x_min, double x_max, TF1* Bkg_Nominal, TF1* Data_Nominal)
{
    const int N = 1000;
    TGraph* gr = new TGraph(N);
    double dx = (x_max - x_min) / N;
    for (int i = 0; i < N; i++) {
        double x = x_min + dx * i;
        double r = (Bkg_Nominal->Eval(x) > 0.)
                   ? Data_Nominal->Eval(x) / Bkg_Nominal->Eval(x) : 1.;
        gr->SetPoint(i, x, r);
    }
    return gr;
}

// -----------------------------------------------------------------------
// Core fitting routine -> outdir added so output files land in trigger_sf/
// -----------------------------------------------------------------------
void Fit_Graph_With_Funcs(TCanvas*& canv, TGraphAsymmErrors*& gr_given,
                           vector<TF1*> funcs, const vector<int>& colors,
                           string name, const string& outdir)
{
    gStyle->SetOptFit(1111);
    canv->cd();

    TPad* pad_gr = new TPad("pad_gr","pad_gr",0,.3,1.,1.);
    pad_gr->SetGridx(); pad_gr->SetGridy();
    pad_gr->Draw(); pad_gr->cd();
    canv->Update();

    vector<TGraphAsymmErrors*> vect_gr;
    for (int i = 0; i < int(funcs.size()); i++) {
        TGraphAsymmErrors* gr = (TGraphAsymmErrors*)gr_given->Clone();
        Format_Graph(gr);
        vect_gr.push_back(gr);
    }

    vector<TPaveStats*> vect_stats;
    vector<TLegend>     vect_leg;
    string XTitle = vect_gr[0]->GetXaxis()->GetTitle();
    double y1 = 0.8;
    double y2 = 0.8 - 0.06 * funcs[0]->GetNpar();

    TMultiGraph* mg = new TMultiGraph();

    for (int i = 0; i < int(funcs.size()); i++) {
        vect_gr[i]->SetMarkerColor(kWhite);
        vect_gr[i]->SetLineColor(kWhite);
        vect_gr[i]->GetXaxis()->SetTitle("");
        if (invert_colors) {
            canv->SetFillColor(kBlack);
            pad_gr->SetFillColor(kBlack);
        }
        funcs[i]->SetNpx(10000);
        funcs[i]->SetLineColor(colors[i]);
        cout << "\nFitting " << name << " with " << funcs[i]->GetName() << endl;
        TFitResultPtr result = vect_gr[i]->Fit(funcs[i], "EMS+");
        mg->Add(vect_gr[i]);

        TLegend leg(0.4, y1, 0.6, y2, "");
        leg.SetTextFont(42); leg.SetTextSize(0.04);
        leg.SetFillColor(invert_colors ? kBlack : kWhite);
        leg.SetTextColor(colors[i]); leg.SetLineColor(colors[i]);
        TString func_name    = funcs[i]->GetName();
        TString status_func  = gMinuit->fCstatu;
        leg.AddEntry((TObject*)0, func_name, "");
        leg.AddEntry((TObject*)0, TString("Status = ")+status_func, "");
        if (invert_colors) leg.SetShadowColor(kBlack);
        vect_leg.push_back(leg);

        pad_gr->Update();
        vect_gr[i]->Draw("AP SAMES");
        pad_gr->Update(); canv->Update();

        TPaveStats* stats = (TPaveStats*)vect_gr[i]->GetListOfFunctions()->FindObject("stats");
        stats->SetName(("stats_"+string(funcs[i]->GetName())).c_str());
        stats->SetTextColor(colors[i]); stats->SetLineColor(colors[i]);
        stats->SetY1NDC(y1); stats->SetY2NDC(y2);
        y1 = y2;
        y2 = y2 - 0.06 * funcs[i]->GetNpar();
        stats->SetX1NDC(0.6); stats->SetX2NDC(0.9);
        if (invert_colors) stats->SetFillColor(kBlack);
        vect_stats.push_back(stats);

        pad_gr->Clear(); pad_gr->Update(); canv->Update();

        // Write fit results to JSON
        Output_Fit_ToJSON(funcs[i], name, string(status_func), result,
                          outdir + "Fit_Parameters.json");
    }

    mg->Draw("AP SAMES");
    Format_Graph(mg);
    mg->GetYaxis()->SetTitle("Efficiency");
    for (int i = 0; i < int(funcs.size()); i++) {
        vect_leg[i].Draw("");
        vect_stats[i]->Draw("SAMES");
    }
    pad_gr->Update(); pad_gr->Modified(); canv->Update();

    TLatex l;
    if (invert_colors) l.SetTextColor(kWhite);
    l.SetTextFont(42); l.SetNDC(); l.SetTextSize(0.04);
    l.DrawLatex(0.55, 0.93, name.c_str());
    l.DrawLatex(0.13, 0.93, CMS_label.c_str());
    pad_gr->Update(); canv->Update();

    // Residuals pad
    canv->cd();
    TPad* pad_res = new TPad("pad_res","pad_res",0,0.03,1,0.3);
    pad_res->SetGridx(); pad_res->SetGridy();
    pad_res->SetTopMargin(1.3); pad_res->SetBottomMargin(0.2);
    pad_res->Draw(); pad_res->cd();
    pad_res->Update(); canv->Update();

    TMultiGraph* mg_res = new TMultiGraph();
    for (int i = 0; i < int(funcs.size()); i++) {
        TGraphAsymmErrors* res = TGAE_TF1(vect_gr[i], funcs[i]);
        Format_Graph(res);
        res->SetMarkerColor(colors[i]);
        res->SetLineColor(colors[i]);
        if (invert_colors) pad_res->SetFillColor(kBlack);
        mg_res->Add(res);
    }
    mg_res->Draw("AP");
    mg_res->GetXaxis()->SetLimits(mg->GetXaxis()->GetXmin(), mg->GetXaxis()->GetXmax());
    mg_res->GetXaxis()->SetTitle(XTitle.c_str());
    mg_res->GetYaxis()->SetTitle("Eff - Fit");
    Format_Graph_res(mg_res);

    TLine* line = new TLine(mg_res->GetXaxis()->GetXmin(), 0.,
                             mg_res->GetXaxis()->GetXmax(), 0.);
    line->SetLineColor(kWhite); line->SetLineStyle(1);
    line->Draw("SAMES");
    pad_res->Modified(); pad_res->Update(); canv->Update();
}

void Get_Fit(TGraphAsymmErrors*& gr, vector<TF1*> funcs, vector<int> colors,
             string outFile, string name, const string& outdir)
{
    if (invert_colors) {
        gStyle->SetFrameFillColor(kBlack);
        gStyle->SetFrameLineColor(kWhite);
    }

    TCanvas* can = new TCanvas(name.c_str(), "", 864., 468.);
    can->SetGridx(); can->SetGridy(); can->Draw(); can->cd();
    if (invert_colors) can->SetFillColor(kBlack);
    can->Modified(); can->Update();

    Fit_Graph_With_Funcs(can, gr, funcs, colors, name, outdir);

    TFile* output = TFile::Open(outFile.c_str(), "UPDATE");
    can->Write();
    output->Close();
    delete can;
    delete output;
}

// -----------------------------------------------------------------------
// Config struct
// -----------------------------------------------------------------------
struct TriggerConfig {
    string bin;
    string year;
    string process;   // "bkg" or "data"
    string data_sample;
    string bkg_sample;
    map<string, string> systs; // label -> sample
};

string JSON_Key(const string& bin, const string& sample)
{
    return bin + "__" + sample;
}

int Parse_Bin_N(const string& bin)
{
    for (const string& prefix : std::vector<std::string>{"Electron", "Muon"}) {
        if (bin.rfind(prefix, 0) == 0)
            return stoi(bin.substr(prefix.size()));
    }
    return -1;
}

string Make_Canvas_Name(const string& bin, const string& sample)
{
    return "can_eff_MET_trigger_eff__TriggerBin_"
           + bin + "__" + sample;
}

string Make_Syst_Canvas_Name(const string& bin,
                              const string& systLabel,
                              const string& sample)
{
    return "can_eff_MET_trigger_eff__TriggerSystBin_"
           + bin + "_" + systLabel
           + "__" + sample;
}

string Make_Nominal_Canvas_Name(const string& bin, const string& sample)
{
    return "can_eff_MET_trigger_eff__TriggerBin_"
           + bin + "__" + sample;
}

struct SampleGroup {
    set<string> nominal_samples;
    map<string, set<string>> syst_samples; // systLabel -> samples
};

// -----------------------------------------------------------------------
// Per-bin priority-ordered list of Data sample prefixes to try.
// Priority logic: two leptons in every event, so we match the dataset
// that best reflects the lepton content of the bin.
//   Electron2 / Muon0  -> DoubleElectron (2 e, 0 mu)  | backup: SingleElectron / SingleMuon
//   Electron1 / Muon1  -> MuonEG         (1 e, 1 mu)  | backup: SingleElectron / SingleMuon
//   Electron0 / Muon2  -> DoubleMuon     (0 e, 2 mu)  | backup: SingleMuon     / SingleElectron
// -----------------------------------------------------------------------
vector<string> Data_Sample_Priority(const string& bin)
{
    if (bin == "Electron2" || bin == "Muon0")
        return {"Data_DoubleElectron", "Data_SingleElectron", "Data_SingleMuon"};
    if (bin == "Electron1" || bin == "Muon1")
        return {"Data_MuonEG", "Data_SingleElectron", "Data_SingleMuon"};
    if (bin == "Electron0" || bin == "Muon2")
        return {"Data_DoubleMuon", "Data_SingleMuon", "Data_SingleElectron"};
    // Unknown bin: try everything in a reasonable order
    return {"Data_DoubleElectron", "Data_MuonEG", "Data_DoubleMuon",
            "Data_SingleElectron", "Data_SingleMuon"};
}

// -----------------------------------------------------------------------
// Discover configs from canvas names in the root file
// -----------------------------------------------------------------------
vector<TriggerConfig> Discover_Configs(const string& fname)
{
    TFile* f = TFile::Open(fname.c_str(), "READ");
    if (!f || f->IsZombie()) {
        cout << "Cannot open " << fname << " for config discovery." << endl;
        return {};
    }

    const string PREFIX = "can_eff_MET_trigger_eff__";

    struct BinGroup {
        set<string>              nominal_samples;
        map<string, set<string>> syst_samples;
    };
    map<pair<string,string>, BinGroup> found;

    TIter next(f->GetListOfKeys());
    TKey* key = nullptr;
    while ((key = (TKey*)next())) {
        string cname = key->GetName();
        if (cname.find(PREFIX) == string::npos) continue;
        if (string(key->GetClassName()) != "TCanvas") continue;

        string rest = cname.substr(PREFIX.length());
        string bin, sample, systLabel;
        bool isSyst = false;

        if (rest.rfind("TriggerBin_", 0) == 0) {
            rest = rest.substr(string("TriggerBin_").length());
            size_t sep = rest.find("__");
            if (sep == string::npos) continue;
            bin    = rest.substr(0, sep);
            sample = rest.substr(sep + 2);

        } else if (rest.rfind("TriggerSystBin_", 0) == 0) {
            rest = rest.substr(string("TriggerSystBin_").length());
            size_t sep = rest.find("__");
            if (sep == string::npos) continue;
            string binAndLabel = rest.substr(0, sep);
            sample             = rest.substr(sep + 2);

            // "Electron0_Gold" -> bin="Electron0", systLabel="Gold"
            // But bin names like "Electron0" don't contain '_', whereas
            // "MuonEG" does not apply here -- systLabel is always a single
            // word appended after the bin, so split on the LAST '_' that
            // follows the bin prefix.  We find the bin prefix length first.
            size_t us = string::npos;
            for (const string& pfx : std::vector<string>{"Electron", "Muon"}) {
                if (binAndLabel.rfind(pfx, 0) == 0) {
                    // bin is prefix + digits, e.g. "Electron0" or "Muon12"
                    size_t i = pfx.size();
                    while (i < binAndLabel.size() && isdigit(binAndLabel[i])) i++;
                    // i now points to '_' before systLabel
                    if (i < binAndLabel.size() && binAndLabel[i] == '_') us = i;
                    break;
                }
            }
            if (us == string::npos) continue;
            bin       = binAndLabel.substr(0, us);
            systLabel = binAndLabel.substr(us + 1);
            isSyst    = true;
        } else {
            continue;
        }

        // Validate bin prefix
        if (bin.rfind("Electron", 0) != 0 && bin.rfind("Muon", 0) != 0) continue;

        // Extract year from tail of sample string (last _YYYY)
        size_t last_us = sample.rfind('_');
        if (last_us == string::npos) continue;
        string year = sample.substr(last_us + 1);
        if (year.size() < 4 || !isdigit(year[0])) continue;

        auto& group = found[{bin, year}];
        if (isSyst) group.syst_samples[systLabel].insert(sample);
        else        group.nominal_samples.insert(sample);
    }
    f->Close(); delete f;

    vector<TriggerConfig> configs;
    for (auto& [key_pair, group] : found) {
        const string& bin  = key_pair.first;
        const string& year = key_pair.second;

        // Find bkg sample
        string bkg_sample = "";
        for (const auto& s : group.nominal_samples)
            if (s.find("bkg") != string::npos) { bkg_sample = s; break; }

        if (bkg_sample.empty()) {
            cout << "WARNING: No bkg sample for " << bin << " " << year
                 << " -- skipping." << endl;
            continue;
        }

        // Find best data sample according to per-bin priority list
        string data_sample = "";
        string chosen_prefix = "";
        for (const string& pfx : Data_Sample_Priority(bin)) {
            for (const auto& s : group.nominal_samples) {
                if (s.find(pfx) != string::npos) {
                    data_sample   = s;
                    chosen_prefix = pfx;
                    break;
                }
            }
            if (!data_sample.empty()) break;
        }

        if (data_sample.empty()) {
            cout << "WARNING: No data sample found for " << bin << " " << year
                 << " (tried all priorities) -- skipping." << endl;
            continue;
        }

        map<string,string> systs;
        for (auto& [lbl, samples] : group.syst_samples)
            systs[lbl] = lbl;

        configs.push_back({bin, year, "", data_sample, bkg_sample, systs});

        cout << "Discovered: " << bin << " " << year
             << " bkg="  << bkg_sample
             << " data=" << data_sample << " (matched: " << chosen_prefix << ")"
             << " systs=[";
        for (auto& [l,_] : systs) cout << l << " ";
        cout << "]" << endl;
    }

    sort(configs.begin(), configs.end(), [](const TriggerConfig& a, const TriggerConfig& b){
        if (a.year != b.year) return a.year < b.year;
        return a.bin < b.bin;
    });
    return configs;
}

// -----------------------------------------------------------------------
// Extract TGraphAsymmErrors from a stored TCanvas via TEfficiency
// -----------------------------------------------------------------------
TGraphAsymmErrors* Get_Graph_From_Canvas(const string& fname,
                                          const string& canvasName,
                                          int color,
                                          TLegend*& leg,
                                          const string& legLabel)
{
    TFile* f = TFile::Open(fname.c_str(), "READ");
    if (!f || f->IsZombie()) {
        cout << "Cannot open " << fname << endl;
        return nullptr;
    }

    TCanvas* src = nullptr;
    f->GetObject(canvasName.c_str(), src);
    if (!src) {
        cout << "Cannot find canvas: " << canvasName << endl;
        f->Close(); delete f;
        return nullptr;
    }

    TEfficiency* eff = nullptr;
    TIter next(src->GetListOfPrimitives());
    TObject* obj = nullptr;
    while ((obj = next())) {
        if (obj->InheritsFrom(TEfficiency::Class())) {
            eff = (TEfficiency*)obj;
            break;
        }
    }

    if (!eff) {
        cout << "No TEfficiency found in canvas: " << canvasName << endl;
        f->Close(); delete f;
        return nullptr;
    }

    TCanvas* tmp = new TCanvas("tmp_paint","tmp_paint",10,10);
    tmp->cd();
    eff->Draw("AP");
    tmp->Update();

    TGraphAsymmErrors* painted = eff->GetPaintedGraph();
    if (!painted) {
        cout << "GetPaintedGraph() returned null for: " << canvasName << endl;
        delete tmp; f->Close(); delete f;
        return nullptr;
    }

    TGraphAsymmErrors* gr = (TGraphAsymmErrors*)painted->Clone();
    delete tmp;
    f->Close(); delete f;

    gr->SetMarkerStyle(20);
    gr->SetMarkerColor(color);
    gr->SetLineColor(color);
    leg->AddEntry(gr, legLabel.c_str(), "PL");
    return gr;
}
