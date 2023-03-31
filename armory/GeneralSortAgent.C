
#include "TTree.h"
#include "TProof.h"
#include "TChain.h"

void GeneralSortAgent(Int_t runNum, int nWorker = 1, int traceMethod = 0){

  TString name;
  name.Form("../root_data/run%03d.root", runNum);

  TChain * chain = new TChain("tree");
  chain->Add(name); 

  chain->GetListOfFiles()->Print();

  printf("----------------------------\n");

  TProof * p = TProof::Open("", Form("workers=%d", abs(nWorker)));
  p->ShowCache();
  printf("----------------------------\n");

  TString option;

  //this is the option for TSelector, the first one is traceMethod, 2nd is save fileName;
  option.Form("%d,../root_data/gen_run%03d.root", traceMethod, runNum);

  chain->SetProof();
  chain->Process("../armory/GeneralSort.C+", option);



}