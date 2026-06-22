#include "BuildFit.h"

ch::Categories BuildFit::BuildCats(JSONFactory* j){
    ch::Categories cats{};
    int binNum=0;
    for (json::iterator it = j->j.begin(); it != j->j.end(); ++it) {
          //std::cout << it.key() <<"\n";
        cats.push_back( {binNum, it.key()} );
        binNum++;
    }
    return cats;
}

void BuildFit::BuildAsimovData(
        std::map<std::string, double>& obs_rates,
        JSONFactory* j
    ){
    //outer loop bin iterator
    for (json::iterator it = j->j.begin(); it != j->j.end(); ++it){
        //inner loop process iterator
        std::string binname = it.key();
        double totalBkg = 0;
        for (json::iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2){
            
            if( BFTool::ContainsAnySubstring( it2.key(), sigkeys)){
                continue;
            }
            else{
                //get the wnevents, index 1 of array
                json json_array = it2.value();
                totalBkg += json_array[1].get<double>();
            }
        }
        obs_rates[binname] = double(int(totalBkg));
    }
}

std::vector<std::string> BuildFit::GetBkgProcs(JSONFactory* j){
	std::vector<std::string> bkgprocs{};

	for (json::iterator it = j->j.begin(); it != j->j.end(); ++it){
                //inner loop process iterator
                std::string binname = it.key();
                for (json::iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2){
                      std::string proc = it2.key();
                      //std::cout<< proc<<"\n";
                        bool is_base = BFTool::ContainsAnySubstring(proc, fakes_skip_list);
                        bool has_fakes = BuildFit::HasProcFAKES(j, proc);
                        if(BFTool::ContainsAnySubstring(proc, sigkeys) || proc.find("data") != std::string::npos || proc.find("Data") != std::string::npos || proc.find("FAKES") != std::string::npos || (is_base && has_fakes)){
                                continue;
                        }
                        else{
				bkgprocs.push_back(proc);
			}
		}
	}//make this set unique 
	std::set<std::string> my_bkg_set(bkgprocs.begin(), bkgprocs.end());
	std::vector<std::string> bkgprocsunique(my_bkg_set.begin(), my_bkg_set.end());

	return bkgprocsunique;
}

std::vector<std::string> BuildFit::GetFakesProcs(JSONFactory* j){
	std::vector<std::string> fakeprocs{};

	for (json::iterator it = j->j.begin(); it != j->j.end(); ++it){
                //inner loop process iterator
                std::string binname = it.key();
                for (json::iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2){
                        if(it2.key().find("FAKES") != std::string::npos){
				fakeprocs.push_back(it2.key());
			} else continue;
		}
	}//make this set unique 
	std::set<std::string> my_fake_set(fakeprocs.begin(), fakeprocs.end());
	std::vector<std::string> fakeprocsunique(my_fake_set.begin(), my_fake_set.end());

	return fakeprocsunique;
}

bool BuildFit::HasDataObs(JSONFactory* j) {
    for (auto &bin : j->j.items()) {
        for (auto &proc : bin.value().items()) {
            if (strcasecmp(proc.key().c_str(), "data_obs") == 0 || proc.key().find("data") != std::string::npos || proc.key().find("Data") != std::string::npos)
                return true;
        }
    }
    return false;
}

bool BuildFit::HasProcFAKES(JSONFactory* j, const std::string& check_proc) {
    for (auto &bin : j->j.items()) {
        for (auto &proc : bin.value().items()) {
            std::string key = proc.key();
            // Skip actual data keys
            if (strcasecmp(key.c_str(), "data_obs") == 0 || 
                key.find("data") != std::string::npos || 
                key.find("Data") != std::string::npos) 
                continue;
            if (key.find(check_proc + "_FAKES") != std::string::npos)
                return true;
        }
    }
    return false;
}

std::string BuildFit::GetSignalMass(const std::string &proc) {
    if (proc.find("SMS") != std::string::npos) {
        size_t pos = proc.rfind("SMS");
        std::string mass = proc.substr(pos + 3);
        if (!mass.empty() && mass[0] == '_') mass.erase(0,1);
        size_t uscore = mass.rfind('_');
        if (uscore != std::string::npos) mass[uscore] = '0';
        return mass;
    }
    if (proc.find("Cascades_") == 0) {
        // Extract first and last mass only
        std::string massStr = proc.substr(strlen("Cascades_"));  
        std::vector<std::string> tokens = BFTool::SplitString(massStr, "_");
        if (tokens.size() >= 2) {
            std::string mass = tokens.front() + tokens.back(); // first + last
            // replace any remaining underscores with zero just in case
            std::replace(mass.begin(), mass.end(), '_', '0');
            return mass;
        } else {
            // fallback: use whatever is present
            std::replace(massStr.begin(), massStr.end(), '_', '0');
            return massStr;
        }
    }
    // fallback for other signals
    std::vector<std::string> tokens = BFTool::SplitString(proc, "_");
    std::string mass;
    for (size_t i=1; i<tokens.size(); i++) mass += tokens[i];
    return mass;
}

std::string BuildFit::GetSignalProcName(const std::string &proc) {
    // SMS: keep the usual prefix
    if (proc.find("SMS") != std::string::npos) {
        size_t pos = proc.rfind("SMS");
        return proc.substr(0, pos + 3); // e.g. "SMS_TChiWZ_SMS"
    }

    // Cascades: keep full original mass info for datacard name
    if (proc.rfind("Cascades_", 0) == 0) {
        return proc; // e.g. "Cascades_220_220_209_200_190_180"
    }

    // fallback: use everything up to first '_' (or whole proc)
    size_t p = proc.find('_');
    if (p != std::string::npos) return proc.substr(0, p);
    return proc;
}

stringlist BuildFit::ExtractSignalDetails(std::string signalPoint)
{
    stringlist splitPoint = BFTool::SplitString(signalPoint, "_");

    std::string analysis = splitPoint[0];
    std::string channel = "dummy";

    std::string mass = GetSignalMass(signalPoint);

    return {analysis, channel, mass};
}

stringlist BuildFit::GetBinSet(JSONFactory* j){
    stringlist bins{};
        for (json::iterator it = j->j.begin(); it != j->j.end(); ++it) {
                //std::cout << it.key() <<"\n";
                bins.push_back(  it.key() );
        }
        return bins;
}

std::string BuildFit::SanitizeName(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (isalnum(c) || c == '_' || c == '-') out += c;
        else out += '_';
    }
    return out;
}

std::vector<std::string> BuildFit::WriteJsonAsFlatHists(
    JSONFactory* j,
    const std::string &outFile,
    std::map<std::string,double>* out_obs_rates,
    const std::string& sig,
    std::set<std::string>* skipped_bins
) {
    constexpr int IDX_RAW  = 0;
    constexpr int IDX_SUMW = 1;
    constexpr int IDX_ERR  = 2;

    TFile *f = TFile::Open(outFile.c_str(), "RECREATE");
    if (!f || f->IsZombie()) {
        std::cerr << "[ERROR] Could not create ROOT file " << outFile << std::endl;
        return {};
    }

    std::vector<std::string> kept_bins;
    int nWritten = 0;
    bool hasDataGlob = HasDataObs(j);

    // iterate bins once and decide per-bin whether we keep it or skip it
    for (auto itBin = j->j.begin(); itBin != j->j.end(); ++itBin) {
        const std::string origBin = itBin.key();
        json &binJson = itBin.value();

        bool hasBackground = false;
        bool hasDataEvents = false;
        bool hasSignal = false;

        // quick scan to decide whether to keep bin
        for (auto itProc = binJson.begin(); itProc != binJson.end(); ++itProc) {
            const std::string procOrig = itProc.key();

            // case-insensitive data detection on original name
            std::string low = procOrig;
            std::transform(low.begin(), low.end(), low.begin(), ::tolower);
            bool isData = (low.find("data") != std::string::npos);

            if (isData) {
                // If there is a data entry, consider "hasDataEvents" true if raw count > 0
                const json &valsObj = itProc.value();
                if (valsObj.contains("nominal")) {
                    const json &vals = valsObj["nominal"];
                    if (vals.is_array() && vals.size() > IDX_RAW) {
                        double nRaw = vals[IDX_RAW];
                        if (nRaw > 0) hasDataEvents = true;
                    }
                }
                continue;
            }

            if (BFTool::ContainsAnySubstring(procOrig, sigkeys)) {
                hasSignal = true;
                // still check background presence in other procs
                continue;
            }

            // non-signal, non-data -> consider as background candidate
            const json &valsObj = itProc.value();
            if (!valsObj.contains("nominal")) continue;
            const json &vals = valsObj["nominal"];
            if (!vals.is_array() || vals.size() <= IDX_SUMW) continue;
            double sumW = vals[IDX_SUMW];
            if (sumW > 0.0) {
                hasBackground = true;
                break; // no need to scan further -> this bin is keepable
            }
        } // end scan procs

        if (hasSignal && !hasBackground && !hasDataEvents) {
            std::cerr << "[WARNING] Skipping bin '" << origBin
                      << "' (signal present but no background or data)" << std::endl;
            if (skipped_bins) skipped_bins->insert(origBin);
            continue; // skip writing anything for this bin
        }

        // keep the bin
        kept_bins.push_back(origBin);

        // Now write histograms for all processes in this bin (backgrounds, data, signals if present)
        double binTotal = 0.0;
        int binRaw = 0;
        int binData = 0;

        for (auto itProc = binJson.begin(); itProc != binJson.end(); ++itProc) {
            const std::string procOrig = itProc.key();
            std::string low = procOrig;
            std::transform(low.begin(), low.end(), low.begin(), ::tolower);
            bool isData = (low.find("data") != std::string::npos);

            const json &valsObj = itProc.value();
            if (!valsObj.contains("nominal")) continue;
            const json &vals = valsObj["nominal"];
            if (!vals.is_array() || vals.size() <= IDX_ERR) continue;

            double nRaw = vals[IDX_RAW];
            double sumW = vals[IDX_SUMW];
            double err  = vals[IDX_ERR];

            // Build procName: if signal, turn into the datacard-style name, else keep original
            std::string procName = procOrig;
            if (BFTool::ContainsAnySubstring(procOrig, sigkeys)) {
                if (procOrig != sig) {
                    // skip signals not matching requested signalPoint
                    continue;
                }
                std::string prefix = GetSignalProcName(procOrig);
                std::string mass   = GetSignalMass(procOrig);
                procName = prefix + "_" + mass;
            }

            // Use original bin & proc names for CH compatibility (exact match)
            std::string hname = origBin + "__" + procName;

            if (!isData && !BFTool::ContainsAnySubstring(procOrig, sigkeys)) {
                binTotal += sumW;
                binRaw   += static_cast<int>(nRaw + 0.5);
            } else if (isData) {
                binData = static_cast<int>(nRaw);
            }

            // Write nominal histogram for non-data processes (CH expects process histos and data_obs)
            if (!isData) {
                //if (sumW <= 0.0 && err <= 0.0) {sumW = 1.e-9; err = 1.e-9;}
                TH1F *h = new TH1F(hname.c_str(), hname.c_str(), 1, 0, 1);
                h->Sumw2();
                h->SetBinContent(1, sumW);
                h->SetBinError(1, err);
                h->SetEntries(nRaw);
                h->Write();
                delete h;
                ++nWritten;

                // systematic variations
                if (valsObj.contains("systematics")) {
                    const json &systs = valsObj["systematics"];
                    for (auto itS = systs.begin(); itS != systs.end(); ++itS) {
                        const std::string systNameOrig = itS.key();
                        const std::string systName = SanitizeName(systNameOrig);
                        const json &ud = itS.value();

                        for (std::string_view udKey : {"Up", "Down"}) {
                            if (!ud.contains(std::string(udKey))) continue;

                            const json &arr = ud[std::string(udKey)];
                            if (!arr.is_array() || arr.size() <= IDX_ERR) continue;

                            double nRaw_ud = arr[IDX_RAW];
                            double sumW_ud = arr[IDX_SUMW];
                            double err_ud  = arr[IDX_ERR];
                            if (sumW_ud <= 0.0 && err_ud <= 0.0) {sumW_ud = 1.e-9; err_ud = 1.e-9;}

                            std::string hsyst = origBin + "__" + procName + "__" + systName + std::string(udKey);

                            TH1F *hs = new TH1F(hsyst.c_str(), hsyst.c_str(), 1, 0, 1);
                            hs->Sumw2();
                            hs->SetBinContent(1, sumW_ud);
                            hs->SetBinError(1, err_ud);
                            hs->SetEntries(nRaw_ud);
                            hs->Write();
                            delete hs;
                            ++nWritten;
                        }
                    }
                }
            } // end non-data
        } // end per-process writing

        // Fill out_obs_rates only for kept bins
        if (out_obs_rates) {
            if (hasDataGlob) (*out_obs_rates)[origBin] = binData;
            else             (*out_obs_rates)[origBin] = binTotal;
        }

        double asimov_val = out_obs_rates ? (*out_obs_rates)[origBin] : binTotal;

        // Write data_obs (for both real data and asimov)
        std::string dataName = origBin + "__data_obs";

        TH1F *hdata = new TH1F(dataName.c_str(), dataName.c_str(), 1, 0, 1);
        hdata->Sumw2();
        hdata->SetBinContent(1, asimov_val);
        hdata->SetEntries(hasDataGlob ? binData : static_cast<int>(asimov_val + 0.5));
        hdata->Write();
        delete hdata;
    } // end per-bin

    f->Write();
    f->Close();
    delete f;

    std::cout << "[BuildFit] Wrote " << nWritten << " histograms to " << outFile << std::endl;
    return kept_bins;
}

ch::Categories BuildFit::BuildCatsFromList(const std::vector<std::string>& bin_names) {
    ch::Categories cats{};
    int binNum = 0;
    for (const auto &b : bin_names) {
        cats.push_back({binNum, b});
        ++binNum;
    }
    return cats;
}

void BuildFit::AddFloatingNorms(stringlist procs, const std::string& type, double val){
    cb.SetFlag("filters-use-regex", true);
    for (const auto& proc: procs){
        cb.cp().process({proc})
            .AddSyst(cb, "scale_"+proc, type, SystMap<>::init(val));
    }
    cb.SetFlag("filters-use-regex", false);
}

void BuildFit::AddSharedFloatingNorm(const stringlist& procs, const std::string& nuis_name, const std::string& type, double val){
    cb.SetFlag("filters-use-regex", true);
    cb.cp()
      .process(procs)   // all processes get the SAME nuisance
      .AddSyst(cb, nuis_name, type, SystMap<>::init(val));
    cb.SetFlag("filters-use-regex", false);
}

void BuildFit::AddFloatingNormsGroupedByFakeType(
    const stringlist& fakesprocs,
    const std::string& type,
    double val)
{
    std::map<std::string, stringlist> groups;
    const std::regex re(R"((?:.*_)?(Run2|Run3)[^_]*_FAKES_(.+)$)");
    std::smatch m;

    for (const auto &fp : fakesprocs) {
        if (std::regex_search(fp, m, re)) {
            std::string run_tag = m[1].str();   // "Run2" or "Run3"
            std::string fake_type = m[2].str(); // "Elec" or "Muon" (or other)
            std::string key = run_tag + "_" + fake_type;
            groups[key].push_back(fp);
        } else {
            // fallback: if it has _FAKES_ but no Run tag, group under ALLRuns_<type>
            const std::string marker = "_FAKES_";
            auto pos = fp.find(marker);
            if (pos != std::string::npos) {
                std::string fake_type = fp.substr(pos + marker.size());
                std::string key = std::string("ALLRuns_") + fake_type;
                groups[key].push_back(fp);
                std::cerr << "[BuildFit] Warning: process '" << fp
                          << "' did not match Run2/Run3. grouped as '" << key << "'\n";
            } else {
                // completely malformed name -> warn and skip
                std::cerr << "[BuildFit] Warning: skipping process with unexpected name: '"
                          << fp << "' (no _FAKES_ token)\n";
            }
        }
    }

    // Create nuisances for each (Run,FakeType) group
    for (const auto &kv : groups) {
        const std::string &key = kv.first;    // e.g. "Run2_Elec" or "ALLRuns_Muon"
        const stringlist &procs = kv.second;
        if (procs.empty()) continue;

        std::string nuis_name = "scale_FAKES_" + key; // e.g. scale_FAKES_Run2_Elec
        AddSharedFloatingNorm(procs, nuis_name, type, val);
    }
}

void BuildFit::AddFakeFamiliesAsSharedNorms(
    const std::vector<std::string>& truebkgprocs,
    const std::vector<std::string>& fakesprocs,
    const std::string& type,
    double val,
    const std::string& match_token,
    bool require_match)
{
    // Map stripped physics base -> full family (Run2 + Run3 + their fakes)
    std::map<std::string, std::vector<std::string>> families;

    for (const auto& base_full : truebkgprocs) {

        // Strip "_Run2" / "_Run3"
        std::string base = base_full;
        size_t run_pos = base.find("_Run");
        if (run_pos != std::string::npos)
            base = base.substr(0, run_pos);

        // Apply filtering logic on stripped physics name
        if (!match_token.empty()) {
            bool contains = (base.find(match_token) != std::string::npos);

            if (require_match && !contains) continue;
            if (!require_match && contains) continue;
        }

        // Add true background process (Run2 or Run3)
        families[base].push_back(base_full);

        // Attach matching fake processes for THIS run-specific base
        for (const auto& fp : fakesprocs) {
            if (fp.rfind(base_full + "_FAKES", 0) == 0) {
                families[base].push_back(fp);
            }
        }
    }

    // Now create one nuisance per physics base
    for (const auto& kv : families) {

        const std::string& base = kv.first;
        const std::vector<std::string>& family = kv.second;

        if (family.empty()) continue;

        const std::string nuis_name = "scale_" + base;
        AddSharedFloatingNorm(family, nuis_name, type, val);
    }
}

void BuildFit::AddShapeSystsFromJSON(JSONFactory* j, const std::vector<std::string>& kept_bins) {
    std::set<std::string> kept_set(kept_bins.begin(), kept_bins.end());
    for (const auto& itBin : j->j.items()) {
        const std::string& bin = itBin.key();
        if (!kept_set.count(bin)) continue; // skip bins that were dropped

        const json& binJson = itBin.value();
        for (const auto& itProc : binJson.items()) {
            const std::string proc = itProc.key();
            const json& valsObj = itProc.value();
            if (!valsObj.contains("systematics")) continue;
            const json& systs = valsObj["systematics"];
            for (const auto& itS : systs.items()) {
                const std::string systName = SanitizeName(itS.key());
                cb.cp().bin({bin}).process({proc})
                    .AddSyst(cb, systName, "shape", SystMap<>::init(1.0));
            }
        }
    }
}

std::string BuildFit::GetMatchingBin(const std::string& bin, const std::string& token, const std::string& match_token){
    size_t pos = bin.find(token);
    std::string match_bin = bin;
    if (pos != std::string::npos)
        match_bin.replace(pos, token.length(), match_token);
    return match_bin;
}

void BuildFit::AddZSys(const stringlist& binset, const stringlist& procs){
    cb.SetFlag("filters-use-regex", true);
    cb.cp().process(procs).bin({".*2L.*0J.*OffZ"})
        .AddSyst(cb, "Z_2L_0J", "lnN", SystMap<>::init(1.10));
    cb.cp().process(procs).bin({".*2L.*1J.*OffZ"})
        .AddSyst(cb, "Z_2L_1J", "lnN", SystMap<>::init(1.10));
    cb.SetFlag("filters-use-regex", false);
}

void BuildFit::AddRaSys(const stringlist& binset, const stringlist& procs){
    cb.SetFlag("filters-use-regex", true);
    cb.cp().process(procs).bin({".*3L.*0J.*Ah.*"})
        .AddSyst(cb, "Ra_3L_0J", "lnN", SystMap<>::init(1.10));
    cb.cp().process(procs).bin({".*3L.*1J.*Ah.*"})
        .AddSyst(cb, "Ra_3L_1J", "lnN", SystMap<>::init(1.10));
    cb.SetFlag("filters-use-regex", false);
}

void BuildFit::AddPTISRSys(const stringlist& binset, const stringlist& procs){
    cb.SetFlag("filters-use-regex", true);

    // overall Run2 to Run3 PTISR factor
    cb.cp().process(procs).bin({".*Run3.*_P.*"})
        .AddSyst(cb, "Run3_PTISR", "lnN", SystMap<>::init(1.20));

    cb.cp().process(procs).bin({".*Run2.*2L.*0J.*P350.*"})
        .AddSyst(cb, "Run2_PTISR_2L_0J", "lnN", SystMap<>::init(1.10));
    cb.cp().process(procs).bin({".*Run2.*2L.*1J.*P350.*"})
        .AddSyst(cb, "Run2_PTISR_2L_1J", "lnN", SystMap<>::init(1.10));
    cb.cp().process(procs).bin({".*Run2.*3L.*P300.*"})
        .AddSyst(cb, "Run2_PTISR_3L", "lnN", SystMap<>::init(1.10));

    cb.cp().process(procs).bin({".*Run3.*2L.*0J.*P350.*"})
        .AddSyst(cb, "Run3_PTISR_2L_0J", "lnN", SystMap<>::init(1.10));
    cb.cp().process(procs).bin({".*Run3.*2L.*1J.*P350.*"})
        .AddSyst(cb, "Run3_PTISR_2L_1J", "lnN", SystMap<>::init(1.10));
    cb.cp().process(procs).bin({".*Run3.*3L.*P300.*"})
        .AddSyst(cb, "Run3_PTISR_3L", "lnN", SystMap<>::init(1.10));

    cb.SetFlag("filters-use-regex", false);
}

void BuildFit::AddFAKETransferSys(const stringlist& binset){
    cb.SetFlag("filters-use-regex", true);

    cb.cp().bin({".*Run2.*Bronze.*"}).process({".*FAKES.*LFMuon.*"})
       .AddSyst(cb, "Run2_IDISO_LFMuon", "lnN", SystMap<>::init(1.20));
   
    cb.cp().bin({".*Run2.*Bronze.*"}).process({".*FAKES.*HFMuon.*"})
       .AddSyst(cb, "Run2_IDISO_HFMuon", "lnN", SystMap<>::init(1.20));
   
    cb.cp().bin({".*Run2.*Bronze.*"}).process({".*FAKES.*LFElec.*"})
       .AddSyst(cb, "Run2_IDISO_LFElec", "lnN", SystMap<>::init(1.20));
   
    cb.cp().bin({".*Run2.*Bronze.*"}).process({".*FAKES.*HFElec.*"})
       .AddSyst(cb, "Run2_IDISO_HFElec", "lnN", SystMap<>::init(1.20));
   
     cb.cp().bin({".*Run2.*Silver.*"}).process({".*FAKES.*LFMuon.*"})
       .AddSyst(cb, "Run2_SIP3D_LFMuon", "lnN", SystMap<>::init(1.20));
   
     cb.cp().bin({".*Run2.*Silver.*"}).process({".*FAKES.*HFMuon.*"})
       .AddSyst(cb, "Run2_SIP3D_HFMuon", "lnN", SystMap<>::init(1.20));
   
     cb.cp().bin({".*Run2.*Silver.*"}).process({".*FAKES.*LFElec.*"})
       .AddSyst(cb, "Run2_SIP3D_LFElec", "lnN", SystMap<>::init(1.20));
   
     cb.cp().bin({".*Run2.*Silver.*"}).process({".*FAKES.*HFElec.*"})
       .AddSyst(cb, "Run2_SIP3D_HFElec", "lnN", SystMap<>::init(1.20));

    cb.cp().bin({".*Run3.*Bronze.*"}).process({".*FAKES.*LFMuon.*"})
       .AddSyst(cb, "Run3_IDISO_LFMuon", "lnN", SystMap<>::init(1.20));
   
    cb.cp().bin({".*Run3.*Bronze.*"}).process({".*FAKES.*HFMuon.*"})
       .AddSyst(cb, "Run3_IDISO_HFMuon", "lnN", SystMap<>::init(1.20));
   
    cb.cp().bin({".*Run3.*Bronze.*"}).process({".*FAKES.*LFElec.*"})
       .AddSyst(cb, "Run3_IDISO_LFElec", "lnN", SystMap<>::init(1.20));
   
    cb.cp().bin({".*Run3.*Bronze.*"}).process({".*FAKES.*HFElec.*"})
       .AddSyst(cb, "Run3_IDISO_HFElec", "lnN", SystMap<>::init(1.20));
   
     cb.cp().bin({".*Run3.*Silver.*"}).process({".*FAKES.*LFMuon.*"})
       .AddSyst(cb, "Run3_SIP3D_LFMuon", "lnN", SystMap<>::init(1.20));
   
     cb.cp().bin({".*Run3.*Silver.*"}).process({".*FAKES.*HFMuon.*"})
       .AddSyst(cb, "Run3_SIP3D_HFMuon", "lnN", SystMap<>::init(1.20));
   
     cb.cp().bin({".*Run3.*Silver.*"}).process({".*FAKES.*LFElec.*"})
       .AddSyst(cb, "Run3_SIP3D_LFElec", "lnN", SystMap<>::init(1.20));
   
     cb.cp().bin({".*Run3.*Silver.*"}).process({".*FAKES.*HFElec.*"})
       .AddSyst(cb, "Run3_SIP3D_HFElec", "lnN", SystMap<>::init(1.20));

     cb.SetFlag("filters-use-regex", false);
}

void BuildFit::AddSilverBronzeGlobal(const stringlist& procs){
     cb.SetFlag("filters-use-regex", true);

     cb.cp().bin({".*Run2.*Bronze.*e.*"}).process(procs)
       .AddSyst(cb, "Run2_nonfake_Bronze_el", "lnN", SystMap<>::init(1.1));

     cb.cp().bin({".*Run2.*Bronze.*mu.*"}).process(procs)
       .AddSyst(cb, "Run2_nonfake_Bronze_mu", "lnN", SystMap<>::init(1.1));

     cb.cp().bin({".*Run2.*Silver.*e.*"}).process(procs)
       .AddSyst(cb, "Run2_nonfake_Silver_el", "lnN", SystMap<>::init(1.1));

     cb.cp().bin({".*Run2.*Silver.*mu.*"}).process(procs)
       .AddSyst(cb, "Run2_nonfake_Silver_mu", "lnN", SystMap<>::init(1.1));

     cb.cp().bin({".*Run3.*Bronze.*e.*"}).process(procs)
       .AddSyst(cb, "Run3_nonfake_Bronze_el", "lnN", SystMap<>::init(1.1));

     cb.cp().bin({".*Run3.*Bronze.*mu.*"}).process(procs)
       .AddSyst(cb, "Run3_nonfake_Bronze_mu", "lnN", SystMap<>::init(1.1));

     cb.cp().bin({".*Run3.*Silver.*e.*"}).process(procs)
       .AddSyst(cb, "Run3_nonfake_Silver_el", "lnN", SystMap<>::init(1.1));

     cb.cp().bin({".*Run3.*Silver.*mu.*"}).process(procs)
       .AddSyst(cb, "Run3_nonfake_Silver_mu", "lnN", SystMap<>::init(1.1));
    
     cb.SetFlag("filters-use-regex", false);
}

void BuildFit::AddSameSignSys(const stringlist& binset, const stringlist& procs){
    cb.SetFlag("filters-use-regex", true);
    cb.cp().process(procs).bin({".*Run2.*2L.*_SS.*"})
        .AddSyst(cb, "Run2_SameSign_2L", "lnN", SystMap<>::init(1.10));
    cb.cp().process(procs).bin({".*Run2.*3L.*_SS.*"})
        .AddSyst(cb, "Run2_SameSign_3L", "lnN", SystMap<>::init(1.10));
    cb.cp().process(procs).bin({".*Run3.*2L.*_SS.*"})
        .AddSyst(cb, "Run3_SameSign_2L", "lnN", SystMap<>::init(1.10));
    cb.cp().process(procs).bin({".*Run3.*3L.*_SS.*"})
        .AddSyst(cb, "Run3_SameSign_3L", "lnN", SystMap<>::init(1.10));
    cb.SetFlag("filters-use-regex", false);
}

void BuildFit::AddOSOFSys(const stringlist& binset, const stringlist& procs){
    cb.SetFlag("filters-use-regex", true);

    cb.cp().process(procs).bin({".*Run2.*2L.*_OS_emu.*"})
        .AddSyst(cb, "Run2_OSOF_2L", "lnN", SystMap<>::init(1.10));
    cb.cp().process(procs).bin({".*Run3.*2L.*_OS_emu.*"})
        .AddSyst(cb, "Run3_OSOF_2L", "lnN", SystMap<>::init(1.10));

    cb.cp().process(procs).bin({".*Run2.*3L.*_OSOFa.*"})
        .AddSyst(cb, "Run2_OSOF_3L", "lnN", SystMap<>::init(1.10));
    cb.cp().process(procs).bin({".*Run3.*3L.*_OSOFa.*"})
        .AddSyst(cb, "Run3_OSOF_3L", "lnN", SystMap<>::init(1.10));

    cb.SetFlag("filters-use-regex", false);
}

void BuildFit::AddOSSFSys(const stringlist& binset, const stringlist& procs){
    cb.SetFlag("filters-use-regex", true);

    cb.cp().process(procs).bin({".*Run2.*2L.*_OS_(ee|mumu).*"})
        .AddSyst(cb, "Run2_OSSF_2L", "lnN", SystMap<>::init(1.10));
    cb.cp().process(procs).bin({".*Run3.*2L.*_OS_(ee|mumu).*"})
        .AddSyst(cb, "Run3_OSSF_2L", "lnN", SystMap<>::init(1.10));

    cb.cp().process(procs).bin({".*Run2.*3L.*_OSSFa.*"})
        .AddSyst(cb, "Run2_OSSF_3L", "lnN", SystMap<>::init(1.10));
    cb.cp().process(procs).bin({".*Run3.*3L.*_OSSFa.*"})
        .AddSyst(cb, "Run3_OSSF_3L", "lnN", SystMap<>::init(1.10));

    cb.SetFlag("filters-use-regex", false);
}

void BuildFit::AddLepHemiSys(const stringlist& binset, const stringlist& procs){
    cb.SetFlag("filters-use-regex", true);
    cb.cp().process(procs).bin({".*Run2.*4L.*_31.*"})
        .AddSyst(cb, "Run2_4L_LepHemi", "lnN", SystMap<>::init(1.10));
    cb.cp().process(procs).bin({".*Run3.*4L.*_31.*"})
        .AddSyst(cb, "Run3_4L_LepHemi", "lnN", SystMap<>::init(1.10));
    cb.SetFlag("filters-use-regex", false);
}

void BuildFit::AddBtagSys(const stringlist& binset, const stringlist& procs){
    cb.SetFlag("filters-use-regex", true);

    cb.cp().process(procs).bin({".*Run2.*2L.*0J.*P250.*Btag.*"})
        .AddSyst(cb, "Run2_Btag_2L_0J_lPTISR", "lnN", SystMap<>::init(1.20));
    cb.cp().process(procs).bin({".*Run2.*2L.*0J.*P350.*Btag.*"})
        .AddSyst(cb, "Run2_Btag_2L_0J_hPTISR", "lnN", SystMap<>::init(1.20));
    cb.cp().process(procs).bin({".*Run2.*2L.*1J.*P250.*Btag.*"})
        .AddSyst(cb, "Run2_Btag_2L_1J_lPTISR", "lnN", SystMap<>::init(1.20));
    cb.cp().process(procs).bin({".*Run2.*2L.*1J.*P350.*Btag.*"})
        .AddSyst(cb, "Run2_Btag_2L_1J_hPTISR", "lnN", SystMap<>::init(1.20));
    cb.cp().process(procs).bin({".*Run2.*3L.*P200.*Btag.*"})
        .AddSyst(cb, "Run2_Btag_3L_lPTISR", "lnN", SystMap<>::init(1.20));
    cb.cp().process(procs).bin({".*Run2.*3L.*P300.*Btag.*"})
        .AddSyst(cb, "Run2_Btag_3L_hPTISR", "lnN", SystMap<>::init(1.20));
    cb.cp().process(procs).bin({".*Run2.*4L.*Btag.*"})
        .AddSyst(cb, "Run2_Btag_4L", "lnN", SystMap<>::init(1.20));

    cb.cp().process(procs).bin({".*Run3.*2L.*0J.*P250.*Btag.*"})
        .AddSyst(cb, "Run3_Btag_2L_0J_lPTISR", "lnN", SystMap<>::init(1.20));
    cb.cp().process(procs).bin({".*Run3.*2L.*0J.*P350.*Btag.*"})
        .AddSyst(cb, "Run3_Btag_2L_0J_hPTISR", "lnN", SystMap<>::init(1.20));
    cb.cp().process(procs).bin({".*Run3.*2L.*1J.*P250.*Btag.*"})
        .AddSyst(cb, "Run3_Btag_2L_1J_lPTISR", "lnN", SystMap<>::init(1.20));
    cb.cp().process(procs).bin({".*Run3.*2L.*1J.*P350.*Btag.*"})
        .AddSyst(cb, "Run3_Btag_2L_1J_hPTISR", "lnN", SystMap<>::init(1.20));
    cb.cp().process(procs).bin({".*Run3.*3L.*P200.*Btag.*"})
        .AddSyst(cb, "Run3_Btag_3L_lPTISR", "lnN", SystMap<>::init(1.20));
    cb.cp().process(procs).bin({".*Run3.*3L.*P300.*Btag.*"})
        .AddSyst(cb, "Run3_Btag_3L_hPTISR", "lnN", SystMap<>::init(1.20));
    cb.cp().process(procs).bin({".*Run3.*4L.*Btag.*"})
        .AddSyst(cb, "Run3_Btag_4L", "lnN", SystMap<>::init(1.20));

    cb.SetFlag("filters-use-regex", false);
}

void BuildFit::BuildFitSkeleton(JSONFactory* j, const std::string& signalPoint, const std::string& datacard_dir){
    std::map<std::string, double> obs_rates;
    stringlist truebkgprocs = GetBkgProcs(j);
    stringlist fakesprocs = GetFakesProcs(j);
    stringlist bkgprocs;
    bkgprocs.reserve(truebkgprocs.size() + fakesprocs.size());
    bkgprocs.insert(bkgprocs.end(), truebkgprocs.begin(), truebkgprocs.end());
    bkgprocs.insert(bkgprocs.end(), fakesprocs.begin(), fakesprocs.end());

    stringlist signalDetails = ExtractSignalDetails(signalPoint);

    // Build path for json->root output
    std::filesystem::path p(j->json_file_name);
    std::string json_to_root_file = (p.parent_path() / "datacards" / signalPoint / "json_shapes_flat.root").string();

    // 1) Write ROOT file and get the kept bins (this also fills obs_rates)
    std::cout << "Writing root file\n";
    std::set<std::string> skipped_bins;
    stringlist kept_bins = WriteJsonAsFlatHists(j, json_to_root_file, &obs_rates, signalPoint, &skipped_bins);

    if (kept_bins.empty()) {
        throw std::runtime_error("No bins kept after filtering signal-only bins. Aborting.");
    }
    std::cout << "Wrote root file\n";

    // 2) Build CH categories from the kept bin list
    ch::Categories cats = BuildCatsFromList(kept_bins);

    // 3) Register observations/processes using cats
    //cb.SetVerbosity(3);
    cb.AddObservations({"*"}, {signalDetails[0]}, {"13.6TeV"}, {signalDetails[1]}, cats);
    cb.AddProcesses({"*"}, {signalDetails[0]}, {"13.6TeV"}, {signalDetails[1]}, bkgprocs, cats, false);
    std::string sigProcPrefix = GetSignalProcName(signalPoint);
    std::string sigMass = GetSignalMass(signalPoint);
    cb.AddProcesses({sigMass}, {signalDetails[0]}, {"13.6TeV"}, {signalDetails[1]}, {sigProcPrefix}, cats, true);

    // 4) Register shape systematics only for kept bins
    AddShapeSystsFromJSON(j, kept_bins);
    std::cout << "Added shape systs\n";

    // 5) Extract shapes (backgrounds + signals for kept bins)
    cb.cp().backgrounds().ExtractShapes(json_to_root_file, "$BIN__$PROCESS", "$BIN__$PROCESS__$SYSTEMATIC");
    cb.cp().signals().ExtractShapes(json_to_root_file, "$BIN__$PROCESS_$MASS", "$BIN__$PROCESS_$MASS__$SYSTEMATIC");
    cb.FilterProcs([](ch::Process const *p){ return p->rate() <= 0; });

    // 6) Add Systematics
    // Turn on autoMCstats
    cb.cp().SetAutoMCStats(cb, 0.); // Second arg is event threshold

    // All non-triboson processes -> rateParam
    AddFakeFamiliesAsSharedNorms(
        truebkgprocs,
        fakesprocs,
        "rateParam",
        1.0,
        "triboson",
        false   // exclude triboson
    );
    
    // Only triboson processes -> lnN 1.4
    AddFakeFamiliesAsSharedNorms(
        truebkgprocs,
        fakesprocs,
        "lnN",
        1.4,
        "triboson",
        true    // include only triboson
    );

    AddFloatingNormsGroupedByFakeType(fakesprocs, "rateParam", 1.0);
    AddFAKETransferSys(kept_bins);
    AddPTISRSys(kept_bins, bkgprocs);
    AddSameSignSys(kept_bins, bkgprocs);
    AddOSOFSys(kept_bins, bkgprocs);
    AddOSSFSys(kept_bins, bkgprocs);
    AddLepHemiSys(kept_bins, bkgprocs);
    AddBtagSys(kept_bins, bkgprocs);

    cb.FilterSysts([](ch::Systematic const *s){ return s->value_u() == 1.0 && s->value_d() == 1.0; });
    std::cout << "Added all systs\n";
    //std::cout << "Printing systematics..." << std::endl; cb.PrintSysts();
    //cb.PrintAll();

    TFile* json_root_file = TFile::Open(json_to_root_file.c_str(), "UPDATE");
    if (!json_root_file || json_root_file->IsZombie()) {
        throw std::runtime_error("Cannot open " + json_to_root_file);
    }
    std::cout << "Writing datacard\n";
    cb.WriteDatacard(datacard_dir+"/"+signalPoint+"/"+signalPoint+".txt", *json_root_file);
    json_root_file->Close();
    delete json_root_file;
}

