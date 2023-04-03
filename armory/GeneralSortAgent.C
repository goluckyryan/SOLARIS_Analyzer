
#include "TTree.h"
#include "TProof.h"
#include "TChain.h"

void GeneralSortAgent(Int_t runNum, int nWorker = 1, int traceMethod = -1){

  TString name;
  name.Form("../root_data/run%03d.root", runNum);

  TChain * chain = new TChain("tree");
  chain->Add(name); 

  chain->GetListOfFiles()->Print();

  printf("----------------------------\n");

  //this is the option for TSelector, the first one is traceMethod, 2nd is save fileName;
  TString option;

  if( nWorker == 1){

    option.Form("%d,../root_data/gen_run%03d.root,%d", traceMethod, runNum, 0);
    chain->Process("../armory/GeneralSort.C+", option);

  }else{

    TProof * p = TProof::Open("", Form("workers=%d", abs(nWorker)));
    p->ShowCache();
    printf("----------------------------\n");

    chain->SetProof();
    option.Form("%d,../root_data/gen_run%03d.root,%d", traceMethod, runNum, 1);
    chain->Process("../armory/GeneralSort.C+", option);
  }


}