
#include "TTree.h"
#include "TProof.h"
#include "TChain.h"
#include "TMacro.h"
#include "TFile.h"

void GeneralSortAgent(Int_t runNum, int nWorker = 1, int traceMethod = -1){

  TString name;
  name.Form("../root_data/run%03d.root", runNum);

  TChain * chain = new TChain("tree");
  chain->Add(name); 

  chain->GetListOfFiles()->Print();

  printf("\033[1;33m---------------------total number of Events %llu\033[0m\n", chain->GetEntries());

  //this is the option for TSelector, the first one is traceMethod, 2nd is save fileName;
  TString option;

  if( abs(nWorker) == 1){

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

  //========== open the output root and copy teh timestamp Marco

  TFile * f1 = new TFile(Form("../root_data/run%03d.root", runNum), "READ");
  TMacro * m = (TMacro* ) f1->Get("timeStamp");
  m->AddLine(Form("%d", runNum));

  TFile * f2 = new TFile(Form("../root_data/gen_run%03d.root", runNum), "UPDATE");
  f2->cd();
  m->Write("timeStamp");

  f1->Close();
  f2->Close();


}