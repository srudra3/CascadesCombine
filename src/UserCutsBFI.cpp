#include "BuildFitInput.h"
#include <iostream>
#include <limits>
#include <TLorentzVector.h>
#include "EventShape.h"
#include "FourVec.h"
// -------------------------
// User-defined cuts loader
// -------------------------
ROOT::RDF::RNode BuildFitInput::loadCutsUser(ROOT::RDF::RNode &node, std::map<std::string, CutDef>& ValidCuts, bool run_validation){
    std::map<std::string, CutDef> cuts;
    RegisterSafeHelpers();

    // --- lead S jet pt < threshold (45 GeV) ---
    CutDef cut_leadSjetPt;
    cut_leadSjetPt.name = "leadSjet_pt";
    cut_leadSjetPt.columns =
        {"leadSjet_Pt45"}; // use what's saved in tree
        //{"PT_jet", "index_jet_S", "Njet_S"};
    cut_leadSjetPt.expression =
        //"(Njet_S == 0) || (Njet_S > 0 && SafeIndex(PT_jet, SafeIndex(index_jet_S, 0, -1), -1.0) < 45)";
        "leadSjet_Pt45==1"; // use what's saved in tree
    cuts[cut_leadSjetPt.name] = cut_leadSjetPt;

    // --- HEM Veto ---
    CutDef cut_HEMVETO;
    cut_HEMVETO.name = "HEM_Veto";
    cut_HEMVETO.columns = {"pass_HEM"};
    cut_HEMVETO.expression = "pass_HEM";
    cuts[cut_HEMVETO.name] = cut_HEMVETO;

    // flags to turn on/off different cut defs (minor computation save)
    bool do_minDR = true;
    bool do_minMll = true;
    bool do_2Dll_low = true;
    //bool do_2Dll_high = false;
    //bool do_lowptmuon_endcap = false;

    // --- min DeltaR between any two leptons ---
    if (do_minDR && node.HasColumn("minDeltaR_leps") == false) {
        node = node.Define("minDeltaR_leps",
            [](const std::vector<double> &pt,
               const std::vector<double> &eta,
               const std::vector<double> &phi,
               const std::vector<double> &mass) -> double {
        
                // determine safe element count
                size_t n = std::min(std::min(pt.size(), eta.size()), std::min(phi.size(), mass.size()));
                if (n < 2) return 999.0; // sentinel for "no pair"
        
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

    // require lep min DeltaR > 0.02
    CutDef cut_minDR_ll;
    cut_minDR_ll.name = "minDR_ll";
    cut_minDR_ll.columns = {"minDeltaR_leps"};
    cut_minDR_ll.expression = "minDeltaR_leps > 0.02";
    cuts[cut_minDR_ll.name] = cut_minDR_ll;


  // --- define PT, Eta, SIP3D for Gold+Silver leptons ---
if(!node.HasColumn("PT_GS_lep")) {
    node = node.Define("PT_GS_lep", [](const std::vector<double> &pt,
                                       const std::vector<int> &qual) {
        std::vector<double> out;
        for (size_t i = 0; i < pt.size(); i++) {
            if (qual[i] == 0 || qual[i] == 1) // 0=Gold,1=Silver
                out.push_back(pt[i]);
        }
        return out;
    }, {"PT_lep","LepQual_lep"});

    node = node.Define("Eta_GS_lep", [](const std::vector<double> &eta,
                                        const std::vector<int> &qual) {
        std::vector<double> out;
        for (size_t i = 0; i < eta.size(); i++) {
            if (qual[i] == 0 || qual[i] == 1)
                out.push_back(eta[i]);
        }
        return out;
    }, {"Eta_lep","LepQual_lep"});

    node = node.Define("SIP3D_GS_lep", [](const std::vector<double> &sip,
                                          const std::vector<int> &qual) {
        std::vector<double> out;
        for (size_t i = 0; i < sip.size(); i++) {
            if (qual[i] == 0 || qual[i] == 1)
                out.push_back(sip[i]);
        }
        return out;
    }, {"SIP3D_lep","LepQual_lep"});
}  

// --- user cut for Gold/Silver leptons ---
CutDef cutPT_GS;
cutPT_GS.name = "PT_GS_cuts";
cutPT_GS.columns = {"PT_GS_lep","Eta_GS_lep","SIP3D_GS_lep"};
cutPT_GS.expression = 
    "(PT_GS_lep.size()>=4) && "
    "(PT_GS_lep[0]>18) && (PT_GS_lep[1]>8) && "
    "(PT_GS_lep[2]>6) && (PT_GS_lep[3]>2) && "
    "(MAX(abs(Eta_GS_lep))<2.1) && "
    "(MAX(SIP3D_GS_lep)<5)";

cuts[cutPT_GS.name] = cutPT_GS;


// --- define pass/fail for PT, Eta, SIP3D using GS leptons ---
if (!node.HasColumn("passPtEtaSip_GS")) {
    node = node.Define("passPtEtaSip_GS",
        [](const std::vector<double>& pt,
           const std::vector<double>& eta,
           const std::vector<double>& sip,
           const std::vector<int>& qual) -> int {

            std::vector<double> pts, etas, sips;

            for (size_t i = 0; i < pt.size(); ++i) {
                if (qual[i] == 0 || qual[i] == 1) {
                    pts.push_back(pt[i]);
                    etas.push_back(std::abs(eta[i]));
                    sips.push_back(sip[i]);
                }
            }

            if (pts.size() < 4) return 0;

            // sort pT descending (CRUCIAL difference vs your earlier version)
            std::sort(pts.begin(), pts.end(), std::greater<double>());

            if (pts[0] < 18 || pts[1] < 8 || pts[2] < 6 || pts[3] < 2)
                return 0;

            double maxEta = *std::max_element(etas.begin(), etas.end());
            double maxSip = *std::max_element(sips.begin(), sips.end());

            if (maxEta >= 2.1) return 0;
            if (maxSip >= 5.0) return 0;

            return 1;
        },
        {"PT_lep","Eta_lep","SIP3D_lep","LepQual_lep"}
    );
}

// --- define the cut ---
CutDef cut_PtEtaSip_GS;
cut_PtEtaSip_GS.name = "PT_GS_cuts_debug";
cut_PtEtaSip_GS.columns = {"passPtEtaSip_GS"};
cut_PtEtaSip_GS.expression = "passPtEtaSip_GS == 1";

cuts[cut_PtEtaSip_GS.name] = cut_PtEtaSip_GS;






// --- define PT, Eta, SIP3D for ALL leptons (no GS filtering) ---
if(!node.HasColumn("PT_lep_all")) {
    node = node.Define("PT_lep_all", [](const std::vector<double> &pt) {
        return pt;
    }, {"PT_lep"});

    node = node.Define("Eta_lep_all", [](const std::vector<double> &eta) {
        return eta;
    }, {"Eta_lep"});

    node = node.Define("SIP3D_lep_all", [](const std::vector<double> &sip) {
        return sip;
    }, {"SIP3D_lep"});
}

// --- user cut (no GS filtering inside!) ---
CutDef cutPT_debug;
cutPT_debug.name = "PT_debug_cuts";
cutPT_debug.columns = {"PT_lep_all","Eta_lep_all","SIP3D_lep_all"};
cutPT_debug.expression =
    "(PT_lep_all.size()>=4) && "
    "(PT_lep_all[0]>18) && (PT_lep_all[1]>8) && "
    "(PT_lep_all[2]>6) && (PT_lep_all[3]>2) && "
    "(MAX(abs(Eta_lep_all))<2.1) && "
    "(MAX(SIP3D_lep_all)<5)";

cuts[cutPT_debug.name] = cutPT_debug;








// --- min mass between any two Gold/Silver leptons ---
if (do_minMll && !node.HasColumn("minMll_GS")) {
    node = node.Define("minMll_GS",
        [](const std::vector<double> &pt,
           const std::vector<double> &eta,
           const std::vector<double> &phi,
           const std::vector<double> &mass,
           const std::vector<int> &qual) -> double {

            // --- select only Gold/Silver leptons ---
            std::vector<double> ptGS, etaGS, phiGS, massGS;
            for (size_t i = 0; i < pt.size(); ++i) {
                if (qual[i] == 0 || qual[i] == 1) { // 0=Gold, 1=Silver
                    ptGS.push_back(pt[i]);
                    etaGS.push_back(eta[i]);
                    phiGS.push_back(phi[i]);
                    massGS.push_back(mass[i]);
                }
            }

            size_t n = ptGS.size();
            if (n < 2) return 999.0;

            double minM = std::numeric_limits<double>::infinity();
            for (size_t i = 0; i + 1 < n; ++i) {
                TLorentzVector li;
                li.SetPtEtaPhiM(ptGS[i], etaGS[i], phiGS[i], massGS[i]);
                for (size_t j = i+1; j < n; ++j) {
                    TLorentzVector lj;
                    lj.SetPtEtaPhiM(ptGS[j], etaGS[j], phiGS[j], massGS[j]);
                    double m = (li + lj).M();
                    if (m < minM) minM = m;
                }
            }

            return (minM == std::numeric_limits<double>::infinity()) ? 999.0 : minM;
        },
        {"PT_lep","Eta_lep","Phi_lep","M_lep","LepQual_lep"}
    );
}

CutDef cut_minMll_gt4_GS;
cut_minMll_gt4_GS.name = "minM_ll_gt4_GS";
cut_minMll_gt4_GS.columns = {"minMll_GS"};
cut_minMll_gt4_GS.expression = "minMll_GS > 4.0";
cuts[cut_minMll_gt4_GS.name] = cut_minMll_gt4_GS;

if (do_minMll && !node.HasColumn("minMll_debug")) {
    node = node.Define("minMll_debug",
        [](const std::vector<double> &pt,
           const std::vector<double> &eta,
           const std::vector<double> &phi,
           const std::vector<double> &mass) -> double {

            if (pt.size() < 4) return 999.0;

            double minM = std::numeric_limits<double>::infinity();

            for (size_t i = 0; i < 4; ++i) {
                TLorentzVector li;
                li.SetPtEtaPhiM(pt[i], eta[i], phi[i], mass[i]);

                for (size_t j = i+1; j < 4; ++j) {
                    TLorentzVector lj;
                    lj.SetPtEtaPhiM(pt[j], eta[j], phi[j], mass[j]);

                    double m = (li + lj).M();
                    if (m < minM) minM = m;
                }
            }

            return minM;
        },
        {"PT_lep","Eta_lep","Phi_lep","M_lep"}
    );
}

CutDef cut_minMll;
cut_minMll.name = "minMll_gt4";
cut_minMll.columns = {"minMll_debug"};
cut_minMll.expression = "minMll_debug > 4.0";
cuts[cut_minMll.name] = cut_minMll;
















const double MZ = 91.1876;

    if (!node.HasColumn("minDeltaMll_GS")) {
      node = node.Define("minDeltaMll_GS",
        [MZ](const std::vector<double> &pt,
             const std::vector<double> &eta,
             const std::vector<double> &phi,
             const std::vector<double> &mass,
             const std::vector<int> &charge,
             const std::vector<int> &pdgId,
             const std::vector<int> &qual) -> double {

            // --- filter Gold+Silver leptons ---
            std::vector<double> ptGS, etaGS, phiGS, massGS;
            std::vector<int> chargeGS, pdgIdGS;
            for (size_t i = 0; i < pt.size(); ++i) {
                if (qual[i] == 0 || qual[i] == 1) { // 0=Gold, 1=Silver
                    ptGS.push_back(pt[i]);
                    etaGS.push_back(eta[i]);
                    phiGS.push_back(phi[i]);
                    massGS.push_back(mass[i]);
                    chargeGS.push_back(charge[i]);
                    pdgIdGS.push_back(pdgId[i]);
                }
            }

            size_t n = ptGS.size();
            if (n < 2) return 999.0;

            double minDelta = std::numeric_limits<double>::infinity();

            for (size_t i = 0; i + 1 < n; ++i) {
                for (size_t j = i+1; j < n; ++j) {

                    // OSSF requirement
                    if (chargeGS[i] * chargeGS[j] >= 0) continue;
                    if (std::abs(pdgIdGS[i]) != std::abs(pdgIdGS[j])) continue;

                    TLorentzVector li, lj;
                    li.SetPtEtaPhiM(ptGS[i], etaGS[i], phiGS[i], massGS[i]);
                    lj.SetPtEtaPhiM(ptGS[j], etaGS[j], phiGS[j], massGS[j]);

                    double mll = (li + lj).M();
                    double delta = std::abs(mll - MZ);

                    if (delta < minDelta) minDelta = delta;
                }
            }

            return (minDelta == std::numeric_limits<double>::infinity()) ? 999.0 : minDelta;
        },
        {"PT_lep","Eta_lep","Phi_lep","M_lep","Charge_lep","PDGID_lep","LepQual_lep"}
    );
}

CutDef cut_Zveto_GS;
cut_Zveto_GS.name = "Z_veto_OSSF_GS";
cut_Zveto_GS.columns = {"minDeltaMll_GS"};
cut_Zveto_GS.expression = "minDeltaMll_GS > 7.5";
cuts[cut_Zveto_GS.name] = cut_Zveto_GS;

if (!node.HasColumn("minDeltaMll_4l")) {
  node = node.Define("minDeltaMll_4l",
    [MZ](const std::vector<double> &pt,
         const std::vector<double> &eta,
         const std::vector<double> &phi,
         const std::vector<double> &mass,
         const std::vector<int> &charge,
         const std::vector<int> &pdgId) -> double {

        if (pt.size() < 4) return 999.0;

        double minDelta = std::numeric_limits<double>::infinity();

        for (size_t i = 0; i < 4; ++i) {
            for (size_t j = i+1; j < 4; ++j) {

                if (charge[i] * charge[j] >= 0) continue;
                if (std::abs(pdgId[i]) != std::abs(pdgId[j])) continue;

                TLorentzVector li, lj;
                li.SetPtEtaPhiM(pt[i], eta[i], phi[i], mass[i]);
                lj.SetPtEtaPhiM(pt[j], eta[j], phi[j], mass[j]);

                double mll = (li + lj).M();
                double delta = std::abs(mll - MZ);

                if (delta < minDelta) minDelta = delta;
            }
        }

        return (minDelta == std::numeric_limits<double>::infinity()) ? 999.0 : minDelta;
    },
    {"PT_lep","Eta_lep","Phi_lep","M_lep","Charge_lep","PDGID_lep"}
  );
}

CutDef cut_Zveto;
cut_Zveto.name = "Z_veto_OSSF";
cut_Zveto.columns = {"minDeltaMll_4l"};
cut_Zveto.expression = "minDeltaMll_4l > 7.5";
cuts[cut_Zveto.name] = cut_Zveto;






if (!node.HasColumn("M4l_GS")) {
    node = node.Define("M4l_GS",
        [](const std::vector<double> &pt,
           const std::vector<double> &eta,
           const std::vector<double> &phi,
           const std::vector<double> &mass,
           const std::vector<int> &qual) -> double {

            // --- select Gold+Silver leptons ---
            std::vector<TLorentzVector> leptons;
            for (size_t i = 0; i < pt.size(); ++i) {
                if (qual[i] == 0 || qual[i] == 1) {
                    TLorentzVector lv;
                    lv.SetPtEtaPhiM(pt[i], eta[i], phi[i], mass[i]);
                    leptons.push_back(lv);
                }
            }

            // --- require at least 4 leptons ---
            if (leptons.size() < 4) return -1.0;

            // --- take the 4 leading PT leptons ---
            std::sort(leptons.begin(), leptons.end(),
                      [](const TLorentzVector &a, const TLorentzVector &b){ return a.Pt() > b.Pt(); });

            TLorentzVector sum = leptons[0] + leptons[1] + leptons[2] + leptons[3];
            return sum.M();
        },
        {"PT_lep","Eta_lep","Phi_lep","M_lep","LepQual_lep"}
    );
}



CutDef cut_M4l_Zveto;
cut_M4l_Zveto.name = "M4l_not_Z";
cut_M4l_Zveto.columns = {"M4l_GS"};
cut_M4l_Zveto.expression = "abs(M4l_GS - 91.1876) > 10.0";
cuts[cut_M4l_Zveto.name] = cut_M4l_Zveto;

CutDef cut_M4l_Hveto;
cut_M4l_Hveto.name = "M4l_not_H";
cut_M4l_Hveto.columns = {"M4l_GS"};
cut_M4l_Hveto.expression = "!(M4l_GS > 121 && M4l_GS < 127)";
cuts[cut_M4l_Hveto.name] = cut_M4l_Hveto;



if (!node.HasColumn("Pt4LJets_GS")) {
    node = node.Define("Pt4LJets_GS",
        [](const std::vector<double> &pt,
           const std::vector<double> &eta,
           const std::vector<double> &phi,
           const std::vector<double> &mass,
           const std::vector<int> &qual,
           const int Njet,
           const std::vector<double> &PT_jet,
           const std::vector<double> &Eta_jet,
           const std::vector<double> &Phi_jet,
           const std::vector<double> &M_jet) -> double {

            // --- select Gold+Silver leptons ---
            std::vector<TLorentzVector> leptons;
            for (size_t i = 0; i < pt.size(); ++i) {
                if (qual[i] == 0 || qual[i] == 1) {
                    TLorentzVector lv;
                    lv.SetPtEtaPhiM(pt[i], eta[i], phi[i], mass[i]);
                    leptons.push_back(lv);
                }
            }

            if (leptons.size() < 4) return -1.0; // not enough leptons

            // --- take 4 leading PT leptons ---
            std::sort(leptons.begin(), leptons.end(),
                      [](const TLorentzVector &a, const TLorentzVector &b){ return a.Pt() > b.Pt(); });
            TLorentzVector sum4L = leptons[0] + leptons[1] + leptons[2] + leptons[3];

            // --- handle jets if present ---
            TLorentzVector sum4LJets = sum4L;
            if (Njet > 0) {
                for (int i = 0; i < Njet; ++i) {
                    TLorentzVector jet;
                    jet.SetPtEtaPhiM(PT_jet[i], Eta_jet[i], Phi_jet[i], M_jet[i]);
                    sum4LJets += jet;
                }
            }

            // --- apply cut logic: 0J vs >=1J ---
            if (Njet == 0) return sum4L.Pt();      // 0-jet → return 4-lepton pt
            else          return sum4LJets.Pt();   // >=1 jet → return 4L+Jets pt
        },
        {"PT_lep","Eta_lep","Phi_lep","M_lep","LepQual_lep",
         "Njet","PT_jet","Eta_jet","Phi_jet","M_jet"});
}

// --- define the cut ---
CutDef cut_Pt4LJets;
cut_Pt4LJets.name = "Pt4LJets_cut";
cut_Pt4LJets.columns = {"Pt4LJets_GS"};
cut_Pt4LJets.expression = "(Njet==0 && Pt4LJets_GS>10.0) || (Njet>=1 && Pt4LJets_GS>30.0)";
cuts[cut_Pt4LJets.name] = cut_Pt4LJets;

// --- max hemisphere mass + topology for GS leptons ---
if (!node.HasColumn("passThrustHemiCut_GS")) {
    node = node.Define("passThrustHemiCut_GS",
        [](const std::vector<double> &pt,
           const std::vector<double> &eta,
           const std::vector<double> &phi,
           const std::vector<double> &mass,
           const std::vector<int> &qual) -> int {

            std::vector<FourVec> leptons;
            for (size_t i = 0; i < pt.size(); ++i) {
                if (qual[i] == 0 || qual[i] == 1) {
                    leptons.emplace_back(FourVec::FromPtEtaPhiM(0, pt[i], eta[i], phi[i], mass[i])
			    );
                }
            }

            if (leptons.size() < 4) return 0;

            leptons.resize(4);

            auto cs = EventShape::ComputeThrustAxisInCS(leptons, 6800.0);
            auto hemis = EventShape::ComputeThrustHemispheres(leptons, cs.zCS);

            int nlepMax = std::max(hemis.nPlus, hemis.nMinus);
            double maxMass = std::max(hemis.massPlus, hemis.massMinus);

            if (nlepMax == 2) {
                return (maxMass < 23.0);
            } else {
                return (maxMass < 39.0);
            }
        },
        {"PT_lep","Eta_lep","Phi_lep","M_lep","LepQual_lep"}
    );
}

CutDef cut_thrustHemi_GS;
cut_thrustHemi_GS.name = "thrustHemi_GS";
cut_thrustHemi_GS.columns = {"passThrustHemiCut_GS"};
cut_thrustHemi_GS.expression = "passThrustHemiCut_GS == 1";

cuts[cut_thrustHemi_GS.name] = cut_thrustHemi_GS;



if (do_2Dll_low && !node.HasColumn("pass2DLeptonCut_low")) {
        node = node.Define(
            "pass2DLeptonCut_low",
            [](const std::vector<double> &pt,
               const std::vector<double> &eta,
               const std::vector<double> &phi,
               const std::vector<double> &mass) -> bool {
    
                size_t n = std::min({pt.size(), eta.size(), phi.size(), mass.size()});
                if (n < 2) return true; // no pair so pass
    
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
    
                        // If ANY pair fails -> event fails
                        if (mll <= mll_min)
                            return false;
                    }
                }
                return true; // all pairs passed
            },
            {"PT_lep","Eta_lep","Phi_lep","M_lep"}
        );
    }

    CutDef cut_2D_ll_low;
    cut_2D_ll_low.name = "minMll_minDR_2D_low";
    cut_2D_ll_low.columns = {"pass2DLeptonCut_low"};
    cut_2D_ll_low.expression = "pass2DLeptonCut_low == true";
    cuts[cut_2D_ll_low.name] = cut_2D_ll_low;

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







    //if (do_2Dll_high && !node.HasColumn("pass2DLeptonCut_high")) {
    //    node = node.Define("pass2DLeptonCut_high",
    //        [](double mll, double dr) -> bool {
    //            // Equation of the boundary line:
    //            //              \/DR          \/Mass
    //            double dr_min = 0.05 * (mll - 1.0);
    //
    //            if (dr_min < 0) dr_min = 0;
    //
    //            return dr > dr_min;
    //        },
    //        {"minMll", "minDeltaR_leps"}
    //    );
    //}

    //CutDef cut_2D_ll_high;
    //cut_2D_ll_high.name = "minMll_minDR_2D_high";
    //cut_2D_ll_high.columns = {"pass2DLeptonCut_high"};
    //cut_2D_ll_high.expression = "pass2DLeptonCut_high == true";
    //cuts[cut_2D_ll_high.name] = cut_2D_ll_high;

    //// --- remove low pt forward muons ---
    //// also need to try only removing events where all muons are low pt and forward
    //if (do_lowptmuon_endcap && !node.HasColumn("lowptmuon_endcap")) {
    //    node = node.Define(
    //        "lowptmuon_endcap",
    //        [](const std::vector<double> &pt,
    //           const std::vector<double> &eta,
    //           const std::vector<int> &pdgid,
    //           int nlep,
    //           int nmu
    //          ) -> bool {
    //            if(nmu == 0) return true; // keep events without muons no matter what

    //            // get number of low pt end-cap muons
    //            int nmu_lowpt_endcap = 0;
    //            for (int i = 0; i < nlep; i++) {
    //                if (std::abs(pdgid[i]) == 13 && pt[i] < 5.0 && std::abs(eta[i]) > 1.566) {
    //                    nmu_lowpt_endcap++;
    //                }
    //            }
    //            //if(nmu_lowpt_endcap != 0) return false; // remove events where any muon is low pt and end-cap
    //            if(nmu_lowpt_endcap == nmu) return false; // remove events where all muons are low pt and end-cap
    //            return true; // keep event
    //        },
    //        {"PT_lep", "Eta_lep", "PDGID_lep", "Nlep", "Nmu"}
    //    );
    //}

    //CutDef cut_lowptmuon_endcap;
    //cut_lowptmuon_endcap.name = "lowptmuon_endcap";
    //cut_lowptmuon_endcap.columns = {"lowptmuon_endcap"};
    //cut_lowptmuon_endcap.expression = "lowptmuon_endcap==1";
    //cuts[cut_lowptmuon_endcap.name] = cut_lowptmuon_endcap;

    //node = node
    //    .Define("My_p4_lep0_a", [](const std::vector<double> &pt,
    //                          const std::vector<double> &eta,
    //                          const std::vector<double> &phi,
    //                          const std::vector<double> &mass,
    //                          const std::vector<int> &index_lep_a_LEP,
    //                          const int &Nlep) {
    //            // Construct TLV for lepton 0 on A side if present; otherwise returns a zero TLV.
    //            TLorentzVector v;
    //            if (index_lep_a_LEP.size() >= 1 && Nlep > 2) {
    //                // safe access to [index_lep_a_LEP[0]]
    //                v.SetPtEtaPhiM(pt[index_lep_a_LEP[0]], eta[index_lep_a_LEP[0]], phi[index_lep_a_LEP[0]], mass[index_lep_a_LEP[0]]);
    //            }
    //            else if (Nlep == 2) {
    //                // safe access to [0]
    //                v.SetPtEtaPhiM(pt[0], eta[0], phi[0], mass[0]);
    //            }
    //            return v;
    //        }, {"PT_lep","Eta_lep","Phi_lep","M_lep","index_lep_a_LEP","Nlep"})
    //    .Define("My_p4_lep1_a", [](const std::vector<double> &pt,
    //                          const std::vector<double> &eta,
    //                          const std::vector<double> &phi,
    //                          const std::vector<double> &mass,
    //                          const std::vector<int> &index_lep_a_LEP,
    //                          const int &Nlep) {
    //            // Construct TLV for lepton 1 on A side if present; otherwise returns a zero TLV.
    //            TLorentzVector v;
    //            if (index_lep_a_LEP.size() >= 2 && Nlep > 2) {
    //                // safe access to [index_lep_a_LEP[1]]
    //                v.SetPtEtaPhiM(pt[index_lep_a_LEP[1]], eta[index_lep_a_LEP[1]], phi[index_lep_a_LEP[1]], mass[index_lep_a_LEP[1]]);
    //            }
    //            else if (Nlep == 2) {
    //                // safe access to [1]
    //                v.SetPtEtaPhiM(pt[1], eta[1], phi[1], mass[1]);
    //            }
    //            return v;
    //        }, {"PT_lep","Eta_lep","Phi_lep","M_lep","index_lep_a_LEP","Nlep"})
    //    .Define("Q_lep0_a", [](const std::vector<int> &charge,
    //                           const std::vector<int> &index_lep_a_LEP,
    //                           const int &Nlep){
    //            if(Nlep > 2)       return index_lep_a_LEP.size() >= 1 ? charge[index_lep_a_LEP[0]] : 0;
    //            else if(Nlep == 2) return charge[0];
    //            else return 0;
    //        }, {"Charge_lep","index_lep_a_LEP","Nlep"})
    //    .Define("Q_lep1_a", [](const std::vector<int> &charge,
    //                           const std::vector<int> &index_lep_a_LEP,
    //                           const int &Nlep){
    //            if(Nlep > 2)       return index_lep_a_LEP.size() >= 2 ? charge[index_lep_a_LEP[1]] : 0;
    //            else if(Nlep == 2) return charge[1];
    //            else return 0;
    //        }, {"Charge_lep","index_lep_a_LEP","Nlep"})
    //    .Define("TLV_Cand", [](const TLorentzVector &l0,
    //                           const TLorentzVector &l1
    //                          ) {
    //            return l0 + l1;
    //        }, {"My_p4_lep0_a","My_p4_lep1_a"
    //           })
    //    .Define("CandBetaZLab", [](const TLorentzVector &Cand){
    //            return (Cand.E() != 0.0) ? Cand.Pz() / Cand.E() : 0.0;
    //        }, {"TLV_Cand"})
    //    .Define("CandCosDecayAngleLab",
    //        [](const TLorentzVector &cand,
    //           const TLorentzVector &l0,
    //           const TLorentzVector &l1,
    //           int q0, int q1) -> double
    //        {
    //            // choose the positively charged child TLV (prefer l0 if both >0, fallback to l1)
    //            TLorentzVector child = TLorentzVector{};
    //            if (q0 > 0) child = l0;
    //            else if (q1 > 0) child = l1;
    //            else return -2.; // no positively charged child -> single lep
    //    
    //            // safety: candidate must have nonzero energy (otherwise boost undefined)
    //            if (cand.E() == 0.0) return -2.;
    //    
    //            TVector3 boost = cand.BoostVector();
    //    
    //            // boost the child into the candidate rest frame
    //            child.Boost(-boost);
    //    
    //            TVector3 childVec = child.Vect();
    //            if (childVec.Mag() == 0.0 || boost.Mag() == 0.0) return -2.;
    //    
    //            return fabs(childVec.Unit().Dot(boost.Unit()));
    //        },
    //        {"TLV_Cand", "My_p4_lep0_a", "My_p4_lep1_a", "Q_lep0_a", "Q_lep1_a"})
    //;
    //CutDef cutBetaZLab0p9;
    //cutBetaZLab0p9.name = "BetaZLab0p9";
    //cutBetaZLab0p9.columns = {"CandBetaZLab"};
    //cutBetaZLab0p9.expression = "CandBetaZLab < 0.9";
    //cuts[cutBetaZLab0p9.name] = cutBetaZLab0p9;
    //CutDef cutBetaZLab0p95;
    //cutBetaZLab0p95.name = "BetaZLab0p95";
    //cutBetaZLab0p95.columns = {"CandBetaZLab"};
    //cutBetaZLab0p95.expression = "CandBetaZLab < 0.95";
    //cuts[cutBetaZLab0p95.name] = cutBetaZLab0p95;
    //CutDef cutCosDecayAngleLab0p9;
    //cutCosDecayAngleLab0p9.name = "CosDecayAngleLab0p9";
    //cutCosDecayAngleLab0p9.columns = {"CandCosDecayAngleLab"};
    //cutCosDecayAngleLab0p9.expression = "CandCosDecayAngleLab < 0.9";
    //cuts[cutCosDecayAngleLab0p9.name] = cutCosDecayAngleLab0p9;
    //CutDef cutCosDecayAngleLab0p95;
    //cutCosDecayAngleLab0p95.name = "CosDecayAngleLab0p95";
    //cutCosDecayAngleLab0p95.columns = {"CandCosDecayAngleLab"};
    //cutCosDecayAngleLab0p95.expression = "CandCosDecayAngleLab < 0.95";
    //cuts[cutCosDecayAngleLab0p95.name] = cutCosDecayAngleLab0p95;

    /*
    // Example 1: invariant mass of leading two jets -> M_jj (double)
    node = node.Define("M_jj", [](const std::vector<double> &pt,
                                  const std::vector<double> &eta,
                                  const std::vector<double> &phi,
                                  const std::vector<double> &mass) {
        // Return -1.0 if not enough jets
        if (pt.size() < 2 || eta.size() < 2 || phi.size() < 2 || mass.size() < 2) return -1.0;
        // compute using temporary TLorentzVector locally (we only return a double)
        TLorentzVector j0, j1;
        j0.SetPtEtaPhiM(pt[0], eta[0], phi[0], mass[0]);
        j1.SetPtEtaPhiM(pt[1], eta[1], phi[1], mass[1]);
        return (j0 + j1).M();
    }, {"PT_jet","Eta_jet","Phi_jet","M_jet"});

    CutDef cut1;
    cut1.name = "M_jj_gt_100";
    cut1.columns = {"M_jj"};
    cut1.expression = "M_jj > 100";
    cuts[cut1.name] = cut1;

    // -----------------------------------------------------------------
    // Example 2: HT_eta24 / MET ratio > 1.5 
    // -----------------------------------------------------------------
    node = node.Define("HT_eta24_over_MET", [](double HT_eta24, double MET) {
        if (MET == 0.0) return 0.0;
        return HT_eta24 / MET;
    }, {"HT_eta24","MET"});

    CutDef cut2;
    cut2.name = "HT_eta24_over_MET_gt_1p5";
    cut2.columns = {"HT_eta24_over_MET"};
    cut2.expression = "HT_eta24_over_MET > 1.5";
    cuts[cut2.name] = cut2;

    // -----------------------------------------------------------------
    // Example 3: Combined lepton-jet cut (pT + DeltaR)
    // Compute DeltaR using TLorentzVector::DeltaR, return double
    // Also define leading pT columns to avoid unsafe indexing
    // -----------------------------------------------------------------
    
    // Leading lepton pT
    node = node.Define("PT_lep0", [](const std::vector<double> &pt){
        return pt.empty() ? 0.0 : pt[0];
    }, {"PT_lep"});
    
    // Leading jet pT
    node = node.Define("PT_jet0", [](const std::vector<double> &pt){
        return pt.empty() ? 0.0 : pt[0];
    }, {"PT_jet"});
    */
    
    /*
    // DeltaR between leading lepton and leading jet
    node = node.Define("DeltaR_lep0_jet0", [](const std::vector<double> &pt_lep,
                                             const std::vector<double> &eta_lep,
                                             const std::vector<double> &phi_lep,
                                             const std::vector<double> &m_lep,
                                             const std::vector<double> &pt_jet,
                                             const std::vector<double> &eta_jet,
                                             const std::vector<double> &phi_jet,
                                             const std::vector<double> &m_jet) {
        if(pt_lep.empty() || eta_lep.empty() || phi_lep.empty() || m_lep.empty() ||
           pt_jet.empty() || eta_jet.empty() || phi_jet.empty() || m_jet.empty()) {
            return 999.0; // safe fallback
        }
    
        TLorentzVector lep, jet;
        lep.SetPtEtaPhiM(pt_lep[0], eta_lep[0], phi_lep[0], m_lep[0]);
        jet.SetPtEtaPhiM(pt_jet[0], eta_jet[0], phi_jet[0], m_jet[0]);
    
        return lep.DeltaR(jet);
    }, {"PT_lep","Eta_lep","Phi_lep","M_lep",
        "PT_jet","Eta_jet","Phi_jet","M_jet"});
    
    // Define the user cut using only doubles, safe for validation
    CutDef cut3;
    cut3.name = "lep0_pt25_jet0_pt30_dR0p4";
    cut3.columns = {"PT_lep0","PT_jet0","DeltaR_lep0_jet0"};
    cut3.expression = "(PT_lep0 > 25) && (PT_jet0 > 30) && (DeltaR_lep0_jet0 > 0.4)";
    cuts[cut3.name] = cut3;
    */
    
    











    // -----------------------------------------------------------------------------
    // Store 4-vectors as ROOT::Math::PtEtaPhiMVector (replaces TLV for RDataFrame)
    // How to create 4-vector columns (less recommended for simple cuts)
    // -----------------------------------------------------------------------------
    /*
    node = node
        .Define("p4_jet0_vect", [](const std::vector<double> &pt,
                                   const std::vector<double> &eta,
                                   const std::vector<double> &phi,
                                   const std::vector<double> &mass) {
            ROOT::Math::PtEtaPhiMVector v;
            if (!pt.empty() && pt.size() == eta.size() && eta.size() == phi.size() && phi.size() == mass.size()) {
                v.SetPtEtaPhiM(pt[0], eta[0], phi[0], mass[0]);
            }
            return v;
        }, {"PT_jet","Eta_jet","Phi_jet","M_jet"})
        .Define("p4_jet1_vect", [](const std::vector<double> &pt,
                                   const std::vector<double> &eta,
                                   const std::vector<double> &phi,
                                   const std::vector<double> &mass) {
            ROOT::Math::PtEtaPhiMVector v;
            if (pt.size() > 1 && pt.size() == eta.size() && eta.size() == phi.size() && phi.size() == mass.size()) {
                v.SetPtEtaPhiM(pt[1], eta[1], phi[1], mass[1]);
            }
            return v;
        }, {"PT_jet","Eta_jet","Phi_jet","M_jet"});

    node = node.Define("M_jj_vect", [](const ROOT::Math::PtEtaPhiMVector &j0,
                                       const ROOT::Math::PtEtaPhiMVector &j1) {
        return (j0 + j1).M();
    }, {"p4_jet0_vect", "p4_jet1_vect"});
    // Used *_vect in the names to avoid conflicts with the scalar-versions above.
    */
    if (run_validation) validateCutsUser(node, ValidCuts, cuts);
    return node;
}

void BuildFitInput::validateCutsUser(ROOT::RDF::RNode &node, std::map<std::string, CutDef>& ValidCuts, std::map<std::string, CutDef>& cuts){
    // Validate the cuts that the user wrote
    ValidCuts = ValidateCuts(node, cuts);
    for (const auto &kv : cuts) {
        if (!ValidCuts.count(kv.first)) {
            std::cerr << "[BuildFitInput loadUserCuts WARN] User cut \"" << kv.first
                      << "\" failed validation and will be ignored.\n";
        }
    }
}
