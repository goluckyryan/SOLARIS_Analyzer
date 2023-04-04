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

      if( isTraceExist && traceMethod > 0){
        teE[i][j] = TMath::QuietNaN();
        teT[i][j] = TMath::QuietNaN();
        teR[i][j] = TMath::QuietNaN();
      }
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

  //@===================================== Trace
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

      //***=================== fit
      if( traceMethod == 1){
        gFit  = new TF1("gFit", fitFunc, 0, traceLength, numPara);
        gFit->SetLineColor(6);
        gFit->SetRange(0, traceLength);

        gFit->SetParameter(0, e[i]);
        gFit->SetParameter(1, 100);   //triggerTime //TODO how to not hardcode?
        gFit->SetParameter(2, 10);    //rise time   //TODO how to not hardcode?
        gFit->SetParameter(3, trace[i][0]);  //base line 
        gFit->SetParameter(4, 100);   // decay  //TODO how to not hardcode?
        gFit->SetParameter(5, -0.01); // pre-rise slope //TODO how to not hardcode?

        gFit->SetParLimits(1, 85, 125); //raneg for the trigger time
        gFit->SetParLimits(5, -2, 0);

        gTrace->Fit("gFit", "QR", "", 0, traceLength);


        int detType = FindDetType(detID, detMaxID);
        int low = (i == 0 ? 0 : detMaxID[detType-1]);
        int reducedDetID = detID - low;

        teE[detType][reducedDetID] = gFit->GetParameter(0);
        teT[detType][reducedDetID] = gFit->GetParameter(1);
        teR[detType][reducedDetID] = gFit->GetParameter(2);

        delete gFit;
        gFit = nullptr;
      }

      //***=================== Trapezoid filter
      if( traceMethod == 2){
        //TODO
      }

    }

  }

  if( !isParallel){
    processedEntry ++;
    float percentage = processedEntry*100/NumEntries;
    if( percentage >= lastPercentage ) {
      TString msg; msg.Form("%lu", NumEntries);
      int len = msg.Sizeof();
      printf("Processed : %*lld, %3.0f%% | Elapsed %6.1f sec | expect %6.1f sec\n\033[A\r", len, entry, percentage, stpWatch.RealTime(), stpWatch.RealTime()/percentage*100);
      stpWatch.Start(kFALSE);
      lastPercentage = percentage + 1.0;
      if( lastPercentage >= 100)  printf("\n");

    }
  }

  newSaveTree->Fill();

  return kTRUE;
}

//^##############################################################
void GeneralSort::Terminate(){

  printf("=============================== %s\n", __func__);

  DecodeOption();

  if( !isParallel){
    stpWatch.Start(kFALSE);
    saveFile->cd();
    newSaveTree->Print("toponly");
    newSaveTree->Write();
    saveFile->Close();
  }

  //get entries
  saveFile = TFile::Open(saveFileName);
  if( saveFile->IsOpen() ){
    TTree * tree = (TTree*) saveFile->FindObjectAny("gen_tree");
    int validCount = tree->GetEntries();
  
    saveFile->Close();
  
    printf("=========================================================================\n");
    PrintTraceMethod();
    printf("----- saved as \033[1;33m%s\033[0m. valid event: %d\n", saveFileName.Data() , validCount); 
    printf("=========================================================================\n");
  }
}

//^##############################################################
void GeneralSort::Begin(TTree * tree){

  printf( "=================================================================\n");
  printf( "=====================   SOLARIS GeneralSort.C   =================\n");
  printf( "=================================================================\n");

  PrintMapping(mapping, detTypeName, detMaxID);

  DecodeOption();
  if(!isParallel) {
    tree->GetEntriesFast();
    stpWatch.Start();
  }

}

void GeneralSort::SlaveBegin(TTree * /*tree*/){

}

void GeneralSort::SlaveTerminate(){
  if( isParallel){
    printf("%s::SaveTree\n", __func__);
    saveFile->cd();
    newSaveTree->Write();
    fOutput->Add(proofFile);
    saveFile->Close();
  }
  
}