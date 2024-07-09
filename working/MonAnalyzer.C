
#include "../Armory/AnalysisLib.h"
#include "../Armory/ClassDetGeo.h"
#include "../Armory/ClassReactionConfig.h"
#include "../Armory/ClassCorrParas.h"
// #include "../Cleopatra/ClassHelios.h"
#include "../Cleopatra/ClassTransfer.h"

#include "ClassMonPlotter.h"
#include "Mapping.h"

#include "TFile.h"
#include "TChain.h"
#include "TH1F.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"
#include "TClonesArray.h"
#include "TGraph.h"
#include "TH2.h"
#include "TStyle.h"
#include "TStopwatch.h"
#include "TMath.h"

#include "vector"
//^############################################ User setting
int rawEnergyRange[2] = {   100,    60000};       /// share with e, xf, xn
int    energyRange[2] = {     0,      10};       /// in the E-Z plot
int     rdtDERange[2] = {     0,      80}; 
int      rdtERange[2] = {     0,      80};
int   thetaCMRange[2] = {0, 80};

double     exRange[3] = {  100,    -2,     10};  /// bin [keV], low[MeV], high[MeV]


//---Gate
bool isTimeGateOn     = true;
int timeGate[2]       = {-20, 12};             /// min, max, 1 ch = 10 ns
double eCalCut[2]     = {0.5, 50};             /// lower & higher limit for eCal
double xGate          = 0.9;                  ///cut out the edge
double thetaCMGate    = 10;                    /// deg

std::vector<int> skipDetID = {11, 16, 23} ;//{2,  11, 17}

TString rdtCutFile1 = "";
TString rdtCutFile2 = "";
TString ezCutFile   = "";//"ezCut.root";

//^############################################ end of user setting

MonPlotter ** plotter = nullptr;
int numGeo = 1;

void MonAnalyzer(){

  printf("#####################################################################\n");
  printf("#######################      MonAnalyzer.C    #######################\n");
  printf("#####################################################################\n");

  TChain *chain = new TChain("gen_tree");
  chain->Add("../root_data/gen_run043.root");

  TObjArray * fileList = chain->GetListOfFiles();
  printf("\033[0;31m========================================== Number of Files : %2d\n",fileList->GetEntries());
  fileList->Print();
  printf("========================================== Number of Files : %2d\033[0m\n",fileList->GetEntries());

  printf("///////////////////////////////////////////////////////////////////\n");
  printf("            Total Number of entries : %llu \n", chain->GetEntries());
  printf("///////////////////////////////////////////////////////////////////\n");

  TTreeReader reader(chain);

  TTreeReaderValue<ULong64_t>  evID = {reader, "evID"};
  TTreeReaderArray<Float_t>     e   = {reader, "e"};
  TTreeReaderArray<ULong64_t>   e_t = {reader, "e_t"};
  TTreeReaderArray<Float_t>      xf = {reader, "xf"};
  TTreeReaderArray<Float_t>      xn = {reader, "xn"};

  TTreeReaderArray<Float_t>     rdt   = {reader, "rdt"};
  TTreeReaderArray<ULong64_t>   rdt_t = {reader, "rdt_t"};

  //TODO
  // TTreeReaderArray<TGraph>   array = {reader, "trace"};

  //*==========================================
  ULong64_t NumEntries = chain->GetEntries();
  CorrParas * corr = new CorrParas;
  corr->LoadAllCorrections();
  corr->CheckCorrParasSize(mapping::NARRAY, mapping::NRDT);

  DetGeo * detGeo = new DetGeo("detectorGeo.txt");
  // TransferReaction * transfer = new TransferReaction("reactionConfig.txt");

  numGeo = detGeo->numGeo;
  printf("================== num. of Arrays : %d\n", numGeo);

  plotter = new MonPlotter *[numGeo];
  for( int i = 0; i < numGeo; i++ ) {
    plotter[i] = new MonPlotter(i, detGeo, mapping::NRDT);
    plotter[i]->SetUpCanvas("haha", 500, 3, 2);
    plotter[i]->SetUpHistograms(rawEnergyRange, energyRange, exRange, thetaCMRange, rdtDERange, rdtERange);
  }

  //TODO make the data class. 
  double eCal[mapping::NARRAY];
  double xfCal[mapping::NARRAY];
  double xnCal[mapping::NARRAY];
  double x[mapping::NARRAY];
  double xCal[mapping::NARRAY];
  double z[mapping::NARRAY];

  //^###########################################################
  //^ * Process
  //^###########################################################
  ULong64_t processedEntries = 0;
  float Frac = 0.1;
  TStopwatch StpWatch;
  StpWatch.Start();

  while (reader.Next()) {

    // printf("%llu | %llu | %lu\n", processedEntries, *evID, e.GetSize());

    //*============================================= Array;
    int arrayMulti[numGeo] ; //array multiplicity, when any  is calculated. 
    int zMulti[numGeo] ; //array multiplicity, when z is calculated. 
    for( int i = 0; i < numGeo; i++ ) {
      arrayMulti[i] = 0;
      zMulti[i] = 0;
    }

    for( int id = 0; id < e.GetSize() ; id++ ){
      short aID = detGeo->GetArrayID(id);
      if( aID < 0 ) continue; 

      //@================== Filling raw data
      plotter[aID]->he[id]->Fill(e[id]);
      plotter[aID]->hxf[id]->Fill(xf[id]);
      plotter[aID]->hxn[id]->Fill(xn[id]);
      plotter[aID]->hxf_xn[id]->Fill(xf[id],xn[id]);
      plotter[aID]->he_xs[id]->Fill(xf[id]+xn[id], e[id]);
      
      plotter[aID]->he_ID->Fill(id, e[id]);
      plotter[aID]->hxf_ID->Fill(id, xf[id]);
      plotter[aID]->hxn_ID->Fill(id, xn[id]);

      //@==================== Basic gate
      if( TMath::IsNaN(e[id]) ) continue ; 
      if( TMath::IsNaN(xn[id]) &&  TMath::IsNaN(xf[id]) ) continue ; 
      
      //@==================== Skip detector 
      bool skipFlag = false;
      for( unsigned int kk = 0; kk < skipDetID.size() ; kk++){
          if( id == skipDetID[kk] ) {
            skipFlag = true;
            break;
          }
      }
      if (skipFlag ) continue;

      //@==================== When e, xn, or xf is valid.
      arrayMulti[aID] ++;

      //@==================== Calibrations go here
      if( corr->xnCorr.size()  && corr->xfxneCorr.size() ) xnCal[id] = xn[id] * corr->xnCorr[id] * corr->xfxneCorr[id][1] + corr->xfxneCorr[id][0];
      if( corr->xfxneCorr.size()  )                        xfCal[id] = xf[id] * corr->xfxneCorr[id][1] + corr->xfxneCorr[id][0];
      if( corr->eCorr.size() )                             eCal[id]  = e[id] / corr->eCorr[id][0] + corr->eCorr[id][1];

      if( eCal[id] < eCalCut[0] ||  eCalCut[1] < eCal[id] ) continue;

      //@===================== fill Calibrated  data
      plotter[aID]->heCal[id]->Fill(eCal[id]);
      plotter[aID]->hxfCal_xnCal[id]->Fill(xfCal[id], xnCal[id]);
      plotter[aID]->he_xsCal[id]->Fill(xnCal[id] + xfCal[id], e[id]);

      //@===================== calculate X
      if( (xf[id] > 0 || !TMath::IsNaN(xf[id])) && ( xn[id] > 0 || !TMath::IsNaN(xn[id])) ) {
        ///x[id] = 0.5*((xf[id]-xn[id]) / (xf[id]+xn[id]))+0.5;
        x[id] = 0.5*((xf[id]-xn[id]) / e[id])+0.5;
      }
      
      /// range of x is (0, 1)
      if  ( !TMath::IsNaN(xf[id]) && !TMath::IsNaN(xn[id]) ) xCal[id] = 0.5 + 0.5 * (xfCal[id] - xnCal[id] ) / e[id];
      if  ( !TMath::IsNaN(xf[id]) &&  TMath::IsNaN(xn[id]) ) xCal[id] = xfCal[id]/ e[id];
      if  (  TMath::IsNaN(xf[id]) && !TMath::IsNaN(xn[id]) ) xCal[id] = 1.0 - xnCal[id]/ e[id];
      
      //@======= Scale xcal from (0,1)      
      if( corr->xScale.size() >= id ) xCal[id] = (xCal[id]-0.5)/corr->xScale[id] + 0.5; /// if include this scale, need to also inclused in Cali_littleTree
      
      if( abs(xCal[id] - 0.5) > xGate/2. ) continue; 

      //@==================== calculate Z
      if( aID >= 0 ){
        int colIndex = id % detGeo->array[aID].colDet;
        if( detGeo->array[aID].firstPos > 0 ) {
          z[id] = detGeo->array[aID].detLength*(1.0-xCal[id]) + detGeo->array[aID].detPos[colIndex];
        }else{
          z[id] = detGeo->array[aID].detLength*(xCal[id]-1.0) + detGeo->array[aID].detPos[colIndex];
        }
      }

      //@===================== When z is calcualted.
      zMulti[aID] ++;

      //@=================== Array fill
      plotter[aID]->he_x[id]->Fill(x[id],e[id]);
      plotter[aID]->heCal_z->Fill(z[id],eCal[id]);

      //@=================== Recoil Gate
      // if( isRDTExist && (cutList1 || cutList2)){
      //   for(int i = 0 ; i < cutList1->GetEntries() ; i++ ){
      //     cutG = (TCutG *)cutList1->At(i) ;
      //     if(cutG->IsInside(rdt[2*i],rdt[2*i+1])) {
      //     // if(cutG->IsInside(rdt[2*i] + rdt[2*i+1],rdt[2*i+1])) {
      //       rdtgate1= true;
      //       break; /// only one is enough
      //     }
      //   }
        
      //   for(int i = 0 ; i < cutList2->GetEntries() ; i++ ){
      //     cutG = (TCutG *)cutList2->At(i) ;
      //     if(cutG->IsInside(rdt[2*i],rdt[2*i+1])) {
      //     //if(cutG->IsInside(rdt[2*i]+ rdt[2*i+1],rdt[2*i+1])) {
      //       rdtgate2= true;
      //       break; /// only one is enough
      //     }
      //   }
        
      // }else{
      //   rdtgate1 = true;
      //   rdtgate2 = true;
      // } 
    
      //@================ coincident with Recoil when z is calculated.
      // if( !TMath::IsNaN(z[id]) ) { 
      //   for( int j = 0; j < mapping::NRDT ; j++){
      //     if( TMath::IsNaN(rdt[j]) ) continue; 
    
      //     int tdiff = rdt_t[j] - e_t[id];
    
      //     if( j%2 == 1) {
      //       htdiff->Fill(tdiff);
      //       if((rdtgate1 || rdtgate2) && (eCalCut[1] > eCal[id] && eCal[id]>eCalCut[0])) {
      //         htdiffg->Fill(tdiff);
      //       }
      //     }

      //     hArrayRDTMatrix->Fill(id, j); 
    
      //     if( isTimeGateOn && timeGate[0] < tdiff && tdiff < timeGate[1] ) {
      //       if(j % 2 == 0 ) hrdt2Dg[j/2]->Fill(rdt[j],rdt[j+1]); /// x=E, y=dE
      //       ///if(j % 2 == 0 ) hrdt2Dg[j/2]->Fill(rdt[j+1],rdt[j]); /// x=dE, y=E
      //       hArrayRDTMatrixG->Fill(id, j); 
      //       ///if( rdtgate1) hArrayRDTMatrixG->Fill(id, j); 
            
      //       hrdtg[j]->Fill(rdt[j]);
      //       coinFlag = true;
            
      //     }
      //   }
      // }
      
      // if( !isTimeGateOn ) coinFlag = true;

      //@================ E-Z gate
      // if( EZCut ) { 
      //   if( EZCut->IsInside(z[id], eCal[id])  ) ezGate = true;
      // }else{
      //   ezGate = true;
      // }
      
      // if( coinFlag && (rdtgate1 || rdtgate2) && ezGate){ 
      //   plotter[arrayID]->heCalVzGC->Fill( z[id] , eCal[id] );
      
      //   heCalVxCalG[id]->Fill(xCal[id]*detGeo->array[arrayID].detLength, eCal[id]);
    
      //   multiEZ ++;
      //   isGoodEventFlag = true;
      // }


    }//*====== end of array loop

    for( int i = 0 ; i < numGeo ; i++ ) plotter[i]->hArrayMulti->Fill(arrayMulti[i]);
    
    //*********** RECOILS ***********************************************/    
    // for( int i = 0; i < mapping::NRDT ; i++){
    //   hrdtID->Fill(i, rdt[i]);
    //   hrdt[i]->Fill(rdt[i]);
    
    //   if( i % 2 == 0  ){            
    //     recoilMulti++; // when both dE and E are hit
    //     hrdt2D[i/2]->Fill(rdt[i],rdt[i+1]); //E-dE
    //   }
    // }

    //@*********** Ex and thetaCM ****************************************/ 
    // for(Int_t id = 0; id < mapping::NARRAY ; id++){
        
    //   if( TMath::IsNaN(e[id]) ) continue ; 
    //   if( TMath::IsNaN(z[id]) ) continue ;
    //   if( eCal[id] <  eCalCut[0] ) continue ;
    //   if( eCal[id] >  eCalCut[1] ) continue ;

    //   short aID = detGeo->GetArrayID(id);
    //   if( aID < 0 ) continue;

    //   std::pair<double, double> ExThetaCM = transfer->CalExThetaCM(eCal[id], z[id], detGeo->Bfield, detGeo->array[aID].detPerpDist);
    //   double Ex = ExThetaCM.first;
    //   double thetaCM = ExThetaCM.second;
      
    //   if( thetaCM > thetaCMGate ) {

    //     plotter[aID]->hEx->Fill(Ex);
    //     plotter[aID]->hExThetaCM->Fill(thetaCM, Ex);
        
    //     // if( rdtgate1 ) {
    //     //   plotter[arrayID]->hExCut1->Fill(Ex);
    //     //   plotter[arrayID]->hExThetaCM->Fill(thetaCM, Ex);
    //     // }
    //     // if( rdtgate2 ) {
    //     //   plotter[arrayID]->hExCut2->Fill(Ex);
    //     //   plotter[arrayID]->hExThetaCM->Fill(thetaCM, Ex);
    //     // }
        
    //     plotter[arrayID]->hExi[id]->Fill(Ex);
    //     plotter[arrayID]->hExVxCal[id]->Fill(xCal[id], Ex);
          
    //   }
    // }

    //*============================================ Progress Bar
    processedEntries ++;
    if (processedEntries >= NumEntries*Frac - 1 ) {
      TString msg; msg.Form("%llu", NumEntries/1000);
      int len = msg.Sizeof();
      printf(" %3.0f%% (%*llu/%llu k) processed in %6.1f sec | expect %6.1f sec\n",
                Frac*100, len, processedEntries/1000,NumEntries/1000,StpWatch.RealTime(), StpWatch.RealTime()/Frac);
      StpWatch.Start(kFALSE);
      Frac += 0.1;
    }
    if( processedEntries > 1000 ) break;
  
  }//^############################################## End of Process

  for( int i = 0; i < detGeo->numGeo ; i++){
    plotter[i]->Plot();
  }

  //^############################################### 


}


//%============================================= 
void rawID(int arrayID = -1){

  if( arrayID < 0 ){
    for( int i = 0; i < numGeo; i++ ) plotter[i]->PlotRawID();
  }else{
    if( arrayID < numGeo) plotter[arrayID]->PlotRawID();
  }

}

