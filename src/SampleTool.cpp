#include "SampleTool.h"

SampleTool::SampleTool(){
  // Path to ntuples in fnal eos
  string pathPrefix = "root://cmseos.fnal.gov//store/user/lpcsusylep/";
  pathPrefix += "NTUPLES_Cascades_v9/";

  LumiDict["HEM_LUMI"] = 21.077794578; // need for HEM veto
  LumiDict["Summer16_102X"] = 16.8+19.5;
  LumiDict["Fall17_102X"] = 41.479680529;
  LumiDict["Autumn18_102X"] = 59.832475339;
  LumiDict["Summer20UL16APV_106X"] = 19.5;
  LumiDict["Summer20UL16_106X"] = 16.8;
  LumiDict["Summer20UL17_106X"] = 42.07;
  LumiDict["Summer20UL18_106X"] = 59.56;
  LumiDict["Summer22_130X"] = 7.99;
  LumiDict["Summer22EE_130X"] = 26.68;
  LumiDict["Summer23_130X"] = 17.96;
  LumiDict["Summer23BPix_130X"] = 9.68;
  LumiDict["Summer24_130X"] = 109.95;
  LumiDict["Summer25_130X"] = 110.84;
  LumiDict["Summer26_130X"] = 28.06;

  double RUN2_TOTAL_LUMI = 0.;
  RUN2_TOTAL_LUMI += LumiDict["Summer20UL16APV_106X"];
  RUN2_TOTAL_LUMI += LumiDict["Summer20UL16_106X"];
  RUN2_TOTAL_LUMI += LumiDict["Summer20UL17_106X"];
  RUN2_TOTAL_LUMI += LumiDict["Summer20UL18_106X"];

  double RUN3_TOTAL_LUMI = 0.;
  RUN3_TOTAL_LUMI += LumiDict["Summer22_130X"]; 
  RUN3_TOTAL_LUMI += LumiDict["Summer22EE_130X"]; 
  RUN3_TOTAL_LUMI += LumiDict["Summer23_130X"]; 
  RUN3_TOTAL_LUMI += LumiDict["Summer23BPix_130X"]; 
  RUN3_TOTAL_LUMI += LumiDict["Summer24_130X"]; 
  RUN3_TOTAL_LUMI += LumiDict["Summer25_130X"]; 
  //RUN3_TOTAL_LUMI += LumiDict["Summer26_130X"]; 

  // Scale lumis since not all samples available yet
  std::vector<std::string> run3Eras = {
      "Summer22_130X",
      "Summer22EE_130X",
      "Summer23_130X",
      "Summer23BPix_130X"
  };
  
  double run3_available_lumi = 0.0;
  for (const auto& era : run3Eras) {
      run3_available_lumi += LumiDict.at(era);
  }
  //double scale_run3 = RUN3_TOTAL_LUMI / run3_available_lumi;
  for (const auto& era : run3Eras) {
      LumiDict.at(era) *= 1.; // 1 = turn off and use default scale (good for CR)
      //LumiDict.at(era) *= scale_run3; // apply scaling for Run3 existing lumi to all lumi (good for Run3 sensitivity)
  }

  std::vector<std::string> run2Eras = {
      "Summer20UL16APV_106X",
      "Summer20UL16_106X",
      "Summer20UL17_106X",
      "Summer20UL18_106X",
  };

  double run2_available_lumi = 0.0;
  for (const auto& era : run2Eras) {
      run2_available_lumi += LumiDict.at(era);
  }
  //double scale_run2 = (RUN2_TOTAL_LUMI + RUN3_TOTAL_LUMI) / run2_available_lumi;
  for (const auto& era : run2Eras) {
      LumiDict.at(era) *= 1.; // 1 = turn off and use default scale 
      //LumiDict.at(era) *= scale_run2; // apply scaling for Run3+Run2 lumi to all Run2 eras
  }

  std::vector<std::string> preUL_run2Eras = {
      "Summer16_102X",
      "Fall17_102X",
      "Autumn18_102X",
  };
  
  double preUL_run2_available_lumi = 0.0;
  for (const auto& era : preUL_run2Eras) {
      preUL_run2_available_lumi += LumiDict.at(era);
  }
  //double scale_preULrun2 = (RUN2_TOTAL_LUMI + RUN3_TOTAL_LUMI) / preUL_run2_available_lumi;
  for (const auto& era : preUL_run2Eras) {
      LumiDict.at(era) *= 1.; // 1 = turn off and use default scale 
      //LumiDict.at(era) *= scale_preULrun2; // apply scaling for Run3+Run2 lumi to all Run2 eras
  }

  LumiDict["Summer20UL16APV_106X_SMS"] = LumiDict["Summer20UL16APV_106X"];
  LumiDict["Summer20UL16_106X_SMS"] = LumiDict["Summer20UL16_106X"];
  LumiDict["Summer20UL17_106X_SMS"] = LumiDict["Summer20UL17_106X"];
  LumiDict["Summer20UL18_106X_SMS"] = LumiDict["Summer20UL18_106X"];

  LumiDict["Summer16_102X_SMS"] = LumiDict["Summer16_102X"];
  LumiDict["Fall17_102X_SMS"] = LumiDict["Fall17_102X"];
  LumiDict["Autumn18_102X_SMS"] = LumiDict["Autumn18_102X"];

  LumiDict["Summer22_130X_SMS"] = LumiDict["Summer22_130X"] * 1.12; // inflate by % for x-sec boost Run2 to Run3
  LumiDict["Summer22EE_130X_SMS"] = LumiDict["Summer22EE_130X"] * 1.12; // inflate by % for x-sec boost Run2 to Run3
  LumiDict["Summer23_130X_SMS"] = LumiDict["Summer23_130X"] * 1.12; // inflate by % for x-sec boost Run2 to Run3
  LumiDict["Summer23BPix_130X_SMS"] = LumiDict["Summer23BPix_130X"] * 1.12; // inflate by % for x-sec boost Run2 to Run3
  LumiDict["Summer24_130X_SMS"] = LumiDict["Summer24_130X"] * 1.12; // inflate by % for x-sec boost Run2 to Run3
  LumiDict["Summer25_130X_SMS"] = LumiDict["Summer25_130X"] * 1.12; // inflate by % for x-sec boost Run2 to Run3
  LumiDict["Summer26_130X_SMS"] = LumiDict["Summer26_130X"] * 1.12; // inflate by % for x-sec boost Run2 to Run3

  LumiDict["Summer22_130X_Cascades"] = RUN2_TOTAL_LUMI + RUN3_TOTAL_LUMI;
  LumiDict["Summer23BPix_130X_Cascades"] = RUN2_TOTAL_LUMI + RUN3_TOTAL_LUMI;

  // File paths start here

  // For Derek:
  MasterDict["ttbar_derek_2018"] = {
    pathPrefix + "Summer20UL18_106X/TTJets_DiLept_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
  };

  MasterDict["ttbar_2026"] = {
    pathPrefix + "Summer26_130X/TTG-1Jets_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TTGG_TuneCP5_13p6TeV_madgraph-madspin-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TTH-Hto2B-TTto2L2Nu_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TTH-Hto2B-TTto4Q_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TTH-Hto2B-TTtoLNu2Q_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TTH-HtoNon2B_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TTLL_Bin-MLL-4to50_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TTLL_Bin-MLL-50_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TTLNu-1Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TTLNu-EWK_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TTTT_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TTW-WtoQQ-1Jets_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TTWW_TuneCP5_13p6TeV_madgraph-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TTZ-ZtoQQ-1Jets_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TTto2L2Nu-3Jets_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TTto4Q-3Jets_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TTtoLminusNu2Q-3Jets_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TTtoLplusNu2Q-3Jets_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TZQB-ZtoLL-TtoL-CPV_TuneCP5_13p6TeV_madgraph-pythia8_Summer24_Summer26_130X.root",
  };

  MasterDict["ttbar_2025"] = {
    pathPrefix + "Summer25_130X/TTG-1Jets_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TTGG_TuneCP5_13p6TeV_madgraph-madspin-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TTH-Hto2B-TTto2L2Nu_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TTH-Hto2B-TTto4Q_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TTH-Hto2B-TTtoLNu2Q_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TTH-HtoNon2B_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TTLL_Bin-MLL-4to50_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TTLL_Bin-MLL-50_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TTLNu-1Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TTLNu-EWK_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TTTT_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TTW-WtoQQ-1Jets_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TTWW_TuneCP5_13p6TeV_madgraph-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TTZ-ZtoQQ-1Jets_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TTto2L2Nu-3Jets_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TTto4Q-3Jets_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TTtoLminusNu2Q-3Jets_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TTtoLplusNu2Q-3Jets_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TZQB-ZtoLL-TtoL-CPV_TuneCP5_13p6TeV_madgraph-pythia8_Summer24_Summer25_130X.root",
  };

  MasterDict["ttbar_2024"] = {
    pathPrefix + "Summer24_130X/TTG-1Jets_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TTGG_TuneCP5_13p6TeV_madgraph-madspin-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TTH-Hto2B-TTto2L2Nu_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TTH-Hto2B-TTto4Q_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TTH-Hto2B-TTtoLNu2Q_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TTH-HtoNon2B_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TTLL_Bin-MLL-4to50_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TTLL_Bin-MLL-50_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TTLNu-1Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TTLNu-EWK_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TTTT_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TTW-WtoQQ-1Jets_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TTWW_TuneCP5_13p6TeV_madgraph-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TTZ-ZtoQQ-1Jets_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TTto2L2Nu-3Jets_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TTto4Q-3Jets_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TTtoLminusNu2Q-3Jets_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TTtoLplusNu2Q-3Jets_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TZQB-ZtoLL-TtoL-CPV_TuneCP5_13p6TeV_madgraph-pythia8_Summer24_130X.root",
  };

  MasterDict["ttbar_2023BPix"] = {
    pathPrefix + "Summer23BPix_130X/TTto2L2Nu-2Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TTtoLminusNu2Q-2Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TTtoLplusNu2Q-2Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TTto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TTTT_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TTWW_TuneCP5_13p6TeV_madgraph-madspin-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TTZ-ZtoQQ-1Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TZQB-ZtoLL-TtoL-CPV_TuneCP5_13p6TeV_madgraph-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TTLL_MLL-4to50_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TTLL_MLL-50_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TTLNu-1Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TTHto2B_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TTHtoNon2B_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TTW-WtoQQ-1Jets_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer23BPix_130X.root",
  };
  MasterDict["ttbar_2023"] = {
    pathPrefix + "Summer23_130X/TTG-1Jets_PTG-100to200_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TTG-1Jets_PTG-10to100_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TTHto2B_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TTHtoNon2B_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TTLL_MLL-4to50_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TTLL_MLL-50_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TTLNu-1Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TTLNu-EWK_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TTTT_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TTWW_TuneCP5_13p6TeV_madgraph-madspin-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TTZ-ZtoQQ-1Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TTto2L2Nu-2Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TTto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TTto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TTtoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TTtoLminusNu2Q-2Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TTtoLplusNu2Q-2Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TZQB-ZtoLL-TtoL-CPV_TuneCP5_13p6TeV_madgraph-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TTW-WtoQQ-1Jets_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer23_130X.root",
  };
  MasterDict["ttbar_2022EE"] = {
    pathPrefix + "Summer22EE_130X/TTG-1Jets_PTG-100to200_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TTG-1Jets_PTG-10to100_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TTHto2B_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TTHtoNon2B_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TTLL_MLL-4to50_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TTLL_MLL-50_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TTLNu-1Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TTLNu-EWK_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TTTT_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TTWW_TuneCP5_13p6TeV_madgraph-madspin-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TTZ-ZtoQQ-1Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TTto2L2Nu-3Jets_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TTtoLminusNu2Q-3Jets_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TTtoLplusNu2Q-3Jets_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TZQB-ZtoLL-TtoL-CPV_TuneCP5_13p6TeV_madgraph-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TTW-WtoQQ-1Jets_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer22EE_130X.root",
  };
  MasterDict["ttbar_2022"] = {
    pathPrefix + "Summer22_130X/TTG-1Jets_PTG-100to200_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TTG-1Jets_PTG-10to100_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TTHto2B_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TTHtoNon2B_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TTLL_MLL-4to50_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TTLL_MLL-50_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TTLNu-1Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TTLNu-EWK_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TTTT_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TTWW_TuneCP5_13p6TeV_madgraph-madspin-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TTZ-ZtoQQ-1Jets_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TTto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TTtoLminusNu2Q-3Jets_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TTtoLplusNu2Q-3Jets_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TTto2L2Nu-3Jets_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TZQB-ZtoLL-TtoL-CPV_TuneCP5_13p6TeV_madgraph-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TTW-WtoQQ-1Jets_TuneCP5_13p6TeV_amcatnloFXFXold-pythia8_Summer22_130X.root",
  };
  MasterDict["ttbar_2018"] = {
    pathPrefix + "Summer20UL18_106X/TGJets_TuneCP5_13TeV-amcatnlo-madspin-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/TTGJets_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/TTJets_DiLept_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/TTJets_SingleLeptFromT_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/TTJets_SingleLeptFromTbar_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/TTTT_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/TTWJetsToQQ_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/TTWW_TuneCP5_13TeV-madgraph-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/TTZToLLNuNu_M-10_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/TTZToQQ_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/tZq_ll_4f_ckm_NLO_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ttHToNonbb_M125_TuneCP5_13TeV-powheg-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ttHTobb_M125_TuneCP5_13TeV-powheg-pythia8_Summer20UL18_106X.root",
  };
  MasterDict["ttbar_2017"] = {
    pathPrefix + "Summer20UL17_106X/tZq_ll_4f_ckm_NLO_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ttHToNonbb_M125_TuneCP5_13TeV-powheg-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ttHTobb_M125_TuneCP5_13TeV-powheg-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ttWJets_TuneCP5_13TeV_madgraphMLM_pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ttZJets_TuneCP5_13TeV_madgraphMLM_pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/TGJets_TuneCP5_13TeV-amcatnlo-madspin-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/TTGJets_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/TTJets_DiLept_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/TTJets_SingleLeptFromT_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/TTJets_SingleLeptFromTbar_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/TTTT_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/TTWJetsToQQ_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/TTWW_TuneCP5_13TeV-madgraph-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/TTZToLLNuNu_M-10_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/TTZToQQ_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL17_106X.root",
  };
  MasterDict["ttbar_2016"] = {
    pathPrefix + "Summer20UL16_106X/tZq_ll_4f_ckm_NLO_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ttHToNonbb_M125_TuneCP5_13TeV-powheg-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ttHTobb_M125_TuneCP5_13TeV-powheg-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ttWJets_TuneCP5_13TeV_madgraphMLM_pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ttZJets_TuneCP5_13TeV_madgraphMLM_pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/TGJets_TuneCP5_13TeV-amcatnlo-madspin-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/TTGJets_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/TTJets_DiLept_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/TTJets_SingleLeptFromT_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/TTJets_SingleLeptFromTbar_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/TTTT_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/TTWJetsToQQ_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/TTWW_TuneCP5_13TeV-madgraph-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/TTZToLLNuNu_M-10_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/TTZToQQ_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16_106X.root",
  };
  MasterDict["ttbar_2016APV"] = {
    pathPrefix + "Summer20UL16APV_106X/tZq_ll_4f_ckm_NLO_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ttHToNonbb_M125_TuneCP5_13TeV-powheg-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ttHTobb_M125_TuneCP5_13TeV-powheg-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ttWJets_TuneCP5_13TeV_madgraphMLM_pythia8_Summer20UL16APV_106X.root",
    //pathPrefix + "Summer20UL16APV_106X/ttZJets_TuneCP5_13TeV_madgraphMLM_pythia8_Summer20UL16APV_106X.root", //
    pathPrefix + "Summer20UL16APV_106X/TGJets_TuneCP5_13TeV-amcatnlo-madspin-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/TTGJets_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/TTJets_DiLept_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/TTJets_SingleLeptFromT_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/TTJets_SingleLeptFromTbar_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/TTTT_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/TTWJetsToQQ_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/TTWW_TuneCP5_13TeV-madgraph-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/TTZToLLNuNu_M-10_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/TTZToQQ_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16APV_106X.root",
  };

  MasterDict["ttbar"] = mergeEntriesList(
    MasterDict,
    {
      "ttbar_2023BPix",
      "ttbar_2023",
      "ttbar_2022EE",
      "ttbar_2022",
      "ttbar_2018",
      "ttbar_2017",
      "ttbar_2016",
      "ttbar_2016APV",
    }
  );

  MasterDict["ST_2026"] = {
    pathPrefix + "Summer26_130X/TBbarQto2Q-t-channel-4FS_TuneCP5_13p6TeV_powheg-madspin-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TBbarQtoLNu-t-channel-4FS_TuneCP5_13p6TeV_powheg-madspin-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TBbartoLNu-s-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TBbartoLplusNuBbar-s-channel-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TbarBtoLNu-s-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TbarBtoLminusNuB-s-channel-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TbarWplusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TbarWplusto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/TbarWplustoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
  };
  MasterDict["ST_2025"] = {
    pathPrefix + "Summer25_130X/TBbarQto2Q-t-channel-4FS_TuneCP5_13p6TeV_powheg-madspin-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TBbarQtoLNu-t-channel-4FS_TuneCP5_13p6TeV_powheg-madspin-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TBbartoLNu-s-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TBbartoLplusNuBbar-s-channel-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TbarBtoLNu-s-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TbarBtoLminusNuB-s-channel-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TbarWplusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TbarWplusto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/TbarWplustoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
  };
  MasterDict["ST_2024"] = {
    pathPrefix + "Summer24_130X/TBbarQto2Q-t-channel-4FS_TuneCP5_13p6TeV_powheg-madspin-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TBbarQtoLNu-t-channel-4FS_TuneCP5_13p6TeV_powheg-madspin-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TBbartoLNu-s-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TBbartoLplusNuBbar-s-channel-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TbarBtoLNu-s-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TbarBtoLminusNuB-s-channel-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TbarWplusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TbarWplusto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/TbarWplustoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
  };
  MasterDict["ST_2023BPix"] = {
    pathPrefix + "Summer23BPix_130X/TBbartoLplusNuBbar-s-channel-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TQbarto2Q-t-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TQbartoLNu-t-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TBbarQ_t-channel_4FS_TuneCP5_13p6TeV_powheg-madspin-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TbarBQ_t-channel_4FS_TuneCP5_13p6TeV_powheg-madspin-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TbarBtoLminusNuB-s-channel-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TbarQto2Q-t-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TbarQtoLNu-t-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TbarWplusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TbarWplusto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TbarWplustoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TWminusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TWminusto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/TWminustoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
  };
  MasterDict["ST_2023"] = {
    pathPrefix + "Summer23_130X/TBbarQ_t-channel_4FS_TuneCP5_13p6TeV_powheg-madspin-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TBbartoLplusNuBbar-s-channel-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TQbarto2Q-t-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TQbartoLNu-t-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TbarBQ_t-channel_4FS_TuneCP5_13p6TeV_powheg-madspin-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TbarBtoLminusNuB-s-channel-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TbarQto2Q-t-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TbarQtoLNu-t-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TbarWplusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TbarWplusto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TbarWplustoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TWminusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TWminusto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/TWminustoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
  };
  MasterDict["ST_2022EE"] = {
    pathPrefix + "Summer22EE_130X/TBbarQ_t-channel_4FS_TuneCP5_13p6TeV_powheg-madspin-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TBbartoLplusNuBbar-s-channel-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TQbarto2Q-t-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TQbartoLNu-t-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TWminusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TWminusto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TWminustoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TbarBQ_t-channel_4FS_TuneCP5_13p6TeV_powheg-madspin-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TbarBtoLminusNuB-s-channel-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TbarQto2Q-t-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TbarQtoLNu-t-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TbarWplusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TbarWplusto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/TbarWplustoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
  };
  MasterDict["ST_2022"] = {
    pathPrefix + "Summer22_130X/TBbarQ_t-channel_4FS_TuneCP5_13p6TeV_powheg-madspin-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TBbartoLplusNuBbar-s-channel-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TQbarto2Q-t-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TQbartoLNu-t-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TbarBQ_t-channel_4FS_TuneCP5_13p6TeV_powheg-madspin-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TbarBtoLminusNuB-s-channel-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TbarQto2Q-t-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TbarQtoLNu-t-channel_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TbarWplusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TbarWplusto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TbarWplustoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TWminusto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TWminusto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/TWminustoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
  };
  MasterDict["ST_2018"] = {
    pathPrefix + "Summer20UL18_106X/ST_s-channel_4f_leptonDecays_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ST_t-channel_antitop_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ST_t-channel_top_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ST_tW_antitop_5f_NoFullyHadronicDecays_TuneCP5_13TeV-powheg-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ST_tW_top_5f_NoFullyHadronicDecays_TuneCP5_13TeV-powheg-pythia8_Summer20UL18_106X.root",
  };
  MasterDict["ST_2017"] = {
    pathPrefix + "Summer20UL17_106X/ST_s-channel_4f_leptonDecays_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ST_t-channel_antitop_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ST_t-channel_top_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ST_tW_antitop_5f_NoFullyHadronicDecays_TuneCP5_13TeV-powheg-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ST_tW_top_5f_NoFullyHadronicDecays_TuneCP5_13TeV-powheg-pythia8_Summer20UL17_106X.root",
  };
  MasterDict["ST_2016"] = {
    pathPrefix + "Summer20UL16_106X/ST_s-channel_4f_leptonDecays_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ST_t-channel_antitop_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ST_t-channel_top_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ST_tW_antitop_5f_NoFullyHadronicDecays_TuneCP5_13TeV-powheg-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ST_tW_top_5f_NoFullyHadronicDecays_TuneCP5_13TeV-powheg-pythia8_Summer20UL16_106X.root",
  };
  MasterDict["ST_2016APV"] = {
    pathPrefix + "Summer20UL16APV_106X/ST_s-channel_4f_leptonDecays_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ST_t-channel_antitop_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ST_t-channel_top_4f_InclusiveDecays_TuneCP5_13TeV-powheg-madspin-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ST_tW_antitop_5f_NoFullyHadronicDecays_TuneCP5_13TeV-powheg-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ST_tW_top_5f_NoFullyHadronicDecays_TuneCP5_13TeV-powheg-pythia8_Summer20UL16APV_106X.root",
  };

  MasterDict["ST"] = mergeEntriesList(
    MasterDict,
    {
      "ST_2023BPix",
      "ST_2023",
      "ST_2022EE",
      "ST_2022",
      "ST_2018",
      "ST_2017",
      "ST_2016",
      "ST_2016APV",
    }
  );

  MasterDict["DY_2026"] = {
    pathPrefix + "Summer26_130X/DYto2E_Bin-MLL-10to50_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2E_Bin-MLL-50to120_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2E_Bin-MLL-120to200_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2E_Bin-MLL-200to400_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2E_Bin-MLL-400to800_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2E_Bin-MLL-800to1500_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2E_Bin-MLL-1500to2500_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2E_Bin-MLL-2500to4000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2E_Bin-MLL-4000to6000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2E_Bin-MLL-6000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Mu_Bin-MLL-10to50_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Mu_Bin-MLL-50to120_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Mu_Bin-MLL-120to200_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Mu_Bin-MLL-200to400_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Mu_Bin-MLL-400to800_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Mu_Bin-MLL-800to1500_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Mu_Bin-MLL-1500to2500_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Mu_Bin-MLL-2500to4000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Mu_Bin-MLL-4000to6000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Mu_Bin-MLL-6000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Tau_Bin-MLL-10to50_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Tau_Bin-MLL-50to120_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Tau_Bin-MLL-120to200_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Tau_Bin-MLL-200to400_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Tau_Bin-MLL-400to800_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Tau_Bin-MLL-800to1500_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Tau_Bin-MLL-1500to2500_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Tau_Bin-MLL-2500to4000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Tau_Bin-MLL-4000to6000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/DYto2Tau_Bin-MLL-6000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
  };

  MasterDict["DY_2025"] = {
    pathPrefix + "Summer25_130X/DYto2E_Bin-MLL-10to50_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2E_Bin-MLL-50to120_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2E_Bin-MLL-120to200_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2E_Bin-MLL-200to400_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2E_Bin-MLL-400to800_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2E_Bin-MLL-800to1500_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2E_Bin-MLL-1500to2500_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2E_Bin-MLL-2500to4000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2E_Bin-MLL-4000to6000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2E_Bin-MLL-6000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Mu_Bin-MLL-10to50_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Mu_Bin-MLL-50to120_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Mu_Bin-MLL-120to200_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Mu_Bin-MLL-200to400_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Mu_Bin-MLL-400to800_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Mu_Bin-MLL-800to1500_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Mu_Bin-MLL-1500to2500_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Mu_Bin-MLL-2500to4000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Mu_Bin-MLL-4000to6000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Mu_Bin-MLL-6000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Tau_Bin-MLL-10to50_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Tau_Bin-MLL-50to120_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Tau_Bin-MLL-120to200_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Tau_Bin-MLL-200to400_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Tau_Bin-MLL-400to800_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Tau_Bin-MLL-800to1500_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Tau_Bin-MLL-1500to2500_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Tau_Bin-MLL-2500to4000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Tau_Bin-MLL-4000to6000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/DYto2Tau_Bin-MLL-6000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
  };

  MasterDict["DY_2024"] = {
    pathPrefix + "Summer24_130X/DYto2E_Bin-MLL-10to50_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2E_Bin-MLL-50to120_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2E_Bin-MLL-120to200_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2E_Bin-MLL-200to400_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2E_Bin-MLL-400to800_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2E_Bin-MLL-800to1500_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2E_Bin-MLL-1500to2500_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2E_Bin-MLL-2500to4000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2E_Bin-MLL-4000to6000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2E_Bin-MLL-6000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Mu_Bin-MLL-10to50_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Mu_Bin-MLL-50to120_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Mu_Bin-MLL-120to200_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Mu_Bin-MLL-200to400_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Mu_Bin-MLL-400to800_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Mu_Bin-MLL-800to1500_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Mu_Bin-MLL-1500to2500_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Mu_Bin-MLL-2500to4000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Mu_Bin-MLL-4000to6000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Mu_Bin-MLL-6000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Tau_Bin-MLL-10to50_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Tau_Bin-MLL-50to120_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Tau_Bin-MLL-120to200_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Tau_Bin-MLL-200to400_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Tau_Bin-MLL-400to800_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Tau_Bin-MLL-800to1500_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Tau_Bin-MLL-1500to2500_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Tau_Bin-MLL-2500to4000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Tau_Bin-MLL-4000to6000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/DYto2Tau_Bin-MLL-6000_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
  };

  MasterDict["DY_2023BPix"] = {
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-120_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-120_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-120_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-120_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-120_HT-40to70_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-120_HT-70to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-120_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-4to50_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-4to50_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-4to50_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-4to50_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-4to50_HT-40to70_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-4to50_HT-70to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-4to50_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-50to120_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-50to120_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-50to120_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-50to120_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-50to120_HT-40to70_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-50to120_HT-70to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/DYto2L-4Jets_MLL-50to120_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
  };
  MasterDict["DY_2023"] = {
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-120_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-120_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-120_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-120_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-120_HT-40to70_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-120_HT-70to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-120_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-4to50_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-4to50_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-4to50_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-4to50_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-4to50_HT-40to70_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-4to50_HT-70to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-4to50_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-50to120_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-50to120_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-50to120_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-50to120_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-50to120_HT-40to70_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-50to120_HT-70to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/DYto2L-4Jets_MLL-50to120_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
  };
  MasterDict["DY_2022EE"] = {
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-120_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-120_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-120_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-120_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-120_HT-40to70_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-120_HT-70to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-120_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-4to50_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-4to50_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-4to50_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-4to50_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-4to50_HT-40to70_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-4to50_HT-70to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-4to50_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-50to120_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-50to120_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-50to120_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-50to120_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-50to120_HT-40to70_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-50to120_HT-70to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/DYto2L-4Jets_MLL-50to120_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
  };
  MasterDict["DY_2022"] = {
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-120_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-120_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-120_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-120_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-120_HT-40to70_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-120_HT-70to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-120_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-4to50_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-4to50_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-4to50_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-4to50_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-4to50_HT-40to70_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-4to50_HT-70to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-4to50_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-50to120_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-50to120_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-50to120_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-50to120_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-50to120_HT-40to70_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-50to120_HT-70to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/DYto2L-4Jets_MLL-50to120_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
  };
  MasterDict["DY_2018"] = {
    pathPrefix + "Summer20UL18_106X/DYJetsToLL_M-50_HT-100to200_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/DYJetsToLL_M-50_HT-1200to2500_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/DYJetsToLL_M-50_HT-200to400_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/DYJetsToLL_M-50_HT-2500toInf_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/DYJetsToLL_M-50_HT-400to600_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/DYJetsToLL_M-50_HT-600to800_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/DYJetsToLL_M-50_HT-70to100_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/DYJetsToLL_M-50_HT-800to1200_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
  };
  MasterDict["DY_2017"] = {
    pathPrefix + "Summer20UL17_106X/DYJetsToLL_M-4to50_HT-100to200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/DYJetsToLL_M-4to50_HT-200to400_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/DYJetsToLL_M-4to50_HT-400to600_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/DYJetsToLL_M-4to50_HT-600toInf_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/DYJetsToLL_M-50_HT-100to200_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/DYJetsToLL_M-50_HT-1200to2500_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/DYJetsToLL_M-50_HT-200to400_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/DYJetsToLL_M-50_HT-2500toInf_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/DYJetsToLL_M-50_HT-400to600_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/DYJetsToLL_M-50_HT-600to800_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/DYJetsToLL_M-50_HT-70to100_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/DYJetsToLL_M-50_HT-800to1200_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
  };
  MasterDict["DY_2016"] = {
    pathPrefix + "Summer20UL16_106X/DYJetsToLL_M-4to50_HT-100to200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/DYJetsToLL_M-4to50_HT-200to400_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/DYJetsToLL_M-4to50_HT-400to600_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/DYJetsToLL_M-4to50_HT-600toInf_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/DYJetsToLL_M-50_HT-70to100_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/DYJetsToLL_M-50_HT-100to200_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/DYJetsToLL_M-50_HT-200to400_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/DYJetsToLL_M-50_HT-400to600_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/DYJetsToLL_M-50_HT-600to800_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/DYJetsToLL_M-50_HT-800to1200_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/DYJetsToLL_M-50_HT-1200to2500_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/DYJetsToLL_M-50_HT-2500toInf_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
  };
  MasterDict["DY_2016APV"] = {
    pathPrefix + "Summer20UL16APV_106X/DYJetsToLL_M-4to50_HT-100to200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/DYJetsToLL_M-4to50_HT-200to400_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/DYJetsToLL_M-4to50_HT-400to600_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/DYJetsToLL_M-4to50_HT-600toInf_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/DYJetsToLL_M-50_HT-100to200_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/DYJetsToLL_M-50_HT-1200to2500_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/DYJetsToLL_M-50_HT-200to400_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/DYJetsToLL_M-50_HT-2500toInf_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/DYJetsToLL_M-50_HT-400to600_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/DYJetsToLL_M-50_HT-600to800_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/DYJetsToLL_M-50_HT-70to100_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/DYJetsToLL_M-50_HT-800to1200_TuneCP5_PSweights_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
  };

  MasterDict["DY"] = mergeEntriesList(
    MasterDict,
    {
      "DY_2023BPix",
      "DY_2023",
      "DY_2022EE",
      "DY_2022",
      "DY_2018",
      "DY_2017",
      "DY_2016",
      "DY_2016APV",
    }
  );

  MasterDict["ZInv_2026"] = {
    pathPrefix + "Summer26_130X/Zto2Nu-4Jets_Bin-HT-100to200_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/Zto2Nu-4Jets_Bin-HT-200to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/Zto2Nu-4Jets_Bin-HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/Zto2Nu-4Jets_Bin-HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/Zto2Nu-4Jets_Bin-HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/Zto2Nu-4Jets_Bin-HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
  };

  MasterDict["ZInv_2025"] = {
    pathPrefix + "Summer25_130X/Zto2Nu-4Jets_Bin-HT-100to200_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/Zto2Nu-4Jets_Bin-HT-200to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/Zto2Nu-4Jets_Bin-HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/Zto2Nu-4Jets_Bin-HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/Zto2Nu-4Jets_Bin-HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/Zto2Nu-4Jets_Bin-HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
  };

  MasterDict["ZInv_2024"] = {
    pathPrefix + "Summer24_130X/Zto2Nu-4Jets_Bin-HT-100to200_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/Zto2Nu-4Jets_Bin-HT-200to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/Zto2Nu-4Jets_Bin-HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/Zto2Nu-4Jets_Bin-HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/Zto2Nu-4Jets_Bin-HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/Zto2Nu-4Jets_Bin-HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
  };

  MasterDict["ZInv_2023BPix"] = {
    pathPrefix + "Summer23BPix_130X/Zto2Nu-4Jets_HT-100to200_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/Zto2Nu-4Jets_HT-200to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/Zto2Nu-4Jets_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/Zto2Nu-4Jets_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/Zto2Nu-4Jets_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/Zto2Nu-4Jets_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
  };
 MasterDict["ZInv_2023"] = {
    pathPrefix + "Summer23_130X/Zto2Nu-4Jets_HT-100to200_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    //pathPrefix + "Summer23_130X/Zto2Nu-4Jets_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/Zto2Nu-4Jets_HT-200to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/Zto2Nu-4Jets_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/Zto2Nu-4Jets_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/Zto2Nu-4Jets_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
  };
  MasterDict["ZInv_2022EE"] = {
    pathPrefix + "Summer22EE_130X/Zto2Nu-4Jets_HT-100to200_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/Zto2Nu-4Jets_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/Zto2Nu-4Jets_HT-200to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/Zto2Nu-4Jets_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/Zto2Nu-4Jets_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X/Zto2Nu-4Jets_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
  };
  MasterDict["ZInv_2022"] = {
    pathPrefix + "Summer22_130X/Zto2Nu-4Jets_HT-100to200_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/Zto2Nu-4Jets_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/Zto2Nu-4Jets_HT-200to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/Zto2Nu-4Jets_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/Zto2Nu-4Jets_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/Zto2Nu-4Jets_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
  };
  MasterDict["ZInv_2018"] = {
    pathPrefix + "Summer20UL18_106X/ZJetsToNuNu_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ZJetsToNuNu_HT-1200To2500_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ZJetsToNuNu_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ZJetsToNuNu_HT-2500ToInf_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ZJetsToNuNu_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ZJetsToNuNu_HT-600To800_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ZJetsToNuNu_HT-800To1200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
  };
  MasterDict["ZInv_2017"] = {
    pathPrefix + "Summer20UL17_106X/ZJetsToNuNu_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ZJetsToNuNu_HT-1200To2500_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ZJetsToNuNu_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ZJetsToNuNu_HT-2500ToInf_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ZJetsToNuNu_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ZJetsToNuNu_HT-600To800_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ZJetsToNuNu_HT-800To1200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
  };
  MasterDict["ZInv_2016"] = {
    pathPrefix + "Summer20UL16_106X/ZJetsToNuNu_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ZJetsToNuNu_HT-1200To2500_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ZJetsToNuNu_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ZJetsToNuNu_HT-2500ToInf_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ZJetsToNuNu_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ZJetsToNuNu_HT-600To800_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ZJetsToNuNu_HT-800To1200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
  };
  MasterDict["ZInv_2016APV"] = {
    pathPrefix + "Summer20UL16APV_106X/ZJetsToNuNu_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ZJetsToNuNu_HT-1200To2500_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ZJetsToNuNu_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ZJetsToNuNu_HT-2500ToInf_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ZJetsToNuNu_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ZJetsToNuNu_HT-600To800_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ZJetsToNuNu_HT-800To1200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
  };

  MasterDict["DB_2026"] = {
    pathPrefix + "Summer26_130X/GluGluH-HTo2Tau_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/GluGluH-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/GluGluH-Hto2G_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/GluGluH-Hto2Zto4L_Par-M-125_TuneCP5_13p6TeV_powheg-jhugen-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/GluGluH-HtoZG_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/GluGluHto2Wto2L2Nu_Par-M-125_TuneCP5_13p6TeV_powheg-jhugen-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WWto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WWto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WWtoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WZto2L2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WZto3LNu-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WZtoL3Nu-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WZtoLNu2Q-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/ZZto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/ZZto2L2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/ZZto2Nu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer26_130X.root",
  };
  MasterDict["DB_2025"] = {
    pathPrefix + "Summer25_130X/GluGluH-HTo2Tau_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/GluGluH-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/GluGluH-Hto2G_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/GluGluH-Hto2Zto4L_Par-M-125_TuneCP5_13p6TeV_powheg-jhugen-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/GluGluH-HtoZG_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/GluGluHto2Wto2L2Nu_Par-M-125_TuneCP5_13p6TeV_powheg-jhugen-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WWto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WWto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WWtoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WZto2L2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WZto3LNu-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WZtoL3Nu-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WZtoLNu2Q-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/ZZto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/ZZto2L2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/ZZto2Nu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8_Summer24_Summer25_130X.root",
  };
  MasterDict["DB_2024"] = {
    pathPrefix + "Summer24_130X/GluGluH-HTo2Tau_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/GluGluH-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/GluGluH-Hto2G_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/GluGluH-Hto2Zto4L_Par-M-125_TuneCP5_13p6TeV_powheg-jhugen-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/GluGluH-HtoZG_Par-M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/GluGluHto2Wto2L2Nu_Par-M-125_TuneCP5_13p6TeV_powheg-jhugen-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WWto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WWto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WWtoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WZto2L2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WZto3LNu-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WZtoL3Nu-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WZtoLNu2Q-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/ZZto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/ZZto2L2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/ZZto2Nu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8_Summer24_130X.root",
  };
  MasterDict["DB_2023BPix"] = {
    pathPrefix + "Summer23BPix_130X/WWto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WWtoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WWto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WZto2L2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WZto3LNu-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WZtoL3Nu-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WZtoLNu2Q-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/ZZto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/ZZto2L2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/ZZto2Nu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WWGtoLNu2QG-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/ZG2JtoG2L2J_EWK_MLL-50_MJJ-120_TuneCP5_13p6TeV_madgraph-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/GluGluHToTauTau_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/GluGluHto2WtoLNu2Q_M-125_TuneCP5_13p6TeV_powheg-JHUGenV752-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/GluGluHto2Wto2L2Nu_M-125_TuneCP5_13p6TeV_powheg-jhugen752-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/GluGluHto2Zto2L2Q_M-125_TuneCP5_13p6TeV_powheg-jhugenv7520-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/GluGluHtoZZto4L_M-125_TuneCP5_13p6TeV_powheg-jhugen-pythia8_Summer23BPix_130X.root",
  };
  MasterDict["TB_2026"] = {
    pathPrefix + "Summer26_130X/WWG_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WWW-4F_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WWZ_4F_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WZGtoLNuZG_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WZZ-5F_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WminusH-HtoNon2B_Par-M-125_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WminusH-Wto2Q-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WminusH-WtoLNu-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WplusH-HtoNon2B_Par-M-125_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WplusH-Wto2Q-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WplusH-WtoLNu-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/ZH-HtoNon2B_Par-M-125_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/ZH-Zto2L-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/ZH-Zto2Nu-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/ZH-Zto2Q-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/ZZZ-5F_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer26_130X.root",
  };
  MasterDict["TB_2025"] = {
    pathPrefix + "Summer25_130X/WWG_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WWW-4F_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WWZ_4F_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WZGtoLNuZG_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WZZ-5F_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WminusH-HtoNon2B_Par-M-125_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WminusH-Wto2Q-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WminusH-WtoLNu-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WplusH-HtoNon2B_Par-M-125_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WplusH-Wto2Q-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WplusH-WtoLNu-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/ZH-HtoNon2B_Par-M-125_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/ZH-Zto2L-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/ZH-Zto2Nu-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/ZH-Zto2Q-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/ZZZ-5F_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_Summer25_130X.root",
  };
  MasterDict["TB_2024"] = {
    pathPrefix + "Summer24_130X/WWG_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WWW-4F_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WWZ_4F_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WZGtoLNuZG_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WZZ-5F_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WminusH-HtoNon2B_Par-M-125_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WminusH-Wto2Q-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WminusH-WtoLNu-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WplusH-HtoNon2B_Par-M-125_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WplusH-Wto2Q-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WplusH-WtoLNu-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/ZH-HtoNon2B_Par-M-125_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/ZH-Zto2L-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/ZH-Zto2Nu-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/ZH-Zto2Q-Hto2B_Par-M-125_TuneCP5_13p6TeV_powhegMINLO-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/ZZZ-5F_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer24_130X.root",
  };
  MasterDict["TB_2023BPix"] = {
    pathPrefix + "Summer23BPix_130X/WWW_4F_TuneCP5_13p6TeV_amcatnlo-madspin-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WWZ_4F_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WZZ_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/ZZZ_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WZGtoLNuZG_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/VH_HtoNonbb_M-125_TuneCP5_13p6TeV_amcatnloFXFX-madspin-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WminusH_Hto2B_WtoLNu_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WplusH_Hto2B_WtoLNu_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/ZH_Hto2B_Zto2L_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/ZH_Hto2B_Zto2Nu_M-125_TuneCP5_13p6TeV_powheg-minlo-pythia8_Summer23BPix_130X.root",
  };

  MasterDict["DB_2023"] = {
    pathPrefix + "Summer23_130X/GluGluHto2Wto2L2Nu_M-125_TuneCP5_13p6TeV_powheg-jhugen752-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/GluGluHto2WtoLNu2Q_M-125_TuneCP5_13p6TeV_powheg-JHUGenV752-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/GluGluHto2Zto2L2Q_M-125_TuneCP5_13p6TeV_powheg-jhugenv7520-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/GluGluHtoZZto4L_M-125_TuneCP5_13p6TeV_powheg-jhugen-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WWto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WWto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WWtoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WZto2L2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WZto3LNu-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WZtoL3Nu-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WZtoLNu2Q-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WWGtoLNu2QG-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/ZG2JtoG2L2J_EWK_MLL-50_MJJ-120_TuneCP5_13p6TeV_madgraph-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/ZZto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/ZZto2L2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/ZZto2Nu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
  };
  MasterDict["TB_2023"] = {
    pathPrefix + "Summer23_130X/VH_HtoNonbb_M-125_TuneCP5_13p6TeV_amcatnloFXFX-madspin-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WWW_4F_TuneCP5_13p6TeV_amcatnlo-madspin-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WWZ_4F_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WZGtoLNuZG_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WZZ_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WminusH_Hto2B_WtoLNu_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WplusH_Hto2B_WtoLNu_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/ZH_Hto2B_Zto2L_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/ZH_Hto2B_Zto2Nu_M-125_TuneCP5_13p6TeV_powheg-minlo-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/ZZZ_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer23_130X.root",
  };
  MasterDict["DB_2022EE"] = {
     pathPrefix + "Summer22EE_130X/GluGluHToTauTau_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/GluGluHto2Wto2L2Nu_M-125_TuneCP5_13p6TeV_powheg-jhugen752-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/GluGluHto2WtoLNu2Q_M-125_TuneCP5_13p6TeV_powheg-JHUGenV752-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/GluGluHto2Zto2L2Q_M-125_TuneCP5_13p6TeV_powheg-jhugenv7520-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/GluGluHtoZZto4L_M-125_TuneCP5_13p6TeV_powheg2-JHUGenV752-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WWto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WWto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WWtoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WZto3LNu-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WZtoL3Nu-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WZtoLNu2Q-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WWGtoLNu2QG-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/ZG2JtoG2L2J_EWK_MLL-50_MJJ-120_TuneCP5_withDipoleRecoil_13p6TeV_madgraph-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/ZZto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/ZZto2L2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/ZZto2Nu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
  };
  MasterDict["TB_2022EE"] = {
     pathPrefix + "Summer22EE_130X/VH_HtoNonbb_M-125_TuneCP5_13p6TeV_amcatnloFXFX-madspin-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WWW_4F_TuneCP5_13p6TeV_amcatnlo-madspin-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WWZ_4F_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WZGtoLNuZG_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WZZ_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WminusH_Hto2B_WtoLNu_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WplusH_Hto2B_WtoLNu_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/ZH_Hto2B_Zto2L_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/ZH_Hto2B_Zto2Nu_M-125_TuneCP5_13p6TeV_powheg-minlo-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/ZZZ_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22EE_130X.root",
  };
  MasterDict["DB_2022"] = {
    pathPrefix + "Summer22_130X/GluGluHToTauTau_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/GluGluHto2Wto2L2Nu_M-125_TuneCP5_13p6TeV_powheg-jhugen752-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/GluGluHto2WtoLNu2Q_M-125_TuneCP5_13p6TeV_powheg-JHUGenV752-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/GluGluHto2Zto2L2Q_M-125_TuneCP5_13p6TeV_powheg-jhugenv7520-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/GluGluHtoZZto4L_M-125_TuneCP5_13p6TeV_powheg2-JHUGenV752-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WWto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WWto4Q_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WWtoLNu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WZto2L2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WZto3LNu-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WZtoL3Nu-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WZtoLNu2Q-1Jets-4FS_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WWGtoLNu2QG-4FS_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/ZG2JtoG2L2J_EWK_MLL-50_MJJ-120_TuneCP5_withDipoleRecoil_13p6TeV_madgraph-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/ZZto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/ZZto2L2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/ZZto2Nu2Q_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
  };
  MasterDict["TB_2022"] = {
    pathPrefix + "Summer22_130X/VH_HtoNonbb_M-125_TuneCP5_13p6TeV_amcatnloFXFX-madspin-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/VHtoGG_M-125_TuneCP5_13p6TeV_amcatnloFXFX-madspin-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WWW_4F_TuneCP5_13p6TeV_amcatnlo-madspin-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WWZ_4F_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WZGtoLNuZG_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WZZ_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WminusH_Hto2B_WtoLNu_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WplusH_Hto2B_WtoLNu_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/ZH_Hto2B_Zto2L_M-125_TuneCP5_13p6TeV_powheg-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/ZH_Hto2B_Zto2Nu_M-125_TuneCP5_13p6TeV_powheg-minlo-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/ZZZ_TuneCP5_13p6TeV_amcatnlo-pythia8_Summer22_130X.root",
  };
  MasterDict["DB_2018"] = {
    pathPrefix + "Summer20UL18_106X/GluGluHToTauTau_M-125_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/GluGluHToWWTo2L2Nu_M-125_TuneCP5_13TeV-powheg-jhugen727-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/GluGluHToWWToLNuQQ_M-125_TuneCP5_13TeV_powheg_jhugen751_pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/GluGluHToZZTo2L2Nu_M125_TuneCP5_13TeV_powheg2_JHUGenV735_pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/GluGluHToZZTo4L_M125_TuneCP5_13TeV_powheg2_minloHJJ_JHUGenV7011_pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WWTo1L1Nu2Q_4f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WWTo2L2Nu_TuneCP5_13TeV-powheg-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WWTo4Q_4f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WZTo1L1Nu2Q_4f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WZTo1L3Nu_4f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WZTo2Q2L_mllmin4p0_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WZTo3LNu_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ZZTo2L2Nu_TuneCP5_13TeV_powheg_pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ZZTo2Q2L_mllmin4p0_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ZZTo2Q2Nu_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ZZTo4L_TuneCP5_13TeV_powheg_pythia8_Summer20UL18_106X.root",
  };
  MasterDict["TB_2018"] = {
    pathPrefix + "Summer20UL18_106X/VHToNonbb_M125_TuneCP5_13TeV-amcatnloFXFX_madspin_pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WGG_5f_TuneCP5_13TeV_amcatnlo-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WWG_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WWW_4F_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WWZ_4F_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WZG_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WZZ_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WminusH_HToBB_WToLNu_M-125_TuneCP5_13TeV-powheg-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WplusH_HToBB_WToLNu_M-125_TuneCP5_13TeV-powheg-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ZH_HToBB_ZToLL_M-125_TuneCP5_13TeV-powheg-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ZH_HToBB_ZToNuNu_M-125_TuneCP5_13TeV-powheg-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/ZZZ_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL18_106X.root",
  };
  MasterDict["DB_2017"] = {
    pathPrefix + "Summer20UL17_106X/GluGluHToTauTau_M-125_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/GluGluHToWWTo2L2Nu_M-125_TuneCP5_13TeV-powheg-jhugen727-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/GluGluHToWWToLNuQQ_M-125_TuneCP5_13TeV_powheg_jhugen751_pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/GluGluHToZZTo2L2Nu_M125_TuneCP5_13TeV_powheg2_JHUGenV735_pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/GluGluHToZZTo4L_M125_TuneCP5_13TeV_powheg2_minloHJJ_JHUGenV7011_pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WWTo1L1Nu2Q_4f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WWTo2L2Nu_TuneCP5_13TeV-powheg-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WWTo4Q_4f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WZTo1L1Nu2Q_4f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WZTo1L3Nu_4f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WZTo2Q2L_mllmin4p0_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WZTo3LNu_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ZZTo2L2Nu_TuneCP5_13TeV_powheg_pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ZZTo2Nu2Q_5f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ZZTo2Q2L_mllmin4p0_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ZZTo4L_TuneCP5_13TeV_powheg_pythia8_Summer20UL17_106X.root",
  };
  MasterDict["TB_2017"] = {
    pathPrefix + "Summer20UL17_106X/VHToNonbb_M125_TuneCP5_13TeV-amcatnloFXFX_madspin_pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WGG_5f_TuneCP5_13TeV_amcatnlo-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WWG_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WWW_4F_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WWZ_4F_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WZG_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WZZ_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WminusH_HToBB_WToLNu_M-125_TuneCP5_13TeV-powheg-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WplusH_HToBB_WToLNu_M-125_TuneCP5_13TeV-powheg-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ZH_HToBB_ZToLL_M-125_TuneCP5_13TeV-powheg-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ZH_HToBB_ZToNuNu_M-125_TuneCP5_13TeV-powheg-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/ZZZ_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL17_106X.root",
  };
  MasterDict["DB_2016"] = {
    pathPrefix + "Summer20UL16_106X/GluGluHToTauTau_M-125_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/GluGluHToWWTo2L2Nu_M-125_TuneCP5_13TeV-powheg-jhugen727-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/GluGluHToWWToLNuQQ_M-125_TuneCP5_13TeV_powheg_jhugen751_pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/GluGluHToZZTo2L2Nu_M125_TuneCP5_13TeV_powheg2_JHUGenV735_pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/GluGluHToZZTo4L_M125_TuneCP5_13TeV_powheg2_minloHJJ_JHUGenV7011_pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WWTo1L1Nu2Q_4f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WWTo2L2Nu_TuneCP5_13TeV-powheg-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WWTo4Q_4f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WZTo1L1Nu2Q_4f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WZTo1L3Nu_4f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WZTo2Q2L_mllmin4p0_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WZTo3LNu_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ZZTo2L2Nu_TuneCP5_13TeV_powheg_pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ZZTo2Nu2Q_5f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ZZTo2Q2L_mllmin4p0_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ZZTo4L_TuneCP5_13TeV_powheg_pythia8_Summer20UL16_106X.root",
  };
  MasterDict["TB_2016"] = {
    pathPrefix + "Summer20UL16_106X/VHToNonbb_M125_TuneCP5_13TeV-amcatnloFXFX_madspin_pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WGG_5f_TuneCP5_13TeV_amcatnlo-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WWG_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WWW_4F_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WWZ_4F_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WZG_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WZZ_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WminusH_HToBB_WToLNu_M-125_TuneCP5_13TeV-powheg-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WplusH_HToBB_WToLNu_M-125_TuneCP5_13TeV-powheg-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ZH_HToBB_ZToLL_M-125_TuneCP5_13TeV-powheg-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ZH_HToBB_ZToNuNu_M-125_TuneCP5_13TeV-powheg-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/ZZZ_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16_106X.root",
  };

  MasterDict["DB_2016APV"] = {
    pathPrefix + "Summer20UL16APV_106X/GluGluHToTauTau_M-125_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/GluGluHToWWTo2L2Nu_M-125_TuneCP5_13TeV-powheg-jhugen727-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/GluGluHToWWToLNuQQ_M-125_TuneCP5_13TeV_powheg_jhugen751_pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/GluGluHToZZTo2L2Nu_M125_TuneCP5_13TeV_powheg2_JHUGenV735_pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/GluGluHToZZTo4L_M125_TuneCP5_13TeV_powheg2_minloHJJ_JHUGenV7011_pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WWTo1L1Nu2Q_4f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WWTo2L2Nu_TuneCP5_13TeV-powheg-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WWTo4Q_4f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WZTo1L1Nu2Q_4f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WZTo1L3Nu_4f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WZTo2Q2L_mllmin4p0_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WZTo3LNu_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ZZTo2L2Nu_TuneCP5_13TeV_powheg_pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ZZTo2Nu2Q_5f_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ZZTo2Q2L_mllmin4p0_TuneCP5_13TeV-amcatnloFXFX-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ZZTo4L_TuneCP5_13TeV_powheg_pythia8_Summer20UL16APV_106X.root",
  };
  MasterDict["TB_2016APV"] = {
    pathPrefix + "Summer20UL16APV_106X/VHToNonbb_M125_TuneCP5_13TeV-amcatnloFXFX_madspin_pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WGG_5f_TuneCP5_13TeV_amcatnlo-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WWG_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WWW_4F_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WWZ_4F_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WZG_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WZZ_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WminusH_HToBB_WToLNu_M-125_TuneCP5_13TeV-powheg-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WplusH_HToBB_WToLNu_M-125_TuneCP5_13TeV-powheg-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ZH_HToBB_ZToLL_M-125_TuneCP5_13TeV-powheg-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ZH_HToBB_ZToNuNu_M-125_TuneCP5_13TeV-powheg-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/ZZZ_TuneCP5_13TeV-amcatnlo-pythia8_Summer20UL16APV_106X.root",
  };

  MasterDict["QCD_2024"] = {
    pathPrefix + "Summer24_130X/QCD-4Jets_HT-1000to1200_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_Summer24_130X.root",
    pathPrefix + "Summer24_130X/QCD-4Jets_HT-100to200_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_Summer24_130X.root",
    pathPrefix + "Summer24_130X/QCD-4Jets_HT-1200to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_Summer24_130X.root",
    pathPrefix + "Summer24_130X/QCD-4Jets_HT-1500to2000_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_Summer24_130X.root",
    pathPrefix + "Summer24_130X/QCD-4Jets_HT-2000_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_Summer24_130X.root",
    pathPrefix + "Summer24_130X/QCD-4Jets_HT-200to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_Summer24_130X.root",
    pathPrefix + "Summer24_130X/QCD-4Jets_HT-400to600_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_Summer24_130X.root",
    pathPrefix + "Summer24_130X/QCD-4Jets_HT-600to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_Summer24_130X.root",
    pathPrefix + "Summer24_130X/QCD-4Jets_HT-800to1000_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_Summer24_130X.root",
  };

  MasterDict["QCD_2023BPix"] = {
    pathPrefix + "Summer23BPix_130X/QCD-4Jets_HT-100to200_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/QCD-4Jets_HT-200to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/QCD-4Jets_HT-400to600_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/QCD-4Jets_HT-600to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/QCD-4Jets_HT-800to1000_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/QCD-4Jets_HT-1000to1200_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/QCD-4Jets_HT-1200to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/QCD-4Jets_HT-1500to2000_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/QCD-4Jets_HT-2000_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
  };
  MasterDict["QCD_2023"] = {
    pathPrefix + "Summer23_130X/QCD-4Jets_HT-1000to1200_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/QCD-4Jets_HT-100to200_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/QCD-4Jets_HT-1200to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/QCD-4Jets_HT-1500to2000_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/QCD-4Jets_HT-2000_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/QCD-4Jets_HT-200to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/QCD-4Jets_HT-400to600_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/QCD-4Jets_HT-600to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/QCD-4Jets_HT-800to1000_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
  };
  MasterDict["QCD_2022EE"] = {
     pathPrefix + "Summer22EE_130X/QCD-4Jets_HT-1000to1200_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/QCD-4Jets_HT-100to200_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/QCD-4Jets_HT-1200to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/QCD-4Jets_HT-1500to2000_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/QCD-4Jets_HT-2000_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/QCD-4Jets_HT-200to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/QCD-4Jets_HT-400to600_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/QCD-4Jets_HT-600to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/QCD-4Jets_HT-800to1000_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
  };
  MasterDict["QCD_2022"] = {
    pathPrefix + "Summer22_130X/QCD-4Jets_HT-1000to1200_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/QCD-4Jets_HT-100to200_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/QCD-4Jets_HT-1200to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/QCD-4Jets_HT-1500to2000_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/QCD-4Jets_HT-2000_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/QCD-4Jets_HT-200to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/QCD-4Jets_HT-400to600_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/QCD-4Jets_HT-600to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/QCD-4Jets_HT-800to1000_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
  };
  MasterDict["QCD_2018"] = {
    pathPrefix + "Summer20UL18_106X/QCD_HT1000to1500_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/QCD_HT100to200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/QCD_HT1500to2000_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/QCD_HT2000toInf_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/QCD_HT200to300_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/QCD_HT300to500_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/QCD_HT500to700_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/QCD_HT50to100_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/QCD_HT700to1000_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
  };
  MasterDict["QCD_2017"] = {
    pathPrefix + "Summer20UL17_106X/QCD_HT1000to1500_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/QCD_HT100to200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/QCD_HT1500to2000_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/QCD_HT2000toInf_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/QCD_HT200to300_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/QCD_HT300to500_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/QCD_HT500to700_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/QCD_HT50to100_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/QCD_HT700to1000_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
  };
  MasterDict["QCD_2016"] = {
    pathPrefix + "Summer20UL16_106X/QCD_HT1000to1500_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/QCD_HT100to200_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/QCD_HT1500to2000_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/QCD_HT2000toInf_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/QCD_HT200to300_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/QCD_HT300to500_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/QCD_HT500to700_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/QCD_HT700to1000_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
  };
  MasterDict["QCD_2016APV"] = {
    pathPrefix + "Summer20UL16APV_106X/QCD_HT1000to1500_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/QCD_HT100to200_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/QCD_HT1500to2000_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/QCD_HT2000toInf_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/QCD_HT200to300_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/QCD_HT300to500_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/QCD_HT500to700_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/QCD_HT700to1000_TuneCP5_PSWeights_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
  };

  MasterDict["Wjets_2026"] = {
    pathPrefix + "Summer26_130X/WtoLNu-4Jets_Bin-HT-40to100-MLNu-0to120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WtoLNu-4Jets_Bin-HT-100to400-MLNu-0to120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WtoLNu-4Jets_Bin-HT-400to800-MLNu-0to120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WtoLNu-4Jets_Bin-HT-800to1500-MLNu-0to120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WtoLNu-4Jets_Bin-HT-1500to2500-MLNu-0to120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WtoLNu-4Jets_Bin-HT-2500-MLNu-0to120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WtoLNu-4Jets_Bin-HT-40to100-MLNu-120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WtoLNu-4Jets_Bin-HT-100to400-MLNu-120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WtoLNu-4Jets_Bin-HT-400to800-MLNu-120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WtoLNu-4Jets_Bin-HT-800to1500-MLNu-120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WtoLNu-4Jets_Bin-HT-1500to2500-MLNu-120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
    pathPrefix + "Summer26_130X/WtoLNu-4Jets_Bin-HT-2500-MLNu-120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer26_130X.root",
  };
  MasterDict["Wjets_2025"] = {
    pathPrefix + "Summer25_130X/WtoLNu-4Jets_Bin-HT-40to100-MLNu-0to120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WtoLNu-4Jets_Bin-HT-100to400-MLNu-0to120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WtoLNu-4Jets_Bin-HT-400to800-MLNu-0to120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WtoLNu-4Jets_Bin-HT-800to1500-MLNu-0to120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WtoLNu-4Jets_Bin-HT-1500to2500-MLNu-0to120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WtoLNu-4Jets_Bin-HT-2500-MLNu-0to120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WtoLNu-4Jets_Bin-HT-40to100-MLNu-120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WtoLNu-4Jets_Bin-HT-100to400-MLNu-120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WtoLNu-4Jets_Bin-HT-400to800-MLNu-120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WtoLNu-4Jets_Bin-HT-800to1500-MLNu-120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WtoLNu-4Jets_Bin-HT-1500to2500-MLNu-120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
    pathPrefix + "Summer25_130X/WtoLNu-4Jets_Bin-HT-2500-MLNu-120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_Summer25_130X.root",
  };
  MasterDict["Wjets_2024"] = {
    pathPrefix + "Summer24_130X/WtoLNu-4Jets_Bin-HT-40to100-MLNu-0to120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WtoLNu-4Jets_Bin-HT-100to400-MLNu-0to120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WtoLNu-4Jets_Bin-HT-400to800-MLNu-0to120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WtoLNu-4Jets_Bin-HT-800to1500-MLNu-0to120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WtoLNu-4Jets_Bin-HT-1500to2500-MLNu-0to120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WtoLNu-4Jets_Bin-HT-2500-MLNu-0to120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WtoLNu-4Jets_Bin-HT-40to100-MLNu-120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WtoLNu-4Jets_Bin-HT-100to400-MLNu-120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WtoLNu-4Jets_Bin-HT-400to800-MLNu-120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WtoLNu-4Jets_Bin-HT-800to1500-MLNu-120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WtoLNu-4Jets_Bin-HT-1500to2500-MLNu-120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
    pathPrefix + "Summer24_130X/WtoLNu-4Jets_Bin-HT-2500-MLNu-120_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer24_130X.root",
  };
  MasterDict["Wjets_2023BPix"] = {
    pathPrefix + "Summer23BPix_130X/WtoLNu-4Jets_MLNu-0to120_HT-40to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WtoLNu-4Jets_MLNu-0to120_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WtoLNu-4Jets_MLNu-0to120_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WtoLNu-4Jets_MLNu-0to120_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WtoLNu-4Jets_MLNu-0to120_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WtoLNu-4Jets_MLNu-0to120_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WtoLNu-4Jets_MLNu-120_HT-40to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WtoLNu-4Jets_MLNu-120_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WtoLNu-4Jets_MLNu-120_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WtoLNu-4Jets_MLNu-120_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WtoLNu-4Jets_MLNu-120_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/WtoLNu-4Jets_MLNu-120_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
  };
  MasterDict["Wjets_2023"] = {
    pathPrefix + "Summer23_130X/WtoLNu-4Jets_MLNu-0to120_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WtoLNu-4Jets_MLNu-0to120_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WtoLNu-4Jets_MLNu-0to120_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WtoLNu-4Jets_MLNu-0to120_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WtoLNu-4Jets_MLNu-0to120_HT-40to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WtoLNu-4Jets_MLNu-0to120_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WtoLNu-4Jets_MLNu-120_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WtoLNu-4Jets_MLNu-120_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WtoLNu-4Jets_MLNu-120_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WtoLNu-4Jets_MLNu-120_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WtoLNu-4Jets_MLNu-120_HT-40to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
    pathPrefix + "Summer23_130X/WtoLNu-4Jets_MLNu-120_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23_130X.root",
  };
  MasterDict["Wjets_2022EE"] = {
     pathPrefix + "Summer22EE_130X/WtoLNu-4Jets_MLNu-0to120_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WtoLNu-4Jets_MLNu-0to120_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WtoLNu-4Jets_MLNu-0to120_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WtoLNu-4Jets_MLNu-0to120_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WtoLNu-4Jets_MLNu-0to120_HT-40to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WtoLNu-4Jets_MLNu-0to120_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WtoLNu-4Jets_MLNu-120_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WtoLNu-4Jets_MLNu-120_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WtoLNu-4Jets_MLNu-120_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WtoLNu-4Jets_MLNu-120_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WtoLNu-4Jets_MLNu-120_HT-40to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
     pathPrefix + "Summer22EE_130X/WtoLNu-4Jets_MLNu-120_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22EE_130X.root",
  };
  MasterDict["Wjets_2022"] = {
    pathPrefix + "Summer22_130X/WtoLNu-4Jets_MLNu-0to120_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WtoLNu-4Jets_MLNu-0to120_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WtoLNu-4Jets_MLNu-0to120_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WtoLNu-4Jets_MLNu-0to120_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WtoLNu-4Jets_MLNu-0to120_HT-40to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WtoLNu-4Jets_MLNu-0to120_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WtoLNu-4Jets_MLNu-120_HT-100to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WtoLNu-4Jets_MLNu-120_HT-1500to2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WtoLNu-4Jets_MLNu-120_HT-2500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WtoLNu-4Jets_MLNu-120_HT-400to800_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WtoLNu-4Jets_MLNu-120_HT-40to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
    pathPrefix + "Summer22_130X/WtoLNu-4Jets_MLNu-120_HT-800to1500_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer22_130X.root",
  };
  MasterDict["Wjets_2018"] = {
    pathPrefix + "Summer20UL18_106X/WJetsToLNu_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WJetsToLNu_HT-1200To2500_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WJetsToLNu_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WJetsToLNu_HT-2500ToInf_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WJetsToLNu_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WJetsToLNu_HT-600To800_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WJetsToLNu_HT-70To100_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X/WJetsToLNu_HT-800To1200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root",
  };
  MasterDict["Wjets_2017"] = {
    pathPrefix + "Summer20UL17_106X/WJetsToLNu_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WJetsToLNu_HT-1200To2500_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WJetsToLNu_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WJetsToLNu_HT-2500ToInf_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WJetsToLNu_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WJetsToLNu_HT-600To800_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WJetsToLNu_HT-70To100_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X/WJetsToLNu_HT-800To1200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root",
  };
  MasterDict["Wjets_2016"] = {
    pathPrefix + "Summer20UL16_106X/WJetsToLNu_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WJetsToLNu_HT-1200To2500_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WJetsToLNu_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WJetsToLNu_HT-2500ToInf_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WJetsToLNu_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WJetsToLNu_HT-600To800_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WJetsToLNu_HT-70To100_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X/WJetsToLNu_HT-800To1200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root",
  };
  MasterDict["Wjets_2016APV"] = {
    pathPrefix + "Summer20UL16APV_106X/WJetsToLNu_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WJetsToLNu_HT-1200To2500_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WJetsToLNu_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WJetsToLNu_HT-2500ToInf_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WJetsToLNu_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WJetsToLNu_HT-600To800_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WJetsToLNu_HT-70To100_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X/WJetsToLNu_HT-800To1200_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root",
  };

  MasterDict["Gjets"] = {
    pathPrefix + "Summer23BPix_130X/GJ-4Jets_HT-40to70_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/GJ-4Jets_HT-70to100_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/GJ-4Jets_HT-100to200_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/GJ-4Jets_HT-200to400_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/GJ-4Jets_HT-400to600_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X/GJ-4Jets_HT-600_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root"
  };

  MasterDict["Cascades_220"] = {
    pathPrefix + "Summer23BPix_130X_Cascades/SlepSnuCascade_MN1-220_MN2-260_MC1-240_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
  };

  MasterDict["Cascades_260"] = {
    pathPrefix + "Summer23BPix_130X_Cascades/SlepSnuCascade_MN1-260_MN2-280_MC1-270_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
  };

  MasterDict["Cascades_270"] = {
    pathPrefix + "Summer23BPix_130X_Cascades/SlepSnuCascade_MN1-270_MN2-280_MC1-275_TuneCP5_13p6TeV_madgraphMLM-pythia8_Summer23BPix_130X.root",
  };

  MasterDict["Cascades_180"] = {
    pathPrefix + "Summer22_130X_Cascades/SlepSnuCascade_220-209_200-190-180_2022_NANO_JustinPrivateMC_Summer22_130X_Cascades_Summer22_130X.root"
  };

  MasterDict["SMS_Gluinos"] = {
    //pathPrefix + "Fall17_102X_SMS/SMS-T1qqqq-compressedGluino_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root",
    //pathPrefix + "Fall17_102X_SMS/SMS-T1qqqqL_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root",
    pathPrefix + "Fall17_102X_SMS/SMS-T1qqqq_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root"
  };

    //pathPrefix + "Summer22_130X_SMS/SMS-TChiWZ_mC1-300_mN2-300_mN1-290_NanoAODv12_JustinPrivateMC_Summer22_130X_SMS_Summer22_130X.root",
  MasterDict["SMS_TChiWZ_2016APV"] = {
    pathPrefix + "Summer20UL16APV_106X_SMS/SMS-TChiWZ_ZToLL_mZMin-0p1_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_106X.root"
  };
  MasterDict["SMS_TChiWZ_2016"] = {
    pathPrefix + "Summer20UL16_106X_SMS/SMS-TChiWZ_ZToLL_mZMin-0p1_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_106X.root"
  };
  MasterDict["SMS_TChiWZ_2017"] = {
    pathPrefix + "Summer20UL17_106X_SMS/SMS-TChiWZ_ZToLL_mZMin-0p1_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_106X.root"
  };
  MasterDict["SMS_TChiWZ_2018"] = {
    pathPrefix + "Summer20UL18_106X_SMS/SMS-TChiWZ_ZToLL_mZMin-0p1_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_106X.root"
  };
  MasterDict["SMS_TChiWZ_2022"] = {
    pathPrefix + "Summer22_130X_SMS/SMS-TChiWZ_ZToLL_mZMin-0p1_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16_Summer22_130X.root"
  };
  MasterDict["SMS_TChiWZ_2022EE"] = {
    pathPrefix + "Summer22EE_130X_SMS/SMS-TChiWZ_ZToLL_mZMin-0p1_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL16APV_Summer22EE_130X.root",
  };
  MasterDict["SMS_TChiWZ_2023"] = {
    pathPrefix + "Summer23_130X_SMS/SMS-TChiWZ_ZToLL_mZMin-0p1_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_Summer23_130X.root"
  };
  MasterDict["SMS_TChiWZ_2023BPix"] = {
    pathPrefix + "Summer23BPix_130X_SMS/SMS-TChiWZ_ZToLL_mZMin-0p1_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_Summer23BPix_130X.root"
  };
  MasterDict["SMS_TChiWZ_2024"] = {
    pathPrefix + "Summer24_130X_SMS/SMS-TChiWZ_ZToLL_mZMin-0p1_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_Summer24_130X.root",
  };
  MasterDict["SMS_TChiWZ_2025"] = {
    pathPrefix + "Summer25_130X_SMS/SMS-TChiWZ_ZToLL_mZMin-0p1_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL18_Summer25_130X.root",
  };
  MasterDict["SMS_TChiWZ_2026"] = {
    pathPrefix + "Summer26_130X_SMS/SMS-TChiWZ_ZToLL_mZMin-0p1_TuneCP5_13TeV-madgraphMLM-pythia8_Summer20UL17_Summer26_130X.root",
  };

  MasterDict["SMS_TChiWZpreUL_2016"] = {
    pathPrefix + "Summer16_102X_SMS/TChiWZ_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_102X.root",
    pathPrefix + "Summer16_102X_SMS/SMS-TChiWZ_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_102X.root",
    pathPrefix + "Summer16_102X_SMS/SMS-TChiWZ_dM-60to90_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_102X.root",
  };
  MasterDict["SMS_TChiWZpreUL_2017"] = {
    pathPrefix + "Fall17_102X_SMS/TChiWZ_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root",
    pathPrefix + "Fall17_102X_SMS/SMS-TChiWZ_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root",
    pathPrefix + "Fall17_102X_SMS/SMS-TChiWZ_dM-60to90_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root",
  };
  MasterDict["SMS_TChiWZpreUL_2018"] = {
    pathPrefix + "Autumn18_102X_SMS/TChiWZ_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_102X.root",
    pathPrefix + "Autumn18_102X_SMS/SMS-TChiWZ_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_102X.root",
    pathPrefix + "Autumn18_102X_SMS/SMS-TChiWZ_dM-60to90_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_102X.root",
  };
  MasterDict["SMS_TChiWZpreUL_2022"] = {
    pathPrefix + "Summer22_130X_SMS/SMS-TChiWZ_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_Summer22_130X.root",
    pathPrefix + "Summer22_130X_SMS/SMS-TChiWZ_dM-60to90_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_Summer22_130X.root",
    pathPrefix + "Summer22_130X_SMS/TChiWZ_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_Summer22_130X.root",
  };
  MasterDict["SMS_TChiWZpreUL_2022EE"] = {
    pathPrefix + "Summer22EE_130X_SMS/SMS-TChiWZ_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X_SMS/SMS-TChiWZ_dM-60to90_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X_SMS/TChiWZ_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_Summer22EE_130X.root",
  };
  MasterDict["SMS_TChiWZpreUL_2023"] = {
    pathPrefix + "Summer23_130X_SMS/SMS-TChiWZ_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_Summer23_130X.root",
    pathPrefix + "Summer23_130X_SMS/SMS-TChiWZ_dM-60to90_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_Summer23_130X.root",
    pathPrefix + "Summer23_130X_SMS/TChiWZ_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_Summer23_130X.root",
  };
  MasterDict["SMS_TChiWZpreUL_2023BPix"] = {
    pathPrefix + "Summer23BPix_130X_SMS/SMS-TChiWZ_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X_SMS/SMS-TChiWZ_dM-60to90_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X_SMS/TChiWZ_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_Summer23BPix_130X.root",
  };
  MasterDict["SMS_TChiWZpreUL_2024"] = {
    pathPrefix + "Summer24_130X_SMS/SMS-TChiWZ_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer24_130X.root",
    pathPrefix + "Summer24_130X_SMS/SMS-TChiWZ_dM-60to90_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer24_130X.root",
    pathPrefix + "Summer24_130X_SMS/TChiWZ_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer24_130X.root",
  };
  MasterDict["SMS_TChiWZpreUL_2025"] = {
    pathPrefix + "Summer25_130X_SMS/SMS-TChiWZ_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer25_130X.root",
    pathPrefix + "Summer25_130X_SMS/SMS-TChiWZ_dM-60to90_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer25_130X.root",
    pathPrefix + "Summer25_130X_SMS/TChiWZ_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer25_130X.root",
  };
  MasterDict["SMS_TChiWZpreUL_2026"] = {
    pathPrefix + "Summer26_130X_SMS/TChiWZ_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer26_130X.root",
    pathPrefix + "Summer26_130X_SMS/SMS-TChiWZ_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer26_130X.root",
    pathPrefix + "Summer26_130X_SMS/SMS-TChiWZ_dM-60to90_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer26_130X.root",
  };

  MasterDict["SMS_TChiWZTEST_2016APV"] = {
    pathPrefix + "Summer20UL16APV_106X_SMS/TChiWZ_MNLSP300_MLSP290_EDFilterOR_TuneCP5_13p6TeV-madgraph-pythia8_Summer20UL16APV_106X.root",
  };
  MasterDict["SMS_TChiWZTEST_2016"] = {
    pathPrefix + "Summer20UL16_106X_SMS/TChiWZ_MNLSP300_MLSP290_EDFilterOR_TuneCP5_13p6TeV-madgraph-pythia8_Summer20UL16_106X.root",
  };
  MasterDict["SMS_TChiWZTEST_2017"] = {
    pathPrefix + "Summer20UL17_106X_SMS/TChiWZ_MNLSP300_MLSP290_EDFilterOR_TuneCP5_13p6TeV-madgraph-pythia8_Summer20UL17_106X.root",
  };
  MasterDict["SMS_TChiWZTEST_2018"] = {
    pathPrefix + "Summer20UL18_106X_SMS/TChiWZ_MNLSP300_MLSP290_EDFilterOR_TuneCP5_13p6TeV-madgraph-pythia8_Summer20UL18_106X.root",
  };

  MasterDict["SMS_TChiWZTEST_2022"] = {
    pathPrefix + "Summer22_130X_SMS/TChiWZ_MNLSP300_MLSP290_EDFilterOR_TuneCP5_13p6TeV-madgraph-pythia8_Summer22_130X.root",
  };
  MasterDict["SMS_TChiWZTEST_2022EE"] = {
    pathPrefix + "Summer22EE_130X_SMS/TChiWZ_MNLSP300_MLSP290_EDFilterOR_TuneCP5_13p6TeV-madgraph-pythia8_Summer22EE_130X.root",
  };
  MasterDict["SMS_TChiWZTEST_2023"] = {
    pathPrefix + "Summer23_130X_SMS/TChiWZ_MNLSP300_MLSP290_EDFilterOR_TuneCP5_13p6TeV-madgraph-pythia8_Summer23_130X.root",
  };
  MasterDict["SMS_TChiWZTEST_2023BPix"] = {
    pathPrefix + "Summer23BPix_130X_SMS/TChiWZ_MNLSP300_MLSP290_EDFilterOR_TuneCP5_13p6TeV-madgraph-pythia8_Summer23BPix_130X.root",
  };
  MasterDict["SMS_TChiWZTEST_2024"] = {
    pathPrefix + "Summer24_130X_SMS/TChiWZ_MNLSP300_MLSP290_EDFilterOR_TuneCP5_13p6TeV-madgraph-pythia8_Summer24_130X.root",
  };
  MasterDict["SMS_TChiWZTEST_2025"] = {
    pathPrefix + "Summer25_130X_SMS/TChiWZ_MNLSP300_MLSP290_EDFilterOR_TuneCP5_13p6TeV-madgraph-pythia8_Summer25_130X.root",
  };
  MasterDict["SMS_TChiWZTEST_2026"] = {
    pathPrefix + "Summer26_130X_SMS/TChiWZ_MNLSP300_MLSP290_EDFilterOR_TuneCP5_13p6TeV-madgraph-pythia8_Summer26_130X.root",
  };

  MasterDict["SMS_TSlepSleppreUL_2016"] = {
    pathPrefix + "Summer16_102X_SMS/SMS-TSlepSlep_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_102X.root",
    pathPrefix + "Summer16_102X_SMS/SMS-TSlepSlep_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_102X.root",
  };
  MasterDict["SMS_TSlepSleppreUL_2017"] = {
    pathPrefix + "Fall17_102X_SMS/SMS-TSlepSlep_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root",
    pathPrefix + "Fall17_102X_SMS/SMS-TSlepSlep_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root",
  };
  MasterDict["SMS_TSlepSleppreUL_2018"] = {
    pathPrefix + "Autumn18_102X_SMS/SMS-TSlepSlep_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_102X.root",
    pathPrefix + "Autumn18_102X_SMS/SMS-TSlepSlep_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_102X.root",
  };
  MasterDict["SMS_TSlepSleppreUL_2022"] = {
    pathPrefix + "Summer22_130X_SMS/SMS-TSlepSlep_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_Summer22_130X.root",
    pathPrefix + "Summer22_130X_SMS/SMS-TSlepSlep_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_Summer22_130X.root",
  };
  MasterDict["SMS_TSlepSleppreUL_2022EE"] = {
    pathPrefix + "Summer22EE_130X_SMS/SMS-TSlepSlep_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X_SMS/SMS-TSlepSlep_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_Summer22EE_130X.root",
  };
  MasterDict["SMS_TSlepSleppreUL_2023"] = {
    pathPrefix + "Summer23_130X_SMS/SMS-TSlepSlep_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_Summer23_130X.root",
    pathPrefix + "Summer23_130X_SMS/SMS-TSlepSlep_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X_Summer23_130X.root",
  };
  MasterDict["SMS_TSlepSleppreUL_2023BPix"] = {
    pathPrefix + "Summer23BPix_130X_SMS/SMS-TSlepSlep_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X_SMS/SMS-TSlepSlep_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X_Summer23BPix_130X.root",
  };
  MasterDict["SMS_TSlepSleppreUL_2024"] = {
    pathPrefix + "Summer24_130X_SMS/SMS-TSlepSlep_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer24_130X.root",
    pathPrefix + "Summer24_130X_SMS/SMS-TSlepSlep_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer24_130X.root",
  };
  MasterDict["SMS_TSlepSleppreUL_2025"] = {
    pathPrefix + "Summer25_130X_SMS/SMS-TSlepSlep_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer25_130X.root",
    pathPrefix + "Summer25_130X_SMS/SMS-TSlepSlep_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer25_130X.root",
  };
  MasterDict["SMS_TSlepSleppreUL_2026"] = {
    pathPrefix + "Summer26_130X_SMS/SMS-TSlepSlep_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer26_130X.root",
    pathPrefix + "Summer26_130X_SMS/SMS-TSlepSlep_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer26_130X.root",
  };

  MasterDict["SMS_TChiWWpreUL_2016"] = {
    pathPrefix + "Summer16_102X_SMS/SMS-TChipmWW_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_102X.root",
    pathPrefix + "Summer16_102X_SMS/SMS-TChipmWW_dM-3to50_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_102X.root",
    pathPrefix + "Summer16_102X_SMS/SMS-TChipmWW_dM-60to90_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_102X.root",
  };
  MasterDict["SMS_TChiWWpreUL_2017"] = {
    pathPrefix + "Fall17_102X_SMS/SMS-TChipmWW_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root",
    pathPrefix + "Fall17_102X_SMS/SMS-TChipmWW_dM-3to50_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root",
    pathPrefix + "Fall17_102X_SMS/SMS-TChipmWW_dM-60to90_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_102X.root",
  };
  MasterDict["SMS_TChiWWpreUL_2018"] = {
    pathPrefix + "Autumn18_102X_SMS/SMS-TChipmWW_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_102X.root",
    pathPrefix + "Autumn18_102X_SMS/SMS-TChipmWW_dM-3to50_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_102X.root",
    pathPrefix + "Autumn18_102X_SMS/SMS-TChipmWW_dM-60to90_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_102X.root",
  };
  MasterDict["SMS_TChiWWpreUL_2022"] = {
    pathPrefix + "Summer22_130X_SMS/SMS-TChipmWW_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_Summer22_130X.root",
    pathPrefix + "Summer22_130X_SMS/SMS-TChipmWW_dM-3to50_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_Summer22_130X.root",
    pathPrefix + "Summer22_130X_SMS/SMS-TChipmWW_dM-60to90_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_Summer22_130X.root",
  };
  MasterDict["SMS_TChiWWpreUL_2022EE"] = {
    pathPrefix + "Summer22EE_130X_SMS/SMS-TChipmWW_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X_SMS/SMS-TChipmWW_dM-3to50_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X_SMS/SMS-TChipmWW_dM-60to90_genHT-160_genMET-80_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16_Summer22EE_130X.root",
  };
  MasterDict["SMS_TChiWWpreUL_2023"] = {
    pathPrefix + "Summer23_130X_SMS/SMS-TChipmWW_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_Summer23_130X.root",
    pathPrefix + "Summer23_130X_SMS/SMS-TChipmWW_dM-3to50_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_Summer23_130X.root",
    pathPrefix + "Summer23_130X_SMS/SMS-TChipmWW_dM-60to90_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_Summer23_130X.root",
  };
  MasterDict["SMS_TChiWWpreUL_2023BPix"] = {
    pathPrefix + "Summer23BPix_130X_SMS/SMS-TChipmWW_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X_SMS/SMS-TChipmWW_dM-3to50_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X_SMS/SMS-TChipmWW_dM-60to90_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Fall17_Summer23BPix_130X.root",
  };
  MasterDict["SMS_TChiWWpreUL_2024"] = {
    pathPrefix + "Summer24_130X_SMS/SMS-TChipmWW_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer24_130X.root",
    pathPrefix + "Summer24_130X_SMS/SMS-TChipmWW_dM-3to50_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer24_130X.root",
    pathPrefix + "Summer24_130X_SMS/SMS-TChipmWW_dM-60to90_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer24_130X.root",
  };
  MasterDict["SMS_TChiWWpreUL_2025"] = {
    pathPrefix + "Summer25_130X_SMS/SMS-TChipmWW_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer25_130X.root",
    pathPrefix + "Summer25_130X_SMS/SMS-TChipmWW_dM-3to50_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer25_130X.root",
    pathPrefix + "Summer25_130X_SMS/SMS-TChipmWW_dM-60to90_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer25_130X.root",
  };
  MasterDict["SMS_TChiWWpreUL_2026"] = {
    pathPrefix + "Summer26_130X_SMS/SMS-TChipmWW_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer26_130X.root",
    pathPrefix + "Summer26_130X_SMS/SMS-TChipmWW_dM-3to50_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer26_130X.root",
    pathPrefix + "Summer26_130X_SMS/SMS-TChipmWW_dM-60to90_genHT-160_genMET-80_TuneCP2_13TeV-madgraphMLM-pythia8_Autumn18_Summer26_130X.root",
  };

  MasterDict["SMS_TChiWZ_Sandwich"] = {
    pathPrefix + "Summer22_130X_SMS/SMS-TChiWZ_mC1-295_mN2-300_mN1-290_NanoAODv12_JustinPrivateMC_Summer22_130X_SMS_Summer22_130X.root"
  };

  MasterDict["SMS_TSlepSlep_TEST_2016APV"] = {
    pathPrefix + "Summer20UL16APV_106X_SMS/TSlepSlep_MSlep250_MLSP245_TuneCP5_13p6TeV-madgraph-pythia8_Summer20UL16APV_106X.root"
  };
  MasterDict["SMS_TSlepSlep_TEST_2016"] = {
    pathPrefix + "Summer20UL16_106X_SMS/TSlepSlep_MSlep250_MLSP245_TuneCP5_13p6TeV-madgraph-pythia8_Summer20UL16_106X.root"
  };
  MasterDict["SMS_TSlepSlep_TEST_2017"] = {
    pathPrefix + "Summer20UL17_106X_SMS/TSlepSlep_MSlep250_MLSP245_TuneCP5_13p6TeV-madgraph-pythia8_Summer20UL17_106X.root"
  };
  MasterDict["SMS_TSlepSlep_TEST_2018"] = {
    pathPrefix + "Summer20UL18_106X_SMS/TSlepSlep_MSlep250_MLSP245_TuneCP5_13p6TeV-madgraph-pythia8_Summer20UL18_106X.root"
  };
  MasterDict["SMS_TSlepSlep_TEST_2022"] = {
    pathPrefix + "Summer22_130X_SMS/TSlepSlep_MSlep250_MLSP245_TuneCP5_13p6TeV-madgraph-pythia8_Summer22_130X.root"
  };
  MasterDict["SMS_TSlepSlep_TEST_2022EE"] = {
    pathPrefix + "Summer22EE_130X_SMS/TSlepSlep_MSlep250_MLSP245_TuneCP5_13p6TeV-madgraph-pythia8_Summer22EE_130X.root"
  };
  MasterDict["SMS_TSlepSlep_TEST_2023"] = {
    pathPrefix + "Summer23_130X_SMS/TSlepSlep_MSlep250_MLSP245_TuneCP5_13p6TeV-madgraph-pythia8_Summer23_130X.root"
  };
  MasterDict["SMS_TSlepSlep_TEST_2023BPix"] = {
    pathPrefix + "Summer23BPix_130X_SMS/TSlepSlep_MSlep250_MLSP245_TuneCP5_13p6TeV-madgraph-pythia8_Summer23BPix_130X.root"
  };
  MasterDict["SMS_TSlepSlep_TEST_2024"] = {
    pathPrefix + "Summer24_130X_SMS/TSlepSlep_MSlep250_MLSP245_TuneCP5_13p6TeV-madgraph-pythia8_Summer24_130X.root"
  };
  MasterDict["SMS_TSlepSlep_TEST_2025"] = {
    pathPrefix + "Summer25_130X_SMS/TSlepSlep_MSlep250_MLSP245_TuneCP5_13p6TeV-madgraph-pythia8_Summer25_130X.root"
  };
  MasterDict["SMS_TSlepSlep_TEST_2026"] = {
    pathPrefix + "Summer26_130X_SMS/TSlepSlep_MSlep250_MLSP245_TuneCP5_13p6TeV-madgraph-pythia8_Summer26_130X.root"
  };

  MasterDict["Data_2016APV"] = {
    pathPrefix + "Summer20UL16APV_106X_Data/MET_Run2016B-ver2_HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/MET_Run2016C-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/MET_Run2016D-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/MET_Run2016E-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/MET_Run2016F-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
  };

  MasterDict["Data_2016"] = {
    pathPrefix + "Summer20UL16_106X_Data/MET_Run2016F-UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16_106X_Data_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X_Data/MET_Run2016G-UL2016_MiniAODv2_NanoAODv9-v1_Summer20UL16_106X_Data_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X_Data/MET_Run2016H-UL2016_MiniAODv2_NanoAODv9-v1_Summer20UL16_106X_Data_Summer20UL16_106X.root",
  };

  MasterDict["Data_2017"] = {
    pathPrefix + "Summer20UL17_106X_Data/MET_Run2017B-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/MET_Run2017C-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/MET_Run2017D-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/MET_Run2017E-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/MET_Run2017F-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
  };

  MasterDict["Data_2018"] = {
    pathPrefix + "Summer20UL18_106X_Data/MET_Run2018A-UL2018_MiniAODv2_NanoAODv9-v2_Summer20UL18_106X_Data_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X_Data/MET_Run2018B-UL2018_MiniAODv2_NanoAODv9-v2_Summer20UL18_106X_Data_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X_Data/MET_Run2018C-UL2018_MiniAODv2_NanoAODv9-v1_Summer20UL18_106X_Data_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X_Data/MET_Run2018D-UL2018_MiniAODv2_NanoAODv9-v1_Summer20UL18_106X_Data_Summer20UL18_106X.root",
  };

  MasterDict["Data_2022"] = {
    pathPrefix + "Summer22_130X_Data/JetMET_Run2022C-22Sep2023-v1_Summer22_130X_Data_Summer22_130X.root",
    pathPrefix + "Summer22_130X_Data/JetMET_Run2022D-22Sep2023-v1_Summer22_130X_Data_Summer22_130X.root",
  };

  MasterDict["Data_2022EE"] = {
    pathPrefix + "Summer22EE_130X_Data/JetMET_Run2022E-22Sep2023-v1_Summer22EE_130X_Data_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X_Data/JetMET_Run2022F-22Sep2023-v2_Summer22EE_130X_Data_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X_Data/JetMET_Run2022G-22Sep2023-v2_Summer22EE_130X_Data_Summer22EE_130X.root",
  };

  MasterDict["Data_2023"] = {
    pathPrefix + "Summer23_130X_Data/JetMET0_Run2023C-22Sep2023_v4-v1_Summer23_130X_Data_Summer23_130X.root",
    pathPrefix + "Summer23_130X_Data/JetMET1_Run2023C-22Sep2023_v4-v1_Summer23_130X_Data_Summer23_130X.root",
  };

  MasterDict["Data_2023BPix"] = {
    pathPrefix + "Summer23BPix_130X_Data/JetMET0_Run2023D-22Sep2023_v2-v1_Summer23BPix_130X_Data_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X_Data/JetMET1_Run2023D-22Sep2023_v2-v1_Summer23BPix_130X_Data_Summer23BPix_130X.root",
  };

  MasterDict["Data_2024"] = {
    pathPrefix + "Summer24_130X_Data/JetMET0_Run2024C-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/JetMET0_Run2024D-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/JetMET0_Run2024E-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/JetMET0_Run2024F-MINIv6NANOv15-v2_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/JetMET0_Run2024G-MINIv6NANOv15-v2_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/JetMET0_Run2024H-MINIv6NANOv15-v2_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/JetMET0_Run2024I-MINIv6NANOv15-v2_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/JetMET1_Run2024C-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/JetMET1_Run2024D-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/JetMET1_Run2024E-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/JetMET1_Run2024F-MINIv6NANOv15-v2_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/JetMET1_Run2024G-MINIv6NANOv15-v2_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/JetMET1_Run2024H-MINIv6NANOv15-v2_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/JetMET1_Run2024I-MINIv6NANOv15_v2-v2_Summer24_130X_Data_Summer24_130X.root",
  };

  MasterDict["Data_DoubleElectron_2016APV"] = {
    pathPrefix + "Summer20UL16APV_106X_Data/DoubleEG_Run2016B-ver1_HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/DoubleEG_Run2016B-ver2_HIPM_UL2016_MiniAODv2_NanoAODv9-v3_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/DoubleEG_Run2016C-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/DoubleEG_Run2016D-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/DoubleEG_Run2016E-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/DoubleEG_Run2016F-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
  };

  MasterDict["Data_DoubleElectron_2016"] = {
    pathPrefix + "Summer20UL16_106X_Data/DoubleEG_Run2016F-UL2016_MiniAODv2_NanoAODv9-v1_Summer20UL16_106X_Data_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X_Data/DoubleEG_Run2016G-UL2016_MiniAODv2_NanoAODv9-v1_Summer20UL16_106X_Data_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X_Data/DoubleEG_Run2016H-UL2016_MiniAODv2_NanoAODv9-v1_Summer20UL16_106X_Data_Summer20UL16_106X.root",
  };

  MasterDict["Data_DoubleElectron_2017"] = {
    pathPrefix + "Summer20UL17_106X_Data/DoubleEG_Run2017B-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/DoubleEG_Run2017C-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/DoubleEG_Run2017D-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/DoubleEG_Run2017E-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/DoubleEG_Run2017F-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
  };

  MasterDict["Data_SingleElectron_2016APV"] = {
    pathPrefix + "Summer20UL16APV_106X_Data/SingleElectron_Run2016B-ver1_HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/SingleElectron_Run2016B-ver2_HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/SingleElectron_Run2016C-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/SingleElectron_Run2016D-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/SingleElectron_Run2016E-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/SingleElectron_Run2016F-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
  };

  MasterDict["Data_SingleElectron_2016"] = {
    pathPrefix + "Summer20UL16_106X_Data/SingleElectron_Run2016F-UL2016_MiniAODv2_NanoAODv9-v1_Summer20UL16_106X_Data_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X_Data/SingleElectron_Run2016G-UL2016_MiniAODv2_NanoAODv9-v1_Summer20UL16_106X_Data_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X_Data/SingleElectron_Run2016H-UL2016_MiniAODv2_NanoAODv9-v1_Summer20UL16_106X_Data_Summer20UL16_106X.root",
  };

  MasterDict["Data_SingleElectron_2017"] = {
    pathPrefix + "Summer20UL17_106X_Data/SingleElectron_Run2017B-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/SingleElectron_Run2017C-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/SingleElectron_Run2017D-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/SingleElectron_Run2017E-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/SingleElectron_Run2017F-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
  };

  MasterDict["Data_SingleElectron_2018"] = {
    pathPrefix + "Summer20UL18_106X_Data/EGamma_Run2018A-UL2018_MiniAODv2_NanoAODv9-v1_Summer20UL18_106X_Data_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X_Data/EGamma_Run2018B-UL2018_MiniAODv2_NanoAODv9-v1_Summer20UL18_106X_Data_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X_Data/EGamma_Run2018C-UL2018_MiniAODv2_NanoAODv9-v1_Summer20UL18_106X_Data_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X_Data/EGamma_Run2018D-UL2018_MiniAODv2_NanoAODv9-v3_Summer20UL18_106X_Data_Summer20UL18_106X.root",
  };

  MasterDict["Data_SingleElectron_2022"] = {
    pathPrefix + "Summer22_130X_Data/EGamma_Run2022C-NanoAODv15-v1_Summer22_130X_Data_Summer22_130X.root",
    pathPrefix + "Summer22_130X_Data/EGamma_Run2022D-NanoAODv15-v1_Summer22_130X_Data_Summer22_130X.root",
  };

  MasterDict["Data_SingleElectron_2022EE"] = {
    pathPrefix + "Summer22EE_130X_Data/EGamma_Run2022E-NanoAODv15-v1_Summer22EE_130X_Data_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X_Data/EGamma_Run2022F-NanoAODv15-v1_Summer22EE_130X_Data_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X_Data/EGamma_Run2022G-NanoAODv15-v1_Summer22EE_130X_Data_Summer22EE_130X.root",
  };

  MasterDict["Data_SingleElectron_2023"] = {
    pathPrefix + "Summer23_130X_Data/EGamma0_Run2023C-NanoAODv15_v4-v1_Summer23_130X_Data_Summer23_130X.root",
    pathPrefix + "Summer23_130X_Data/EGamma1_Run2023C-NanoAODv15_v4-v1_Summer23_130X_Data_Summer23_130X.root",
  };

  MasterDict["Data_SingleElectron_2023BPix"] = {
    pathPrefix + "Summer23BPix_130X_Data/EGamma0_Run2023D-NanoAODv15_v2-v1_Summer23BPix_130X_Data_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X_Data/EGamma1_Run2023D-NanoAODv15_v2-v1_Summer23BPix_130X_Data_Summer23BPix_130X.root",
  };

  MasterDict["Data_SingleElectron_2024"] = {
    pathPrefix + "Summer24_130X_Data/EGamma0_Run2024C-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/EGamma0_Run2024D-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/EGamma0_Run2024E-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/EGamma0_Run2024F-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/EGamma0_Run2024G-MINIv6NANOv15-v2_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/EGamma0_Run2024H-MINIv6NANOv15-v2_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/EGamma0_Run2024I-MINIv6NANOv15_v2-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/EGamma1_Run2024C-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/EGamma1_Run2024D-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/EGamma1_Run2024E-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/EGamma1_Run2024F-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/EGamma1_Run2024G-MINIv6NANOv15-v2_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/EGamma1_Run2024H-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/EGamma1_Run2024I-MINIv6NANOv15_v2-v1_Summer24_130X_Data_Summer24_130X.root",
  };

  MasterDict["Data_MuonEG_2016APV"] = {
    pathPrefix + "Summer20UL16APV_106X_Data/MuonEG_Run2016B-ver1_HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/MuonEG_Run2016B-ver2_HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/MuonEG_Run2016C-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/MuonEG_Run2016D-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/MuonEG_Run2016E-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/MuonEG_Run2016F-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
  };

  MasterDict["Data_MuonEG_2016"] = {
    pathPrefix + "Summer20UL16_106X_Data/MuonEG_Run2016F-UL2016_MiniAODv2_NanoAODv9-v1_Summer20UL16_106X_Data_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X_Data/MuonEG_Run2016G-UL2016_MiniAODv2_NanoAODv9-v1_Summer20UL16_106X_Data_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X_Data/MuonEG_Run2016H-UL2016_MiniAODv2_NanoAODv9-v1_Summer20UL16_106X_Data_Summer20UL16_106X.root",
  };

  MasterDict["Data_MuonEG_2017"] = {
    pathPrefix + "Summer20UL17_106X_Data/MuonEG_Run2017B-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/MuonEG_Run2017C-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/MuonEG_Run2017D-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/MuonEG_Run2017E-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/MuonEG_Run2017F-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
  };

  MasterDict["Data_MuonEG_2018"] = {
    pathPrefix + "Summer20UL18_106X_Data/MuonEG_Run2018A-UL2018_MiniAODv2_NanoAODv9-v1_Summer20UL18_106X_Data_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X_Data/MuonEG_Run2018B-UL2018_MiniAODv2_NanoAODv9-v1_Summer20UL18_106X_Data_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X_Data/MuonEG_Run2018C-UL2018_MiniAODv2_NanoAODv9-v1_Summer20UL18_106X_Data_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X_Data/MuonEG_Run2018D-UL2018_MiniAODv2_NanoAODv9-v1_Summer20UL18_106X_Data_Summer20UL18_106X.root",
  };

  MasterDict["Data_MuonEG_2022"] = {
    pathPrefix + "Summer22_130X_Data/MuonEG_Run2022C-NanoAODv15-v1_Summer22_130X_Data_Summer22_130X.root",
    pathPrefix + "Summer22_130X_Data/MuonEG_Run2022D-NanoAODv15-v1_Summer22_130X_Data_Summer22_130X.root",
  };

  MasterDict["Data_MuonEG_2022EE"] = {
    pathPrefix + "Summer22EE_130X_Data/MuonEG_Run2022E-NanoAODv15-v1_Summer22EE_130X_Data_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X_Data/MuonEG_Run2022F-NanoAODv15-v1_Summer22EE_130X_Data_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X_Data/MuonEG_Run2022G-NanoAODv15-v1_Summer22EE_130X_Data_Summer22EE_130X.root",
  };

  MasterDict["Data_MuonEG_2023"] = {
    pathPrefix + "Summer23_130X_Data/MuonEG_Run2023C-NanoAODv15_v4-v1_Summer23_130X_Data_Summer23_130X.root",
  };

  MasterDict["Data_MuonEG_2023BPix"] = {
    pathPrefix + "Summer23BPix_130X_Data/MuonEG_Run2023D-NanoAODv15_v2-v1_Summer23BPix_130X_Data_Summer23BPix_130X.root",
  };

  MasterDict["Data_MuonEG_2024"] = {
    pathPrefix + "Summer24_130X_Data/MuonEG_Run2024C-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/MuonEG_Run2024D-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/MuonEG_Run2024E-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/MuonEG_Run2024F-MINIv6NANOv15-v2_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/MuonEG_Run2024G-MINIv6NANOv15-v3_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/MuonEG_Run2024H-MINIv6NANOv15-v2_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/MuonEG_Run2024I-MINIv6NANOv15_v2-v2_Summer24_130X_Data_Summer24_130X.root",
  };

  MasterDict["Data_SingleMuon_2016APV"] = {
    pathPrefix + "Summer20UL16APV_106X_Data/SingleMuon_Run2016B-ver1_HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/SingleMuon_Run2016B-ver2_HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/SingleMuon_Run2016C-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/SingleMuon_Run2016D-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/SingleMuon_Run2016E-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/SingleMuon_Run2016F-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
  };

  MasterDict["Data_SingleMuon_2016"] = {
    pathPrefix + "Summer20UL16_106X_Data/SingleMuon_Run2016F-UL2016_MiniAODv2_NanoAODv9-v1_Summer20UL16_106X_Data_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X_Data/SingleMuon_Run2016G-UL2016_MiniAODv2_NanoAODv9-v1_Summer20UL16_106X_Data_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X_Data/SingleMuon_Run2016H-UL2016_MiniAODv2_NanoAODv9-v1_Summer20UL16_106X_Data_Summer20UL16_106X.root",
  };

  MasterDict["Data_SingleMuon_2017"] = {
    pathPrefix + "Summer20UL17_106X_Data/SingleMuon_Run2017B-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/SingleMuon_Run2017C-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/SingleMuon_Run2017D-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/SingleMuon_Run2017E-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/SingleMuon_Run2017F-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
  };

  MasterDict["Data_SingleMuon_2018"] = {
    pathPrefix + "Summer20UL18_106X_Data/SingleMuon_Run2018A-UL2018_MiniAODv2_NanoAODv9-v2_Summer20UL18_106X_Data_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X_Data/SingleMuon_Run2018B-UL2018_MiniAODv2_NanoAODv9-v2_Summer20UL18_106X_Data_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X_Data/SingleMuon_Run2018C-UL2018_MiniAODv2_NanoAODv9-v2_Summer20UL18_106X_Data_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X_Data/SingleMuon_Run2018D-UL2018_MiniAODv2_NanoAODv9-v1_Summer20UL18_106X_Data_Summer20UL18_106X.root",
  };

  MasterDict["Data_SingleMuon_2022"] = {
    pathPrefix + "Summer22_130X_Data/Muon_Run2022C-NanoAODv15-v1_Summer22_130X_Data_Summer22_130X.root",
    pathPrefix + "Summer22_130X_Data/Muon_Run2022D-NanoAODv15-v1_Summer22_130X_Data_Summer22_130X.root",
  };

  MasterDict["Data_SingleMuon_2022EE"] = {
    pathPrefix + "Summer22EE_130X_Data/Muon_Run2022E-NanoAODv15-v1_Summer22EE_130X_Data_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X_Data/Muon_Run2022F-NanoAODv15-v1_Summer22EE_130X_Data_Summer22EE_130X.root",
    pathPrefix + "Summer22EE_130X_Data/Muon_Run2022G-NanoAODv15-v1_Summer22EE_130X_Data_Summer22EE_130X.root",
  };

  MasterDict["Data_SingleMuon_2023"] = {
    pathPrefix + "Summer23_130X_Data/Muon0_Run2023C-NanoAODv15_v4-v1_Summer23_130X_Data_Summer23_130X.root",
    pathPrefix + "Summer23_130X_Data/Muon1_Run2023C-NanoAODv15_v4-v1_Summer23_130X_Data_Summer23_130X.root",
  };

  MasterDict["Data_SingleMuon_2023BPix"] = {
    pathPrefix + "Summer23BPix_130X_Data/Muon0_Run2023D-NanoAODv15_v2-v1_Summer23BPix_130X_Data_Summer23BPix_130X.root",
    pathPrefix + "Summer23BPix_130X_Data/Muon1_Run2023D-NanoAODv15_v2-v1_Summer23BPix_130X_Data_Summer23BPix_130X.root",
  };

  MasterDict["Data_SingleMuon_2024"] = {
    pathPrefix + "Summer24_130X_Data/Muon0_Run2024C-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/Muon0_Run2024D-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/Muon0_Run2024E-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/Muon0_Run2024F-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/Muon0_Run2024G-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/Muon0_Run2024H-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/Muon0_Run2024I-MINIv6NANOv15_v2-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/Muon1_Run2024C-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/Muon1_Run2024D-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/Muon1_Run2024E-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/Muon1_Run2024F-MINIv6NANOv15-v1_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/Muon1_Run2024G-MINIv6NANOv15-v2_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/Muon1_Run2024H-MINIv6NANOv15-v2_Summer24_130X_Data_Summer24_130X.root",
    pathPrefix + "Summer24_130X_Data/Muon1_Run2024I-MINIv6NANOv15_v2-v1_Summer24_130X_Data_Summer24_130X.root",
  };

  MasterDict["Data_DoubleMuon_2016APV"] = {
    pathPrefix + "Summer20UL16APV_106X_Data/DoubleMuon_Run2016B-ver1_HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/DoubleMuon_Run2016B-ver2_HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/DoubleMuon_Run2016C-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/DoubleMuon_Run2016D-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/DoubleMuon_Run2016E-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
    pathPrefix + "Summer20UL16APV_106X_Data/DoubleMuon_Run2016F-HIPM_UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16APV_106X_Data_Summer20UL16APV_106X.root",
  };

  MasterDict["Data_DoubleMuon_2016"] = {
    pathPrefix + "Summer20UL16_106X_Data/DoubleMuon_Run2016F-UL2016_MiniAODv2_NanoAODv9-v1_Summer20UL16_106X_Data_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X_Data/DoubleMuon_Run2016G-UL2016_MiniAODv2_NanoAODv9-v2_Summer20UL16_106X_Data_Summer20UL16_106X.root",
    pathPrefix + "Summer20UL16_106X_Data/DoubleMuon_Run2016H-UL2016_MiniAODv2_NanoAODv9-v1_Summer20UL16_106X_Data_Summer20UL16_106X.root",
  };

  MasterDict["Data_DoubleMuon_2017"] = {
    pathPrefix + "Summer20UL17_106X_Data/DoubleMuon_Run2017B-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/DoubleMuon_Run2017C-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/DoubleMuon_Run2017D-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/DoubleMuon_Run2017E-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
    pathPrefix + "Summer20UL17_106X_Data/DoubleMuon_Run2017F-UL2017_MiniAODv2_NanoAODv9-v1_Summer20UL17_106X_Data_Summer20UL17_106X.root",
  };

  MasterDict["Data_DoubleMuon_2018"] = {
    pathPrefix + "Summer20UL18_106X_Data/DoubleMuon_Run2018A-UL2018_MiniAODv2_NanoAODv9-v1_Summer20UL18_106X_Data_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X_Data/DoubleMuon_Run2018B-UL2018_MiniAODv2_NanoAODv9-v1_Summer20UL18_106X_Data_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X_Data/DoubleMuon_Run2018C-UL2018_MiniAODv2_NanoAODv9-v1_Summer20UL18_106X_Data_Summer20UL18_106X.root",
    pathPrefix + "Summer20UL18_106X_Data/DoubleMuon_Run2018D-UL2018_MiniAODv2_NanoAODv9-v2_Summer20UL18_106X_Data_Summer20UL18_106X.root",
  };

  MasterDict["DBTB_2023BPix"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2023BPix",
      "TB_2023BPix",
    }
  );

  MasterDict["DBTB_2023"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2023",
      "TB_2023",
    }
  );

  MasterDict["DBTB_2022EE"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2022EE",
      "TB_2022EE",
    }
  );

  MasterDict["DBTB_2022"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2022",
      "TB_2022",
    }
  );

  MasterDict["DBTB_2018"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2018",
      "TB_2018",
    }
  );

  MasterDict["DBTB_2017"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2017",
      "TB_2017",
    }
  );

  MasterDict["DBTB_2016"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2016",
      "TB_2016",
    }
  );

  MasterDict["DBTB_2016APV"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2016APV",
      "TB_2016APV",
    }
  );

  MasterDict["diboson_2026"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2026",
    }
  );

  MasterDict["diboson_2025"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2025",
    }
  );

  MasterDict["diboson_2024"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2024",
    }
  );

  MasterDict["diboson_2023BPix"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2023BPix",
    }
  );

  MasterDict["diboson_2023"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2023",
    }
  );

  MasterDict["diboson_2022EE"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2022EE",
    }
  );

  MasterDict["diboson_2022"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2022",
    }
  );

  MasterDict["diboson_2018"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2018",
    }
  );

  MasterDict["diboson_2017"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2017",
    }
  );

  MasterDict["diboson_2016"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2016",
    }
  );

  MasterDict["diboson_2016APV"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2016APV",
    }
  );

  MasterDict["triboson_2026"] = mergeEntriesList(
    MasterDict,
    {
      "TB_2026",
    }
  );

  MasterDict["triboson_2025"] = mergeEntriesList(
    MasterDict,
    {
      "TB_2025",
    }
  );

  MasterDict["triboson_2024"] = mergeEntriesList(
    MasterDict,
    {
      "TB_2024",
    }
  );

  MasterDict["triboson_2023BPix"] = mergeEntriesList(
    MasterDict,
    {
      "TB_2023BPix",
    }
  );

  MasterDict["triboson_2023"] = mergeEntriesList(
    MasterDict,
    {
      "TB_2023",
    }
  );

  MasterDict["triboson_2022EE"] = mergeEntriesList(
    MasterDict,
    {
      "TB_2022EE",
    }
  );

  MasterDict["triboson_2022"] = mergeEntriesList(
    MasterDict,
    {
      "TB_2022",
    }
  );

  MasterDict["triboson_2018"] = mergeEntriesList(
    MasterDict,
    {
      "TB_2018",
    }
  );

  MasterDict["triboson_2017"] = mergeEntriesList(
    MasterDict,
    {
      "TB_2017",
    }
  );

  MasterDict["triboson_2016"] = mergeEntriesList(
    MasterDict,
    {
      "TB_2016",
    }
  );

  MasterDict["triboson_2016APV"] = mergeEntriesList(
    MasterDict,
    {
      "TB_2016APV",
    }
  );

  MasterDict["DBTB"] = mergeEntriesList(
    MasterDict,
    {
      "DBTB_2023BPix",
      "DBTB_2023",
      "DBTB_2022EE",
      "DBTB_2022",
      "DBTB_2018",
      "DBTB_2017",
      "DBTB_2016",
      "DBTB_2016APV",
    }
  );

  MasterDict["QCD"] = mergeEntriesList(
    MasterDict,
    {
      "QCD_2023BPix",
      "QCD_2023",
      "QCD_2022EE",
      "QCD_2022",
      "QCD_2018",
      "QCD_2017",
      "QCD_2016",
      "QCD_2016APV",
    }
  );

  MasterDict["ZInv"] = mergeEntriesList(
    MasterDict,
    {
      "ZInv_2023BPix",
      "ZInv_2023",
      "ZInv_2022EE",
      "ZInv_2022",
      "ZInv_2018",
      "ZInv_2017",
      "ZInv_2016",
      "ZInv_2016APV",
    }
  );

  MasterDict["Wjets"] = mergeEntriesList(
    MasterDict,
    {
      "Wjets_2023BPix",
      "Wjets_2023",
      "Wjets_2022EE",
      "Wjets_2022",
      "Wjets_2018",
      "Wjets_2017",
      "Wjets_2016",
      "Wjets_2016APV",
    }
  );

  MasterDict["top_2026"] = mergeEntriesList(
    MasterDict,
    {
      "ttbar_2026",
      "ST_2026",
    }
  );
  MasterDict["top_2025"] = mergeEntriesList(
    MasterDict,
    {
      "ttbar_2025",
      "ST_2025",
    }
  );
  MasterDict["top_2024"] = mergeEntriesList(
    MasterDict,
    {
      "ttbar_2024",
      "ST_2024",
    }
  );
  MasterDict["top_2023BPix"] = mergeEntriesList(
    MasterDict,
    {
      "ttbar_2023BPix",
      "ST_2023BPix",
    }
  );
  MasterDict["top_2023"] = mergeEntriesList(
    MasterDict,
    {
      "ttbar_2023",
      "ST_2023",
    }
  );
  MasterDict["top_2022EE"] = mergeEntriesList(
    MasterDict,
    {
      "ttbar_2022EE",
      "ST_2022EE",
    }
  );
  MasterDict["top_2022"] = mergeEntriesList(
    MasterDict,
    {
      "ttbar_2022",
      "ST_2022",
    }
  );
  MasterDict["top_2018"] = mergeEntriesList(
    MasterDict,
    {
      "ttbar_2018",
      "ST_2018",
    }
  );
  MasterDict["top_2017"] = mergeEntriesList(
    MasterDict,
    {
      "ttbar_2017",
      "ST_2017",
    }
  );
  MasterDict["top_2016"] = mergeEntriesList(
    MasterDict,
    {
      "ttbar_2016",
      "ST_2016",
    }
  );
  MasterDict["top_2016APV"] = mergeEntriesList(
    MasterDict,
    {
      "ttbar_2016APV",
      "ST_2016APV",
    }
  );

  MasterDict["Vfakeleps_2023BPix"] = mergeEntriesList(
    MasterDict,
    {
      "Wjets_2023BPix",
      "QCD_2023BPix",
      "ZInv_2023BPix",
    }
  );

  MasterDict["Vfakeleps_2023"] = mergeEntriesList(
    MasterDict,
    {
      "Wjets_2023",
      "QCD_2023",
      "ZInv_2023",
    }
  );

  MasterDict["Vfakeleps_2022EE"] = mergeEntriesList(
    MasterDict,
    {
      "Wjets_2022EE",
      "QCD_2022EE",
      "ZInv_2022EE",
    }
  );

  MasterDict["Vfakeleps_2022"] = mergeEntriesList(
    MasterDict,
    {
      "Wjets_2022",
      "QCD_2022",
      "ZInv_2022",
    }
  );

  MasterDict["Vfakeleps_2018"] = mergeEntriesList(
    MasterDict,
    {
      "Wjets_2018",
      "QCD_2018",
      "ZInv_2018",
    }
  );

  MasterDict["Vfakeleps_2017"] = mergeEntriesList(
    MasterDict,
    {
      "Wjets_2017",
      "QCD_2017",
      "ZInv_2017",
    }
  );

  MasterDict["Vfakeleps_2016"] = mergeEntriesList(
    MasterDict,
    {
      "Wjets_2016",
      "QCD_2016",
      "ZInv_2016",
    }
  );

  MasterDict["Vfakeleps_2016APV"] = mergeEntriesList(
    MasterDict,
    {
      "Wjets_2016APV",
      "QCD_2016APV",
      "ZInv_2016APV",
    }
  );

  MasterDict["boson_2026"] = mergeEntriesList(
    MasterDict,
    {
      "DY_2026",
      "ZInv_2026",
      "Wjets_2026",
    }
  );
  MasterDict["boson_2025"] = mergeEntriesList(
    MasterDict,
    {
      "DY_2025",
      "ZInv_2025",
      "Wjets_2025",
    }
  );
  MasterDict["boson_2024"] = mergeEntriesList(
    MasterDict,
    {
      "DY_2024",
      "ZInv_2024",
      "Wjets_2024",
    }
  );
  MasterDict["boson_2023BPix"] = mergeEntriesList(
    MasterDict,
    {
      "DY_2023BPix",
      "ZInv_2023BPix",
      "Wjets_2023BPix",
    }
  );
  MasterDict["boson_2023"] = mergeEntriesList(
    MasterDict,
    {
      "DY_2023",
      "ZInv_2023",
      "Wjets_2023",
    }
  );
  MasterDict["boson_2022EE"] = mergeEntriesList(
    MasterDict,
    {
      "DY_2022EE",
      "ZInv_2022EE",
      "Wjets_2022EE",
    }
  );
  MasterDict["boson_2022"] = mergeEntriesList(
    MasterDict,
    {
      "DY_2022",
      "ZInv_2022",
      "Wjets_2022",
    }
  );
  MasterDict["boson_2018"] = mergeEntriesList(
    MasterDict,
    {
      "DY_2018",
      "ZInv_2018",
      "Wjets_2018",
    }
  );
  MasterDict["boson_2017"] = mergeEntriesList(
    MasterDict,
    {
      "DY_2017",
      "ZInv_2017",
      "Wjets_2017",
    }
  );
  MasterDict["boson_2016"] = mergeEntriesList(
    MasterDict,
    {
      "DY_2016",
      "ZInv_2016",
      "Wjets_2016",
    }
  );
  MasterDict["boson_2016APV"] = mergeEntriesList(
    MasterDict,
    {
      "DY_2016APV",
      "ZInv_2016APV",
      "Wjets_2016APV",
    }
  );

  MasterDict["Vfakeleps_Run2"] = mergeEntriesList(
    MasterDict,
    {
      "Vfakeleps_2018",
      "Vfakeleps_2017",
      "Vfakeleps_2016",
      "Vfakeleps_2016APV",
    }
  );

  MasterDict["Wjets_Run2"] = mergeEntriesList(
    MasterDict,
    {
      "Wjets_2018",
      "Wjets_2017",
      "Wjets_2016",
      "Wjets_2016APV",
    }
  );

  MasterDict["top_Run2"] = mergeEntriesList(
    MasterDict,
    {
      "top_2018",
      "top_2017",
      "top_2016",
      "top_2016APV",
    }
  );

  MasterDict["boson_Run2"] = mergeEntriesList(
    MasterDict,
    {
      "boson_2018",
      "boson_2017",
      "boson_2016",
      "boson_2016APV",
    }
  );

  MasterDict["diboson_Run2"] = mergeEntriesList(
    MasterDict,
    {
      "DB_2018",
      "DB_2017",
      "DB_2016",
      "DB_2016APV",
    }
  );

  MasterDict["triboson_Run2"] = mergeEntriesList(
    MasterDict,
    {
      "TB_2018",
      "TB_2017",
      "TB_2016",
      "TB_2016APV",
    }
  );

  MasterDict["DY_Run2"] = mergeEntriesList(
    MasterDict,
    {
      "DY_2018",
      "DY_2017",
      "DY_2016",
      "DY_2016APV",
    }
  );

  MasterDict["Vfakeleps_Run3"] = mergeEntriesList(
    MasterDict,
    {
      "Vfakeleps_2023BPix",
      "Vfakeleps_2023",
      "Vfakeleps_2022EE",
      "Vfakeleps_2022",
    }
  );

  MasterDict["DY_Run3"] = mergeEntriesList(
    MasterDict,
    {
      "DY_2026",
      "DY_2025",
      "DY_2024",
      "DY_2023BPix",
      "DY_2023",
      "DY_2022EE",
      "DY_2022",
    }
  );

  MasterDict["Wjets_Run3"] = mergeEntriesList(
    MasterDict,
    {
      "Wjets_2026",
      "Wjets_2025",
      "Wjets_2024",
      "Wjets_2023BPix",
      "Wjets_2023",
      "Wjets_2022EE",
      "Wjets_2022",
    }
  );

  MasterDict["top_2022_Run3"] = mergeEntriesList(
    MasterDict,
    {
      "ttbar_2022",
    }
  );

  MasterDict["top_Run3CR"] = mergeEntriesList(
    MasterDict,
    {
      //"top_2026",
      //"top_2025",
      "top_2024",
      "top_2023BPix",
      "top_2023",
      "top_2022EE",
      "top_2022",
    }
  );

  MasterDict["boson_Run3CR"] = mergeEntriesList(
    MasterDict,
    {
      //"boson_2026",
      //"boson_2025",
      "boson_2024",
      "boson_2023BPix",
      "boson_2023",
      "boson_2022EE",
      "boson_2022",
    }
  );

  MasterDict["diboson_Run3CR"] = mergeEntriesList(
    MasterDict,
    {
      //"DB_2026",
      //"DB_2025",
      "DB_2024",
      "DB_2023BPix",
      "DB_2023",
      "DB_2022EE",
      "DB_2022",
    }
  );

  MasterDict["triboson_Run3CR"] = mergeEntriesList(
    MasterDict,
    {
      //"TB_2026",
      //"TB_2025",
      "TB_2024",
      "TB_2023BPix",
      "TB_2023",
      "TB_2022EE",
      "TB_2022",
    }
  );

  MasterDict["top_Run3"] = mergeEntriesList(
    MasterDict,
    {
      //"top_2026",
      "top_2025",
      "top_2024",
      "top_2023BPix",
      "top_2023",
      "top_2022EE",
      "top_2022",
    }
  );

  MasterDict["boson_Run3"] = mergeEntriesList(
    MasterDict,
    {
      //"boson_2026",
      "boson_2025",
      "boson_2024",
      "boson_2023BPix",
      "boson_2023",
      "boson_2022EE",
      "boson_2022",
    }
  );

  MasterDict["diboson_Run3"] = mergeEntriesList(
    MasterDict,
    {
      //"DB_2026",
      "DB_2025",
      "DB_2024",
      "DB_2023BPix",
      "DB_2023",
      "DB_2022EE",
      "DB_2022",
    }
  );

  MasterDict["triboson_Run3"] = mergeEntriesList(
    MasterDict,
    {
      //"TB_2026",
      "TB_2025",
      "TB_2024",
      "TB_2023BPix",
      "TB_2023",
      "TB_2022EE",
      "TB_2022",
    }
  );

  MasterDict["top"] = mergeEntriesList(
    MasterDict,
    {
      //"top_2026",
      "top_2025",
      "top_2024",
      "top_2023BPix",
      "top_2023",
      "top_2022EE",
      "top_2022",
      "top_2018",
      "top_2017",
      "top_2016",
      "top_2016APV",
    }
  );

  MasterDict["Vfakeleps"] = mergeEntriesList(
    MasterDict,
    {
      "Vfakeleps_2023BPix",
      "Vfakeleps_2023",
      "Vfakeleps_2022EE",
      "Vfakeleps_2022",
      "Vfakeleps_2018",
      "Vfakeleps_2017",
      "Vfakeleps_2016",
      "Vfakeleps_2016APV",
    }
  );

  MasterDict["boson"] = mergeEntriesList(
    MasterDict,
    {
      //"boson_2026",
      "boson_2025",
      "boson_2024",
      "boson_2023BPix",
      "boson_2023",
      "boson_2022EE",
      "boson_2022",
      "boson_2018",
      "boson_2017",
      "boson_2016",
      "boson_2016APV",
    }
  );

  MasterDict["diboson"] = mergeEntriesList(
    MasterDict,
    {
      //"DB_2026",
      "DB_2025",
      "DB_2024",
      "DB_2023BPix",
      "DB_2023",
      "DB_2022EE",
      "DB_2022",
      "DB_2018",
      "DB_2017",
      "DB_2016",
      "DB_2016APV",
    }
  );

  MasterDict["triboson"] = mergeEntriesList(
    MasterDict,
    {
      //"TB_2026",
      "TB_2025",
      "TB_2024",
      "TB_2023BPix",
      "TB_2023",
      "TB_2022EE",
      "TB_2022",
      "TB_2018",
      "TB_2017",
      "TB_2016",
      "TB_2016APV",
    }
  );

  MasterDict["bkg_2016APV"] = mergeEntriesList(
    MasterDict,
    {
      "top_2016APV",
      "boson_2016APV",
      "DB_2016APV",
      "TB_2016APV",
    }
  );

  MasterDict["bkg_2016"] = mergeEntriesList(
    MasterDict,
    {
      "top_2016",
      "boson_2016",
      "DB_2016",
      "TB_2016",
    }
  );

  MasterDict["bkg_2016Comb"] = mergeEntriesList(
    MasterDict,
    {
      "top_2016APV",
      "boson_2016APV",
      "DB_2016APV",
      "TB_2016APV",
      "top_2016",
      "boson_2016",
      "DB_2016",
      "TB_2016",
    }
  );

  MasterDict["bkg_2017"] = mergeEntriesList(
    MasterDict,
    {
      "top_2017",
      "boson_2017",
      "DB_2017",
      "TB_2017",
    }
  );

  MasterDict["bkg_2018"] = mergeEntriesList(
    MasterDict,
    {
      "top_2018",
      "boson_2018",
      "DB_2018",
      "TB_2018",
    }
  );

  MasterDict["bkg_2022"] = mergeEntriesList(
    MasterDict,
    {
      "top_2022",
      "boson_2022",
      "DB_2022",
      "TB_2022",
    }
  );

  MasterDict["bkg_2022EE"] = mergeEntriesList(
    MasterDict,
    {
      "top_2022EE",
      "boson_2022EE",
      "DB_2022EE",
      "TB_2022EE",
    }
  );

  MasterDict["bkg_2022Comb"] = mergeEntriesList(
    MasterDict,
    {
      "top_2022",
      "boson_2022",
      "DB_2022",
      "TB_2022",
      "top_2022EE",
      "boson_2022EE",
      "DB_2022EE",
      "TB_2022EE",
    }
  );

  MasterDict["bkg_2023"] = mergeEntriesList(
    MasterDict,
    {
      "top_2023",
      "boson_2023",
      "DB_2023",
      "TB_2023",
    }
  );

  MasterDict["bkg_2023BPix"] = mergeEntriesList(
    MasterDict,
    {
      "top_2023BPix",
      "boson_2023BPix",
      "DB_2023BPix",
      "TB_2023BPix",
    }
  );

  MasterDict["bkg_2023Comb"] = mergeEntriesList(
    MasterDict,
    {
      "bkg_2023",
      "bkg_2023BPix",
    }
  );

  MasterDict["bkg_2024"] = mergeEntriesList(
    MasterDict,
    {
      "top_2024",
      "boson_2024",
      "DB_2024",
      "TB_2024",
    }
  );

  MasterDict["bkg_2025"] = mergeEntriesList(
    MasterDict,
    {
      "top_2025",
      "boson_2025",
      "DB_2025",
      "TB_2025",
    }
  );

  MasterDict["bkg_2026"] = mergeEntriesList(
    MasterDict,
    {
      "top_2026",
      "boson_2026",
      "DB_2026",
      "TB_2026",
    }
  );

  MasterDict["bkg_Run2"] = mergeEntriesList(
    MasterDict,
    {
      "top_Run2",
      "boson_Run2",
      "diboson_Run2",
      "triboson_Run2",
    }
  );

  MasterDict["bkg_Run3"] = mergeEntriesList(
    MasterDict,
    {
      "top_Run3",
      "boson_Run3",
      "diboson_Run3",
      "triboson_Run3",
    }
  );

  MasterDict["bkg"] = mergeEntriesList(
    MasterDict,
    {
      "top_Run2",
      "boson_Run2",
      "diboson_Run2",
      "triboson_Run2",
      "top_Run3",
      "boson_Run3",
      "diboson_Run3",
      "triboson_Run3",
    }
  );

  MasterDict["data_obs"] = mergeEntriesList(
    MasterDict,
    {
      "Data_2016APV",
      "Data_2016",
      "Data_2017",
      "Data_2018",
      "Data_2022",
      "Data_2022EE",
      "Data_2023",
      "Data_2023BPix",
      "Data_2024",
    }
  );

  MasterDict["Data_DoubleElectron_2016Comb"] = mergeEntriesList(
    MasterDict,
    {
      "Data_DoubleElectron_2016APV",
      "Data_DoubleElectron_2016",
    }
  );

  MasterDict["Data_DoubleMuon_2016Comb"] = mergeEntriesList(
    MasterDict,
    {
      "Data_DoubleMuon_2016APV",
      "Data_DoubleMuon_2016",
    }
  );

  MasterDict["Data_SingleElectron_2016Comb"] = mergeEntriesList(
    MasterDict,
    {
      "Data_SingleElectron_2016APV",
      "Data_SingleElectron_2016",
    }
  );

  MasterDict["Data_SingleElectron_2022Comb"] = mergeEntriesList(
    MasterDict,
    {
      "Data_SingleElectron_2022",
      "Data_SingleElectron_2022EE",
    }
  );

  MasterDict["Data_SingleElectron_2023Comb"] = mergeEntriesList(
    MasterDict,
    {
      "Data_SingleElectron_2023",
      "Data_SingleElectron_2023BPix",
    }
  );

  MasterDict["Data_SingleMuon_2016Comb"] = mergeEntriesList(
    MasterDict,
    {
      "Data_SingleMuon_2016APV",
      "Data_SingleMuon_2016",
    }
  );

  MasterDict["Data_SingleMuon_2022Comb"] = mergeEntriesList(
    MasterDict,
    {
      "Data_SingleMuon_2022",
      "Data_SingleMuon_2022EE",
    }
  );

  MasterDict["Data_SingleMuon_2023Comb"] = mergeEntriesList(
    MasterDict,
    {
      "Data_SingleMuon_2023",
      "Data_SingleMuon_2023BPix",
    }
  );

  MasterDict["Data_MuonEG_2016Comb"] = mergeEntriesList(
    MasterDict,
    {
      "Data_MuonEG_2016APV",
      "Data_MuonEG_2016",
    }
  );

  MasterDict["Data_MuonEG_2022Comb"] = mergeEntriesList(
    MasterDict,
    {
      "Data_MuonEG_2022",
      "Data_MuonEG_2022EE",
    }
  );

  MasterDict["Data_MuonEG_2023Comb"] = mergeEntriesList(
    MasterDict,
    {
      "Data_MuonEG_2023",
      "Data_MuonEG_2023BPix",
    }
  );

  MasterDict["Data_SingleElectron"] = mergeEntriesList(
    MasterDict,
    {
      "Data_SingleElectron_2016APV",
      "Data_SingleElectron_2016",
      "Data_SingleElectron_2017",
      "Data_SingleElectron_2018",
      "Data_SingleElectron_2022",
      "Data_SingleElectron_2022EE",
      "Data_SingleElectron_2023",
      "Data_SingleElectron_2023BPix",
    }
  );

  MasterDict["Data_SingleMuon"] = mergeEntriesList(
    MasterDict,
    {
      "Data_SingleMuon_2016APV",
      "Data_SingleMuon_2016",
      "Data_SingleMuon_2017",
      "Data_SingleMuon_2018",
      "Data_SingleMuon_2022",
      "Data_SingleMuon_2022EE",
      "Data_SingleMuon_2023",
      "Data_SingleMuon_2023BPix",
    }
  );

  MasterDict["Cascades"] = mergeEntriesList(
    MasterDict,
    {
      "Cascades_180",
      "Cascades_270",
      "Cascades_260",
      "Cascades_220",
    }
  );

  MasterDict["SMS_TChiWZ"] = mergeEntriesList(
    MasterDict,
    {
      "SMS_TChiWZ_2016APV",
      "SMS_TChiWZ_2016",
      "SMS_TChiWZ_2017",
      "SMS_TChiWZ_2018",
      "SMS_TChiWZ_2022",
      "SMS_TChiWZ_2022EE",
      "SMS_TChiWZ_2023",
      "SMS_TChiWZ_2023BPix",
      "SMS_TChiWZ_2024",
      "SMS_TChiWZ_2025",
      //"SMS_TChiWZ_2026",
    }
  );
  MasterDict["SMS_TChiWZpreUL"] = mergeEntriesList(
    MasterDict,
    {
      "SMS_TChiWZpreUL_2016",
      "SMS_TChiWZpreUL_2017",
      "SMS_TChiWZpreUL_2018",
      "SMS_TChiWZpreUL_2022",
      "SMS_TChiWZpreUL_2022EE",
      "SMS_TChiWZpreUL_2023",
      "SMS_TChiWZpreUL_2023BPix",
      "SMS_TChiWZpreUL_2024",
      "SMS_TChiWZpreUL_2025",
      //"SMS_TChiWZpreUL_2026",
    }
  );
  MasterDict["SMS_TSlepSleppreUL"] = mergeEntriesList(
    MasterDict,
    {
      "SMS_TSlepSleppreUL_2016",
      "SMS_TSlepSleppreUL_2017",
      "SMS_TSlepSleppreUL_2018",
      "SMS_TSlepSleppreUL_2022",
      "SMS_TSlepSleppreUL_2022EE",
      "SMS_TSlepSleppreUL_2023",
      "SMS_TSlepSleppreUL_2023BPix",
      "SMS_TSlepSleppreUL_2024",
      "SMS_TSlepSleppreUL_2025",
      //"SMS_TSlepSleppreUL_2026",
    }
  );
  MasterDict["SMS_TChiWZTEST"] = mergeEntriesList(
    MasterDict,
    {
      "SMS_TChiWZTEST_2016APV",
      "SMS_TChiWZTEST_2016",
      "SMS_TChiWZTEST_2017",
      "SMS_TChiWZTEST_2018",
      "SMS_TChiWZTEST_2022",
      "SMS_TChiWZTEST_2022EE",
      "SMS_TChiWZTEST_2023",
      "SMS_TChiWZTEST_2023BPix",
      "SMS_TChiWZTEST_2024",
      "SMS_TChiWZTEST_2025",
      //"SMS_TChiWZTEST_2026",
    }
  );
  MasterDict["SMS_TSlepSlep_TEST"] = mergeEntriesList(
    MasterDict,
    {
      "SMS_TSlepSlep_TEST_2016APV",
      "SMS_TSlepSlep_TEST_2016",
      "SMS_TSlepSlep_TEST_2017",
      "SMS_TSlepSlep_TEST_2018",
      "SMS_TSlepSlep_TEST_2022",
      "SMS_TSlepSlep_TEST_2022EE",
      "SMS_TSlepSlep_TEST_2023",
      "SMS_TSlepSlep_TEST_2023BPix",
      "SMS_TSlepSlep_TEST_2024",
      "SMS_TSlepSlep_TEST_2025",
      //"SMS_TSlepSlep_TEST_2026",
    }
  );

}

void SampleTool::LoadBkgs( const stringlist& bkglist ){
    for( long unsigned int i=0; i<bkglist.size(); i++){
        //check if background exists
        std::string bkg = bkglist[i];
        std::size_t pos_FAKE = bkg.find("_FAKE");
        if(pos_FAKE != std::string::npos)
            bkg = bkg.substr(0, pos_FAKE);
        if( MasterDict.count(bkg) == 0 ){
            std::cout<<"Bkg: "<<bkg<<" not found ... skipping ...\n";
            continue;
        } 
        BkgDict[bkg] = MasterDict[bkg];        
    }
}

void SampleTool::LoadSigs( const stringlist& siglist ){
    for( long unsigned int i=0; i<siglist.size(); i++){
        if( MasterDict.count(siglist[i]) == 0 ){
            std::cout<<"Sig: "<<siglist[i]<<" not found ... skipping ...\n";
            continue;
        }
        SigDict[siglist[i]] = MasterDict[siglist[i]];
    }
    //build signal keys
    stringlist s_strings{};
    for(long unsigned int i=0; i<siglist.size(); i++){
        std::vector< std::string > keylist{};
        s_strings = SigDict[siglist[i]];
        for( long unsigned int j=0; j< s_strings.size(); j++){
            if(s_strings[j].find("X_Cascades") != std::string::npos) 
                SignalKeys.push_back( BFTool::GetSignalTokensCascades( s_strings[j] ) );
            else if(s_strings[j].find("X_SMS") != std::string::npos){
                stringlist sms_temp = BFTool::GetSignalTokensSMS( s_strings[j] );
                stringlist sms_filters_tmp;
                for (const auto& sms_entry : sms_temp){
                    SignalKeys.push_back( sms_entry );
                    sms_filters_tmp.push_back( sms_entry );
                }
                if (BFTool::filterSignalsSMS.empty())
                    BFTool::SetFilterSignalsSMS(sms_filters_tmp);
            }
        }
    }
}

void SampleTool::LoadData(const stringlist& datalist) {
    for (long unsigned int i = 0; i < datalist.size(); i++) {
        const std::string& name = datalist[i];

        // Check if dataset exists in the master list
        if (MasterDict.count(name) == 0) {
            std::cout << "Data: " << name << " not found ... skipping ...\n";
            continue;
        }

        DataDict[name] = MasterDict[name];
    }
}

void SampleTool::LoadAllData() {
    stringlist allData = {"Data_2016", "Data_2016APV", "Data_2017", "Data_2018", "Data_2022", "Data_2022EE", "Data_2023", "Data_2023BPix", "Data_2024"};
    LoadData(allData);
}

void SampleTool::LoadAllBkgs() {
    stringlist allBkgs = {"ttbar","ST","DY","ZInv","DB","TB","QCD","Wjets","top","boson","Vfakeleps","bkg"};
    LoadBkgs(allBkgs);
}

void SampleTool::LoadAllSigs() {
    stringlist allSigs = {"Cascades","SMS_Gluinos","SMS_TChiWZ"};
    LoadSigs(allSigs);
}

void SampleTool::LoadAllFromMaster() {
    for (const auto &kv : MasterDict) {
        const std::string &group = kv.first;
        const stringlist &files = kv.second;

        if (group == "Cascades" || group.find("SMS") != std::string::npos) {
            SigDict[group] = files;
        } else if (group.find("Data") != std::string::npos || group.find("data") != std::string::npos) {
            DataDict[group] = files;
        } else {
            BkgDict[group] = files;
        }
    }
}

void SampleTool::PrintDict( map<string,stringlist>& d ){
    for(auto it = d.cbegin(); it != d.cend(); ++it){
     std::cout << "key:"<< it->first << ":\n";
     stringlist str = it->second;
     for (std::vector<string>::iterator it2 = str.begin(); it2 != str.end(); ++it2) {
  std::cout << *it2 << " \n";
     }
     
    }
    std::cout<<"\n";
    
}

void SampleTool::PrintKeys( stringlist sl ){
    for( long unsigned int i = 0; i<sl.size(); i++)
        std::cout<<sl[i]<<"\n";
}

template <typename... Keys>
stringlist SampleTool::mergeEntriesSafe(
    const std::map<std::string, stringlist>& dict,
    const Keys&... keys) const
{
    stringlist result;
    auto appendIfExists = [&](const std::string& key) {
        auto it = dict.find(key);
        if (it != dict.end()) {
            result.insert(result.end(), it->second.begin(), it->second.end());
        }
    };
    (appendIfExists(keys), ...);
    return result;
}

stringlist SampleTool::mergeEntriesList(
    const std::map<std::string, stringlist>& dict,
    const stringlist& keys) const
{
    stringlist result;
    for (const auto& key : keys) {
        auto it = dict.find(key);
        if (it != dict.end()) {
            result.insert(result.end(), it->second.begin(), it->second.end());
        }
    }
    return result;
}

// Load preferred groups from YAML file (reads processes.bkg and processes.sig in order)
stringlist SampleTool::loadPreferredGroupsFromYaml(const std::string &yamlPath) {
    stringlist out;
    try {
        YAML::Node root = YAML::LoadFile(yamlPath);
        if (!root) return out;

        if (root["processes"]) {
            YAML::Node procs = root["processes"];
            if (procs["bkg"] && procs["bkg"].IsSequence()) {
                for (const auto &n : procs["bkg"]) {
                    if (n.IsScalar()) {
                        std::string name = n.as<std::string>();
                        std::size_t pos_FAKE = name.find("_FAKE");
                        if (pos_FAKE != std::string::npos)
                            name = name.substr(0, pos_FAKE);
                        out.push_back(name);
                    }
                }
            }
            if (procs["sig"] && procs["sig"].IsSequence()) {
                for (const auto &n : procs["sig"]) {
                    if (n.IsScalar()) out.push_back(n.as<std::string>());
                }
            }
            if (procs["data"] && procs["data"].IsSequence()) {
                for (const auto &n : procs["data"]) {
                    if (n.IsScalar()) out.push_back(n.as<std::string>());
                }
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "[loadPreferredGroupsFromYaml] Failed to read YAML '" << yamlPath << "': " << e.what() << "\n";
    }
    return out;
}

// Helper: get basename of a path
static std::string basename_of(const std::string &p) {
    size_t pos = p.find_last_of("/\\");
    if (pos == std::string::npos) return p;
    return p.substr(pos+1);
}

static double ReadXSecFromFile(const std::string &path) {
    double nan = std::numeric_limits<double>::quiet_NaN();
    TFile *f = TFile::Open(path.c_str(), "READ");
    if (!f || f->IsZombie()) {
        std::cerr << "[ReadXSec] ERROR opening file: " << path << "\n";
        if (f) { f->Close(); delete f; }
        return nan;
    }

    // KUAnalysis must be present
    TTree *t = dynamic_cast<TTree*>(f->Get("KUAnalysis"));
    if (!t) {
        std::cerr << "[ReadXSec] No KUAnalysis tree in " << path << "\n";
        f->Close();
        delete f;
        return nan;
    }

    // Look for branch named "XSec"
    if (!t->GetBranch("XSec")) {
        std::cerr << "[ReadXSec] No XSec branch in KUAnalysis in " << path << "\n";
        f->Close();
        delete f;
        return nan;
    }

    double xsec = 0.0;
    t->SetBranchAddress("XSec", &xsec);
    // read first entry
    if (t->GetEntries() > 0) {
        t->GetEntry(0);
        f->Close();
        delete f;
        return xsec;
    } else {
        std::cerr << "[ReadXSec] KUAnalysis has no entries in " << path << "\n";
    }

    f->Close();
    delete f;
    return nan;
}

void SampleTool::WriteLatexTablesForGroups(const std::vector<std::string>& groups, const std::string& outdir) const {
    // Ensure output directory exists (best-effort)
    gSystem->Exec(("mkdir -p " + outdir).c_str());

    // Regex (kept for potential future use; not used for membership inference here)
    std::regex r_proc_year(R"(^(.+)_((20\d{2}.*))$)");

    // Iterate only over requested groups that are actually loaded in BkgDict
    for (const auto &group : groups) {
        if (BkgDict.count(group) == 0) {
            std::cerr << "[WriteLatexTablesForGroups] Group '" << group << "' not found in BkgDict. Did you call LoadBkgs()? Skipping.\n";
            continue;
        }

        // Discover year-specific group keys from MasterDict that start with "group_"
        std::string prefix = group + "_";
        std::vector<std::string> groupYearKeys;
        for (const auto &kv : MasterDict) {
            const std::string &masterKey = kv.first;
            if (masterKey.rfind(prefix, 0) == 0) {
                groupYearKeys.push_back(masterKey);
            }
        }

        // If no year-specific keys were found, fall back to the merged group key itself (if present).
        if (groupYearKeys.empty()) {
            if (MasterDict.count(group)) {
                groupYearKeys.push_back(group); // produce a single merged/allYears table
            } else {
                std::cerr << "[WriteLatexTablesForGroups] No year-specific keys and no merged key found for group '" << group << "'. Skipping.\n";
                continue;
            }
        }

        // For each discovered groupKey (e.g. "boson_2016", or "boson" if fallback), produce a table
        for (const auto &groupKey : groupYearKeys) {
            // yearSuffix is used in filenames/labels; if groupKey==group (merged), label as "allYears"
            std::string yearSuffix = (groupKey == group) ? std::string("allYears") : groupKey.substr(prefix.size());
            std::string outname = outdir + "/samples_" + group + "_" + yearSuffix + ".tex";

            std::ofstream out(outname);
            if (!out.is_open()) {
                std::cerr << "[WriteLatexTablesForGroups] failed to open " << outname << "\n";
                continue;
            }

            // LaTeX header
            out << "\\begin{table}\n"
                << "    \\centering\n"
                << "    \\addtolength{\\leftskip}{-2cm}\n"
                << "    \\addtolength{\\rightskip}{-2cm}\n"
                << "    \\begin{tabular}{c|l|l} {\\textbf{process}}  & {\\textbf{" << group << " " << yearSuffix << "}} &  {\\textbf{$\\sigma$}} [pb$^{-1}$] \\\\ \\hline\n";

            // The authoritative file list for this groupKey (this defines membership)
            const stringlist &groupFiles = MasterDict.at(groupKey);

            // Stage 1: collect candidate procKeys that are strict subsets of groupFiles,
            // excluding groupKey itself (may add it later if no candidates found).
            std::vector<std::string> prockeys;
            for (const auto &kv2 : MasterDict) {
                const std::string &procKey = kv2.first;

                // Skip the exact groupKey in this first pass to avoid listing the merged group itself.
                if (procKey == groupKey) continue;

                // Check subset: all files of procKey must be present in groupFiles
                bool isSubset = true;
                for (const auto &f : kv2.second) {
                    if (std::find(groupFiles.begin(), groupFiles.end(), f) == groupFiles.end()) {
                        isSubset = false;
                        break;
                    }
                }
                if (isSubset) prockeys.push_back(procKey);
            }

            // Stage 2: if no sub-processes found, fall back to the groupKey itself
            if (prockeys.empty()) {
                auto itg = MasterDict.find(groupKey);
                if (itg != MasterDict.end() && !itg->second.empty()) {
                    prockeys.push_back(groupKey);
                } else {
                    // Nothing found write a comment and continue.
                    out << "    % NOTE: no matching subprocess keys found for " << groupKey << "\n";
                }
            }

            // Sort prockeys by base name (strip trailing _suffix if present)
            std::sort(prockeys.begin(), prockeys.end(), [](const std::string &a, const std::string &b){
                auto ap = a.find_last_of('_');
                auto bp = b.find_last_of('_');
                std::string abase = (ap==std::string::npos)? a : a.substr(0, ap);
                std::string bbase = (bp==std::string::npos)? b : b.substr(0, bp);
                return abase < bbase;
            });

            // For each proc, emit rows for its files
            for (const auto &procKey : prockeys) {
                // get base proc name for the "process" column (strip final suffix)
                size_t pos = procKey.rfind('_');
                std::string procName = (pos == std::string::npos) ? procKey : procKey.substr(0, pos);

                // retrieve file list from MasterDict (exists because procKey came from MasterDict)
                const stringlist &files = MasterDict.at(procKey);
                bool firstRowForProc = true;

                for (const auto &fpath : files) {
                    // Read KUAnalysis -> XSec
                    double xsec = ReadXSecFromFile(fpath);
                    if (!std::isnan(xsec)) xsec = xsec / 1000.0; // convert fb^{-1} to pb^{-1}

                    // format sigma: print "N/A" if nan
                    std::ostringstream sigoss;
                    if (std::isnan(xsec)) sigoss << "N/A";
                    else sigoss << std::fixed << std::setprecision(3) << xsec;

                    // file basename for the second column
                    std::string fname = basename_of(fpath);
                    
                    // strip everything from "_Summer" onward
                    size_t summerPos = fname.find("_Summer");
                    if (summerPos != std::string::npos) {
                        fname = fname.substr(0, summerPos);
                    }
                    
                    // make LaTeX-safe
                    fname = std::regex_replace(fname, std::regex("_"), "\\_");

                    if (firstRowForProc) {
                        out << "        " << procName << "&" << fname << " & " << sigoss.str() << " \\\\\n";
                        firstRowForProc = false;
                    } else {
                        out << "        &" << fname << " & " << sigoss.str() << " \\\\\n";
                    }
                } // files
                if(files.size() > 0) out << "        \\hline\n"; // separate processes
            } // prockeys

            out << "    \\end{tabular}\\\\\n"
                << "    \\caption{"
                << yearSuffix << " " << group
                << " MC samples. "
                //<< "simulated using the \\texttt{106X\\_mcRun2\\_asymptotic\\_v17-v2} global tag. "
                << "Cross sections are given for each sample; the process column denotes the type of " << group << " process."
                << "}\n"
                << "    \\label{tab:samples_" << group << "_" << yearSuffix << "}\n"
                << "\\end{table}\n";

            out.close();
            std::cout << "[WriteLatexTablesForGroups] wrote " << outname << "\n";
        } // each discovered groupKey
    } // each requested base group
}
