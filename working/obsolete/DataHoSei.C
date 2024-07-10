#define DataHoSei_cxx

#include "DataHoSei.h"
#include <TH2.h>
#include <TH1.h>
#include <TMath.h>
#include <TStyle.h>

//^***************************** User Settings




//******************************** end of User Settings

TH1F * h1 = new TH1F("h1", "h1", 400, 0, 50000);


void DataHoSei::Begin(TTree * /*tree*/){
  printf("--------- %s \n", __func__);
  TString option = GetOption();

}


Bool_t DataHoSei::Process(Long64_t entry){

  if( entry == 0 ) printf("--------- %s \n", __func__);

  b_e[0]->GetEntry(entry);
  //printf("%f \n", e[0][1]);

  h1->Fill(e[0][1]);

  // std::vector<std::vector<Float_t>> dataList; // {detID, e, xf, xn}
  //  for( int i = 0; i < mapping::nDetType; i++){
     std::vector<Float_t> temp;
    //  for( int j = 0; j < mapping::detNum[i]; j++){
      //  if( TMath::IsNaN( e[i][j] ) ) continue;
    //  }
  //  }

  return kTRUE;
}




void DataHoSei::Terminate(){
  printf("--------- %s \n", __func__);

  h1->Draw();



  printf("----------- A \n");
}
