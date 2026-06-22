// PlotFitDiagnostics.cpp
#include "PlottingTools.h"

int main(int argc, char* argv[]) {

    string inputFile;
    string treeName = "shapes_prefit";
    string patternFile;
    loadFormatMaps();

    for(int i=1;i<argc;++i){
        string arg = argv[i];

        if(arg=="-i"||arg=="--input") {
            if(i+1 < argc) inputFile = argv[++i];
            else { cerr<<"[ERROR PlotFitDiagnostics] Missing value for "<<arg<<endl; return 1; }
        }
        else if(arg=="-o"||arg=="--output") {
            if(i+1 < argc) outputDir = argv[++i];
            else { cerr<<"[ERROR PlotFitDiagnostics] Missing value for "<<arg<<endl; return 1; }
        }
        else if(arg=="-t"||arg=="--tree") {
            if(i+1 < argc) treeName = argv[++i]; // shapes_prefit or shapes_fit_b
            else { cerr<<"[ERROR PlotFitDiagnostics] Missing value for "<<arg<<endl; return 1; }
        }
        else if(arg=="--config") {
            if(i+1<argc) patternFile = argv[++i];
            else { cerr<<"[ERROR PlotFitDiagnostics] Missing value for "<<arg<<endl; return 1; }
        }
        else if(arg=="--help"){
            cout << "Usage: " << argv[0] << "\n"
                 << "  -i  <fitDiagnostics.root>\n"
                 << "  -o  <outputDir>\n"
                 << "  -t  <tree: shapes_prefit or shapes_fit_b>\n"
                 << "  -b  <bin pattern, can be wildcard> (repeatable)\n"
                 << "  --patternFile <file with bin patterns>\n"
                 << "\n";
            return 0;
        }
        else {
            cerr<<"[ERROR PlotFitDiagnostics] Unknown arg "<<arg<<endl;
            return 1;
        }
    }

    if(inputFile.empty()){
        cerr<<"[ERROR PlotFitDiagnostics] No input file provided.\n";
        return 1;
    }

    TFile* fIn = TFile::Open(inputFile.c_str(), "READ");
    if(!fIn || fIn->IsZombie()){
        cerr<<"[ERROR PlotFitDiagnostics] Cannot open file: "<<inputFile<<endl;
        return 1;
    }

    TDirectory* fitDir = (TDirectory*)fIn->Get(treeName.c_str());
    if(!fitDir){
        cerr<<"[ERROR PlotFitDiagnostics] Tree "<<treeName<<" not found in file.\n";
        return 1;
    }

    // Ensure output directory exists
    if(outputDir.back() != '/') outputDir += '/';
    gSystem->mkdir(outputDir.c_str(), kTRUE);
    gSystem->mkdir((outputDir+"pdfs").c_str(), kTRUE);

    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);

    vector<string> allBinNames;
    {
        TIter next(fitDir->GetListOfKeys());
        TKey* key;
        while((key = (TKey*)next())){
            TObject* obj = key->ReadObj();
            if(obj->InheritsFrom(TDirectoryFile::Class())) {
                allBinNames.push_back(obj->GetName());
            }
        }
    }

    YamlConfig cfg = LoadYamlConfig(patternFile);
    auto binLookup = BuildBinLookup(cfg);
    auto risrNeighborMap = BuildRISRNeighborMap(cfg);

    if (patternFile.empty()) { std::cout << "[PlotFitDiagnostics] NEED TO SUPPLY CONFIG FILE FOR RULES\n"; return 1; }
    vector<MergedBinGroup> groups = BuildMergedBinGroupsFromYaml(allBinNames, cfg);
    for (const auto& g : groups)
        gSystem->mkdir((outputDir+"pdfs/"+SanitizeString(g.group_name)).c_str(), kTRUE);
    
    for (const auto& grp : groups) {
        CombinedBinHists mergedHists = LoadAndCombineBinHists(
            fitDir, grp.group_name, grp.bin_names, cfg.process_merges);
        PlotMergedStack(treeName+"_"+grp.group_name, mergedHists, grp.pattern, grp.bin_names, binLookup, risrNeighborMap, &grp.pattern);
    }
    cout << "[PlotFitDiagnostics] All plots saved in: " << outputDir << endl;
    return 0;
}
