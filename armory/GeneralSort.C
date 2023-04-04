#define GeneralSort_cxx

#include <iostream>
#include <fstream>
#include <string>

#include "GeneralSort.h"

#include <TH2.h>
#include <TStyle.h>
#include <TString.h>
#include <TSystem.h>
#include <TMath.h>


Long64_t processedEntry = 0;
float lastPercentage = 0;

//^##############################################################
Bool_t GeneralSort::Process(Long64_t entry){

  if( entry < 1 ) printf("============================== start processing data\n");

  ///initialization
  for( int i = 0; i < nDetType; i++){
    for( int j = 0; j < detNum[i]; j++){
      eE[i][j] = TMath::QuietNaN();
      eT[i][j] = 0;
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
    int detType = FindDetType(detID, detMaxID);

    int low = (i == 0 ? 0 : detMaxID[detType-1]);

    int reducedDetID = detID - low;

    eE[detType][reducedDetID] = e[i] * detParity[detType];
    eT[detType][reducedDetID] = e_t[i];

  }


  if(  isTraceExist && traceMethod >= 0 ){

    b_tl->GetEntry(entry);
    b_trace->GetEntry(entry);

    int countTrace = 0;

    arr->Clear("C");

    for( int i = 0; i < multi; i++){
      int detID = mapping[bd[i]][ch[i]];

      int traceLength = tl[i];
      gTrace = (TGraph*) arr->ConstructedAt(countTrace, "C");
      gTrace->Clear();         
      gTrace->Set(traceLength);         
         
      gTrace->SetTitle(Form("ev:%llu,nHit:%d,id:%d,len:%d", evID, i, detID, traceLength));
      countTrace ++;

      for( int k = 0 ; k < traceLength; k++){
        gTrace->SetPoint(k, k, trace[i][k]);
      }

    }

  }

  if( !isParallel){
    processedEntry ++;
    float percentage = processedEntry*100/NumEntries;
    if( percentage > lastPercentage + 1.0) {
      printf("Processed : %lld, %.0f%% \n\033[A\r", entry, percentage);
      lastPercentage = percentage;
    }
  }

  newTree->Fill();

  return kTRUE;
}

//^##############################################################
void GeneralSort::Terminate(){

  printf("========================= %s\n", __func__);

  DecodeOption();

  if( !isParallel){
    printf("%s::SaveTree %p, %p\n", __func__, saveFile, newTree);
    saveFile->cd();
    newTree->Write();
    saveFile->Close();
  }

  printf("=======================================================\n");
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

  PrintMapping(mapping, detTypeName, detMaxID);

  DecodeOption();
  if(!isParallel) tree->GetEntriesFast();

}

void GeneralSort::SlaveBegin(TTree * /*tree*/){
  printf("%s\n", __func__);

}

void GeneralSort::SlaveTerminate(){

  printf("\n%s\n", __func__);

  if( isParallel){
    printf("%s::SaveTree\n", __func__);
    saveFile->cd();
    newTree->Write();
    fOutput->Add(proofFile);
    saveFile->Close();
  }
  
}