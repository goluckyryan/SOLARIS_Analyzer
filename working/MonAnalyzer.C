
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
#include "TH2.h"
#include "TStyle.h"
#include "TStopwatch.h"

#include "vector"
//^############################################ User setting
int rawEnergyRange[2] = {   100,    4000};       /// share with e, xf, xn
int    energyRange[2] = {     0,      10};       /// in the E-Z plot
int     rdtDERange[2] = {     0,      80}; 
int      rdtERange[2] = {     0,      80};
int   thetaCMRange[2] = {0, 80};

double     exRange[3] = {  100,    -2,     10};  /// bin [keV], low[MeV], high[MeV]


//---Gate
bool isTimeGateOn     = true;
int timeGate[2]       = {-20, 12};             /// min, max, 1 ch = 10 ns
double eCalCut[2]     = {0.5, 50};             /// lower & higher limit for eCal

std::vector<int> skipDetID = {11, 16, 23} ;//{2,  11, 17}
//^############################################ end of user setting


void MonAnalyzer(){

//   TFile * file = new TFile("../root_data/gen_run043.root");

  TChain *chain = new TChain("gen_tree");
  chain->Add("../root_data/gen_run043.root");

  // chain->Print();

  TTreeReader reader(chain);

  TTreeReaderValue<ULong64_t>  evID = {reader, "evID"};
  TTreeReaderArray<Float_t>     e   = {reader, "e"};
  TTreeReaderArray<ULong64_t>   e_t = {reader, "e_t"};
  TTreeReaderArray<Float_t>      xf = {reader, "xf"};
  TTreeReaderArray<Float_t>      xn = {reader, "xn"};

  TTreeReaderArray<Float_t>     rdt   = {reader, "rdt"};
  TTreeReaderArray<ULong64_t>   rdt_t = {reader, "rdt_t"};

  //TODO
  // TTreeReaderArray<TClonesArray>   array = {reader, "trace"};

  //*==========================================
  ULong64_t NumEntries = chain->GetEntries();
  CorrParas * corr = new CorrParas;
  DetGeo * detGeo = new DetGeo("detectorGeo.txt");
  // TransferReaction * transfer = new TransferReaction("reactionConfig.txt");

  MonPlotter ** plotter = new MonPlotter *[detGeo->numGeo];
  for( int i = 0; i < detGeo->numGeo; i++ ) {
    plotter[i] = new MonPlotter(i, detGeo, mapping::NRDT);
    plotter[i]->SetUpCanvas("haha", 500, 3, 2);
    plotter[i]->SetUpHistograms(rawEnergyRange, energyRange, exRange, thetaCMRange, rdtDERange, rdtERange);
  }

  double eCal[mapping::NARRAY];
  double xfCal[mapping::NARRAY];
  double xnCal[mapping::NARRAY];

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

      // //@==================== Basic gate
      // if( TMath::IsNaN(e[id]) ) continue ; 
      // if( TMath::IsNaN(xn[id]) &&  TMath::IsNaN(xf[id]) ) continue ; 
      
      // //@==================== Skip detector 
      // bool skipFlag = false;
      // for( unsigned int kk = 0; kk < skipDetID.size() ; kk++){
      //     if( id == skipDetID[kk] ) {
      //       skipFlag = true;
      //       break;
      //     }
      // }
      // if (skipFlag ) continue;

      // //@==================== Calibrations go here
      // if( corr->xnCorr.size() >= id && corr->xfxneCorr.size() >= id ) xnCal[id] = xn[id] * corr->xnCorr[id] * corr->xfxneCorr[id][1] + corr->xfxneCorr[id][0];
      // if( corr->xfxneCorr.size() >= id )                              xfCal[id] = xf[id] * corr->xfxneCorr[id][1] + corr->xfxneCorr[id][0];
      // if( corr->eCorr.size() >= id)                                   eCal[id]  = e[id] / corr->eCorr[id][0] + corr->eCorr[id][1];

      // if( eCal[id] < eCalCut[0] ||  eCalCut[1] < eCal[id] ) continue;



    }//*==== end of array




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

}

