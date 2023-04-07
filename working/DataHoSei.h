//////////////////////////////////////////////////////////
// 
// Made by Ryan Tang on 2023, April 6
// 
// 
//////////////////////////////////////////////////////////

#ifndef DataHoSei_h
#define DataHoSei_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>

// Header file for the classes stored in the TTree if any.
#include "TClonesArray.h"
#include "TObject.h"
#include "TNamed.h"
#include "TAttLine.h"
#include "TAttFill.h"
#include "TAttMarker.h"
#include "TGraph.h"

#include "Mapping.h"
#include "../armory/AnalysisLib.h"

class DataHoSei : public TSelector {
public :
  TTree          *fChain;   //!pointer to the analyzed TTree or TChain

  // Declaration of leaf types
  ULong64_t       evID;

  Float_t  **  e;   //  all kind of energy
  // ULong64_t ** e_t; //!  all kind of timestamp
  
  // Float_t   ** we;   //!  wave energy
  // Float_t   ** weT;  //!  wave time
  // Float_t   ** weR;  //!  wave rise time

  // List of branches
  TBranch        *b_evID; //!
  TBranch       **b_e;    //!
  // TBranch       **b_e_t;  //!

  DataHoSei(TTree * /*tree*/ =0) : fChain(0) {
    printf("--------- %s \n", __func__);

    e = new Float_t * [mapping::nDetType];
    b_e = new TBranch * [mapping::nDetType];
    for( int i = 0 ; i < mapping::nDetType; i++)  e[i] = new Float_t[mapping::detNum[i]];

  }
  virtual ~DataHoSei() {

    printf("--------- %s \n", __func__);

  
    printf("----------- B \n");
  }
  virtual Int_t   Version() const { return 2; }
  virtual void    Begin(TTree *tree);
  virtual void    SlaveBegin(TTree *tree);
  virtual void    Init(TTree *tree);
  virtual Bool_t  Notify();
  virtual Bool_t  Process(Long64_t entry);
  virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
  virtual void    SetOption(const char *option) { fOption = option; }
  virtual void    SetObject(TObject *obj) { fObject = obj; }
  virtual void    SetInputList(TList *input) { fInput = input; }
  virtual TList  *GetOutputList() const { return fOutput; }
  virtual void    SlaveTerminate();
  virtual void    Terminate();

  ClassDef(DataHoSei,0);

};

#endif

#ifdef DataHoSei_cxx

void DataHoSei::Init(TTree *tree){

  printf("--------- %s \n", __func__);

  // Set branch addresses and branch pointers
  if (!tree) return;
  fChain = tree;
  fChain->SetMakeClass(1);

  fChain->SetBranchAddress("evID",  &evID, &b_evID);

  fChain->SetBranchAddress(mapping::detTypeName[0].c_str(),          e[0], &b_e[0]);
  //for( int i = 0; i < mapping::nDetType; i++){
  //   fChain->SetBranchAddress((mapping::detTypeName[i] + "_t").c_str(), e_t[i], &b_e_t[i]);

  //}

  // TObjArray * branchList = fChain->GetListOfBranches();


}

void DataHoSei::SlaveBegin(TTree * /*tree*/){
  printf("--------- %s \n", __func__);
  TString option = GetOption();
}

void DataHoSei::SlaveTerminate(){
  printf("--------- %s \n", __func__);
}

Bool_t DataHoSei::Notify(){
  printf("--------- %s \n", __func__);
  return kTRUE;
}

#endif // #ifdef DataHoSei_cxx
