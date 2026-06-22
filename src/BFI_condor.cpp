// src/BFI_condor.cpp
#include <getopt.h>
#include "TFile.h"
#include "TROOT.h"

#include "BFICondorTools.h"

// ----------------------
// Helpers
// ----------------------

static void usage(const char* me) {
    std::cerr << "Usage: " << me
              << " --bin BINNAME[;BINNAME2;...] --file ROOTFILE [--json-output OUT.json] "
                 "[--root-output OUT.root] [--cuts CUT1;CUT2;...] [--lep-cuts LEPCUT1;LEPCUT2;...] "
                 "[--predefined-cuts NAME1;NAME2;...] [--user-cuts NAME1;NAME2;...] [--hist] [--hist-yaml HISTS.yaml] [--json] [--proc-yaml PROCS.yaml]\n\n";
    std::cerr << "Required arguments:\n";
    std::cerr << "  --bin           Name (or semicolon-separated list) of the bin(s) to process (e.g. TEST or BIN1;BIN2)\n";
    std::cerr << "  --file          Path to one ROOT file to process\n\n";
    std::cerr << "Optional arguments:\n";
    std::cerr << "  --json-output      Path to write partial JSON output (use {bin} to place bin name)\n";
    std::cerr << "  --root-output      Path to write ROOT/histogram output\n";
    std::cerr << "  --cuts             Semicolon-separated list of normal tree cuts "
                 "(e.g. MET>=150;PTISR>=250)\n";
    std::cerr << "  --lep-cuts         Semicolon-separated list of lepton cuts for BuildLeptonCut\n";
    std::cerr << "  --predefined-cuts  Semicolon-separated list of predefined cuts\n";
    std::cerr << "  --user-cuts        Semicolon-separated list of user cuts\n";
    std::cerr << "  --proc-name        Set process name\n";
    std::cerr << "  --hist             Fill histograms\n";
    std::cerr << "  --hist-yaml        YAML file defining histogram expressions\n";
    std::cerr << "  --proc-yaml        YAML file containg processes inputted for ST lookup\n";
    std::cerr << "  --json             Write JSON yields\n";
    std::cerr << "  --cutflow          Make cut flow plot\n";
    std::cerr << "  --signal           Mark this process as signal\n";
    std::cerr << "  --sig-type TYPE    Specify signal type (sets --signal automatically)\n";
    std::cerr << "  --lumi VALUE       Integrated luminosity to scale yields\n";
    std::cerr << "  --sample-name NAME Optional name of the sample\n";
    std::cerr << "  --sms-filters LIST Comma-separated list of SMS filters\n";
    std::cerr << "  --systematics      Semicolon- or comma-separated list of systematic names\n";
    std::cerr << "  --help             Display this help message\n";
}

// ----------------------
// Main
// ----------------------
int main(int argc, char** argv) {
    RegisterSafeHelpers();
    std::string binArg, cutsStr, lepCutsStr, predefCutsStr, userCutsStr, rootFilePath, outputJsonPathBase, processName, sampleName, histOutputPath, cutsMultiStr, lepCutsMultiStr, predefCutsMultiStr, userCutsMultiStr, binsCfgName;
    std::vector<std::string> smsFilters;
    std::vector<std::string> systematicNames;
    bool isSignal=false, doHist=false, doJSON=false, doCutFlow=false;
    std::string sigType, histYamlPath, procYamlPath;
    double Lumi=1.0;

    static struct option long_options[] = {
        {"bin", required_argument, 0, 'b'},
        {"file", required_argument, 0, 'f'},
        {"json-output", required_argument, 0, 'o'},
        {"cuts", required_argument, 0, 'c'},
        {"lep-cuts", required_argument, 0, 'l'},
        {"predefined-cuts", required_argument, 0, 'p'},
        {"user-cuts", required_argument, 0, 'u'},
        {"signal", no_argument, 0, 's'},
        {"sig-type", required_argument, 0, 't'},
        {"lumi", required_argument, 0, 'S'},
        {"sample-name", required_argument, 0, 'n'},
        {"sms-filters", required_argument, 0, 'm'},
        {"proc-name", required_argument, 0, 'N'},
        {"hist", no_argument, 0, 'H'},
        {"hist-yaml", required_argument, 0, 'y'},
        {"proc-yaml", required_argument, 0, 'd'},
        {"json", no_argument, 0, 'J'},
        {"cutflow", no_argument, 0, 'F'},
        {"root-output", required_argument, 0, 'O'},
        {"cuts-multi", required_argument, 0, 'M'},
        {"lep-cuts-multi", required_argument, 0, 'L'},
        {"predefined-cuts-multi", required_argument, 0, 'P'},
        {"user-cuts-multi", required_argument, 0, 'U'},
        {"bins-cfg", required_argument, 0, 'B'},
        {"systematics", required_argument, 0, 'R'},
        {"help", no_argument, 0, 'h'},
        {0,0,0,0}
    };

    int opt, opt_index=0;
    while ((opt = getopt_long(argc, argv, "b:f:o:c:l:p:st:S:u:n:m:N:Hy:d:JFO:M:L:P:U:B:R:h", long_options, &opt_index)) != -1) {
        switch(opt){
            case 'b': binArg=optarg; break;
            case 'f': rootFilePath=optarg; break;
            case 'o': outputJsonPathBase=optarg; break;
            case 'c': cutsStr=optarg; break;
            case 'l': lepCutsStr=optarg; break;
            case 'p': predefCutsStr=optarg; break;
            case 'u': userCutsStr=optarg; break;
            case 's': isSignal=true; break;
            case 't': sigType=optarg; isSignal=true; break;
            case 'S': Lumi=atof(optarg); break;
            case 'n': sampleName=optarg; break;
            case 'm': smsFilters=BFTool::SplitString(optarg,","); break;
            case 'N': processName = optarg; break;
            case 'H': doHist=true; break;
            case 'y': histYamlPath=optarg; break;
            case 'd': procYamlPath=optarg; break;
            case 'J': doJSON=true; break;
            case 'F': doCutFlow=true; break;
            case 'O': histOutputPath = optarg; break;
            case 'M': cutsMultiStr = optarg; break;
            case 'L': lepCutsMultiStr = optarg; break;
            case 'P': predefCutsMultiStr = optarg; break;
            case 'U': userCutsMultiStr = optarg; break;
            case 'B': binsCfgName = optarg; break;
            case 'R': systematicNames = splitTopLevel(optarg); break;
            case 'h':
            default: usage(argv[0]); return 1;
        }
    }

    ST.LoadAllFromMaster();
    bool IsData = SampleIsData(rootFilePath);
    int year = 0;
    if (rootFilePath.find("Summer16_102X") != std::string::npos) year = 2016;
    else if (rootFilePath.find("Fall17_102X") != std::string::npos) year = 2017;
    else if (rootFilePath.find("Autumn18_102X") != std::string::npos) year = 2018;
    else if (rootFilePath.find("Summer20UL16_106X") != std::string::npos) year = 2016;
    else if (rootFilePath.find("Summer20UL16APV_106X") != std::string::npos) year = 2016;
    else if (rootFilePath.find("Summer20UL17_106X") != std::string::npos) year = 2017;
    else if (rootFilePath.find("Summer20UL18_106X") != std::string::npos) year = 2018;
    else if (rootFilePath.find("Summer22_130X") != std::string::npos) year = 2022;
    else if (rootFilePath.find("Summer22EE_130X") != std::string::npos) year = 2022;
    else if (rootFilePath.find("Summer23_130X") != std::string::npos) year = 2023;
    else if (rootFilePath.find("Summer23BPix_130X") != std::string::npos) year = 2023;
    else if (rootFilePath.find("Summer24_130X") != std::string::npos) year = 2024;
    else if (rootFilePath.find("Summer25_130X") != std::string::npos) year = 2025;
    else if (rootFilePath.find("Summer26_130X") != std::string::npos) year = 2026;
    if (Lumi <= 0.) Lumi = GetLumiFromKey(rootFilePath); // get lumi from ST for file
    if (sampleName.empty()) sampleName = GetSampleNameFromKey(rootFilePath);
    if (binArg.empty() || rootFilePath.empty() || (!doHist && !doJSON)) { usage(argv[0]); return 1; }
    std::cout << "Using Lumi: " << Lumi << std::endl;

    // split semicolon-separated bin list
    std::vector<std::string> binNames = splitTopLevel(binArg);
    if (binNames.empty()) { std::cerr << "[BFI_condor] No bins parsed from --bin\n"; return 1; }

    BuildFitInput* BFI=nullptr;
    try{BFI=new BuildFitInput();}catch(...){std::cerr<<"[BFI_condor] Failed to construct BuildFitInput\n";return 3;}

    // parse common cut lists (these are strings from CLI)
    std::vector<std::string> cutsVec=splitTopLevel(cutsStr);
    std::vector<std::string> lepCutsVec=splitTopLevel(lepCutsStr);
    std::vector<std::string> predefCutsVec=splitTopLevel(predefCutsStr);
    std::vector<std::string> userCutsVec=splitTopLevel(userCutsStr);

    // Build map bin -> finalCutsExpanded (vector<string>), honoring cuts-multi if provided
    std::map<std::string, std::vector<std::string>> finalCutsExpandedMap;
    // Also keep per-bin user-cuts (names referring to allUserCuts map) to expand them later
    std::map<std::string, std::vector<std::string>> userCutsPerBin;
    
    auto split_on_delim = [](const std::string &s, const std::string &delim) {
        std::vector<std::string> out;
        size_t pos = 0;
        while (pos < s.size()) {
            size_t found = s.find(delim, pos);
            if (found == std::string::npos) {
                out.push_back(s.substr(pos));
                break;
            } else {
                out.push_back(s.substr(pos, found - pos));
                pos = found + delim.size();
            }
        }
        return out;
    };
    
    // Helper to split semicolon-delimited (reuse existing splitTopLevel semantics)
    auto splitSemicolon = [&](const std::string &s) {
        return splitTopLevel(s);
    };
    
    if (!cutsMultiStr.empty() || !lepCutsMultiStr.empty() || !predefCutsMultiStr.empty() || !userCutsMultiStr.empty()) {
        // split each multi-string on |||
        std::vector<std::string> cutsParts = split_on_delim(cutsMultiStr, "|||");
        std::vector<std::string> lepCutsParts = split_on_delim(lepCutsMultiStr, "|||");
        std::vector<std::string> predefParts = split_on_delim(predefCutsMultiStr, "|||");
        std::vector<std::string> userParts = split_on_delim(userCutsMultiStr, "|||");
    
        for (size_t i = 0; i < binNames.size(); ++i) {
            const std::string bin = binNames[i];
    
            // pick the corresponding part or empty string
            std::string cutsForBin = (i < cutsParts.size()) ? cutsParts[i] : "";
            std::string lepForBin = (i < lepCutsParts.size()) ? lepCutsParts[i] : "";
            std::string predefForBin = (i < predefParts.size()) ? predefParts[i] : "";
            std::string userForBin = (i < userParts.size()) ? userParts[i] : "";
    
            // split into vectors that match buildCutsForBin signature
            std::vector<std::string> normalCuts = splitSemicolon(cutsForBin);
            std::vector<std::string> lepCuts = splitSemicolon(lepForBin);
            std::vector<std::string> predefCuts = splitSemicolon(predefForBin);
    
            // call helper which will call BFI->BuildLeptonCut(...) for each lepCut
            std::vector<std::string> outCuts;
            if (!buildCutsForBin(BFI, normalCuts, lepCuts, predefCuts, outCuts)) {
                std::cerr << "[BFI_condor] Failed to build final cuts for bin (multi): " << bin << "\n";
                delete BFI;
                return 2;
            }
    
            // Expand macros on the resulting cuts and store per-bin
            std::vector<std::string> expanded;
            expanded.reserve(outCuts.size());
            for (const auto &c : outCuts) expanded.push_back(BFI->ExpandMacros(c));
            finalCutsExpandedMap[bin] = std::move(expanded);

            // user-cuts: store per-bin list of user-cut *names* (they will be looked up in allUserCuts later)
            userCutsPerBin[bin] = splitSemicolon(userForBin);
        }
    } else {
        // Fallback: original logic (compute finalCutsExpandedMap by calling buildCutsForBin)
        for (const auto &b : binNames) {
            // global vectors built earlier (cutsVec, lepCutsVec, predefCutsVec) reflect CLI single-valued inputs
            std::vector<std::string> finalCuts;
            if(!buildCutsForBin(BFI, cutsVec, lepCutsVec, predefCutsVec, finalCuts)){
                std::cerr<<"[BFI_condor] Failed to build final cuts for bin: "<<b<<"\n"; delete BFI; return 2;
            }
            std::vector<std::string> finalCutsExpanded;
            for(const auto &c : finalCuts) finalCutsExpanded.push_back(BFI->ExpandMacros(c));
            finalCutsExpandedMap[b] = std::move(finalCutsExpanded);
    
            // preserve original global userCutsVec for each bin
            userCutsPerBin[b] = userCutsVec;
        }
    }

    std::unique_ptr<TFile> histFile;
    if(doHist && !histOutputPath.empty()){
        histFile.reset(TFile::Open(histOutputPath.c_str(),"RECREATE"));
        if(!histFile || histFile->IsZombie()){
            std::cerr << "[BFI_condor] ERROR opening hist output file: " << histOutputPath << "\n";
            delete BFI;
            return 6;
        }
    }
    if(!smsFilters.empty()) BFTool::filterSignalsSMS=smsFilters;

    if(isSignal && sigType.empty())
        sigType=(rootFilePath.find("SMS")!=std::string::npos)?"sms":"cascades";

    std::map<std::string, std::map<std::string, std::map<std::string, FileYields>>> fileResultsByBin;
    std::map<std::string, std::map<std::string, ProcTotals>> totalsByBin;

    auto preferredGroups = ST.loadPreferredGroupsFromYaml(procYamlPath);
    if (processName.empty()) {
        if (isSignal && sigType == "cascades")
            processName = BFTool::GetSignalTokensCascades(rootFilePath);
        else if (isSignal && sigType == "sms")
            processName = GetProcessNameFromKey(rootFilePath, preferredGroups) + "_" + BFTool::GetFilterSignalsSMS()[0];
        else
            processName = GetProcessNameFromKey(rootFilePath, preferredGroups);
    }
    bool doFAKES = isProcFAKES(procYamlPath, processName);

    // --- Build active systematics config ---
    SystematicsConfig activeSystematics;
    // always default SF systematics for now (can later allow CLI override)
    activeSystematics.sf = kDefaultSFSystematics;

    // if user passed -R, interpret these as tree-based syst names (override)
    if (!systematicNames.empty()) {
        // treat CLI list as tree systs (user override)
        activeSystematics.tree = systematicNames;
    } else {
        // otherwise use built-in defaults
        activeSystematics.tree = kDefaultTreeSystematics;
    }

    // Control whether to run internal (SF / weight-based) systematic block on the nominal pass.
    bool runInternalSystsOnNominal = true;

    // processTree compute per-bin results
    auto processTree=[&](const std::string &tree_name,
                         const std::string &key,
                         const SystematicsConfig &sysCfg,
                         bool runInternalSysts, // SF systs
                         const std::string &treeSystTag,
                         bool treeSystIsUp,
                         bool runFAKES // whether or not to use genmatching for adding FAKES procs
                        ) {
        // precompute fake-key variants
        std::string key_HFelec = key + "_FAKES_HFElec";
        std::string key_HFmuon = key + "_FAKES_HFMuon";
        std::string processName_HFelec = processName + "_FAKES_HFElec";
        std::string processName_HFmuon = processName + "_FAKES_HFMuon";
        std::string key_LFelec = key + "_FAKES_LFElec";
        std::string key_LFmuon = key + "_FAKES_LFMuon";
        std::string processName_LFelec = processName + "_FAKES_LFElec";
        std::string processName_LFmuon = processName + "_FAKES_LFMuon";
        std::map<std::string,std::string> map_key_to_process;
        map_key_to_process.insert({key, processName});
        map_key_to_process.insert({key_HFelec, processName_HFelec});
        map_key_to_process.insert({key_HFmuon, processName_HFmuon});
        map_key_to_process.insert({key_LFelec, processName_LFelec});
        map_key_to_process.insert({key_LFmuon, processName_LFmuon});

        auto fillZeroJSONForBinProc = [&](const std::string &bin,
                                          const std::string &proc_key,
                                          const std::string &filePath,
                                          bool isTreeSyst,
                                          const std::string &treeSystTag,
                                          bool treeSystIsUp)
        {
            FileYields fy;
            // nominal always zero
            fy.nominal = {0.0, 0.0, 0.0};
            // internal SF systematics
            if (!IsData && !isSignal && runInternalSystsOnNominal) {
                for (const auto &s : activeSystematics.sf) {
                    fy.systs[s.tag].up   = {0.0, 0.0, 0.0};
                    fy.systs[s.tag].down = {0.0, 0.0, 0.0};
                }
            }
            auto &entry = fileResultsByBin[bin][proc_key][filePath];
            // tree systematic pass: only touch the variation
            if (isTreeSyst && !treeSystTag.empty()) {
                if (treeSystIsUp)
                    entry.systs[treeSystTag].up = {0.0, 0.0, 0.0};
                else
                    entry.systs[treeSystTag].down = {0.0, 0.0, 0.0};
            } else {
                entry = fy;
            }
            // totals set to zero
            auto &tot = totalsByBin[bin][proc_key];
            tot.nominal = {0.0, 0.0, 0.0};
            if (!IsData && !isSignal && runInternalSystsOnNominal) {
                for (const auto &s : activeSystematics.sf) {
                    tot.systs[s.tag].up   = {0.0, 0.0, 0.0};
                    tot.systs[s.tag].down = {0.0, 0.0, 0.0};
                }
            }
        };

        if (!TreeExistsInFile(rootFilePath, tree_name)) {
            std::cout << "[BFI_condor] Tree '" << tree_name
                      << "' not found in file '" << rootFilePath
                      << "'. Treating as empty.\n";

            if (doJSON) {
                for (const auto &bin : binNames) {

                    std::vector<std::string> proc_keys;

                    if (!runFAKES) {
                        proc_keys.push_back(key);
                    } else {
                        proc_keys = {
                            key_HFelec,
                            key_HFmuon,
                            key_LFelec,
                            key_LFmuon,
                            key
                        };
                    }

                    for (const auto &proc_key : proc_keys) {
                        fillZeroJSONForBinProc(
                            bin,
                            proc_key,
                            rootFilePath,
                            !treeSystTag.empty(),
                            treeSystTag,
                            treeSystIsUp
                        );
                    }
                }
            }

            return;
        }
    
        if(!ROOT::IsImplicitMTEnabled()) ROOT::EnableImplicitMT();

        if(doHist) histFile->cd();
    
        // --- Define any other derived variables from YAML (these are independent of bin) ---
        std::vector<DerivedVar> derivedVars;
        if(doHist && !histYamlPath.empty()){
            derivedVars = loadDerivedVariablesYAML(histYamlPath);
        }
        std::vector<DerivedVar> validatedDerivedVars;

        BaseNodeHandle valHandle =
            MakeBaseNode(tree_name, rootFilePath, BFI, Lumi, IsData, year, derivedVars);
        ROOT::RDF::RNode base_node_val = valHandle.node;
        BaseNodeHandle fillHandle =
            MakeBaseNode(tree_name, rootFilePath, BFI, Lumi, IsData, year, validatedDerivedVars);
        ROOT::RDF::RNode base_node_fill = fillHandle.node;
    
        auto test_cnt = base_node_val.Count();
        unsigned long long nEntries = test_cnt.GetValue();
        std::map<std::string, CutDef> allUserCuts;
        // --- Load all user cuts ---
        base_node_val = BuildFitInput::loadCutsUser(base_node_val, allUserCuts, true);
        base_node_fill = BuildFitInput::loadCutsUser(base_node_fill, allUserCuts, true);
        
        // skip validation if there are no events to validate on
        if(nEntries > 0) {
            // --- Validate derived variables ---
            for (const auto &dv : derivedVars) {
                if(ValidateDerivedVarNode(base_node_val, dv))
                    validatedDerivedVars.push_back(dv);
            }
        }

        // Prepare histogram definitions
        auto userHists = loadHistogramsUser();
        base_node_val = loadHistogramsUserCols(base_node_val);
        base_node_fill = loadHistogramsUserCols(base_node_fill);
        auto histDefs = loadHistogramsYAML(histYamlPath, BFI);
        histDefs.insert(histDefs.end(), userHists.begin(), userHists.end());
        size_t N = histDefs.size();
    
        for (const auto &bin : binNames) {
            bool binEraMismatch = false;
            // Skip Run2 bins if rootFilePath doesn't contain 106X or 102X
            if (bin.find("Run2") != std::string::npos &&
                rootFilePath.find("106X") == std::string::npos &&
                rootFilePath.find("102X") == std::string::npos) {
                binEraMismatch = true;
            }

            // Skip Run3 bins if rootFilePath doesn't contain 130X
            if (bin.find("Run3") != std::string::npos &&
                rootFilePath.find("130X") == std::string::npos) {
                binEraMismatch = true;
            }
            
            // Skip running if tree has zero events or bin era mismatch
            if (binEraMismatch || nEntries == 0) {
                if(doJSON) {
                    // Build the same proc_key list
                    std::vector<std::string> proc_keys;
                    if (!runFAKES) {
                        proc_keys.push_back(key);
                    } else {
                        proc_keys = {
                            key_HFelec, key_HFmuon,
                            key_LFelec, key_LFmuon,
                            key
                        };
                    }
            
                    for (const auto &proc_key : proc_keys) {
                        fillZeroJSONForBinProc(
                            bin,
                            proc_key,
                            rootFilePath,
                            !treeSystTag.empty(),
                            treeSystTag,
                            treeSystIsUp
                        );
                    }
                }
                // skip JSON and HIST filling
                continue;
            }

            ROOT::RDF::RNode node = base_node_val;
            // Retrieve the already-expanded bin-specific cuts
            const auto &finalCutsExpanded = finalCutsExpandedMap.at(bin);
    
            // Build validUserCuts for this bin (use per-bin userCutsPerBin mapping)
            std::vector<DerivedVar> validUserCuts;
            auto itUserList = userCutsPerBin.find(bin);
            const std::vector<std::string> &userListForThisBin = (itUserList != userCutsPerBin.end()) ? itUserList->second : userCutsVec;
    
            for (const auto &cutName : userListForThisBin) {
                if (cutName.empty()) continue;
                auto it = allUserCuts.find(cutName);
                if (it == allUserCuts.end()) {
                    std::cerr << "[BFI_condor] Requested cut not found: " << cutName << " (bin " << bin << ")\n";
                    continue;
                }
                const auto &cut = it->second;
                std::string expanded = BFI->ExpandMacros(cut.expression);
                if (!expanded.empty())
                    validUserCuts.push_back({cutName, expanded});
            }
    
            // --- Histograms / CutFlow per-bin ---
            if(doHist){
                // --- Build ordered cuts list (bin-specific final cuts + user cuts)
                // Ensure every cutsOrdered entry has a corresponding human-readable label
                std::vector<std::string> cutsOrdered;
                std::vector<std::string> cutLabels;
    
                // helper to create a readable, length-limited label from an expression
                auto make_readable_label = [&](const std::string &expr) -> std::string {
                    std::string s = expr;
                    auto l = s.find_first_not_of(" \t\n\r");
                    auto r = s.find_last_not_of(" \t\n\r");
                    if (l == std::string::npos) s = "";
                    else s = s.substr(l, r - l + 1);
    
                    if (s.size() > 2 && s.front() == '(' && s.back() == ')') {
                        s = s.substr(1, s.size() - 2);
                        l = s.find_first_not_of(" \t\n\r");
                        r = s.find_last_not_of(" \t\n\r");
                        if (l == std::string::npos) s = "";
                        else s = s.substr(l, r - l + 1);
                    }
    
                    std::string out;
                    bool lastSpace = false;
                    for (char ch : s) {
                        if (isspace((unsigned char)ch)) {
                            if (!lastSpace) { out.push_back(' '); lastSpace = true; }
                        } else {
                            out.push_back(ch);
                            lastSpace = false;
                        }
                    }
                    s = out;
    
                    const size_t maxLen = 52;
                    if (s.size() > maxLen) {
                        std::string head = s.substr(0, 28);
                        std::string tail = s.substr(s.size() - 16);
                        s = head + "..." + tail;
                    }
    
                    if (s.empty()) s = std::string("Cut_") + std::to_string(cutLabels.size() + 1);
                    return s;
                };
    
                // --- Determine which CLI cut lists correspond to this bin ---
                size_t binIndex = std::distance(binNames.begin(), std::find(binNames.begin(), binNames.end(), bin));
                std::vector<std::string> cutsCLI = cutsVec;
                std::vector<std::string> lepCLI = lepCutsVec;
                std::vector<std::string> predefCLI = predefCutsVec;
    
                if (!cutsMultiStr.empty() || !lepCutsMultiStr.empty() || !predefCutsMultiStr.empty()) {
                    std::vector<std::string> cutsParts = split_on_delim(cutsMultiStr, "|||");
                    std::vector<std::string> lepParts = split_on_delim(lepCutsMultiStr, "|||");
                    std::vector<std::string> predefParts = split_on_delim(predefCutsMultiStr, "|||");
    
                    if (binIndex < cutsParts.size()) cutsCLI = splitTopLevel(cutsParts[binIndex]);
                    if (binIndex < lepParts.size())  lepCLI  = splitTopLevel(lepParts[binIndex]);
                    if (binIndex < predefParts.size()) predefCLI = splitTopLevel(predefParts[binIndex]);
                }
    
                // Add finalCutsExpanded with best-effort labels (prefer CLI names if supplied)
                for (size_t idx = 0; idx < finalCutsExpanded.size(); ++idx) {
                    const auto &c = finalCutsExpanded[idx];
                    if (c.empty()) continue;
    
                    cutsOrdered.push_back(c);
    
                    std::string label;
                    if (idx < cutsCLI.size() && !cutsCLI[idx].empty()) {
                        label = cutsCLI[idx];
                    }
                    else if (idx - cutsCLI.size() < lepCLI.size() && (idx >= cutsCLI.size()) && !lepCLI[idx - cutsCLI.size()].empty()) {
                        label = lepCLI[idx - cutsCLI.size()];
                    }
                    else if (idx - cutsCLI.size() - lepCLI.size() < predefCLI.size() && (idx >= cutsCLI.size() + lepCLI.size()) && !predefCLI[idx - cutsCLI.size() - lepCLI.size()].empty()) {
                        label = predefCLI[idx - cutsCLI.size() - lepCLI.size()];
                    }
                    else {
                        label = make_readable_label(c);
                    }
    
                    cutLabels.push_back(label);
                }
    
                // Append user cuts with their explicit names (they should be human-readable)
                for (const auto &uc : validUserCuts) {
                    cutsOrdered.push_back(uc.expr);
                    if (!uc.name.empty()) cutLabels.push_back(uc.name);
                    else cutLabels.push_back(make_readable_label(uc.expr));
                }
    
                const int Ncuts = static_cast<int>(cutsOrdered.size());
    
                // --- VALIDATE & PREP hist plans ---
                std::vector<HistFilterPlan> plans_val(N);
                std::vector<HistFilterPlan> plans_fill(N);
                std::vector<char> keep(N,0);
                
                for (size_t i = 0; i < N; ++i) {
                
                    const auto &h = histDefs[i];
                
                    plans_val[i]  = BuildHistFilterPlan(h, BFI, allUserCuts);
                    plans_fill[i] = BuildHistFilterPlan(h, BFI, allUserCuts);
                
                    ROOT::RDF::RNode hnode = node; // validation node
                    bool ok = ValidateAndRecordAppliedUserCuts(hnode, plans_val[i], h, BFI);
                    keep[i] = ok ? 1 : 0;
                    if (ok) plans_fill[i] = plans_val[i]; // propagate validated plan
                } 
                if(doCutFlow) {
                    // --- FILL CUTFLOW per-process ---
                    // create separate cutflows per FAKE-split process
                    std::vector<std::pair<std::string, ROOT::RDF::RNode>> proc_nodes_val;
                    if (!runFAKES) {
                        proc_nodes_val.emplace_back(key, node);
                    } else {
                        // Create mutually exclusive filters
                        auto node_HFelec = node.Filter("isHFFakeElectron");
                        auto node_HFmuon = node.Filter("isHFFakeMuon");
                        auto node_LFelec = node.Filter("isLFFakeElectron");
                        auto node_LFmuon = node.Filter("isLFFakeMuon");
                        auto node_clean  = node.Filter("hasNoFake");
    
                        proc_nodes_val.emplace_back(key_HFelec, node_HFelec);
                        proc_nodes_val.emplace_back(key_HFmuon, node_HFmuon);
                        proc_nodes_val.emplace_back(key_LFelec, node_LFelec);
                        proc_nodes_val.emplace_back(key_LFmuon, node_LFmuon);
                        proc_nodes_val.emplace_back(key, node_clean);
                    }
    
                    for (auto &pkv : proc_nodes_val) {
                        const std::string &proc_key = pkv.first;
                        ROOT::RDF::RNode  proc_node = pkv.second;
    
                        // Book CutFlow specific to (bin, proc_key)
                        std::string cfName = bin + "__" + map_key_to_process[proc_key] + "__CutFlow";
                        auto hist_CutFlow = std::make_shared<TH1D>(cfName.c_str(), cfName.c_str(), Ncuts+1, 0.0, double(Ncuts+1));
                        hist_CutFlow->Sumw2();
    
                        // Total events from proc_node (no cuts)
                        auto sumW_NoCuts = proc_node.Sum<double>("weight_scaled");
                        auto sumW2_NoCuts = proc_node.Sum<double>("weight_sq_scaled");
                        double sW_NoCuts = sumW_NoCuts.GetValue();
                        double sW2_NoCuts = sumW2_NoCuts.GetValue();
                        double err_NoCuts = (sW2_NoCuts>=0)?std::sqrt(sW2_NoCuts):0.0;
                        hist_CutFlow->SetBinContent(0, sW_NoCuts);
                        hist_CutFlow->SetBinError(0, err_NoCuts);
                        hist_CutFlow->SetBinContent(1, sW_NoCuts);
                        hist_CutFlow->SetBinError(1, err_NoCuts);
                        hist_CutFlow->GetXaxis()->SetBinLabel(1, "NTUPLES");
    
                        if (Ncuts > 1) {
                            // local make_pass_name uses proc_key to avoid column name collisions across procs
                            auto make_pass_name = [&](int i){ return map_key_to_process[proc_key] + std::string("_pass_") + std::to_string(i+1); };
    
                            ROOT::RDF::RNode defNode = proc_node;
                            for (int i = 0; i < Ncuts; ++i) {
                                std::string expr = (i == 0) ? ("(" + cutsOrdered[0] + ")")
                                                            : (make_pass_name(i-1) + " && (" + cutsOrdered[i] + ")");
                                defNode = defNode.Define(make_pass_name(i), expr);
                            }

                            // npassed = sum(pass_i ? 1 : 0)
                            std::string npassedExpr;
                            for (int i = 0; i < Ncuts; ++i) {
                                if (i) npassedExpr += " + ";
                                npassedExpr += "(" + make_pass_name(i) + " ? 1 : 0)";
                            }
                            std::string npassed_col = map_key_to_process[proc_key] + std::string("_npassed");
                            defNode = defNode.Define(npassed_col, npassedExpr);
    
                            // Fill Histo1D once (use .c_str())
                            std::string histNameTmp = map_key_to_process[proc_key] + std::string("_npassed_tmp");
                            auto r_h_npassed = defNode.Histo1D(
                                { histNameTmp.c_str(), histNameTmp.c_str(), Ncuts, 0.0, double(Ncuts) },
                                npassed_col.c_str(),
                                "weight_scaled"
                            );
    
                            // Execute once and get the TH1D by value (copy)
                            auto h_npassed = r_h_npassed.GetValue();
    
                            // Fill classical CutFlow: bins 1..Ncuts = events surviving cut1..cutN
                            for (int i = 2; i <= Ncuts+1; ++i) {
                                double surv = 0.0;
                                double surv_err2 = 0.0;
                                for (int k = i-1; k <= Ncuts; ++k) {
                                    int rootBin = k + 1;
                                    double c = h_npassed.GetBinContent(rootBin);
                                    double e = h_npassed.GetBinError(rootBin);
                                    surv += c;
                                    surv_err2 += e * e;
                                }
                                hist_CutFlow->SetBinContent(i, surv);
                                hist_CutFlow->SetBinError(i, std::sqrt(surv_err2));
    
                                std::string lbl = (i - 2 < (int)cutLabels.size()) ? cutLabels[i - 2] : ("Cut_" + std::to_string(i-1));
                                hist_CutFlow->GetXaxis()->SetBinLabel(i, lbl.c_str());
                            }
                        }
                        // --- Write CutFlow for this proc (bin) ---
                        hist_CutFlow->Write();
                    } // end proc_nodes_val loop
                } // end CutFlow
    
                // --- FILL PASS per-bin ---
                ROOT::RDF::RNode fill_node = base_node_fill;
    
                std::cout << "[BFI_condor] Filling histograms (bin=" << bin << ")\n";
    
                // For each histogram plan, fill once per (possibly split) process
                for (size_t i = 0; i < N; ++i) {
                    if (!keep[i]) continue;
                    const auto &h = histDefs[i];
    
                    // Build list of proc-specific nodes for fill stage
                    std::vector<std::pair<std::string, ROOT::RDF::RNode>> proc_nodes_fill;
                    if (!runFAKES) {
                        proc_nodes_fill.emplace_back(key, fill_node);
                    } else {
                        auto node_HFelec = fill_node.Filter("isHFFakeElectron");
                        auto node_HFmuon = fill_node.Filter("isHFFakeMuon");
                        auto node_LFelec = fill_node.Filter("isLFFakeElectron");
                        auto node_LFmuon = fill_node.Filter("isLFFakeMuon");
                        auto node_clean = fill_node.Filter("hasNoFake");
    
                        proc_nodes_fill.emplace_back(key_HFelec, node_HFelec);
                        proc_nodes_fill.emplace_back(key_HFmuon, node_HFmuon);
                        proc_nodes_fill.emplace_back(key_LFelec, node_LFelec);
                        proc_nodes_fill.emplace_back(key_LFmuon, node_LFmuon);
                        proc_nodes_fill.emplace_back(key, node_clean);
                    }
    
                    for (auto &pkv : proc_nodes_fill) {
                        const std::string &proc_key = pkv.first;
                        ROOT::RDF::RNode proc_node_to_fill = pkv.second;
                        for (const auto &c : finalCutsExpanded) if (!c.empty()) proc_node_to_fill = proc_node_to_fill.Filter(c);
                        for (const auto &vc : validUserCuts) proc_node_to_fill = proc_node_to_fill.Filter(vc.expr);
    
                        std::string hname = bin + "__" + map_key_to_process[proc_key] + "__" + h.name;
    
                        // Use the recorded plan; appliedUserCuts were stored in validation
                        FillHistFromPlan(proc_node_to_fill, plans_fill[i], h, hname);
                    }
                }
    
            } // end doHist
    
            // --- JSON output per-bin ---
            if(doJSON){
                std::cout << "[BFI_condor] Filling json (bin=" << bin << ")\n";
                ROOT::RDF::RNode json_node_base = base_node_fill;
    
                // Build per-proc json nodes
                std::vector<std::pair<std::string, ROOT::RDF::RNode>> proc_json_nodes;
                if (!runFAKES) {
                    proc_json_nodes.emplace_back(key, json_node_base);
                } else {
                    auto node_HFelec = json_node_base.Filter("isHFFakeElectron");
                    auto node_HFmuon = json_node_base.Filter("isHFFakeMuon");
                    auto node_LFelec = json_node_base.Filter("isLFFakeElectron");
                    auto node_LFmuon = json_node_base.Filter("isLFFakeMuon");
                    auto node_clean  = json_node_base.Filter("hasNoFake");
    
                    proc_json_nodes.emplace_back(key_HFelec, node_HFelec);
                    proc_json_nodes.emplace_back(key_HFmuon, node_HFmuon);
                    proc_json_nodes.emplace_back(key_LFelec, node_LFelec);
                    proc_json_nodes.emplace_back(key_LFmuon, node_LFmuon);
                    proc_json_nodes.emplace_back(key, node_clean);
                }
    
                // For each proc, apply finalCutsExpanded and validUserCuts filters, then sum
                for (auto &pkv : proc_json_nodes) {
                    const std::string &proc_key = pkv.first;
                    ROOT::RDF::RNode proc_json_node = pkv.second;
    
                    // Apply bin final cuts
                    for (const auto &c : finalCutsExpanded) if (!c.empty()) proc_json_node = proc_json_node.Filter(c);
                    for (const auto &vc : validUserCuts) proc_json_node = proc_json_node.Filter(vc.expr);
    
                    // --- basic nominal ---
                    auto cnt = proc_json_node.Count();
                    auto sumW = proc_json_node.Sum<double>("weight_scaled");
                    auto sumW2 = proc_json_node.Sum<double>("weight_sq_scaled");
    
                    unsigned long long n_entries = cnt.GetValue();
                    double sW = sumW.GetValue();
                    double sW2Val = sumW2.GetValue();
                    double err = (sW2Val >= 0.0) ? std::sqrt(sW2Val) : 0.0;
    
                    // Prepare container for this file
                    FileYields fy;
                    fy.nominal = { (double)n_entries, sW, err };
    
                    // --- For each systematic compute Up/Down sums ---
                    // for now turn off sys for signal
                    if(!IsData && !isSignal && runInternalSysts) {
                        for (const auto &s : sysCfg.sf) {
                            std::string colUp   = "weight_scaled_" + s.tag + "Up";
                            std::string colDown = "weight_scaled_" + s.tag + "Down";
    
                            // Sum for Up
                            double sum_up = 0.0, sum2_up = 0.0;
                            try {
                                sum_up  = proc_json_node.Sum<double>(colUp).GetValue();
                                std::string colUp2 = colUp;
                                colUp2.replace(0, strlen("weight_scaled_"), "weight_sq_");
                                sum2_up = proc_json_node.Sum<double>(colUp2).GetValue();
                            } catch (...) {
                                sum2_up = 0.0;
                            }
                            double err_up = (sum2_up >= 0.0) ? std::sqrt(sum2_up) : 0.0;
    
                            // Sum for Down
                            double sum_down = 0.0, sum2_down = 0.0;
                            try {
                                std::string colDownName = "weight_scaled_" + s.tag + "Down";
                                sum_down  = proc_json_node.Sum<double>(colDownName).GetValue();
                                std::string colDown2 = colDown;
                                colDown2.replace(0, strlen("weight_scaled_"), "weight_sq_");
                                sum2_down = proc_json_node.Sum<double>(colDown2).GetValue();
                            } catch (...) {
                                sum2_down = 0.0;
                            }
                            double err_down = (sum2_down >= 0.0) ? std::sqrt(sum2_down) : 0.0;
    
                            // Record into file yields
                            SystYields sy;
                            sy.up   = { (double)n_entries, sum_up, err_up };
                            sy.down = { (double)n_entries, sum_down, err_down };
                            fy.systs[s.tag] = sy;
                        }
                    }
                    // --- store per-file result and accumulate into totals (only once per-file) ---
                    // First handle tree-systematic passes: ALWAYS store the variation under .systs[tag]
                    // (create the file entry if needed)
                    if (!treeSystTag.empty()) {
                        auto &fyr = fileResultsByBin[bin][proc_key][rootFilePath]; // creates entry if absent
                        if (treeSystIsUp) {
                            fyr.systs[treeSystTag].up = { (double)n_entries, sW, err };
                        } else {
                            fyr.systs[treeSystTag].down = { (double)n_entries, sW, err };
                        }
                        // Do NOT touch nominal or totals for tree-systematic pass
                        continue;
                    }
                    
                    // Nominal pass: only write nominal + accumulate totals only once
                    if (fileResultsByBin[bin].find(proc_key) == fileResultsByBin[bin].end() ||
                        fileResultsByBin[bin][proc_key].find(rootFilePath) == fileResultsByBin[bin][proc_key].end())
                    {
                        // Record nominal per-bin, per-process, per-rootfile
                        fileResultsByBin[bin][proc_key][rootFilePath] = fy;
                    
                        // Accumulate totals
                        auto &tot = totalsByBin[bin][proc_key];
                    
                        // nominal
                        tot.nominal[0] += (double)n_entries;
                        tot.nominal[1] += sW;
                        // accumulate variance (sqrt at the end) -> store as sum(sigma^2)
                        tot.nominal[2] += (err * err);
                    
                        // systematics accumulation (internal SF systs that we computed into fy.systs)
                        if(!IsData){
                            for (const auto &s_kv : fy.systs) {
                                const std::string &stag = s_kv.first;
                                const SystYields &sy = s_kv.second;
                    
                                // Ensure entry exists
                                auto &dest_syst = tot.systs[stag];
                                // accumulate up
                                dest_syst.up[0]  += sy.up[0];   // entries (will be the same)
                                dest_syst.up[1]  += sy.up[1];   // sum
                                dest_syst.up[2]  += (sy.up[2] * sy.up[2]); // accumulate variance (err^2)
                                // accumulate down
                                dest_syst.down[0]+= sy.down[0];
                                dest_syst.down[1]+= sy.down[1];
                                dest_syst.down[2]+= (sy.down[2] * sy.down[2]);
                            }
                        }
                    }
                } // end proc_json_nodes loop
            } // end doJSON
    
        } // end loop over bins
    }; // end processTree

    // Dispatch tree(s)
    std::string keyPT = sampleName; // key for processTree

    auto doTrees = [&](const std::string &baseTree, bool is_data, bool runFAKES) {
        if(is_data) runFAKES = false;
        if(doHist) { // ignore systematics for histogram filling
            SystematicsConfig histSystConfig;
            std::vector<SystInfo> histSystSFConfig;
            std::vector<string> histSystTreeConfig;
            histSystConfig.sf = histSystSFConfig;
            histSystConfig.tree = histSystTreeConfig;
            processTree(baseTree, keyPT, histSystConfig, false, "", false, runFAKES);
        } else {
            processTree(baseTree, keyPT, activeSystematics, runInternalSystsOnNominal, "", false, runFAKES);
            // for now turn off sys for signal
            if(!is_data && !isSignal){
                for (const auto &treeSyst : activeSystematics.tree) {
                    processTree(baseTree+"_"+treeSyst+"Up",   keyPT, activeSystematics, false, treeSyst, true, runFAKES);
                    processTree(baseTree+"_"+treeSyst+"Down", keyPT, activeSystematics, false, treeSyst, false, runFAKES);
                }
            }
        }
    };

    if (!isSignal) {
        doTrees("KUAnalysis", IsData, doFAKES);
    }
    else if (sigType == "cascades") {
        keyPT = processName;
        doTrees("KUAnalysis", false, doFAKES);
    }
    else if (sigType == "sms") {
        // base process name (without filter suffix)
        const std::string baseProcName = GetProcessNameFromKey(rootFilePath, preferredGroups);

        // get the configured SMS filters (may be empty)
        const auto filters = BFTool::GetFilterSignalsSMS();

        if (filters.empty()) { // might need to debug this later
            // no filters configured -> fall back to base behaviour (single entry)
            processName = baseProcName;
            keyPT = processName;
            for (const auto &tree_name : BFTool::GetSignalTokensSMS(rootFilePath)) {
                doTrees(tree_name, false, doFAKES);
            }
        } else {
            // iterate filters: set processName and keyPT per-filter, then run trees
            for (const auto &filter : filters) {
                processName = baseProcName + "_" + filter; // used inside processTree (hist names, etc.)
                keyPT = processName;                       // passed as 'key' to processTree (JSON keys / totals)
                std::string tree_name = keyPT;
                std::size_t pos = keyPT.find("_SMS");
                if (pos != std::string::npos)
                    tree_name = keyPT.substr(pos + 1);
                doTrees(tree_name, false, doFAKES);
            }
        }
    }
    else {
        std::cerr << "[BFI_condor] Unknown sig-type: " << sigType << "\n";
        delete BFI;
        return 4;
    }

    // finalize totals: convert accumulated err^2 -> sqrt(err^2)
    for (auto &bin_kv : totalsByBin) {
        for (auto &proc_kv : bin_kv.second) {
            auto &tot = proc_kv.second;
    
            // nominal
            tot.nominal[2] = std::sqrt(tot.nominal[2]);
    
            // each syst
            for (auto &syst_kv : tot.systs) {
                auto &sy = syst_kv.second;
    
                sy.up[2]   = std::sqrt(sy.up[2]);
                sy.down[2] = std::sqrt(sy.down[2]);
            }
        }
    }

    // write JSON: if an explicit --json-output base was given, produce a single aggregated JSON
    if (doJSON) {
        if (!outputJsonPathBase.empty()) {
            std::ofstream ofs(outputJsonPathBase);
            if (!ofs) {
                std::cerr << "[BFI_condor] ERROR opening JSON for write: " << outputJsonPathBase << "\n";
                delete BFI;
                return 5;
            }
    
            ofs << "{\n";
    
            for (size_t ib = 0; ib < binNames.size(); ++ib) {
                const auto &bin = binNames[ib];
                if (ib) ofs << ",\n";
                ofs << "  \"" << bin << "\": {\n";
    
                // totals_for_write should mirror the in-memory totals type (ProcTotals)
                std::map<std::string, ProcTotals> totals_for_write;
                auto itT = totalsByBin.find(bin);
                if (itT != totalsByBin.end()) {
                    totals_for_write = itT->second;
                }
    
                std::map<std::string, std::map<std::string, FileYields>> files_for_write;
                auto itF = fileResultsByBin.find(bin);
                if (itF != fileResultsByBin.end()) {
                    const auto &procMap = itF->second; // process -> key -> filePath -> FileYields
                
                    for (const auto &proc_kv : procMap) {
                        const std::string &proc = proc_kv.first;
                        const auto &keyMap = proc_kv.second; // key -> filePath -> FileYields
                
                        // flatten: take all filePath -> FileYields for this process
                        std::map<std::string, FileYields> mergedFiles;
                        for (auto itF = keyMap.begin(); itF != keyMap.end(); ++itF) {
                            const std::string &fName = itF->first;
                            const FileYields &fy = itF->second;
                            mergedFiles[fName] = fy;
                        }
                        files_for_write[proc] = mergedFiles;
                    }
                }
                  
                // Iterate samples (process keys come from totals_for_write)
                writeSamplesJSON(ofs, files_for_write, totals_for_write, "  ");
                ofs << "\n  }"; // close this bin
            } // end bins loop
    
            ofs << "\n}\n";
            ofs.close();
    
            std::cout << "[BFI_condor] Wrote JSON output to: " << outputJsonPathBase << std::endl;
        } else {
            std::cout << "[BFI_condor] NEED TO PROVIDE --json-output";
        }
    }

    if(histFile) histFile->Close();
    if(doHist) std::cout <<"[BFI_condor] Wrote ROOT output to: " << histOutputPath << std::endl;

    delete BFI;
    return 0;
}

