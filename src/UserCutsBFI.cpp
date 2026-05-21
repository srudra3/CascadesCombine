#include "BuildFitInput.h"
#include <iostream>
#include <limits>
#include <TLorentzVector.h>
#include <cmath>
#include <algorithm>
#include "FourVec.h"
#include "EventShape.h"
// =====================================================
// QuadLeptonInfo
// =====================================================
struct QuadLeptonResult {
    double minmll;
    double maxmll;
    int nossf;
    double devZ;
    double othermll;
    double minm3l;
    int minPairCode;
    int compPairCode;
    int minQPairCode;
    int compQPairCode;
    double min2lPairedMass;
    double minOthermll;
    double minOthermllOS;
};

// Helper: Compute pair mass from (pt, eta, phi, m)
inline double ComputeMass(double pt1, double eta1, double phi1, double m1,
                          double pt2, double eta2, double phi2, double m2) {
    double pz1 = pt1 * std::sinh(eta1);
    double pz2 = pt2 * std::sinh(eta2);
    double E1 = std::sqrt(pt1*pt1 + pz1*pz1 + m1*m1);
    double E2 = std::sqrt(pt2*pt2 + pz2*pz2 + m2*m2);
    
    double px1 = pt1 * std::cos(phi1);
    double py1 = pt1 * std::sin(phi1);
    double px2 = pt2 * std::cos(phi2);
    double py2 = pt2 * std::sin(phi2);
    
    double E = E1 + E2;
    double px = px1 + px2;
    double py = py1 + py2;
    double pz = pz1 + pz2;
    
    double mass2 = E*E - (px*px + py*py + pz*pz);
    return mass2 > 0 ? std::sqrt(mass2) : 0.0;
}

// Helper: PairCode - same as Ana.C (lines 193-234)
inline int PairCode(int i, int j, const std::vector<int>& qfs) {
    int qfi = qfs[i];
    int qfj = qfs[j];
    int code = 0;
    
    if (qfi * qfj < 0) {
        // opposite sign
        if (std::abs(qfi) == std::abs(qfj)) {
            if(std::abs(qfi) == 1){
                code = 1; // OSSF-ee
            } else {
                code = 2; // OSSF-mm
            }
        } else {  // different flavor
            code = 3;     // OSDF-em
        }
    } else {
        // same sign
        if (std::abs(qfi) == std::abs(qfj)) {
            if(std::abs(qfi) == 1){
                code = 4; // SSSF-ee
            } else {
                code = 5; // SSSF-mm
            }
        } else {  // different flavor
            code = 6;     // SSDF-em
        }
    }
    return code;
}

// Helper: QPairCode - same as Ana.C (lines 237-294)
inline int QPairCode(int i, int j, const std::vector<int>& qfs) {
    int qfi = qfs[i];
    int qfj = qfs[j];
    int code = 0;
    
    if (qfi * qfj < 0) {
        // opposite sign
        if (std::abs(qfi) == std::abs(qfj)) {
            if(std::abs(qfi) == 1){
                code = 1; // OSSF-ee
            } else {
                code = 2; // OSSF-mm
            }
        } else {  // different flavor
            code = 3;     // OSDF-em
        }
    } else {   // same sign
        if (std::abs(qfi) == std::abs(qfj)) {   // same flavor
            if( qfi == -1){
                code = 4; // NSS-ee
            } else if ( qfi == -2 ){
                code = 5; // NSS-mm
            } else if ( qfi ==  1 ){
                code = 7; // PSS-ee
            } else if ( qfi ==  2 ){
                code = 8; // PSS-mm
            } else{
                code = 0;
            }
        } else {  // different flavor
            if( (qfi == -1 && qfj == -2) || (qfi == -2 && qfj == -1) ){
                code = 6;  // NSS-em
            } else if ( (qfi ==  1 && qfj ==  2) || (qfi ==  2 && qfj ==  1) ){
                code = 9;  // PSS-em
            } else{
                code = 0;
            }
        }
    }
    return code;
}

// Main QuadLeptonInfo function - Ana.C lines 371-499
QuadLeptonResult ComputeQuadLeptonInfo(
    const std::vector<double>& PT_lep,
    const std::vector<double>& Eta_lep,
    const std::vector<double>& Phi_lep,
    const std::vector<double>& M_lep,
    const std::vector<int>& Charge_lep,
    const std::vector<int>& PDGID_lep) {
    
    QuadLeptonResult result{};
    
    if (PT_lep.size() < 4) return result;
    
    // Helper to get charge*flavor code (from PDGID and Charge)
    // Ana.C: int = charge * (PDG==11 ? 1 : 2)
    auto getQF = [&](int idx) -> int {
        if (idx >= (int)PDGID_lep.size() || idx >= (int)Charge_lep.size()) return 0;
        int pdg = std::abs(PDGID_lep[idx]);
        int charge = Charge_lep[idx];
        if (pdg == 11) return charge;        // electron: ±1
        else if (pdg == 13) return 2*charge; // muon: ±2
        else return 0;
    };
    
    std::vector<int> qfs = {getQF(0), getQF(1), getQF(2), getQF(3)};
    
    // Compute all 6 pair masses
    double m12 = ComputeMass(PT_lep[0], Eta_lep[0], Phi_lep[0], M_lep[0],
                             PT_lep[1], Eta_lep[1], Phi_lep[1], M_lep[1]);
    double m13 = ComputeMass(PT_lep[0], Eta_lep[0], Phi_lep[0], M_lep[0],
                             PT_lep[2], Eta_lep[2], Phi_lep[2], M_lep[2]);
    double m14 = ComputeMass(PT_lep[0], Eta_lep[0], Phi_lep[0], M_lep[0],
                             PT_lep[3], Eta_lep[3], Phi_lep[3], M_lep[3]);
    double m23 = ComputeMass(PT_lep[1], Eta_lep[1], Phi_lep[1], M_lep[1],
                             PT_lep[2], Eta_lep[2], Phi_lep[2], M_lep[2]);
    double m24 = ComputeMass(PT_lep[1], Eta_lep[1], Phi_lep[1], M_lep[1],
                             PT_lep[3], Eta_lep[3], Phi_lep[3], M_lep[3]);
    double m34 = ComputeMass(PT_lep[2], Eta_lep[2], Phi_lep[2], M_lep[2],
                             PT_lep[3], Eta_lep[3], Phi_lep[3], M_lep[3]);
    
    // Store max dilepton mass
    result.maxmll = std::max({m12, m13, m14, m23, m24, m34});
    
    // Count OSSF pairs and find Z-mass deviation (Ana.C lines 420-454)
    result.nossf = 0;
    const double MZ = 91.2;
    result.devZ = -95.0;
    
    auto checkOSSF = [&](double m, int i1, int i2) {
        if (Charge_lep[i1] * Charge_lep[i2] < 0 &&
            std::abs(PDGID_lep[i1]) == std::abs(PDGID_lep[i2])) {
            result.nossf++;
            double thisdevZ = m - MZ;
            if (std::abs(thisdevZ) < std::abs(result.devZ)) result.devZ = thisdevZ;
            return true;
        }
        return false;
    };
    
    checkOSSF(m12, 0, 1);
    checkOSSF(m13, 0, 2);
    checkOSSF(m14, 0, 3);
    checkOSSF(m23, 1, 2);
    checkOSSF(m24, 1, 3);
    checkOSSF(m34, 2, 3);
    
    // Find min dilepton mass pair (Ana.C lines 385-409)
    struct Entry {
        double mass;
        double comp_mass;
        int i1, i2, j1, j2;
    };
    
    Entry entries[] = {
        {m12, m34, 0, 1, 2, 3},
        {m13, m24, 0, 2, 1, 3},
        {m14, m23, 0, 3, 1, 2},
        {m23, m14, 1, 2, 0, 3},
        {m24, m13, 1, 3, 0, 2},
        {m34, m12, 2, 3, 0, 1}
    };
    
    auto min_entry = entries[0];
    for (int i = 1; i < 6; ++i) {
        if (entries[i].mass < min_entry.mass) {
            min_entry = entries[i];
        }
    }
    
    result.minmll = min_entry.mass;
    result.othermll = min_entry.comp_mass;
    result.minPairCode = PairCode(min_entry.i1, min_entry.i2, qfs);
    result.compPairCode = PairCode(min_entry.j1, min_entry.j2, qfs);
    result.minQPairCode = QPairCode(min_entry.i1, min_entry.i2, qfs);
    result.compQPairCode = QPairCode(min_entry.j1, min_entry.j2, qfs);
    
    // 2+2 pairing masses and analysis (Ana.C lines 471-494)
    double msq1 = m12*m12 + m34*m34;
    double msq2 = m13*m13 + m24*m24;
    double msq3 = m14*m14 + m23*m23;
    double minmsq = std::min({msq1, msq2, msq3});
    result.min2lPairedMass = std::sqrt(minmsq);
    
    // Min/max "other" mass for 2+2 assignments
    double othermllA = std::max(m12, m34);
    double othermllB = std::max(m13, m24);
    double othermllC = std::max(m14, m23);
    result.minOthermll = std::min({othermllA, othermllB, othermllC});
    
    // OS-only assignments
    double osmllA = 99999.0;
    double osmllB = 99999.0;
    double osmllC = 99999.0;
    if ((qfs[0]*qfs[3])<0 && (qfs[1]*qfs[2])<0) osmllA = std::max(m12, m34);
    if ((qfs[0]*qfs[2])<0 && (qfs[1]*qfs[3])<0) osmllB = std::max(m13, m24);
    if ((qfs[0]*qfs[3])<0 && (qfs[1]*qfs[2])<0) osmllC = std::max(m14, m23);
    result.minOthermllOS = std::min({osmllA, osmllB, osmllC});
    
    result.minm3l = 0.0;
    
    return result;
}

// -------------------------
// User-defined cuts loader
// -------------------------
ROOT::RDF::RNode BuildFitInput::loadCutsUser(ROOT::RDF::RNode &node, std::map<std::string, CutDef>& ValidCuts, bool run_validation){
    std::map<std::string, CutDef> cuts;
    RegisterSafeHelpers();

    // =====================================================
    // 4-LEPTON SELECTION CUTS (from Ana.C)
    // Following Ana.C lines 1008-1124 EXACTLY
    // =====================================================
    
    // Step 1: Create vlidx_4l - indices of GOLD+SILVER leptons, sorted by pT (Ana.C lines 779-795, 1008-1011)
    if (!node.HasColumn("vlidx_4l")) {
        node = node.Define("vlidx_4l",
            [](const std::vector<double>& pt, const std::vector<int>& qual) -> std::vector<int> {
                // Get indices of gold+silver leptons
                std::vector<std::pair<int, double>> idx_pt;
                for (size_t i = 0; i < pt.size(); ++i) {
                    if (qual[i] <= 1) {  // Gold (0) or Silver (1)
                        idx_pt.push_back({i, pt[i]});
                    }
                }
                // Sort by pT descending (matches Ana.C vlidx behavior)
                std::sort(idx_pt.begin(), idx_pt.end(),
                         [](const auto& a, const auto& b) { return a.second > b.second; });
                
                std::vector<int> indices;
                for (auto& p : idx_pt) indices.push_back(p.first);
                return indices;
            },
            {"PT_lep", "LepQual_lep"}
        );
    }
    
    // Step 2: Check we have at least 4 gold+silver leptons (Ana.C line 1008)
    CutDef cut_4l_nlep;
    cut_4l_nlep.name = "cut_4l_nlep";
    cut_4l_nlep.columns = {"vlidx_4l"};
    cut_4l_nlep.expression = "vlidx_4l.size() >= 4";
    cuts[cut_4l_nlep.name] = cut_4l_nlep;
    
    // Step 3: Extract pT of the 4 selected leptons (Ana.C lines 1012-1015)
    // ptcuts4l = {18.0, 8.0, 6.0, 2.0}
    if (!node.HasColumn("quad_pt1")) {
        node = node.Define("quad_pt1",
            [](const std::vector<int>& vlidx, const std::vector<double>& pt) -> double {
                if (vlidx.size() < 1) return -1.0;
                return pt[vlidx[0]];
            },
            {"vlidx_4l", "PT_lep"}
        );
    }
    
    if (!node.HasColumn("quad_pt2")) {
        node = node.Define("quad_pt2",
            [](const std::vector<int>& vlidx, const std::vector<double>& pt) -> double {
                if (vlidx.size() < 2) return -1.0;
                return pt[vlidx[1]];
            },
            {"vlidx_4l", "PT_lep"}
        );
    }
    
    if (!node.HasColumn("quad_pt3")) {
        node = node.Define("quad_pt3",
            [](const std::vector<int>& vlidx, const std::vector<double>& pt) -> double {
                if (vlidx.size() < 3) return -1.0;
                return pt[vlidx[2]];
            },
            {"vlidx_4l", "PT_lep"}
        );
    }
    
    if (!node.HasColumn("quad_pt4")) {
        node = node.Define("quad_pt4",
            [](const std::vector<int>& vlidx, const std::vector<double>& pt) -> double {
                if (vlidx.size() < 4) return -1.0;
                return pt[vlidx[3]];
            },
            {"vlidx_4l", "PT_lep"}
        );
    }
   
    
    if (!node.HasColumn("quad_eta_max")) {
        node = node.Define("quad_eta_max",
            [](const std::vector<int>& vlidx, const std::vector<double>& eta_lep) -> double {
                if (vlidx.size() < 4) return 999.0;
                return std::max({std::abs(eta_lep[vlidx[0]]), std::abs(eta_lep[vlidx[1]]), 
                           std::abs(eta_lep[vlidx[2]]), std::abs(eta_lep[vlidx[3]])});
            },
            {"vlidx_4l", "Eta_lep"}
        );
    }


 
    CutDef cut_4l_pt1;
    cut_4l_pt1.name = "cut_4l_pt1";
    cut_4l_pt1.columns = {"quad_pt1"};
    cut_4l_pt1.expression = "quad_pt1 >= 18.0";
    cuts[cut_4l_pt1.name] = cut_4l_pt1;
    
    CutDef cut_4l_pt2;
    cut_4l_pt2.name = "cut_4l_pt2";
    cut_4l_pt2.columns = {"quad_pt2"};
    cut_4l_pt2.expression = "quad_pt2 >= 8.0";
    cuts[cut_4l_pt2.name] = cut_4l_pt2;
    
    CutDef cut_4l_pt3;
    cut_4l_pt3.name = "cut_4l_pt3";
    cut_4l_pt3.columns = {"quad_pt3"};
    cut_4l_pt3.expression = "quad_pt3 >= 6.0";
    cuts[cut_4l_pt3.name] = cut_4l_pt3;
    
    CutDef cut_4l_pt4;
    cut_4l_pt4.name = "cut_4l_pt4";
    cut_4l_pt4.columns = {"quad_pt4"};
    cut_4l_pt4.expression = "quad_pt4 >= 2.0";
    cuts[cut_4l_pt4.name] = cut_4l_pt4;
	    
	    
	    
    CutDef cut_4l_eta;
    cut_4l_eta.name = "cut_4l_eta";
    cut_4l_eta.columns = {"quad_eta_max"};
    cut_4l_eta.expression = "quad_eta_max < 2.1";
    cuts[cut_4l_eta.name] = cut_4l_eta;  
    // Step 4: Compute QuadLeptonInfo from the 4 leptons (Ana.C lines 1034-1072)
    if (!node.HasColumn("quad_info")) {
        node = node.Define("quad_info",
            [](const std::vector<int>& vlidx,
               const std::vector<double>& pt, const std::vector<double>& eta,
               const std::vector<double>& phi, const std::vector<double>& mass,
               const std::vector<int>& charge, const std::vector<int>& pdgid) -> std::vector<double> {
                
                std::vector<double> info(13, 0.0);  // Return 13 values
                
                if (vlidx.size() < 4) return info;
                
                // Extract the 4 leptons
                std::vector<double> pt4 = {pt[vlidx[0]], pt[vlidx[1]], pt[vlidx[2]], pt[vlidx[3]]};
                std::vector<double> eta4 = {eta[vlidx[0]], eta[vlidx[1]], eta[vlidx[2]], eta[vlidx[3]]};
                std::vector<double> phi4 = {phi[vlidx[0]], phi[vlidx[1]], phi[vlidx[2]], phi[vlidx[3]]};
                std::vector<double> m4 = {mass[vlidx[0]], mass[vlidx[1]], mass[vlidx[2]], mass[vlidx[3]]};
                std::vector<int> q4 = {charge[vlidx[0]], charge[vlidx[1]], charge[vlidx[2]], charge[vlidx[3]]};
                std::vector<int> pdg4 = {pdgid[vlidx[0]], pdgid[vlidx[1]], pdgid[vlidx[2]], pdgid[vlidx[3]]};
                
                QuadLeptonResult result = ComputeQuadLeptonInfo(pt4, eta4, phi4, m4, q4, pdg4);
                
                info[0] = result.minmll;
                info[1] = result.maxmll;
                info[2] = result.nossf;
                info[3] = result.devZ;
                info[4] = result.othermll;
                info[5] = result.minm3l;
                info[6] = result.minPairCode;
                info[7] = result.compPairCode;
                info[8] = result.minQPairCode;
                info[9] = result.compQPairCode;
                info[10] = result.min2lPairedMass;
                info[11] = result.minOthermll;
                info[12] = result.minOthermllOS;
                
                return info;
            },
            {"vlidx_4l", "PT_lep", "Eta_lep", "Phi_lep", "M_lep", "Charge_lep", "PDGID_lep"}
        );
    }
    
    // Extract individual quantities from quad_info vector
    if (!node.HasColumn("quad_minmll")) {
        node = node.Define("quad_minmll",
            [](const std::vector<double>& info) { return info.size() > 0 ? info[0] : 0.0; },
            {"quad_info"}
        );
    }
    
    if (!node.HasColumn("quad_devZ")) {
        node = node.Define("quad_devZ",
            [](const std::vector<double>& info) { return info.size() > 3 ? info[3] : -95.0; },
            {"quad_info"}
        );
    }
    
    if (!node.HasColumn("quad_maxsip3d")) {
        node = node.Define("quad_maxsip3d",
            [](const std::vector<int>& vlidx, const std::vector<double>& sip3d) -> double {
                if (vlidx.size() < 4) return 999.0;
                return std::max({sip3d[vlidx[0]], sip3d[vlidx[1]], sip3d[vlidx[2]], sip3d[vlidx[3]]});
            },
            {"vlidx_4l", "SIP3D_lep"}
        );
    }
    
    // Step 5: Apply 4-lepton cuts (Ana.C lines 1073-1123)
    
    // Min dilepton mass > 4 GeV (Ana.C line 1073)
    CutDef cut_4l_minmll;
    cut_4l_minmll.name = "cut_4l_minmll";
    cut_4l_minmll.columns = {"quad_minmll"};
    cut_4l_minmll.expression = "quad_minmll >= 4.0";  // Ana.C line 1073: if(minmll4l < 4.0)
    cuts[cut_4l_minmll.name] = cut_4l_minmll;
    
    // OffZ cut: |devZ| >= 7.5 (Ana.C line 1075)
    CutDef cut_4l_offz;
    cut_4l_offz.name = "cut_4l_offz";
    cut_4l_offz.columns = {"quad_devZ"};
    cut_4l_offz.expression = "abs(quad_devZ) >= 7.5";
    cuts[cut_4l_offz.name] = cut_4l_offz;
    
    // Pt4L cut: 4-lepton system pT (Ana.C line 1081)
    // if( (*Njet == 0 && pt4l < 10.0) || (*Njet >=1 && pt4ljets < 30.0)) fail
    if (!node.HasColumn("quad_pt4l")) {
        node = node.Define("quad_pt4l",
            [](const std::vector<int>& vlidx, const std::vector<double>& pt_lep,
               const std::vector<double>& eta_lep, const std::vector<double>& phi_lep,
               const std::vector<double>& m_lep, int njet, const std::vector<double>& jet_pt,
               const std::vector<double>& jet_eta, const std::vector<double>& jet_phi) -> bool {
                if (vlidx.size() < 4) return false;
                
                // Build 4-lepton FourVec (Ana.C lines 1034-1037, 1076)
                FourVec l1 = FourVec::FromPtEtaPhiM(11, pt_lep[vlidx[0]], eta_lep[vlidx[0]], phi_lep[vlidx[0]], m_lep[vlidx[0]]);
                FourVec l2 = FourVec::FromPtEtaPhiM(11, pt_lep[vlidx[1]], eta_lep[vlidx[1]], phi_lep[vlidx[1]], m_lep[vlidx[1]]);
                FourVec l3 = FourVec::FromPtEtaPhiM(11, pt_lep[vlidx[2]], eta_lep[vlidx[2]], phi_lep[vlidx[2]], m_lep[vlidx[2]]);
                FourVec l4 = FourVec::FromPtEtaPhiM(11, pt_lep[vlidx[3]], eta_lep[vlidx[3]], phi_lep[vlidx[3]], m_lep[vlidx[3]]);
                FourVec l1234 = (l1 + l2) + (l3 + l4);
                
                double pt4l = l1234.Pt();
                double pt4ljets = pt4l;
                
                // Add jets if present (Ana.C lines 1079-1080)
                if (njet > 0 && !jet_pt.empty()) {
                    FourVec allJets;
                    for (size_t j = 0; j < jet_pt.size() && j < jet_eta.size() && j < jet_phi.size(); ++j) {
                        FourVec jet = FourVec::FromPtEtaPhiM(0, jet_pt[j], jet_eta[j], jet_phi[j], 0.0);
                        allJets = allJets + jet;
                    }
                    FourVec ljets = l1234 + allJets;
                    pt4ljets = ljets.Pt();
                }
                
                // Return the appropriate pT value for the cut
                if (njet == 0) {
                    return pt4l >= 10.0;  // Will be cut as pt4l >= 10.0
                } else {
                    return pt4ljets >= 30.0;  // Will be cut as pt4ljets >= 30.0
                }
            },
            {"vlidx_4l", "PT_lep", "Eta_lep", "Phi_lep", "M_lep", "Njet", "PT_jet", "Eta_jet", "Phi_jet"}
        );
    }
    


     
    CutDef cut_4l_pt4l;
    cut_4l_pt4l.name = "cut_4l_pt4l";
    cut_4l_pt4l.columns = {"quad_pt4l"};
    cut_4l_pt4l.expression = "quad_pt4l == true";
    cuts[cut_4l_pt4l.name] = cut_4l_pt4l;
    
    // M4LZV cut: ZZ-like mass veto (Ana.C line 1084)
    // Vetoes: ZZlike && (|m4l - 91.2| < 10.0 || (m4l > 121.0 && m4l < 127.0))
    if (!node.HasColumn("quad_m4l")) {
        node = node.Define("quad_m4l",
            [](const std::vector<int>& vlidx, const std::vector<double>& pt_lep,
               const std::vector<double>& eta_lep, const std::vector<double>& phi_lep,
               const std::vector<double>& m_lep) -> double {
                if (vlidx.size() < 4) return 0.0;
                
                // Build 4-lepton system mass
                FourVec l1 = FourVec::FromPtEtaPhiM(11, pt_lep[vlidx[0]], eta_lep[vlidx[0]], phi_lep[vlidx[0]], m_lep[vlidx[0]]);
                FourVec l2 = FourVec::FromPtEtaPhiM(11, pt_lep[vlidx[1]], eta_lep[vlidx[1]], phi_lep[vlidx[1]], m_lep[vlidx[1]]);
                FourVec l3 = FourVec::FromPtEtaPhiM(11, pt_lep[vlidx[2]], eta_lep[vlidx[2]], phi_lep[vlidx[2]], m_lep[vlidx[2]]);
                FourVec l4 = FourVec::FromPtEtaPhiM(11, pt_lep[vlidx[3]], eta_lep[vlidx[3]], phi_lep[vlidx[3]], m_lep[vlidx[3]]);
                FourVec l1234 = (l1 + l2) + (l3 + l4);
                
                return l1234.M();
            },
            {"vlidx_4l", "PT_lep", "Eta_lep", "Phi_lep", "M_lep"}
        );
    }
    
    if (!node.HasColumn("quad_zzlike")) {
        node = node.Define("quad_zzlike",
            [](const std::vector<int>& vlidx, const std::vector<int>& charge_lep,
               const std::vector<int>& pdgid_lep) -> bool {
                if (vlidx.size() < 4) return false;
                
                // Check if leptons can form 2 OSSF pairs (Ana.C ZZlikeEvent function)
                auto hasOSSF = [&](int i, int j) -> bool {
                    if (charge_lep[vlidx[i]] * charge_lep[vlidx[j]] < 0 &&
                        std::abs(pdgid_lep[vlidx[i]]) == std::abs(pdgid_lep[vlidx[j]])) {
                        return true;
                    }
                    return false;
                };
                
                // Check all 3 possible pairings for 2 OSSF pairs
                if ((hasOSSF(0, 1) && hasOSSF(2, 3)) ||
                    (hasOSSF(0, 2) && hasOSSF(1, 3)) ||
                    (hasOSSF(0, 3) && hasOSSF(1, 2))) {
                    return true;
                }
                return false;
            },
            {"vlidx_4l", "Charge_lep", "PDGID_lep"}
        );
    }
    
    if (!node.HasColumn("quad_m4lzv_pass")) {
        node = node.Define("quad_m4lzv_pass",
            [](bool zzlike, double m4l) -> bool {
                if (!zzlike) return true;  // Cut only applies to ZZlike events
                
                // Veto if |m4l - 91.2| < 10.0 (near Z) or 121.0 < m4l < 127.0 (near Higgs)
                if (std::abs(m4l - 91.2) < 10.0) return false;
                if (m4l > 121.0 && m4l < 127.0) return false;
                
                return true;  // Passes the veto
            },
            {"quad_zzlike", "quad_m4l"}
        );
    }
    
    CutDef cut_4l_m4lzv;
    cut_4l_m4lzv.name = "cut_4l_m4lzv";
    cut_4l_m4lzv.columns = {"quad_m4lzv_pass"};
    cut_4l_m4lzv.expression = "quad_m4lzv_pass == true";
    cuts[cut_4l_m4lzv.name] = cut_4l_m4lzv;
    
    // SIP3D cut: max SIP3D of 4 leptons <= 5.0 (Ana.C line 1032)
    CutDef cut_4l_sip3d;
    cut_4l_sip3d.name = "cut_4l_sip3d";
    cut_4l_sip3d.columns = {"quad_maxsip3d"};
    cut_4l_sip3d.expression = "quad_maxsip3d <= 5.0";
    cuts[cut_4l_sip3d.name] = cut_4l_sip3d;
    
    // Complementary mass cut - Hemisphere/Thrust-based (Ana.C lines 1104-1123)
    // Exact implementation matching Ana.C
    if (!node.HasColumn("quad_mxcompmll")) {
        node = node.Define("quad_mxcompmll",
            [](const std::vector<int>& vlidx, const std::vector<double>& pt_lep,
               const std::vector<double>& eta_lep, const std::vector<double>& phi_lep,
               const std::vector<double>& m_lep) -> bool {
                if (vlidx.size() < 4) return false;
                
                // Reconstruct 4 leptons as FourVec (Ana.C lines 1034-1037)
                FourVec l1 = FourVec::FromPtEtaPhiM(11, pt_lep[vlidx[0]], eta_lep[vlidx[0]], phi_lep[vlidx[0]], m_lep[vlidx[0]]);
                FourVec l2 = FourVec::FromPtEtaPhiM(11, pt_lep[vlidx[1]], eta_lep[vlidx[1]], phi_lep[vlidx[1]], m_lep[vlidx[1]]);
                FourVec l3 = FourVec::FromPtEtaPhiM(11, pt_lep[vlidx[2]], eta_lep[vlidx[2]], phi_lep[vlidx[2]], m_lep[vlidx[2]]);
                FourVec l4 = FourVec::FromPtEtaPhiM(11, pt_lep[vlidx[3]], eta_lep[vlidx[3]], phi_lep[vlidx[3]], m_lep[vlidx[3]]);
                
                // Build lepton vector for EventShape (Ana.C lines 1105-1106)
                std::vector<FourVec> leptons;
                leptons.push_back(l1);
                leptons.push_back(l2);
                leptons.push_back(l3);
                leptons.push_back(l4);
                
                // Compute Collins-Soper variables (Ana.C lines 1108-1110)
                auto cs = EventShape::ComputeThrustAxisInCS(leptons, 6800.0);
                
                // Compute thrust hemispheres (Ana.C lines 1111-1114)
                auto hemis = EventShape::ComputeThrustHemispheres(leptons, cs.zCS);
                int nlepMaxHemi4l = std::max(hemis.nPlus, hemis.nMinus);
                double maxMassHemi4l = std::max(hemis.massPlus, hemis.massMinus);
                
                // Apply cut logic (Ana.C lines 1116-1123)
                int failsCut = 0;
                if (nlepMaxHemi4l == 2) {        // 2+2 configuration
                    if (maxMassHemi4l > 23.0) failsCut = 1;     // tighter than prior 27 GeV
                } else {                        // 3+1 configuration
                    if (maxMassHemi4l > 39.0) failsCut = 2;     // hard-code - similar to before
                }
                
                // Return true if cut passes (failsCut == 0)
                return (failsCut == 0);
            },
            {"vlidx_4l", "PT_lep", "Eta_lep", "Phi_lep", "M_lep"}
        );
    }
    
    CutDef cut_4l_mxcompmll;
    cut_4l_mxcompmll.name = "cut_4l_mxcompmll";
    cut_4l_mxcompmll.columns = {"quad_mxcompmll"};
    cut_4l_mxcompmll.expression = "quad_mxcompmll == true";
    cuts[cut_4l_mxcompmll.name] = cut_4l_mxcompmll;
   












    if (!node.HasColumn("passTrigger")) {
    node = node.Define(
        "passTrigger",
        [](bool SingleElectrontrigger,
           bool SingleMuontrigger,
           bool DoubleElectrontrigger,
           bool DoubleMuontrigger,
           bool EMutrigger,
           bool TripleElectrontrigger,
           bool DiEleMutrigger,
           bool DiMuEleLowPTtrigger,
           bool DiMuEleHighPTtrigger,
           bool TripleMuonLowPTtrigger,
           bool TripleMuonHighPTtrigger) {

            return (
                SingleElectrontrigger ||
                SingleMuontrigger ||
                DoubleElectrontrigger ||
                DoubleMuontrigger ||
                EMutrigger ||
                TripleElectrontrigger ||
                DiEleMutrigger ||
                DiMuEleLowPTtrigger ||
                DiMuEleHighPTtrigger ||
                TripleMuonLowPTtrigger ||
                TripleMuonHighPTtrigger
            );
        },
        {
            "SingleElectrontrigger",
            "SingleMuontrigger",
            "DoubleElectrontrigger",
            "DoubleMuontrigger",
            "EMutrigger",
            "TripleElectrontrigger",
            "DiEleMutrigger",
            "DiMuEleLowPTtrigger",
            "DiMuEleHighPTtrigger",
            "TripleMuonLowPTtrigger",
            "TripleMuonHighPTtrigger"
        }
    );
    }



    CutDef cut_trigger;
    cut_trigger.name = "trigger3l";
    cut_trigger.columns = {"passTrigger"};
    cut_trigger.expression = "passTrigger == true";
    cuts[cut_trigger.name] = cut_trigger;
    // --- Existing 2/3 lepton cuts (unchanged) ---
    
    CutDef cut_leadSjetPt;
    cut_leadSjetPt.name = "leadSjet_pt";
    cut_leadSjetPt.columns = {"leadSjet_Pt45"};
    cut_leadSjetPt.expression = "leadSjet_Pt45==1";
    cuts[cut_leadSjetPt.name] = cut_leadSjetPt;

    CutDef cut_HEMVETO;
    cut_HEMVETO.name = "HEM_Veto";
    cut_HEMVETO.columns = {"pass_HEM"};
    cut_HEMVETO.expression = "pass_HEM";
    cuts[cut_HEMVETO.name] = cut_HEMVETO;

    bool do_minDR = true;
    bool do_minMll = true;
    bool do_2Dll_low = true;

    if (do_minDR && node.HasColumn("minDeltaR_leps") == false) {
        node = node.Define("minDeltaR_leps",
            [](const std::vector<double> &pt,
               const std::vector<double> &eta,
               const std::vector<double> &phi,
               const std::vector<double> &mass) -> double {
        
                size_t n = std::min(std::min(pt.size(), eta.size()), std::min(phi.size(), mass.size()));
                if (n < 2) return 999.0;
        
                double minDR = std::numeric_limits<double>::infinity();
                for (size_t i = 0; i + 1 < n; ++i) {
                    TLorentzVector li;
                    li.SetPtEtaPhiM(pt[i], eta[i], phi[i], mass[i]);
                    for (size_t j = i + 1; j < n; ++j) {
                        TLorentzVector lj;
                        lj.SetPtEtaPhiM(pt[j], eta[j], phi[j], mass[j]);
                        double dr = li.DeltaR(lj);
                        if (dr < minDR) minDR = dr;
                    }
                }
                return (minDR == std::numeric_limits<double>::infinity()) ? 999.0 : minDR;
            },
            {"PT_lep","Eta_lep","Phi_lep","M_lep"}
        );
    }

    CutDef cut_minDR_ll;
    cut_minDR_ll.name = "minDR_ll";
    cut_minDR_ll.columns = {"minDeltaR_leps"};
    cut_minDR_ll.expression = "minDeltaR_leps > 0.02";
    cuts[cut_minDR_ll.name] = cut_minDR_ll;

    if (do_minMll && !node.HasColumn("minMll")) {
        node = node.Define("minMll",
            [](const std::vector<double> &pt,
               const std::vector<double> &eta,
               const std::vector<double> &phi,
               const std::vector<double> &mass) -> double {
    
                size_t n = std::min({pt.size(), eta.size(), phi.size(), mass.size()});
                if (n < 2) return 999.0;
    
                double minM = std::numeric_limits<double>::infinity();
                for (size_t i = 0; i + 1 < n; ++i) {
                    TLorentzVector li;
                    li.SetPtEtaPhiM(pt[i], eta[i], phi[i], mass[i]);
                    for (size_t j = i+1; j < n; ++j) {
                        TLorentzVector lj;
                        lj.SetPtEtaPhiM(pt[j], eta[j], phi[j], mass[j]);
                        double m = (li + lj).M();
                        if (m < minM) minM = m;
                    }
                }
                return (minM == std::numeric_limits<double>::infinity()) ? 999.0 : minM;
            },
            {"PT_lep","Eta_lep","Phi_lep","M_lep"}
        );
    }
    
    CutDef cut_minMll_gt1;
    cut_minMll_gt1.name = "minM_ll";
    cut_minMll_gt1.columns = {"minMll"};
    cut_minMll_gt1.expression = "minMll > 1.0";
    cuts[cut_minMll_gt1.name] = cut_minMll_gt1;

    if (do_2Dll_low && !node.HasColumn("pass2DLeptonCut_low")) {
        node = node.Define(
            "pass2DLeptonCut_low",
            [](const std::vector<double> &pt,
               const std::vector<double> &eta,
               const std::vector<double> &phi,
               const std::vector<double> &mass) -> bool {
    
                size_t n = std::min({pt.size(), eta.size(), phi.size(), mass.size()});
                if (n < 2) return true;
    
                for (size_t i = 0; i + 1 < n; ++i) {
                    TLorentzVector li;
                    li.SetPtEtaPhiM(pt[i], eta[i], phi[i], mass[i]);
    
                    for (size_t j = i + 1; j < n; ++j) {
                        TLorentzVector lj;
                        lj.SetPtEtaPhiM(pt[j], eta[j], phi[j], mass[j]);
    
                        double dr  = li.DeltaR(lj);
                        double mll = (li + lj).M();
    
                        double mll_min = -1.0 * dr + 1.5;
                        if (mll_min < 0.0) mll_min = 0.0;
    
                        if (mll <= mll_min)
                            return false;
                    }
                }
                return true;
            },
            {"PT_lep","Eta_lep","Phi_lep","M_lep"}
        );
    }

    CutDef cut_2D_ll_low;
    cut_2D_ll_low.name = "minMll_minDR_2D_low";
    cut_2D_ll_low.columns = {"pass2DLeptonCut_low"};
    cut_2D_ll_low.expression = "pass2DLeptonCut_low == true";
    cuts[cut_2D_ll_low.name] = cut_2D_ll_low;

    if (run_validation) validateCutsUser(node, ValidCuts, cuts);
    return node;
}

void BuildFitInput::validateCutsUser(ROOT::RDF::RNode &node, std::map<std::string, CutDef>& ValidCuts, std::map<std::string, CutDef>& cuts){
    ValidCuts = ValidateCuts(node, cuts);
    for (const auto &kv : cuts) {
        if (!ValidCuts.count(kv.first)) {
            std::cerr << "[BuildFitInput loadUserCuts WARN] User cut \"" << kv.first
                      << "\" failed validation and will be ignored.\n";
        }
    }
}
