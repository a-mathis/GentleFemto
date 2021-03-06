#include "CATSInput.h"
#include "DreamPlot.h"
#include "DreamSystematics.h"
#include "ForgivingReader.h"
#include "CandidateCounter.h"
#include "TCanvas.h"
#include <iostream>

void EvalDreamSystematics(TString InputDir, TString prefix, float upperFitRange) {
  gROOT->ProcessLine("gErrorIgnoreLevel = 3001");
  TString filename = Form("%s/AnalysisResults.root", InputDir.Data());
  DreamPlot::SetStyle(false, true);
  auto CATSinput = new CATSInput();
  CATSinput->SetNormalization(0.240, 0.340);
  CATSinput->SetFixedkStarMinBin(true, 0. );
  const int rebin = 20;
  auto counter = new CandidateCounter();

  ReadDreamFile* DreamFile = new ReadDreamFile(6, 6);
  DreamFile->SetAnalysisFile(filename.Data(), prefix, "0");

  ForgivingReader* ForgivingFile = new ForgivingReader(filename.Data(), prefix,
                                                       "0");
  counter->SetNumberOfCandidates(ForgivingFile);
  const int nTracks = counter->GetNumberOfTracks();
  const int nv0 = counter->GetNumberOfV0s();
  counter->ResetCounter();
  //Proton - L
  DreamDist* pL = DreamFile->GetPairDistributions(0, 2, "");
  DreamDist* ApAL = DreamFile->GetPairDistributions(1, 3, "");
  DreamCF* CFpLDef = CATSinput->ObtainCFSyst(rebin, "pLDef", pL, ApAL);
  const int pairCountsDefault = CFpLDef->GetFemtoPairs(0, 0.2);
  DreamSystematics protonL(DreamSystematics::pL);
//  protonL.SetUpperFitRange(0.080);
  protonL.SetDefaultHist(CFpLDef, "hCk_ReweightedpLDefMeV_1");
  protonL.SetUpperFitRange(upperFitRange);
  int iPLCounter = 0;
  for (int i = 1; i <= protonL.GetNumberOfVars(); ++i) {
    ReadDreamFile* DreamVarFile = new ReadDreamFile(6, 6);
    DreamVarFile->SetAnalysisFile(filename.Data(), prefix, Form("%u", i));
    TString VarName = TString::Format("pLVar%u", i);
    DreamCF* CFpLVar = CATSinput->ObtainCFSyst(
        rebin, VarName.Data(), DreamVarFile->GetPairDistributions(0, 2, ""),
        DreamVarFile->GetPairDistributions(1, 3, ""));
    int femtoPairVar= CFpLVar->GetFemtoPairs(0, 0.2);
    float relDiff = (femtoPairVar-pairCountsDefault)/(float)pairCountsDefault;
    if (TMath::Abs(relDiff) > 0.2) {
      continue;
    }
    protonL.SetVarHist(CFpLVar,
                        TString::Format("Reweighted%sMeV_1", VarName.Data()));
    TString VarString = TString::Format("%u", i);
    ForgivingReader* ForgivingFile = new ForgivingReader(filename.Data(),
                                                         prefix,
                                                         VarString.Data());
    counter->SetNumberOfCandidates(ForgivingFile);
    protonL.SetPair(pairCountsDefault, CFpLVar->GetFemtoPairs(0, 0.2));
    protonL.SetParticles(nTracks, nv0, counter->GetNumberOfTracks(),
                         counter->GetNumberOfV0s());
    counter->ResetCounter();
    iPLCounter++;
  }
  protonL.EvalSystematics();
  protonL.EvalDifferenceInPairs();
  protonL.EvalDifferenceInParticles();
  protonL.WriteOutput();
  auto file = new TFile(
      Form("Systematics_%s.root", protonL.GetPairName().Data()), "update");
  CFpLDef->WriteOutput(file, true);
  std::cout << "Worked through " << iPLCounter << " variations" << std::endl;
  std::cout << "Upper fit range " << upperFitRange << std::endl;
}

int main(int argc, char* argv[]) {
  EvalDreamSystematics(argv[1], argv[2], atof(argv[3]));

  return 1;
}
