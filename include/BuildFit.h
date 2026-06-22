#ifndef BUILDFIT_H
#define BUILDFIT_H

#include "JSONFactory.h"
#include "BuildFitTools.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <filesystem>

#include <TFile.h>
#include <TH1F.h>

#include "CombineHarvester/CombineTools/interface/CombineHarvester.h"
#include "CombineHarvester/CombineTools/interface/Observation.h"
#include "CombineHarvester/CombineTools/interface/Process.h"
#include "CombineHarvester/CombineTools/interface/Utilities.h"
#include "CombineHarvester/CombineTools/interface/Systematics.h"
#include "CombineHarvester/CombineTools/interface/BinByBin.h"
using ch::syst::SystMap;
using ch::syst::SystMapFunc;
using ch::syst::bin;
using json = nlohmann::json;
using ProcMap = std::map<std::string, std::set<std::string>>;

class BuildFit{
    
    public:
    ch::CombineHarvester cb{};
    ch::Categories BuildCats(JSONFactory* j);
    std::string SanitizeName(const std::string &s);
    ch::Categories BuildCatsFromList(const stringlist& bin_names);
    stringlist WriteJsonAsFlatHists(JSONFactory* j, const std::string &outFile, std::map<std::string,double>* out_obs_rates = nullptr, const std::string& sig = "", std::set<std::string>* skipped_bins = nullptr);
    stringlist GetBkgProcs(JSONFactory* j);
    stringlist GetFakesProcs(JSONFactory* j);
    bool HasDataObs(JSONFactory* j);
    bool HasProcFAKES(JSONFactory* j, const std::string& check_proc);
    //bool IsRun2(const std::string& str);
    //bool IsRun3(const std::string& str);
    std::string GetSignalMass(const std::string& sig);
    std::string GetSignalProcName(const std::string &proc);
    std::vector<std::string> ExtractSignalDetails( std::string signalPoint);
    std::vector<std::string> GetBinSet( JSONFactory* j);
    void AddFloatingNorms(stringlist bkgprocs, const std::string& type = "lnN", double val = 1.2);
    void AddSharedFloatingNorm(const stringlist& procs, const std::string& nuis_name, const std::string& type, double val);
    void AddFloatingNormsGroupedByFakeType(const stringlist& fakesprocs, const std::string& type, double val);
    void AddFakeFamiliesAsSharedNorms(const std::vector<std::string>& truebkgprocs, const std::vector<std::string>& fakesprocs, const std::string& type, double val, const std::string& match_token = "", bool require_match = false);
    void BuildAsimovData(std::map<std::string, double>& obs_rates, JSONFactory* j);
    void BuildFitSkeleton(JSONFactory* j, const std::string &signalPoint, const std::string &datacard_dir);
    void AddMCStatBinByBin(JSONFactory* j);
    void AddMCStatProcByProc(const std::string& bin, JSONFactory* j);
    void AddPTISRSys(const stringlist& binset, const stringlist& procs);
    void AddSameSignSys(const stringlist& binset, const stringlist& procs);
    void AddOSOFSys(const stringlist& binset, const stringlist& procs);
    void AddOSSFSys(const stringlist& binset, const stringlist& procs);
    void AddLepHemiSys(const stringlist& binset, const stringlist& procs);
    void AddRaSys(const stringlist& binset, const stringlist& procs);
    void AddZSys(const stringlist& binset, const stringlist& procs);
    void AddBtagSys(const stringlist& binset, const stringlist& procs);
    void AddShapeSystsFromJSON(JSONFactory* j, const std::vector<std::string>& kept_bins);
    void AddFAKETransferSys(const stringlist& binset);
    void AddSilverBronzeGlobal(const stringlist& procs);
    std::string GetMatchingBin(const std::string& bin, const std::string& token, const std::string& match_token);
    std::vector<std::string> sigkeys = { "Cascades", "SMS" };

    private:
    const stringlist fakes_skip_list = {"ZInv", "QCD", "Vfakeleps", "Wjets"};

};
#endif
