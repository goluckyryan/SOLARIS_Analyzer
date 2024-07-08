#define Monitor_cxx

#include <TH2.h>
#include <TH1.h>
#include <TF1.h>
#include <TStyle.h>
#include <TCutG.h>
#include <TGraph.h>
#include <TMath.h>
#include <TMultiGraph.h>
#include <TString.h>
#include <TLatex.h>
#include <TSystem.h>
#include <TMacro.h>
#include <TLine.h>
#include <TStopwatch.h>
#include <TCanvas.h>
#include <TBox.h>
#include <TDatime.h>
#include <TMD5.h>
#include <TObjArray.h>
#include <fstream>
#include <vector>
#include "../Cleopatra/ClassIsotope.h"
#include "Mapping.h"

#define tick2ns 8. // 1clock tick = 8 ns
#define tick2min tick2ns/1e9/60.

using namespace std;

//############################################ User setting
//---histogram setting
int rawEnergyRange[2] = {   100,    4000};       /// share with e, xf, xn
int    energyRange[2] = {     0,      10};       /// in the E-Z plot
int     rdtDERange[2] = {     0,      80}; 
int      rdtERange[2] = {     0,      80};
int   thetaCMRange[2] = {0, 80};

double     exRange[3] = {  100,    -2,     10};  /// bin [keV], low[MeV], high[MeV]

int  coinTimeRange[2] = { -200, 200};
int  timeRangeUser[2] = {0, 99999999}; /// min, use when cannot find time, this set the min and max

bool isUseArrayTrace = false;
bool isUseRDTTrace = false;

//---Gate
bool isTimeGateOn     = true;
int timeGate[2]       = {-20, 12};             /// min, max, 1 ch = 10 ns
double eCalCut[2]     = {0.5, 50};             /// lower & higher limit for eCal
int dEgate[2]         = {  500,  1500};
int Eresgate[2]       = { 1000,  4000};
double thetaCMGate    = 10;                    /// deg
double xGate          = 0.9;                  ///cut out the edge
vector<int> skipDetID = {11, 16, 23} ;//{2,  11, 17}

TString rdtCutFile1 = "";
TString rdtCutFile2 = "";
TString ezCutFile   = "";//"ezCut.root";

//############################################ end of user setting


// //======= Recoil
// TH2F * hrdtID;
// TH1F ** hrdt; // single recoil
// TH1F ** hrdtg; 

// TH2F ** hrdt2D;
// TH2F ** hrdt2Dg;

// TH1F * hrdtRate1;
// TH1F * hrdtRate2;

// //======= multi-Hit
// TH2I * hmult;
// TH1I * hmultEZ;
// TH2I * hArrayRDTMatrix;
// TH2I * hArrayRDTMatrixG;

// //======= ARRAY-RDT time diff
// TH1I * htdiff;
// TH1I * htdiffg;

/***************************
 ***************************/
TLatex text;

ULong64_t NumEntries = 0;
ULong64_t ProcessedEntries = 0;
Float_t Frac = 0.1; ///Progress bar
TStopwatch StpWatch;

//======= Recoil Cut
TCutG* cutG; //!  //general temeprary pointer to cut

TObjArray * cutList1;
TObjArray * cutList2;

//======= Other Cuts
TCutG * EZCut;

#include "Monitor.h"

//^###########################################################
//^ * Begin
//^###########################################################
void Monitor::Begin(TTree *tree){

  TString option = GetOption();

  NumEntries = tree->GetEntries();
  canvasTitle = GetCanvasTitle();
  
  printf("###########################################################\n");
  printf("##########           SOLARIS Monitors.C           #########\n");
  printf("###########################################################\n");
  
  for( int i = 0; i < detGeo->numGeo ; i++) plotter[i]->SetUpHistograms(energyRange, exRange, thetaCMRange);

  //===================================================== loading parameter
  
  // corr->LoadDetGeoAndReactionConfigFile();
  corr->LoadXNCorr();
  corr->LoadXFXN2ECorr();
  corr->LoadXScaleCorr();
  corr->LoadECorr();
  corr->LoadRDTCorr();

  if( (int) corr->xnCorr.size()    < mapping::NARRAY ) { printf(" !!!!!!!! size of xnCorr < NARRAY .\n"); }
  if( (int) corr->xfxneCorr.size() < mapping::NARRAY ) { printf(" !!!!!!!! size of xfxneCorr < NARRAY .\n"); }
  if( (int) corr->eCorr.size()     < mapping::NARRAY ) { printf(" !!!!!!!! size of eCorr < NARRAY .\n"); }
  if( (int) corr->xScale.size()    < mapping::NARRAY ) { printf(" !!!!!!!! size of xScale < NARRAY .\n"); }
  if( (int) corr->rdtCorr.size()   < mapping::NRDT   ) { printf(" !!!!!!!! size of rdtCorr < NRDT .\n"); }


  // printf("=====================================================\n");
  // printf(" time Range : %5.0f - %5.0f min\n", timeRangeInMin[0], timeRangeInMin[1]);
  // printf("=====================================================\n");

  //================  Get Recoil cuts;
  cutG = new TCutG();

  cutList1 = AnalysisLib::LoadListOfTCut(rdtCutFile1, "cutList");
  cutList2 = AnalysisLib::LoadListOfTCut(rdtCutFile2, "cutList");
  
  //================  Get EZ cuts;
  EZCut = AnalysisLib::LoadSingleTCut(ezCutFile);
  
  //========================= Generate all of the histograms needed for drawing later on
  printf("============================================ Histograms declaration\n");

  gROOT->cd();


  // int startIndex = 0;
  // for( int i = 0; i < detGeo->numGeo; i++ ){
  //   CreateListOfHist2D(heVx        , startIndex, detGeo->array[i].numDet, "heVx",   "Raw PSD E vs. X (ch=%d);X (channel);E (channel)",   500,  -2.5,  detGeo->array[i].detLength + 2.5, 500, rawEnergyRange[0], rawEnergyRange[1]);
  //   CreateListOfHist2D(heCalVxCal  , startIndex, detGeo->array[i].numDet, "heCalVxCal",   "Cal PSD E vs. X (ch=%d);X (cm);E (MeV)",      500,  -2.5,  detGeo->array[i].detLength + 2.5, 500,    energyRange[0],    energyRange[1]);
  //   CreateListOfHist2D(heCalVxCalG , startIndex, detGeo->array[i].numDet, "heCalVxCalG",  "Cal PSD E vs. X (ch=%d);X (cm);E (MeV)",      500,  -2.5,  detGeo->array[i].detLength + 2.5, 500,    energyRange[0],    energyRange[1]);
  //   startIndex += detGeo->array[i].numDet;
  // }

  // heVID    = new TH2F("heVID",    "Raw e vs channel",  mapping::NARRAY, 0, mapping::NARRAY, 500, rawEnergyRange[0], rawEnergyRange[1]);
  // hxfVID   = new TH2F("hxfVID",   "Raw xf vs channel", mapping::NARRAY, 0, mapping::NARRAY, 500, rawEnergyRange[0], rawEnergyRange[1]);
  // hxnVID   = new TH2F("hxnVID",   "Raw xn vs channel", mapping::NARRAY, 0, mapping::NARRAY, 500, rawEnergyRange[0], rawEnergyRange[1]);
  
  // heCalID = new TH2F("heCalID", "Corrected E vs id; id; E / 10 keV", mapping::NARRAY, 0, mapping::NARRAY, 2000, energyRange[0], energyRange[1]);
  
  // hMultiHit = new TH1F("hMultiHit", "Multi-Hit of Energy", 10, 0, 1);

  // //===================== Recoils
  // hrdtID = new TH2F("hrdtID", "RDT vs ID; ID; energy [ch]", 8, 0, 8, 500, TMath::Min(rdtERange[0], rdtDERange[0]), TMath::Max(rdtERange[1], rdtDERange[1])); 

  // hrdt  = new TH1F * [mapping::NRDT];
  // hrdtg = new TH1F * [mapping::NRDT];

  // hrdt2D    = new TH2F * [mapping::NRDT/2];
  // // hrdt2Dg   = new TH2F * [mapping::NRDT/2];

  // for (Int_t i = 0; i < mapping::NRDT ; i++) {
  //   if( i % 2 == 0 ) hrdt[i]  = new TH1F(Form("hrdt%d",i), Form("Raw Recoil E(ch=%d); E (channel)",i),         500,  rdtERange[0],  rdtERange[1]);
  //   if( i % 2 == 0 ) hrdtg[i] = new TH1F(Form("hrdt%dg",i),Form("Raw Recoil E(ch=%d) gated; E (channel)",i),   500,  rdtERange[0],  rdtERange[1]);
  //   if( i % 2 == 1 ) hrdt[i]  = new TH1F(Form("hrdt%d",i), Form("Raw Recoil DE(ch=%d); DE (channel)",i),       500, rdtDERange[0], rdtDERange[1]);
  //   if( i % 2 == 1 ) hrdtg[i] = new TH1F(Form("hrdt%dg",i),Form("Raw Recoil DE(ch=%d) gated; DE (channel)",i), 500, rdtDERange[0], rdtDERange[1]);
    
  //   ///dE vs E      
  //   if( i % 2 == 0 ) {
  //     int tempID = i / 2;
  //     hrdt2D[tempID]    = new TH2F(Form("hrdt2D%d",tempID) ,    Form("Raw Recoil DE vs Eres (dE=%d, E=%d); Eres (channel); DE (channel)", i+1, i),       500, rdtERange[0], rdtERange[1],500,rdtDERange[0],rdtDERange[1]);
  //     hrdt2Dg[tempID]   = new TH2F(Form("hrdt2Dg%d",tempID),   Form("Gated Raw Recoil DE vs Eres (dE=%d, E=%d); Eres (channel); DE (channel)",i+1, i),  500, rdtERange[0], rdtERange[1],500,rdtDERange[0], rdtDERange[1]);
  //   }
  // }
  
  // hrdtRate1 = new TH1F("hrdtRate1", "recoil rate 1 / min; min; count / 1 min", timeRangeInMin[1] - timeRangeInMin[0], timeRangeInMin[0], timeRangeInMin[1]);
  // hrdtRate2 = new TH1F("hrdtRate2", "recoil rate 2 / min; min; count / 1 min", timeRangeInMin[1] - timeRangeInMin[0], timeRangeInMin[0], timeRangeInMin[1]);
  // hrdtRate1->SetLineColor(2);
  // hrdtRate2->SetLineColor(4);

  // //===================== multiplicity
  // hmultEZ           = new TH1I("hmultEZ",          "Filled EZ with coinTime and recoil",                       10,  0,  10);
  // hmult             = new TH2I("hmult",            "Array Multiplicity vs Recoil Multiplicity; Array ; Recoil",10,  0,  10, 10,  0, 10);
  // hArrayRDTMatrix   = new TH2I("hArrayRDTMatrix",  "Array ID vs Recoil ID; Array ID; Recoil ID",               30,  0,  30,  8,  0,  8);
  // hArrayRDTMatrixG  = new TH2I("hArrayRDTMatrixG", "Array ID vs Recoil ID / g; Array ID; Recoil ID",           30,  0,  30,  8,  0,  8);

  // //===================== coincident time 
  // htdiff  = new TH1I("htdiff" ,"Coincident time (recoil-dE - array); time [ch = 10ns]; count", coinTimeRange[1] - coinTimeRange[0], coinTimeRange[0], coinTimeRange[1]);   
  // htdiffg = new TH1I("htdiffg","Coincident time (recoil-dE - array) w/ recoil gated; time [ch = 10ns]; count", coinTimeRange[1] - coinTimeRange[0], coinTimeRange[0], coinTimeRange[1]);
  

  printf("============================================ End of histograms Declaration\n");
  StpWatch.Start();

}

//^###########################################################
//^ * Process
//^###########################################################
Bool_t Monitor::Process(Long64_t entry){

  if( entry == 0 ) {
    treeID ++;
    baseTimeStamp = (treeID == 0 ? 0 : endTime[treeID-1]);
    printf("============================================ %s , treeID : %d\n", __func__, treeID);
  }

  ProcessedEntries++;
  
  //@*********** Progress Bar ******************************************/ 
  if (ProcessedEntries >= NumEntries*Frac - 1 ) {
    TString msg; msg.Form("%llu", NumEntries/1000);
    int len = msg.Sizeof();
    printf(" %3.0f%% (%*llu/%llu k) processed in %6.1f sec | expect %6.1f sec\n",
              Frac*100, len, ProcessedEntries/1000,NumEntries/1000,StpWatch.RealTime(), StpWatch.RealTime()/Frac);
    StpWatch.Start(kFALSE);
    Frac += 0.1;
  }

  //@********** Get Branch *********************************************/
  b_Energy->GetEntry(entry);
  b_XF->GetEntry(entry);
  b_XN->GetEntry(entry);
  b_EnergyTimestamp->GetEntry(entry);

  if( isRDTExist ){
    b_RDT->GetEntry(entry);
    b_RDTTimestamp->GetEntry(entry);
  }
   
  // if( isArrayTraceExist ) {
  //   ///b_Trace_Energy->GetEntry(entry);
  //   b_Trace_Energy_RiseTime->GetEntry(entry);
  //   b_Trace_Energy_Time->GetEntry(entry);
  // }
   
  // if( isRDTTraceExist ){
  //   ///b_Trace_RDT->GetEntry(entry); 
  //   b_Trace_RDT_Time->GetEntry(entry);
  //   b_Trace_RDT_RiseTime->GetEntry(entry);
  // }
  
  //@*********** initization ******************************************/
  for( int i = 0 ; i < mapping::NARRAY; i++){
      z[i] = TMath::QuietNaN();
      x[i] = TMath::QuietNaN();
      xCal[i] = TMath::QuietNaN();
      eCal[i] = TMath::QuietNaN();
  }
    
  //@*********** Apply Recoil correction here *************************/
  if( corr->rdtCorr.size() >= mapping::NRDT  ){
    for( int i = 0 ; i < mapping::NRDT; i++){
      rdt[i] = corr->rdtCorr[i][0] + rdt[i]*corr->rdtCorr[i][1] ;
    }
  }

  //@*********** Array ************************************************/ 
  //Do calculations and fill histograms
  Int_t recoilMulti = 0;
  Int_t arrayMulti = 0;
  Int_t multiEZ = 0;
  bool rdtgate1 = false;
  bool rdtgate2 = false;
  bool coinFlag = false;
  bool ezGate = false;
  bool isGoodEventFlag = false;

  for (Int_t id = 0; id < mapping::NARRAY; id++) {
      
    //@================== Filling raw data
    he[id]->Fill(e[id]);
    hxf[id]->Fill(xf[id]);
    hxn[id]->Fill(xn[id]);
    hxfVxn[id]->Fill(xf[id],xn[id]);
    heVxs[id]->Fill(xf[id]+xn[id], e[id]);
    
    heVID->Fill(id, e[id]);
    hxfVID->Fill(id, xf[id]);
    hxnVID->Fill(id, xn[id]);

    //if( !TMath::IsNaN(e[id]) ) printf("%llu | %d | %f %f %f \n", entry, id, e[id], xf[id], xn[id]);
      
    //@==================== Basic gate
    if( TMath::IsNaN(e[id]) ) continue ; 
    ///if( ring[id] < -100 || ring[id] > 100 ) continue; 
    ///if( ring[id] > 300 ) continue; 
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

    //@==================== Basic gate
    if( TMath::IsNaN(e[id]) ) continue ; 
    ///if( ring[id] < -100 || ring[id] > 100 ) continue; 
    ///if( ring[id] > 300 ) continue; 
    if( TMath::IsNaN(xn[id]) &&  TMath::IsNaN(xf[id]) ) continue ; 

    //@==================== Calibrations go here
    if( corr->xnCorr.size() >= id && corr->xfxneCorr.size() >= id ) xnCal[id] = xn[id] * corr->xnCorr[id] * corr->xfxneCorr[id][1] + corr->xfxneCorr[id][0];
    if( corr->xfxneCorr.size() >= id )                              xfCal[id] = xf[id] * corr->xfxneCorr[id][1] + corr->xfxneCorr[id][0];
    if( corr->eCorr.size() >= id)                                   eCal[id]  = e[id] / corr->eCorr[id][0] + corr->eCorr[id][1];

    if( eCal[id] < eCalCut[0] ||  eCalCut[1] < eCal[id] ) continue;
      
    //@===================== fill Calibrated  data
    heCal[id]->Fill(eCal[id]);
    heCalID->Fill(id, eCal[id]);
    hxfCalVxnCal[id]->Fill(xfCal[id], xnCal[id]);
    heVxsCal[id]->Fill(xnCal[id] + xfCal[id], e[id]);
      
    //@===================== calculate X
    if( (xf[id] > 0 || !TMath::IsNaN(xf[id])) && ( xn[id]>0 || !TMath::IsNaN(xn[id])) ) {
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
    short arrayID = detGeo->GetArrayID(id);
    if( arrayID >= 0 ){
      int colIndex = id % detGeo->array[arrayID].colDet;
      if( detGeo->array[arrayID].firstPos > 0 ) {
        z[id] = detGeo->array[arrayID].detLength*(1.0-xCal[id]) + detGeo->array[arrayID].detPos[colIndex];
      }else{
        z[id] = detGeo->array[arrayID].detLength*(xCal[id]-1.0) + detGeo->array[arrayID].detPos[colIndex];
      }
    }

    //@===================== multiplicity
    arrayMulti++; /// multi-hit when both e, xf, xn are not NaN

    //@=================== Array fill
    heVx[id]->Fill(x[id],e[id]);
    
    heCalVxCal[id]->Fill(xCal[id]*detGeo->array[arrayID].detLength, eCal[id]);

    plotter[arrayID]->heCalVz->Fill(z[id],eCal[id]);

    //@=================== Recoil Gate
    if( isRDTExist && (cutList1 || cutList2)){
      for(int i = 0 ; i < cutList1->GetEntries() ; i++ ){
        cutG = (TCutG *)cutList1->At(i) ;
        if(cutG->IsInside(rdt[2*i],rdt[2*i+1])) {
        // if(cutG->IsInside(rdt[2*i] + rdt[2*i+1],rdt[2*i+1])) {
          rdtgate1= true;
          break; /// only one is enough
        }
      }
      
      for(int i = 0 ; i < cutList2->GetEntries() ; i++ ){
        cutG = (TCutG *)cutList2->At(i) ;
        if(cutG->IsInside(rdt[2*i],rdt[2*i+1])) {
        //if(cutG->IsInside(rdt[2*i]+ rdt[2*i+1],rdt[2*i+1])) {
          rdtgate2= true;
          break; /// only one is enough
        }
      }
      
    }else{
      rdtgate1 = true;
      rdtgate2 = true;
    }
      
    //================ coincident with Recoil when z is calculated.
    if( !TMath::IsNaN(z[id]) ) { 
      for( int j = 0; j < mapping::NRDT ; j++){
        if( TMath::IsNaN(rdt[j]) ) continue; 
  
        int tdiff = rdt_t[j] - e_t[id];
  
        if( j%2 == 1) {
          htdiff->Fill(tdiff);
          if((rdtgate1 || rdtgate2) && (eCalCut[1] > eCal[id] && eCal[id]>eCalCut[0])) {
            htdiffg->Fill(tdiff);
          }
        }

        hArrayRDTMatrix->Fill(id, j); 
  
        if( isTimeGateOn && timeGate[0] < tdiff && tdiff < timeGate[1] ) {
          if(j % 2 == 0 ) hrdt2Dg[j/2]->Fill(rdt[j],rdt[j+1]); /// x=E, y=dE
          ///if(j % 2 == 0 ) hrdt2Dg[j/2]->Fill(rdt[j+1],rdt[j]); /// x=dE, y=E
          hArrayRDTMatrixG->Fill(id, j); 
          ///if( rdtgate1) hArrayRDTMatrixG->Fill(id, j); 
          
          hrdtg[j]->Fill(rdt[j]);
          coinFlag = true;
          
        }
      }
    }
      
    if( !isTimeGateOn ) coinFlag = true;
    
    //================ E-Z gate
    if( EZCut ) { 
      if( EZCut->IsInside(z[id], eCal[id])  ) ezGate = true;
    }else{
      ezGate = true;
    }
    
    if( coinFlag && (rdtgate1 || rdtgate2) && ezGate){ 
      plotter[arrayID]->heCalVzGC->Fill( z[id] , eCal[id] );
    
      heCalVxCalG[id]->Fill(xCal[id]*detGeo->array[arrayID].detLength, eCal[id]);
  
      multiEZ ++;
      isGoodEventFlag = true;
    }
      
    
  }//end of array loop
  if( EZCut == nullptr ) ezGate = true;
    
  //@*********** RECOILS ***********************************************/    
  for( int i = 0; i < mapping::NRDT ; i++){
    hrdtID->Fill(i, rdt[i]);
    hrdt[i]->Fill(rdt[i]);
  
    if( i % 2 == 0  ){            
      recoilMulti++; // when both dE and E are hit
      hrdt2D[i/2]->Fill(rdt[i],rdt[i+1]); //E-dE
    }
  }

  hrdtRate1->Fill( (e_t[1] + baseTimeStamp) * tick2min ); //incorrect
   
  //@******************* Multi-hit *************************************/
  hmultEZ->Fill(multiEZ);
  hmult->Fill(recoilMulti,arrayMulti);
  hMultiHit->Fill(arrayMulti);

  //@*********** Good event Gate ***************************************/ 
  if( !isGoodEventFlag ) return kTRUE;
   
  //@*********** Ex and thetaCM ****************************************/ 
  for(Int_t id = 0; id < mapping::NARRAY ; id++){
     	
    if( TMath::IsNaN(e[id]) ) continue ; 
    if( TMath::IsNaN(z[id]) ) continue ;
    if( eCal[id] <  eCalCut[0] ) continue ;
    if( eCal[id] >  eCalCut[1] ) continue ;

    short arrayID = detGeo->GetArrayID(id);
    if( arrayID < 0 ) continue;

    std::pair<double, double> ExThetaCM = transfer->CalExThetaCM(eCal[id], z[id], detGeo->Bfield, detGeo->array[arrayID].detPerpDist);
    double Ex = ExThetaCM.first;
    double thetaCM = ExThetaCM.second;
     
    if( thetaCM > thetaCMGate ) {

      plotter[arrayID]->hEx->Fill(Ex);
      plotter[arrayID]->hExThetaCM->Fill(thetaCM, Ex);
      
      if( rdtgate1 ) {
        plotter[arrayID]->hExCut1->Fill(Ex);
        plotter[arrayID]->hExThetaCM->Fill(thetaCM, Ex);
      }
      if( rdtgate2 ) {
        plotter[arrayID]->hExCut2->Fill(Ex);
        plotter[arrayID]->hExThetaCM->Fill(thetaCM, Ex);
      }
      
      plotter[arrayID]->hExi[id]->Fill(Ex);
      plotter[arrayID]->hExVxCal[id]->Fill(xCal[id], Ex);
        
    }
  }
  
  return kTRUE;
}

//^###########################################################
//^ * Terminate
//^###########################################################
void Monitor::Terminate(){
  printf("============================================ Drawing Canvas.\n");

  gROOT->cd();

  //############################################ User is free to edit this section
  //--- Canvas Size
  int canvasXY[2] = {1200 , 800} ;// x, y
  int canvasDiv[2] = {3,2};

  gStyle->SetOptStat("neiou");
  text.SetNDC();
  text.SetTextFont(82);
  text.SetTextSize(0.04);
  text.SetTextColor(2);

  double yMax = 0;

  // Isotope hRecoil(AnalysisLib::reactionConfig.recoilHeavyA, AnalysisLib::reactionConfig.recoilHeavyZ);
  // double Sn = hRecoil.CalSp(0,1);
  // double Sp = hRecoil.CalSp(1,0);
  // double Sa = hRecoil.CalSp2(4,2);

  for( int i = 0; i < detGeo->numGeo; i++ ){

    plotter[i]->SetUpCanvas(canvasTitle + " | " + rdtCutFile1, canvasXY[0],canvasXY[1], canvasDiv[0], canvasDiv[1]);
    // cCanvas[i]  = new TCanvas("cCanvas",canvasTitle + " | " + rdtCutFile1,canvasXY[0],canvasXY[1]);
    // cCanvas[i]->Modified(); cCanvas->Update();
    // cCanvas[i]->cd(); cCanvas->Divide(canvasDiv[0],canvasDiv[1]);

    plotter[i]->Plot();
    
    ///----------------------------------- Canvas - 1
    // PlotEZ(1); /// raw EZ
      
    ///----------------------------------- Canvas - 2
    // PlotEZ(0); ///gated EZ

    ///----------------------------------- Canvas - 3
    // PlotTDiff(1, 1); ///with Gated Tdiff, isLog
    
    ///----------------------------------- Canvas - 4
    // padID++; cCanvas->cd(padID); 
    
    // //hEx->Draw();
    // hExCut1->Draw("");
    // hExCut2->Draw("same");
    // DrawLine(hEx, Sn);
    // DrawLine(hEx, Sa);
    
    // if(isTimeGateOn)text.DrawLatex(0.15, 0.8, Form("%d < coinTime < %d", timeGate[0], timeGate[1])); 
    // if( xGate < 1 ) text.DrawLatex(0.15, 0.75, Form("with |x-0.5|<%.4f", xGate/2.));
    // if( cutList1 ) text.DrawLatex(0.15, 0.7, "with recoil gated"); 

    ///----------------------------------- Canvas - 5
  //   padID++; cCanvas->cd(padID); 
    
  //   //Draw2DHist(hExThetaCM);
  //   //heVIDG->Draw();
  //   //text.DrawLatex(0.15, 0.75, Form("#theta_{cm} > %.1f deg", thetaCMGate));

  //   Draw2DHist(hrdt2D[0]);
  // //      Draw2DHist(hrdt2Dsum[0]);

  //   if( cutList1 && cutList1->GetEntries() > 0 ) {cutG = (TCutG *)cutList1->At(0) ; cutG->Draw("same");}
  //   if( cutList2 && cutList2->GetEntries() > 0 ) {cutG = (TCutG *)cutList2->At(0) ; cutG->Draw("same");}


    //helum4D->Draw();
    //text.DrawLatex(0.25, 0.3, Form("gated from 800 to 1200 ch\n"));
    
    ///----------------------------------- Canvas - 6
    // PlotRDT(0,0);
    
  // padID++; cCanvas->cd(padID); 
  //  Draw2DHist(hrdtExGated);
    
    //padID++; cCanvas->cd(padID); 
    //Draw2DHist(htacEx);
    
    ///------------------------------------- Canvas - 7
    //PlotRDT(0, 0);
    
    ///----------------------------------- Canvas - 8
    //PlotRDT(1, 0);

    ///yMax = hic2->GetMaximum()*1.2;
    ///hic2->GetYaxis()->SetRangeUser(0, yMax);
    ///hic2->Draw();
    ///TBox * box14N = new TBox (-10, 0, -2, yMax);
    ///box14N->SetFillColorAlpha(2, 0.1);
    ///box14N->Draw();
    ///
    ///TBox * box14C = new TBox (8, 0, 16, yMax);
    ///box14C->SetFillColorAlpha(4, 0.1);
    ///box14C->Draw();
    ///
    ///text.SetTextColor(2); text.DrawLatex(0.38, 0.50, "14N");
    ///text.SetTextColor(4); text.DrawLatex(0.6, 0.45, "14C");
    ///text.SetTextColor(2);
    ///----------------------------------- Canvas - 9
    //padID++; cCanvas->cd(padID);  
    
    //Draw2DHist(hic01);

    ///----------------------------------- Canvas - 10
    //PlotRDT(3,0);
    
    //TH1F * helumDBIC = new TH1F("helumDBIC", "elum(d)/BIC; time [min]; count/min", timeRangeInMin[1]-timeRangeInMin[0], timeRangeInMin[0], timeRangeInMin[1]);
    //helumDBIC = (TH1F*) helum4D->Clone();
    //helumDBIC->SetTitle("elum(d)/BIC; time [min]; count/min");
    //helumDBIC->SetName("helumDBIC");
    //helumDBIC->SetLineColor(2);
    
    //helumDBIC->Divide(hBIC);
    
    //yMax = helumDBIC->GetMaximum();
    //if( yMax < hBIC->GetMaximum() ) yMax = hBIC->GetMaximum();
    
    //helumDBIC->SetMaximum(yMax * 1.2);
    //hBIC->SetMaximum(yMax * 1.2);
    
    //hBIC->Draw();
    //helumDBIC->Draw("same");
    
    //text.DrawLatex(0.15, 0.5, Form("Elum(D) / BIC \n"));
    
    ///----------------------------------- Canvas - 11
    //PlotRDT(2,0);
    
    ///----------------------------------- Canvas - 12
    //padID++; cCanvas->cd(padID);
    //htac->Draw();

    ///----------------------------------- Canvas - 13
    //padID++; cCanvas->cd(padID);
    
    ///hicT14N->Draw("");
    ///hicT14C->Draw("same");
    ///
    ///text.SetTextColor(2); text.DrawLatex(0.15, 0.60, "14N");
    ///text.SetTextColor(4); text.DrawLatex(0.15, 0.25, "14C");
    ///text.SetTextColor(2);
    
    ///----------------------------------- Canvas - 14
    // padID++; cCanvas->cd(padID);
    
    // hrdtRate1->Draw("");
    // hrdtRate2->Draw("same");
    
    ///----------------------------------- Canvas - 15
    //padID++; cCanvas->cd(padID);  
      
    ///----------------------------------- Canvas - 16
    //padID++; cCanvas->cd(padID); 

    ///----------------------------------- Canvas - 17
    //padID++; cCanvas->cd(padID);    

    ///----------------------------------- Canvas - 18
    //padID++; cCanvas->cd(padID);

    ///----------------------------------- Canvas - 19
    //padID++; cCanvas->cd(padID);
    
    ///----------------------------------- Canvas - 20
    //padID++; cCanvas->cd(padID);
    
  
  }

  /************************************/
  gStyle->GetAttDate()->SetTextSize(0.02);
  gStyle->SetOptDate(1);
  gStyle->SetDateX(0);
  gStyle->SetDateY(0);
  
  /************************************/
  StpWatch.Start(kFALSE);
  
  // gROOT->ProcessLine(".L ../armory/Monitor_Util.C"); //TODO some pointer is empty
  // printf("============================================ loaded Monitor_Utils.C\n");
  //gROOT->ProcessLine(".L ../armory/AutoFit.C");
  //printf("============================================ loaded armory/AutoFit.C\n");
  // gROOT->ProcessLine(".L ../armory/RDTCutCreator.C");
  // printf("============================================ loaded armory/RDTCutCreator.C\n");
  // gROOT->ProcessLine(".L ../armory/Check_rdtGate.C");
  // printf("============================================ loaded armory/Check_rdtGate.C\n");
  // gROOT->ProcessLine(".L ../armory/readTrace.C");
  // printf("============================================ loaded Armory/readTrace.C\n");
  // gROOT->ProcessLine(".L ../armory/readRawTrace.C");
  // printf("============================================ loaded Armory/readRawTrace.C\n");
  // gROOT->ProcessLine("listDraws()");
  
  /************************* Save histograms to root file*/
  
  gROOT->cd();
  

  /************************************/
  //gROOT->ProcessLine("recoils()");

}
