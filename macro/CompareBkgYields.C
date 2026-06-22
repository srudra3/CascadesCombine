#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "TCanvas.h"
#include "TGraph.h"
#include "TLine.h"
#include "TAxis.h"
#include "TApplication.h"

void CompareBkgYields(const std::string& fileA,
                     const std::string& fileB)
{
    vector<std::string> types = {
        "events",
        "err",
        "perc_events",
        "perc_err",
    };

    for (int i = 0; i < int(types.size()); i++) {

        std::ifstream finA(fileA);
        std::ifstream finB(fileB);

        if (!finA.is_open() || !finB.is_open()) {
            std::cerr << "Failed to open input files\n";
            return;
        }

        std::vector<double> xvals;
        std::vector<double> yvals;
        int binnumA, binnumB;
        std::string binnameA, binnameB;
        double eventsA, errA;
        double eventsB, errB;
        int dataA, dataB;
        int maxEvents = 5000;

        while (finA >> binnumA >> binnameA >> eventsA >> errA >> dataA &&
               finB >> binnumB >> binnameB >> eventsB >> errB >> dataB)
        {
            if (binnameA != binnameB) {
                std::cerr << "Bin mismatch:\n"
                          << "  " << binnameA << "\n"
                          << "  " << binnameB << "\n";
                return;
            }
            if(eventsA > maxEvents || eventsB > maxEvents) continue;
            if(types[i] == "events") {
                xvals.push_back(eventsA);
                yvals.push_back(eventsB);
            }
            else if(types[i] == "err") {
                xvals.push_back(errA);
                yvals.push_back(errB);
            }
            else if(types[i] == "perc_events") {
                xvals.push_back(eventsA);
                yvals.push_back((eventsB - eventsA)/eventsA*100.);
            }
            else if(types[i] == "perc_err") {
                xvals.push_back(errA);
                yvals.push_back(100.*(errB - errA)/errA);
            }
        }

        TCanvas *c = new TCanvas(("c_"+types[i]).c_str(), "Comparison", 800, 800);

        TGraph *g = new TGraph(xvals.size(),
                               xvals.data(),
                               yvals.data());

        if(types[i] == "events")
            g->SetTitle("Background Yield Comparison;File A Yield;File B Yield");
        else if(types[i] == "err")
            g->SetTitle("Background Yield Comparison;File A Err;File B Err");
        else if(types[i] == "perc_events")
            g->SetTitle("Background Yield Comparison;File A Yield;Percent Change Err");
        else if(types[i] == "perc_err")
            g->SetTitle("Background Yield Comparison;File A Err;Percent Change Err");

        g->SetMarkerStyle(20);
        g->SetMarkerSize(0.8);

        g->Draw("AP");

        double xmin = std::min(g->GetXaxis()->GetXmin(),
                               g->GetYaxis()->GetXmin());
        double xmax = std::max(g->GetXaxis()->GetXmax(),
                               g->GetYaxis()->GetXmax());

        TLine *line;
        if(types[i] == "events" || types[i] == "err")
            line = new TLine(xmin, xmin, xmax, xmax);
        else
            line = new TLine(xmin, 0., xmax, 0.);
        if(types[i] != "perc_err") {
            line->SetLineStyle(2);
            line->Draw();
        }

        c->SaveAs(("yield_comparison_"+types[i]+".pdf").c_str());
        delete line;
        delete g;
        delete c;
        finA.close();
        finB.close();

    }
    gApplication->Terminate();
}
