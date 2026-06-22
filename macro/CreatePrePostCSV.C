#include "TApplication.h"

// based on https://github.com/Jphsx/ChisqDiagnostic/tree/master
void CreatePrePostCSV(std::string inputDiagnostic, std::string csvname="prefit.csv"){
	TFile* fitDiagnosticFile = TFile::Open(inputDiagnostic.c_str());
	//generate keylist of regions
	TDirectory* shapes_prefit =(TDirectory*) fitDiagnosticFile->Get("shapes_prefit");
	TDirectory* shapes_fit_b = (TDirectory*) fitDiagnosticFile->Get("shapes_fit_b");
	TList* keyList = shapes_prefit->GetListOfKeys();
	//keyList->Print();
	//keyList = keyList[0].GetListOfKeys();
	
	TH1F *b_prefit, *b_postfit;
	int Nbins;
	TGraphAsymmErrors *data;

        std::string zscore_name = csvname;
        zscore_name.replace(zscore_name.rfind(".csv"), 4, "_zscore.txt");       	
        ofstream zscore_file;
        zscore_file.open (zscore_name);
	ofstream csvfile;
  	csvfile.open (csvname);
        csvfile<<"RegionName BinNumber bprefit bprefit_err bpostfit bpostfit_err data data_err"<<"\n";

	for(int i=0; i<keyList->GetSize(); i++){
		//do prefit
		
		b_prefit =(TH1F*)  shapes_prefit->Get( (std::string(keyList->At(i)->GetName())+"/total_background").c_str());
		data =(TGraphAsymmErrors*)  shapes_prefit->Get( (std::string(keyList->At(i)->GetName())+"/data").c_str());
		
		//do postfit
		b_postfit =(TH1F*)  shapes_fit_b->Get( (std::string(keyList->At(i)->GetName())+"/total_background").c_str());

		Nbins = b_prefit->GetNbinsX();
		//std::cout<<Nbins<<" ";
		for(int j=1; j<=Nbins; j++){
			csvfile<<keyList->At(i)->GetName()<<" "<<j<<" ";
			csvfile<<b_prefit->GetBinContent(j)<<" "<<b_prefit->GetBinError(j)<<" ";
			csvfile<<b_postfit->GetBinContent(j)<<" "<<b_postfit->GetBinError(j)<<" ";
			csvfile<<data->GetPointY(j-1)<<" "<<data->GetErrorY(j-1)<<"\n";

                        zscore_file<<i+1<<" ";
                        zscore_file<<keyList->At(i)->GetName()<<" ";
                        zscore_file<<b_postfit->GetBinContent(j)<<" "<<b_postfit->GetBinError(j)<<" ";
                        zscore_file<<data->GetPointY(j-1)<<"\n";
		}
	}
	csvfile.close();
        zscore_file.close();
	//b_postfit->Draw();
	gApplication->Terminate();
}
