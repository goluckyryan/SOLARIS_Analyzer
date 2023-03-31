#define GeneralSort_cxx

#include <iostream>
#include <fstream>
#include <string>

#include "GeneralSort.h"

#include <TH2.h>
#include <TStyle.h>
#include <TString.h>
#include <TMath.h>

//^##############################################################
Bool_t GeneralSort::Process(Long64_t entry){

  if( entry < 2 ) printf("%s %lld\n", __func__, entry);

  ///initialization
  for( int i = 0; i < detNum[0]; i++){
    eE[i] = TMath::QuietNaN();
    xf[i] = TMath::QuietNaN();
    xn[i] = TMath::QuietNaN();
    eT [i] = 0;
    xfT[i] = 0;
    xnT[i] = 0;

    if( isTraceExist && traceMethod > 0 ){
      teE[i] = TMath::QuietNaN();
      teT[i] = TMath::QuietNaN();
      teR[i] = TMath::QuietNaN();
    }
  }

  for( int i = 0; i < detNum[1]; i++){
    rdt[i]  = TMath::QuietNaN();
    rdtT[i] = 0;
    if( isTraceExist && traceMethod > 0 ){
      trdt[i]  = TMath::QuietNaN();
      trdtT[i] = TMath::QuietNaN();
      trdtR[i] = TMath::QuietNaN();
    }
  }
  
  multi = 0;
  b_event_ID->GetEntry(entry);
  b_multi->GetEntry(entry);
  b_bd->GetEntry(entry);
  b_ch->GetEntry(entry);
  b_e->GetEntry(entry);
  b_e_t->GetEntry(entry);


  for( int i = 0 ; i < multi; i++){    
    int detID = mapping[bd[i]][ch[i]];
    int kindIndex = -1;

    for( int g = 0; g < (int) detMaxID.size(); g ++){
      int low = ( g == 0 ? 0 : detMaxID[g-1]);
      int high = detMaxID[g];

      if(  low <= detID && detID  < high ){
        
        //************** array
        if( g == 0 ){
          kindIndex = (kindIndex + 1) % 3;
          switch (kindIndex){
            case 0 : { eE[detID] = e[i] * detParity[g];  eT[detID] = e_t[i]; } break;
            case 1 : { xf[detID] = e[i] * detParity[g]; xfT[detID] = e_t[i]; } break;
            case 2 : { xn[detID] = e[i] * detParity[g]; xnT[detID] = e_t[i]; } break;
          }
        }

        //************** rdt
        if( g == 1 ){
          detID = detID - detMaxID[g-1];
          rdt[detID] = e[i] * detParity[g];  rdtT[detID] = e_t[i]; 
        }

      }

    }
  }


  if(  isTraceExist && traceMethod >= 0 ){

    int countTrace = 0;

    arr->Clear("C");

    for( int i = 0; i < multi; i++){
      int detID = mapping[bd[i]][ch[i]];

    }


  }


  newTree->Fill();

  return kTRUE;
}

//^##############################################################
void GeneralSort::Terminate(){

  printf("========================= %s\n", __func__);

  TString option = GetOption();
  TObjArray * tokens = option.Tokenize(",");
  traceMethod = ((TObjString*) tokens->At(0))->String().Atoi();
  saveFileName = ((TObjString*) tokens->At(1))->String();
  
  //get entries
  saveFile = TFile::Open(saveFileName);
  if( saveFile->IsOpen() ){
    TTree * tree = (TTree*) saveFile->FindObjectAny("gen_tree");
    int validCount = tree->GetEntries();
  
    saveFile->Close();
  
    printf("=======================================================\n");
    PrintTraceMethod();
    printf("----- saved as \033[1;33m%s\033[0m. valid event: %d\n", saveFileName.Data() , validCount); 
  }
}

//^##############################################################
void GeneralSort::Begin(TTree * tree){

  printf( "=================================================================\n");
  printf( "=====================   SOLARIS GeneralSort.C   =================\n");
  printf( "=================================================================\n");

  PrintMapping(mapping, detName, detMaxID);

}

void GeneralSort::SlaveBegin(TTree * /*tree*/){
  printf("%s\n", __func__);

}

void GeneralSort::SlaveTerminate(){
  printf("%s\n", __func__);
  saveFile->cd();
  newTree->Write();
  fOutput->Add(proofFile);
  saveFile->Close();
}