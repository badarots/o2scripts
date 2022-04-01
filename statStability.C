#include "DataFormatsTPC/CalibdEdxCorrection.h"
#include "DataFormatsTPC/TrackTPC.h"
#include "TPCCalibration/CalibdEdx.h"

#include "TFile.h"
#include "TTree.h"

#include <array>
#include <chrono>
#include <cmath>
#include <sstream>
#include <string_view>
#include <vector>

struct CalibOpt {
  float mindEdx = 20;
  float maxdEdx = 90;
  float field = 2;
  float minP = 0.3;
  float maxP = 0.7;
  float minClusters = 60;
  int dEdxBins = 60;
  int angBins = 36;
  int entriesSector = 1;
  int entries1D = 1;
  int entries2D = 1;
  int passes = 3;
  float outlierCut = 0.2;
  bool fitSnp = false;
};

void statStability(std::string input_file = "tpctracks.root",
                   CalibOpt opt = CalibOpt{}) {
  using namespace o2::tpc;
  using std::chrono::duration;
  using std::chrono::high_resolution_clock;

  std::vector<TrackTPC> *tracks = nullptr;

  TFile file(input_file.c_str());
  auto tree = file.Get<TTree>("tpcrec"); // change to events or tpcrec
  tree->SetBranchAddress("TPCTracks", &tracks);

  int nTracks = 0;
  int nTFs = 0;
  int minEntries = 0;
  float fillTime = 0;
  float fitTime = 0;
  CalibdEdxCorrection corr{};

  TFile outFile("calibStat.root", "recreate");
  TTree calibTree("calibStat", "CalibStat");
  calibTree.Branch("tfs", &nTFs);
  calibTree.Branch("tracks", &nTracks);
  calibTree.Branch("minEntries", &minEntries);
  calibTree.Branch("fillTime", &fillTime);
  calibTree.Branch("fitTime", &fitTime);
  calibTree.Branch("corr", &corr);

  std::cout << "Total number of TF: " << tree->GetEntries() << "\n";
  int minTfs = 100;
  int maxDiv = tree->GetEntries() / minTfs;
  for (int step = 1; step < maxDiv; step *= 2) {
    CalibdEdx calibrator(opt.dEdxBins, opt.mindEdx, opt.maxdEdx, opt.angBins,
                         opt.fitSnp);

    calibrator.setCuts({opt.minP, opt.maxP, opt.minClusters});
    calibrator.setApplyCuts(true);
    calibrator.setSectorFitThreshold(opt.entriesSector);
    calibrator.set1DFitThreshold(opt.entries1D);
    calibrator.set2DFitThreshold(opt.entries2D);
    calibrator.setElectronCut(opt.outlierCut, opt.passes);
    calibrator.setField(opt.field);

    nTracks = 0;
    nTFs = 0;

    auto start = high_resolution_clock::now();
    for (int i = 0; i < tree->GetEntries(); i += step) {
      tree->GetEntry(i);
      calibrator.fill(*tracks);
      nTracks += tracks->size();
      ++nTFs;
    }
    fillTime = duration<float, std::milli>(high_resolution_clock::now() - start)
                   .count();

    start = high_resolution_clock::now();
    calibrator.finalize();
    fitTime = duration<float, std::milli>(high_resolution_clock::now() - start)
                  .count();

    minEntries = calibrator.minStackEntries();
    corr = calibrator.getCalib();
    calibTree.Fill();

    std::ostringstream treeName{};
    treeName << "stattree-" << nTFs << ".root";
    calibrator.writeTTree(treeName.str().c_str());

    std::cout << "TFs: " << nTFs << "\n"
              << "Tracks: " << nTracks << "\n"
              << "Min. Entries: " << minEntries << "\n"
              << "Fill time: " << fillTime * 1e-3 << " s\n"
              << "Fill time per track: " << fillTime * 1e3 / nTracks << " us\n"
              << "Fit time: " << fitTime * 1e-3 << " s\n";
  }
  outFile.Write();
}
