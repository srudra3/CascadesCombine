#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <filesystem>
#include <optional>
#include <unordered_set>
#include <unordered_map>
#include "SampleTool.h"
#include "DefineUserHists.h"

namespace fs = std::filesystem;

// Small POD to keep nominal or variation yield info: {entries, sum, sumErr (stddev)}
using YieldTriple = std::array<double,3>;

// Systematic yields (each side stores a YieldTriple: entries, sum, err)
struct SystYields {
    YieldTriple up  = {0.0, 0.0, 0.0};
    YieldTriple down= {0.0, 0.0, 0.0};
};

// Per-file result: nominal + per-syst up/down
struct FileYields {
    YieldTriple nominal = {0.0, 0.0, 0.0};
    std::map<std::string, SystYields> systs; // keyed by SystInfo.tag
};

// Per-process totals: nominal + per-syst up/down
struct ProcTotals {
    YieldTriple nominal = {0.0, 0.0, 0.0};
    std::map<std::string, SystYields> systs;
};

SampleTool ST;
inline std::string GetSampleNameFromKey(const std::string& keyOrPath) {
    size_t lastSlash = keyOrPath.find_last_of("/\\");
    std::string name = (lastSlash == std::string::npos) ? keyOrPath : keyOrPath.substr(lastSlash + 1);
    size_t dot = name.rfind(".root");
    if (dot != std::string::npos) name = name.substr(0, dot);
    return name;
}

inline std::string GetProcessNameFromKey(const std::string &keyOrPath,
                                         const std::vector<std::string> &preferredGroups = {})
{
    // --- Helper: get prefix before first underscore ---
    auto getPrefix = [](const std::string &fname) -> std::string {
        std::string base = fs::path(fname).filename().string();
        auto pos = base.find("_");
        return (pos == std::string::npos) ? base : base.substr(0, pos);
    };

    // --- Recursive group expansion (same as in mergeJSONs.cpp) ---
    std::function<void(const std::string&, std::unordered_set<std::string>&, std::unordered_set<std::string>&)> expandGroupRec;
    expandGroupRec = [&expandGroupRec](const std::string &group,
                                            std::unordered_set<std::string> &out,
                                            std::unordered_set<std::string> &visited) -> void {
        if (visited.find(group) != visited.end()) return;
        visited.insert(group);

        auto it = ST.MasterDict.find(group);
        if (it == ST.MasterDict.end()) return;

        for (const auto &entry : it->second) {
            if (ST.MasterDict.find(entry) != ST.MasterDict.end()) {
                expandGroupRec(entry, out, visited);
            } else {
                out.insert(entry);
            }
        }
    };

    // --- Cached expansion ---
    std::unordered_map<std::string, std::vector<std::string>> expandedCache;
    auto getExpandedEntries = [&expandedCache, &expandGroupRec](const std::string &group) -> const std::vector<std::string>& {
        auto cit = expandedCache.find(group);
        if (cit != expandedCache.end()) return cit->second;

        std::unordered_set<std::string> expandedSet, visited;
        expandGroupRec(group, expandedSet, visited);
        std::vector<std::string> vec(expandedSet.begin(), expandedSet.end());
        auto ret = expandedCache.emplace(group, std::move(vec));
        return ret.first->second;
    };

    // --- Resolution logic ---
    auto resolveGroup = [&preferredGroups, &getExpandedEntries, &getPrefix](const std::string &jsonKey) -> std::string {
        std::string keyBase = fs::path(jsonKey).filename().string();

        // 1) Try preferred groups (YAML order)
        for (const auto &pref : preferredGroups) {
            const auto &entries = getExpandedEntries(pref);
            for (const auto &entry : entries) {
                std::string p = getPrefix(entry);
                //if (!p.empty() && keyBase.rfind(p, 0) == 0) {
                if (keyBase == fs::path(entry).filename().string()) {
                    return pref;
                }
            }
        }

        // 2) Fallback: search all groups
        for (const auto &kv : ST.MasterDict) {
            const auto &group = kv.first;
            const auto &entries = getExpandedEntries(group);
            for (const auto &entry : entries) {
                std::string p = getPrefix(entry);
                //if (!p.empty() && keyBase.rfind(p, 0) == 0) {
                if (keyBase == fs::path(entry).filename().string()) {
                    return group;
                }
            }
        }

        // 3) No match -> fallback to filename
        return keyBase;
    };

    return resolveGroup(keyOrPath);
}

bool isProcFAKES(const std::string &yamlPath, const std::string &procName) {
    try {
        YAML::Node root = YAML::LoadFile(yamlPath);
        if (!root || !root["processes"]) return false;

        YAML::Node procs = root["processes"];

        auto checkList = [&](const YAML::Node &seq) -> bool {
            if (!seq || !seq.IsSequence()) return false;
            for (const auto &n : seq) {
                if (!n.IsScalar()) continue;
                std::string name = n.as<std::string>();
                if (name.rfind(procName, 0) == 0) {
                    if (name.find("_FAKES") != std::string::npos)
                        return true;
                }
            }
            return false;
        };

        // Check both bkg and sig groups
        if (checkList(procs["bkg"])) return true;
        if (checkList(procs["sig"])) return true;
    }
    catch (const std::exception &e) {
        std::cerr << "[isProcFAKES] Error reading YAML '" << yamlPath
                  << "': " << e.what() << "\n";
    }

    return false;
}

inline bool SampleIsData(const std::string& keyOrPath){
    std::string sample = GetSampleNameFromKey(keyOrPath);
    std::transform(sample.begin(), sample.end(), sample.begin(),
               [](unsigned char c){ return std::tolower(c); });
    if (sample.find("data") != std::string::npos) return true;
    return false;
}

inline double GetLumiFromKey(const std::string& keyOrPath) {
    SampleTool ST;
    ST.LoadAllFromMaster();
    // Helper: strip known suffixes
    auto stripSuffixes = [](std::string s) -> std::string {
        const std::vector<std::string> suffixes = {"_SMS", "_Cascades"};
        for (const auto& suf : suffixes) {
            if (s.size() >= suf.size() &&
                s.compare(s.size() - suf.size(), suf.size(), suf) == 0) {
                s.erase(s.size() - suf.size());
            }
        }
        return s;
    };

    // Use stripSuffixes later when not mixing and matching signals across eras
    // Helper: search for the best (longest) LumiDict key appearing as a path-segment prefix
    auto findLumiKeyInString = [&ST, &stripSuffixes](const std::string &s) -> std::optional<std::string> {
        std::string best;
        for (const auto &kv : ST.LumiDict) {
            std::string tag = kv.first; // stripSuffixes(kv.first);
            std::size_t pos = 0;
            while (pos < s.size()) {
                std::size_t next = s.find('/', pos);
                std::string seg = (next == std::string::npos) ? s.substr(pos) : s.substr(pos, next - pos);
                //seg = stripSuffixes(seg);

                if (seg == tag ||
                    (seg.rfind(tag, 0) == 0 &&
                     (seg.size() == tag.size() || seg[tag.size()] == '_' || seg[tag.size()] == '-'))) {
                    if (tag.size() > best.size()) best = tag;
                }

                if (next == std::string::npos) break;
                pos = next + 1;
            }
        }
        if (!best.empty()) return best;
        return std::nullopt;
    };

    // 1) Try direct search in the provided path/string
    if (auto k = findLumiKeyInString(keyOrPath); k.has_value()) {
        return ST.LumiDict[*k];
    }

    // 2) Fallback: try to match by filename against MasterDict entries
    std::string keyBase = fs::path(keyOrPath).filename().string();
    for (const auto &kv : ST.MasterDict) {
        for (const auto &entry : kv.second) {
            std::string entryBase = fs::path(entry).filename().string();
            if (keyBase == entryBase) {
                if (auto k = findLumiKeyInString(entry); k.has_value()) {
                    return ST.LumiDict[*k];
                }
            }
        }
    }

    // 3) Give up
    std::cerr << "Warning: GetLumiFromKey could not determine lumi for '" << keyOrPath << "'. Returning 1.0\n";
    return 1.0;
}

static bool buildCutsForBin(BuildFitInput* BFI,
                            const std::vector<std::string>& normalCuts,
                            const std::vector<std::string>& lepCuts,
                            const std::vector<std::string>& predefinedCuts,
                            std::vector<std::string>& outCuts) 
{
    if (!BFI) return false;
    outCuts.clear();
    outCuts = normalCuts;
    for (const auto &lepCut : lepCuts) {
        std::string builtCut = BFI->BuildLeptonCut(lepCut);
        if (!builtCut.empty()) outCuts.push_back(builtCut);
    }
    for (const auto &pcut : predefinedCuts) {
        std::string cut;
        if (BFI->GetCutByName(pcut, cut))
          outCuts.push_back(cut);
        else
          std::cerr << "[BFI_condor] Unknown predefined cut: " << pcut << "\n";
    }
    return true;
}

static std::vector<DerivedVar> loadDerivedVariablesYAML(const std::string &yamlPath) {
    std::vector<DerivedVar> vars;
    YAML::Node root = YAML::LoadFile(yamlPath);
    if(!root["derived_variables"]) return vars;

    for(const auto &vnode : root["derived_variables"]) {
        DerivedVar dv;
        dv.name = vnode["name"].as<std::string>();
        dv.expr = vnode["expr"].as<std::string>();
        vars.push_back(dv);
    }
    return vars;
}

static void writeSamplesJSON(
    std::ostream& os,
    const std::map<std::string, std::map<std::string, FileYields>>& files_for_write,
    const std::map<std::string, ProcTotals>& totals_for_write,
    const std::string& indent = "  "
){
    bool firstSample = true;
    for (const auto& kv : totals_for_write) {
        if (!firstSample) os << ",\n";
        firstSample = false;

        const std::string& sname = kv.first;
        std::string sampleId = GetSampleNameFromKey(sname);
        
        os << indent << "  \"" << sampleId << "\": {\n";

        // --- Files ---
        os << indent << "    \"files\": {\n";
        bool firstFile = true;
        auto itFiles = files_for_write.find(sname);
        if (itFiles != files_for_write.end()) {
            for (const auto& fkv : itFiles->second) {
                if (!firstFile) os << ",\n";
                firstFile = false;

                const std::string& fname = fkv.first;
                const FileYields& fy = fkv.second;

                os << indent << "      \"" << fname << "\": {\n";

                // Nominal
                os << indent << "        \"nominal\": ["
                   << (long long)fy.nominal[0] << ", "
                   << fy.nominal[1] << ", "
                   << fy.nominal[2] << "],\n";

                // Systematics
                os << indent << "        \"systematics\": {\n";
                bool firstS = true;
                for (const auto& s_kv : fy.systs) {
                    if (!firstS) os << ",\n";
                    firstS = false;

                    const std::string& stag = s_kv.first;
                    const SystYields& sy = s_kv.second;

                    os << indent << "          \"" << stag << "\": {\n";
                    os << indent << "            \"Up\":   ["
                       << (long long)sy.up[0] << ", " << sy.up[1] << ", " << sy.up[2] << "],\n";
                    os << indent << "            \"Down\": ["
                       << (long long)sy.down[0] << ", " << sy.down[1] << ", " << sy.down[2] << "]\n";
                    os << indent << "          }";
                }
                os << "\n" << indent << "        }\n"; // close systematics
                os << indent << "      }"; // close file
            }
        }
        os << "\n" << indent << "    }\n"; // close files
        os << indent << "  }"; // close sample
    }
}

bool TreeExistsInFile(const std::string& filename,
                      const std::string& treeName)
{
    std::unique_ptr<TFile> f(TFile::Open(filename.c_str(), "READ"));
    if (!f || f->IsZombie())
        return false;

    TObject* obj = f->Get(treeName.c_str());
    return (obj && obj->InheritsFrom(TTree::Class()));
}


struct SystInfo {
    std::string tag;       // Example: "METtrig"
    std::string nominal;   // Example: "MetTrigSFweight"
    std::string up;        // Example: "MetTrigSFweight_up"
    std::string down;      // Example: "MetTrigSFweight_down"
};

static const std::vector<SystInfo> kDefaultSFSystematics = {
    //{"METtrig", "MetTrigSFweight", "MetTrigSFweight_up", "MetTrigSFweight_down"},
    //{"PU",      "PUweight",        "PUweight_up",        "PUweight_down"}
};

static const std::vector<std::string> kDefaultTreeSystematics = {
    //"JESUncer_CMS_scale_j_Total",
    //"JERUncer_Total",
    //"METUncer_UnClust"
};

// Combined container so callers can pass a single object
struct SystematicsConfig {
    std::vector<SystInfo> sf;       // internal SF (weight-based) systematics
    std::vector<std::string> tree;  // tree-based systematics (strings - appended to tree name)
};

// default combined config
static const SystematicsConfig kDefaultSystematicsConfig = {
    kDefaultSFSystematics,
    kDefaultTreeSystematics
};

ROOT::RDF::RNode MultiSystWeights(ROOT::RDF::RNode node,
                                  bool is_data,
                                  int year,
                                  const std::vector<SystInfo>& systs = kDefaultSFSystematics)
{
    if (is_data) {
        return node
            .Define("weight_scaled",    [](){ return 1.0; })
            .Define("weight_sq_scaled", [](){ return 1.0; });
    }

    // --- Nominal product ---
    node = node.Define(
        "syst_nomin_product",
        // all weights
        //[](
        //    double pu, double MuF, double MuR, double PDF,
        //    double BtagHF, double BtagLF, double met_trig,
        //    double elBLP_over_COL, double elID_over_BLP, double elISO_over_ID, double elPrompt_ISOID, double elNOT_Prompt_ISOID, double elNOT_ID_nor_ISO,
        //    double muBLP_over_COL, double muID_over_BLP, double muISO_over_ID, double muPrompt_ISOID, double muNOT_Prompt_ISOID, double muNOT_ID_nor_ISO,
        //    double prefire
        //  ) {
        //      return pu * MuF * MuR * PDF * BtagHF * BtagLF * met_trig * elBLP_over_COL * elID_over_BLP * elISO_over_ID * elPrompt_ISOID * elNOT_Prompt_ISOID * elNOT_ID_nor_ISO * muBLP_over_COL * muID_over_BLP * muISO_over_ID * muPrompt_ISOID * muNOT_Prompt_ISOID * muNOT_ID_nor_ISO * prefire;
        //},
        //{
        //  "PUweight", "MuFweight", "MuRweight", "PDFweight",
        //  "BtagHFSFweight", "BtagLFSFweight", "MetTrigSFweight",
        //  "elBLP_over_COL_SFweight", "elID_over_BLP_SFweight", "elISO_over_ID_SFweight", "elPrompt_ISOID_SFweight", "elNOT_Prompt_ISOID_SFweight", "elNOT_ID_nor_ISO_SFweight",
        //  "muBLP_over_COL_SFweight", "muID_over_BLP_SFweight", "muISO_over_ID_SFweight", "muPrompt_ISOID_SFweight", "muNOT_Prompt_ISOID_SFweight", "muNOT_ID_nor_ISO_SFweight",
        //  "PrefireWeight",
        //}
        
        //[](double pu) {
        //    return pu;
        //},
        //{"PUweight"}
        
        // No weights
        []() {
            return 1.;
        },
        {}
    );

    // --- Multiply base weight by nominal product ---
    node = node.Define("weight_scaled",
        [](double base, double sysNom) { return base * sysNom; },
        {"weight_scaled_raw", "syst_nomin_product"}
    );

    node = node.Define("weight_sq_scaled",
        [](double baseSq, double sysNom) { return baseSq * sysNom * sysNom; },
        {"weight_sq_scaled_raw", "syst_nomin_product"}
    );

    // --- Up/Down variations ---
    for (const auto& s : systs)
    {   
        std::string colUp   = "weight_scaled_" + s.tag + "Up";
        std::string colDown = "weight_scaled_" + s.tag + "Down";
    
        node = node.Define(
            colUp,
            [=](double base, double nomProd, double nomVal, double upVal, double) {
                double val = base * nomProd * (upVal / nomVal);
                if (!std::isfinite(val)) val = base; // fallback if NaN or Inf
                return val;
            },  
            {"weight_scaled_raw", "syst_nomin_product", s.nominal, s.up, s.down}
        );  
    
        node = node.Define(
            colDown,
            [=](double base, double nomProd, double nomVal, double, double downVal) {
                double val = base * nomProd * (downVal / nomVal);
                if (!std::isfinite(val)) val = base; // fallback if NaN or Inf
                return val;
            },  
            {"weight_scaled_raw", "syst_nomin_product", s.nominal, s.up, s.down}
        );  
    }

    return node;
}

// Keeps RDataFrame alive while returning a convenient RNode
struct BaseNodeHandle {
    std::shared_ptr<ROOT::RDataFrame> df; // owns the RDataFrame lifetime
    ROOT::RDF::RNode node;                // node built from *df
};

// https://cms-talk.web.cern.ch/t/question-about-hem15-16-issue-in-2018-ultra-legacy/38654/8?u=gagarwal
// Small functor for computing pass_HEM
struct PassHEM {
    bool is_data;
    bool is2018;
    Long64_t hemModulo;  // apply veto every Nth event
    PassHEM(bool data, int y, double fullLumi, double hemLumi)
        : is_data(data), is2018(y==2018), hemModulo(0)
    {
        if (!is_data && is2018 && fullLumi > 0.0) {
            double frac = hemLumi / fullLumi;
            double fracRounded = std::round(frac * 100.0) / 100.0;
            hemModulo = static_cast<Long64_t>(std::round(1.0 / fracRounded));
            if (hemModulo < 1) hemModulo = 1;
        }
    }
    bool operator()(Long64_t eventnum, int runnum, bool hem_veto) const {
        // --- DATA ---
        if (is_data) {
            if (is2018 && runnum >= 319077 && hem_veto)
                return false;
            return true;
        }
        // --- MC ---
        if (!is2018 || !hem_veto)
            return true;
        // Apply HEM veto to a fraction of events
        return (eventnum % hemModulo) != 0;
    }
};

// Build the common base node (weights + lepton counts + pair kinematics).
// Caller is responsible for calling ROOT::EnableImplicitMT(...) to set MT state
// BEFORE calling this helper so the returned RNode is constructed under the
// desired implicit-MT configuration.
static BaseNodeHandle MakeBaseNode(const std::string &tree_name,
                                   const std::string &rootFilePath,
                                   BuildFitInput *BFI,
                                   double Lumi,
                                   bool is_data,
                                   int year,
                                   std::vector<DerivedVar> validatedDerivedVars
                                  )
{
    auto df = std::make_shared<ROOT::RDataFrame>(tree_name, rootFilePath);

    const double fullLumi = Lumi;
    const double hemLumi  = ST.LumiDict.at("HEM_LUMI"); // assume key exists

    // Instantiate functors
    PassHEM passHEM(is_data, year, fullLumi, hemLumi);

    ROOT::RDF::RNode node = (*df)
        .Define("pass_HEM", passHEM, {"eventnum", "runnum", "HEM_Veto"})
        .Define("event_lumi",
            [is_data, Lumi]() {
                return is_data ? 1.0 : Lumi;
            })
        .Define("weight_scaled_raw",
            [is_data](double w, double lumi) {
                return is_data ? 1.0 : w * lumi;
            },
            {"weight", "event_lumi"})
        .Define("weight_sq_scaled_raw",
            [is_data](double w2, double lumi) {
                return is_data ? 1.0 : w2 * lumi * lumi;
            },
            {"weight2", "event_lumi"})

    ;

    node = MultiSystWeights(node, is_data, year);

    // lepton counts / kinematics
    node = BFI->DefineLeptonPairCounts(node, "");
    node = BFI->DefineLeptonPairCounts(node, "A");
    node = BFI->DefineLeptonPairCounts(node, "B");
    node = BFI->DefinePairKinematics(node, "");
    node = BFI->DefinePairKinematics(node, "A");
    node = BFI->DefinePairKinematics(node, "B");

    // --- Define validated derived variables on base node ---
    for(const auto &dv : validatedDerivedVars){
        try{
            node = node.Define(dv.name, dv.expr);
        }catch(const std::exception &e){
            std::cerr << "[BFI_condor] WARNING: Failed to define derived variable '"
                      << dv.name << "' Expression: " << dv.expr
                      << " Exception: " << e.what() << "\n";
        }
    }
    
    // --- Define fake lepton columns ---
    if(!is_data){
        node = node
            // Count how many fake e/mu
            .Define("nFakeElectron", [](const std::vector<int>& sid, const std::vector<int>& pdg) {
                int n = 0;
                for (size_t i = 0; i < sid.size(); ++i)
                    if (sid[i] > 1 && std::abs(pdg[i]) == 11)
                        ++n;
                return n;
            }, {"SourceID_lep", "PDGID_lep"})
        
            .Define("nFakeMuon", [](const std::vector<int>& sid, const std::vector<int>& pdg) {
                int n = 0;
                for (size_t i = 0; i < sid.size(); ++i)
                    if (sid[i] > 1 && std::abs(pdg[i]) == 13)
                        ++n;
                return n;
            }, {"SourceID_lep", "PDGID_lep"})
        
            // Mutually exclusive flags
            .Define("hasFakeElectron", "nFakeElectron > 0 && nFakeMuon == 0")
            .Define("hasFakeMuon",     "nFakeMuon > 0 && nFakeElectron == 0")
        
            // Event has both
            .Define("hasFakeBoth",     "nFakeElectron > 0 && nFakeMuon > 0")
            // Event has neither
            .Define("hasNoFake",     "nFakeElectron == 0 && nFakeMuon == 0")

            .Define("nHFFakeElectron", [](const std::vector<int>& sid, const std::vector<int>& pdg) {
                int n = 0;
                for (size_t i = 0; i < sid.size(); ++i)
                    if (sid[i] > 1 && sid[i] < 4 && std::abs(pdg[i]) == 11)
                        ++n;
                return n;
            }, {"SourceID_lep", "PDGID_lep"})

            .Define("nLFFakeElectron", [](const std::vector<int>& sid, const std::vector<int>& pdg) {
                int n = 0;
                for (size_t i = 0; i < sid.size(); ++i)
                    if (sid[i] >= 4 && std::abs(pdg[i]) == 11)
                        ++n;
                return n;
            }, {"SourceID_lep", "PDGID_lep"})

            .Define("nHFFakeMuon", [](const std::vector<int>& sid, const std::vector<int>& pdg) {
                int n = 0;
                for (size_t i = 0; i < sid.size(); ++i)
                    if (sid[i] > 1 && sid[i] < 4 && std::abs(pdg[i]) == 13)
                        ++n;
                return n;
            }, {"SourceID_lep", "PDGID_lep"})

            .Define("nLFFakeMuon", [](const std::vector<int>& sid, const std::vector<int>& pdg) {
                int n = 0;
                for (size_t i = 0; i < sid.size(); ++i)
                    if (sid[i] >= 4 && std::abs(pdg[i]) == 13)
                        ++n;
                return n;
            }, {"SourceID_lep", "PDGID_lep"})
        
            // Mutually exclusive flags
            .Define("isHFFakeMuon",     "nHFFakeMuon > 0")
            .Define("isHFFakeElectron", "nHFFakeElectron > 0 && nHFFakeMuon == 0")
            .Define("isLFFakeElectron", "nLFFakeElectron > 0 && nHFFakeMuon == 0 && nHFFakeElectron == 0")
            .Define("isLFFakeMuon",     "nLFFakeMuon > 0 && nHFFakeMuon == 0 && nHFFakeElectron == 0 && nLFFakeElectron == 0");
    }

    return {df, node};
}
