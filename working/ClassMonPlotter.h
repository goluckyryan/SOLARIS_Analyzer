#ifndef ClassMonitorPlotter_H
#define ClassMonitorPlotter_H

#include "../armory/ClassDetGeo.h"
#include "../armory/ClassReactionConfig.h"
#include "../cleopatra/ClassTransfer.h"
#include "../cleopatra/ClassIsotope.h"

#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TStyle.h"

/******************************************************************
* This is Plotter for Monitor.C. It contains
* 1) Tcanvas
* 2) various Histograms ( exclude raw data histogram )
* 
* The reason for having Plotter is suppert multiple arrays.
* contained the Canvas and Histogram in a class, have better memory management
* 
*******************************************************************/
/******************************************************************
*   variable and histogram naming rules                           *
*   name are case sensitive, so as any C/C++ code                 *
*                                                                 *
*   ID is detector ID                                             *
*                                                                 *
*   raw data from gen_tree are e, xf, xn, ring.                   *
*   the x from raw data is x                                      *
*                                                                 *
*   xf + xn = xs, s for sum                                       *
*                                                                 *
*   calibrated data are  eCal, xfCal, xnCal, ringCal.             *
*   the x from cal data is xCal                                   *
*                                                                 *
*   xfCal + xnCal = xsCal                                         *
*                                                                 *
*   since the z is always from xCal, so it calls z.               *
*                                                                 *
*   Excitation energy calls Ex                                    *
*                                                                 *
*                                                                 *
*   TH2D is always using "_" to seperate 2 variables, like e_x    *
*                                                                 *
*  histogram with TCutG, add suffix "GC" for Graphical-Cut.       *
*                                                                 *
*******************************************************************/

class MonPlotter{
public:

  MonPlotter(unsigned short arrayID, DetGeo * detGeo, int numRDT);
  ~MonPlotter();

  void SetUpCanvas(TString title, int padSize, int divX, int divY);
  void SetUpHistograms(int * rawEnergyRange, 
                       int * energyRange, 
                       double * exRange, 
                       int * thetaCMRange, 
                       int * rdtDERange, 
                       int * rdtERange,
                       int * coinTimeRange);

  void LoadRDTGate(TString rdtCutFile);

  void Plot();

  void PlotRaw(bool isLog = false);
  void PlotCal();
  void PlotEZ();
  void PlotEx();
  
  void PlotRDT(bool isLog = false);

  TCanvas * canvas;

  //====================== Histograms
  //======== raw data
  TH2F * he_ID, * hxf_ID, * hxn_ID; // vs ID

  TH1I * hArrayMulti;

  TH1F ** he, ** hxf, ** hxn; //basic data
  TH2F ** hxf_xn, ** he_xs; // correlation

  //====== cal data
  TH1F ** heCal;
  TH2F ** hxfCal_xnCal; 
  TH2F ** he_xsCal; // raw e vs xf
  TH2F ** he_x; // raw e vs x
  TH2F *  heCal_ID;

  //===== eCal V z
  TH2F  * heCal_z;
  TH2F  * heCal_zGC;

  //======= Recoil
  TH2F * hrdt_ID;
  TH1F ** hrdt; // single recoil

  TH1I * hrdtMulti;

  TH2F ** hrdt2D;
  TH2F ** hrdt2Dg; // gated

  //====== tDiff 
  TH1F * htDiff;
  TH1F * htDiffg;

  //====== Ex data
  TH1F  * hEx;
  TH1F ** hExi;
  TH2F ** hEx_xCal;

  TH1F * hExCut1;
  TH1F * hExCut2;

  TH2F * hEx_ThetaCM;
  //=======================

  //======= Recoil Cut
  TObjArray * cutList;

private:

  unsigned short aID;
  int numDet, colDet, rowDet; //array
  float detLength;
  int numRDT;
  float recoilOutter;
  double zRange[2] ; // zMin, zMax
  
  TString canvasTitle;
  TString suffix;
  int numPad;

  template<typename T> void CreateListOfHist1D(T ** &histList, int size, const char * namePrefix, const char * TitleForm, int binX, float xMin, float xMax);
  template<typename T> void CreateListOfHist2D(T ** &histList, int size, const char * namePrefix, const char * TitleForm, int binX, float xMin, float xMax, int binY, float yMin, float yMax);

};
//^#################################################################################
MonPlotter::MonPlotter(unsigned short arrayID, DetGeo * detGeo, int numRDT){
  aID = arrayID;
  numDet = detGeo->array[aID].numDet;
  colDet = detGeo->array[aID].colDet;
  rowDet = numDet/colDet;
  detLength = detGeo->array[aID].detLength;

  suffix = Form("_%d", arrayID);

  this->numRDT = numRDT;
  recoilOutter = detGeo->aux[aID].outerRadius;

  zRange[0] = detGeo->array[aID].zMin - 50;
  zRange[1] = detGeo->array[aID].zMax + 50;

  canvas = nullptr;
  cutList = nullptr;

}

MonPlotter::~MonPlotter(){

  printf("=============== %s\n", __func__);

  delete canvas;

  delete he_ID;
  delete hxf_ID;
  delete hxn_ID;
  delete hArrayMulti;

  delete heCal_ID;
  delete heCal_zGC;
  delete heCal_z;

  delete hEx_ThetaCM;
  delete hExCut1;
  delete hExCut2;

  delete hrdt_ID;
  delete hrdtMulti;

  delete htDiff;
  delete htDiffg;
  
  for( int i = 0; i < numDet ; i++ ){
    delete he[i];
    delete hxf[i];
    delete hxn[i];
    delete hxf_xn[i];
    delete he_xs[i];
    delete he_x[i];
    delete heCal[i];
    delete hExi[i];
    delete hEx_xCal[i];
  }

  for( int i = 0; i < numRDT; i++){
    delete hrdt[i];
  }
  for( int i = 0; i < numRDT/2; i++){
    delete hrdt2D[i];
    delete hrdt2Dg[i];
  }

  delete [] he;
  delete [] hxf;
  delete [] hxn;
  delete [] hxf_xn;
  delete [] he_xs;
  delete [] he_x;
  delete [] heCal;
  delete [] hExi;
  delete [] hEx_xCal;

  delete [] hrdt;
  delete [] hrdt2D;
  delete [] hrdt2Dg;

  delete cutList;

}

void MonPlotter::SetUpCanvas(TString title, int padSize, int divX, int divY){

  canvas = new TCanvas("canavs" + suffix, title, 500 * aID, 0, divX * padSize, divY * padSize);
  canvas->Divide(divX, divY);
  numPad = divX * divY;
  canvasTitle = title;

}

template<typename T> void MonPlotter::CreateListOfHist1D(T ** &histList, 
                                                         int size, 
                                                         const char * namePrefix, 
                                                         const char * TitleForm, 
                                                         int binX, float xMin, float xMax){

  //printf(" Making %d of %s.\n", size, namePrefix);
  histList = new T * [size];
  for(int i = 0; i < size; i++) {
    histList[i] = new T(Form("%s%d", namePrefix, i) + suffix, Form(TitleForm, i), binX, xMin, xMax);  
  }
}

template<typename T> void MonPlotter::CreateListOfHist2D(T ** &histList, 
                                                         int size, 
                                                         const char * namePrefix, 
                                                         const char * TitleForm, 
                                                         int binX, float xMin, float xMax,
                                                         int binY, float yMin, float yMax){

  //printf(" Making %d of %s.\n", size, namePrefix);
  histList = new T * [size];
  for(int i = 0; i < size; i++) {
    histList[i] = new T(Form("%s%d", namePrefix, i) + suffix, Form(TitleForm, i), binX, xMin, xMax, binY, yMin, yMax);
  }
}

void MonPlotter::SetUpHistograms(int * rawEnergyRange, 
                                 int * energyRange, 
                                 double * exRange, 
                                 int * thetaCMRange, 
                                 int * rdtDERange, 
                                 int * rdtERange,
                                 int * coinTimeRange){

  he_ID  = new TH2F("he_ID"  + suffix, "Raw e vs array ID; Array ID; Raw e",    numDet, 0, numDet,  200, rawEnergyRange[0], rawEnergyRange[1]);
  hxf_ID = new TH2F("hxf_ID" + suffix, "Raw xf vs array ID; Array ID; Raw xf",  numDet, 0, numDet,  200, rawEnergyRange[0], rawEnergyRange[1]);
  hxn_ID = new TH2F("hxn_ID" + suffix, "Raw xn vs array ID; Array ID; Raw xn",  numDet, 0, numDet,  200, rawEnergyRange[0], rawEnergyRange[1]);

  hArrayMulti = new TH1I("hArrayMulti", "Array Multiplicity ( e and (xf or xn) )", numDet, 0, numDet);

  CreateListOfHist1D(he,   numDet, "he",  "Raw e (ch=%d); e (channel); count",            200, rawEnergyRange[0], rawEnergyRange[1]);
  CreateListOfHist1D(hxf,  numDet, "hxf", "Raw xf (ch=%d); e (channel); count",           200, rawEnergyRange[0], rawEnergyRange[1]);
  CreateListOfHist1D(hxn,  numDet, "hxn", "Raw xn (ch=%d); e (channel); count",           200, rawEnergyRange[0], rawEnergyRange[1]);

  CreateListOfHist2D(hxf_xn, numDet, "hxf_xn", "Raw xf vs. xn (ch=%d);xf (channel);xn (channel)"     , 500, rawEnergyRange[0], rawEnergyRange[1], 500, rawEnergyRange[0], rawEnergyRange[1]);
  CreateListOfHist2D(he_xs,  numDet, "he_xs",  "Raw e vs xf+xn (ch=%d); xf+xn (channel); e (channel)", 500, rawEnergyRange[0], rawEnergyRange[1], 500, rawEnergyRange[0], rawEnergyRange[1]);

  CreateListOfHist2D(he_x        , numDet, "he_x",     "Raw e vs x (ch=%d); x (mm); Raw e (channel)",  500, rawEnergyRange[0], rawEnergyRange[1], 500, -0.5, 1.5);
  CreateListOfHist2D(hxfCal_xnCal, numDet, "hxfCal_xnCal", "Corrected XF vs. XN (ch=%d);XF (channel);XN (channel)",                         500,                 0, rawEnergyRange[1], 500,                 0, rawEnergyRange[1]);      
  CreateListOfHist2D(he_xsCal    , numDet, "he_xsCal",     "Raw e vs Corrected xf+xn (ch=%d); corrected xf+xn (channel); Raw e (channel)",  500, rawEnergyRange[0], rawEnergyRange[1], 500, rawEnergyRange[0], rawEnergyRange[1]);
  
  CreateListOfHist1D(heCal, numDet, "heCal",       "Corrected e (ch=%d); e (MeV); count", 2000,    energyRange[0],    energyRange[1]);
  
  //====================== E-Z plot
  heCal_ID  = new TH2F("heCal_ID" + suffix ,  "E vs. ID; ID;E (MeV)"       , numDet,         0,    numDet, 400, energyRange[0], energyRange[1]);
  heCal_z   = new TH2F("heCal_z" + suffix ,  "E vs. Z;Z (mm);E (MeV)"      ,    400, zRange[0], zRange[1], 400, energyRange[0], energyRange[1]);
  heCal_zGC = new TH2F("heCal_zGC" + suffix ,"E vs. Z gated;Z (mm);E (MeV)",    400, zRange[0], zRange[1], 400, energyRange[0], energyRange[1]);

  //===================== Recoil
  int rdtRange[2];
  rdtRange[0] = rdtDERange[0] < rdtERange[0] ? rdtDERange[0] : rdtERange[0];
  rdtRange[1] = rdtDERange[1] > rdtERange[1] ? rdtDERange[1] : rdtERange[1];

  hrdt_ID = new TH2F("hrdt_ID" + suffix, "Raw RDT vs ID; ID; Raw RDT", numRDT, 0, numRDT, 400, rdtRange[0], rdtRange[1]);

  hrdtMulti = new TH1I("hrdtMulti" + suffix, "RDT Multiplicity", numRDT, 0, numRDT);

  hrdt  = new TH1F * [numRDT];
  hrdt2D    = new TH2F * [numRDT/2];
  hrdt2Dg   = new TH2F * [numRDT/2];

  for (Int_t i = 0; i < numRDT ; i++) {
    if( i % 2 == 0 ) hrdt[i]  = new TH1F(Form("hrdt%d",i), Form("Raw Recoil E(ch=%d); E (channel)",i),         500,  rdtERange[0],  rdtERange[1]);
    if( i % 2 == 1 ) hrdt[i]  = new TH1F(Form("hrdt%d",i), Form("Raw Recoil DE(ch=%d); DE (channel)",i),       500, rdtDERange[0], rdtDERange[1]);
    
    ///dE vs E      
    if( i % 2 == 0 ) {
      int tempID = i / 2;
      hrdt2D[tempID]    = new TH2F(Form("hrdt2D%d",tempID) ,    Form("Raw Recoil DE vs Eres (dE=%d, E=%d); Eres (channel); DE (channel)", i+1, i),       500, rdtERange[0], rdtERange[1],500,rdtDERange[0],rdtDERange[1]);
      hrdt2Dg[tempID]   = new TH2F(Form("hrdt2Dg%d",tempID),   Form("Gated Raw Recoil DE vs Eres (dE=%d, E=%d); Eres (channel); DE (channel)",i+1, i),  500, rdtERange[0], rdtERange[1],500,rdtDERange[0], rdtDERange[1]);
    }
  }

  //===================== tDiff = array_t - rdt_t
  htDiff  = new TH1F("htDiff" + suffix,  "tDiff = e_t - rdt_t",         (coinTimeRange[1]-coinTimeRange[0]), coinTimeRange[0], coinTimeRange[1]);
  htDiffg = new TH1F("htDiffg" + suffix, "tDiff = e_t - rdt_t (gated)", (coinTimeRange[1]-coinTimeRange[0]), coinTimeRange[0], coinTimeRange[1]);
  htDiffg->SetLineColor(2);

  //===================== energy spectrum
  hEx = new TH1F("hEx" + suffix, Form("excitation spectrum w/ goodFlag; Ex [MeV] ; Count / %4.0f keV", exRange[0]), (int) (exRange[2]-exRange[1])/exRange[0]*1000, exRange[1], exRange[2]);
  
  TString haha = "Ex (det=%i) w/goodFlag; Ex [MeV]; Count / " +std::to_string(exRange[0]) + "keV";

  hExi = new TH1F * [numDet];
  hEx_xCal = new TH2F * [numDet];
  for(int i = 0; i < numDet; i++ ){
    hExi[i] = new TH1F(Form("hExi%d", i) + suffix, haha, (int) (exRange[2]-exRange[1])/exRange[0]*1000, exRange[1], exRange[2]);

    hEx_xCal[i] = new TH2F(Form("hEx_xCal%d", i) + suffix,
                           Form("Ex vs X (ch=%d); X (cm); Ex (MeV)", i),
                           500,   -0.1,  1.1, 
                           (int) (exRange[2]-exRange[1])/exRange[0]*1000, exRange[1], exRange[2]);
  }

  hExCut1  = new TH1F("hExCut1" + suffix,Form("excitation spectrum w/ goodFlag; Ex [MeV] ; Count / %4.0f keV", exRange[0]), (int) (exRange[2]-exRange[1])/exRange[0]*1000, exRange[1], exRange[2]);
  hExCut2  = new TH1F("hExCut2" + suffix,Form("excitation spectrum w/ goodFlag; Ex [MeV] ; Count / %4.0f keV", exRange[0]), (int) (exRange[2]-exRange[1])/exRange[0]*1000, exRange[1], exRange[2]);
  hExCut1->SetLineColor(2);
  hExCut2->SetLineColor(4);

  hEx_ThetaCM = new TH2F("hExThetaCM" + suffix, "Ex vs ThetaCM; ThetaCM [deg]; Ex [MeV]", 200, thetaCMRange[0], thetaCMRange[1],  (int) (exRange[2]-exRange[1])/exRange[0]*1000, exRange[1], exRange[2]);

}

//^####################################################### Plot
void MonPlotter::Plot(){

  //TODO a more user-friendly way. 
  //TODO display text on the plot. 

  for( int i = 1; i <= numPad; i++ ){
    canvas->cd(i);
    switch (i){
      case 1: heCal_z->Draw("colz");break;
      case 2: heCal_zGC->Draw("colz");break;
      case 3: {
        htDiff->Draw("");
        htDiffg->Draw("same");
      }break;
      case 4: hEx->Draw("colz");break;
      default:break;
    }
  }
}

//^####################################################### 
void MonPlotter::LoadRDTGate(TString rdtCutFile){

  if( rdtCutFile == "" ) return ;

  TFile * fCut = new TFile(rdtCutFile);
  bool isCutFileOpen = fCut->IsOpen();
  if(!isCutFileOpen) {
    printf( "Failed to open rdt-cutfile 1 : %s\n" , rdtCutFile.Data());
  }else{
    cutList = (TObjArray *) fCut->FindObjectAny("cutList");

    if( cutList ){
      int numCut = cutList->GetEntries();
      printf("=========== found %d cutG in %s \n", numCut, fCut->GetName());

      for(int i = 0; i < numCut ; i++){
        printf("cut name : %s , VarX: %s, VarY: %s, numPoints: %d \n",
          cutList->At(i)->GetName(),
          ((TCutG*)cutList->At(i))->GetVarX(),
          ((TCutG*)cutList->At(i))->GetVarY(),
          ((TCutG*)cutList->At(i))->GetN()
        );
      }
    }
  }

}

//^####################################################### 

void MonPlotter::PlotRaw(bool isLog){

  TCanvas * cRawID = new TCanvas("cRawID", Form("Raw e, Ring, xf, xn vs ID | %s", canvasTitle.Data()), 100 + 500 * aID, 100, 1200, 800);
  cRawID->Clear(); cRawID->Divide(2,2);
  cRawID->cd(1); he_ID->Draw("colz");
  cRawID->cd(2); hArrayMulti->Draw();
  cRawID->cd(3); hxf_ID->Draw("colz");
  cRawID->cd(4); hxn_ID->Draw("colz");

  int padSize = 200;
  int canvasSize[2] = {padSize * colDet, padSize * rowDet};

  TCanvas * cRawE = new TCanvas("cRawE" + suffix,Form("E raw | %s", canvasTitle.Data()), 200 + 500 * aID, 200, canvasSize[0], canvasSize[1]);
  cRawE->Clear(); cRawE->Divide(colDet,rowDet);
  for (Int_t i=0; i < numDet; i++) {
    cRawE->cd(i+1); 
    cRawE->cd(i+1)->SetGrid();
    if( isLog ) cRawE->cd(i+1)->SetLogy();
    he[i]->Draw("");
  }

  TCanvas *cRawXf = new TCanvas("cRawXf" + suffix,Form("Xf raw | %s", canvasTitle.Data()), 300 + 500 * aID, 300, canvasSize[0], canvasSize[1]);
  cRawXf->Clear(); cRawXf->Divide(colDet,rowDet);
  for (Int_t i=0; i<numDet; i++) {
    cRawXf->cd(i+1); 
    cRawXf->cd(i+1)->SetGrid();
    if( isLog ) cRawXf->cd(i+1)->SetLogy();
    hxf[i]->Draw("");
  }

  TCanvas *cRawXn = new TCanvas("cRawXn" + suffix,Form("Xn raw | %s", canvasTitle.Data()), 400 + 500 * aID, 400, canvasSize[0], canvasSize[1]);
  cRawXn->Clear();cRawXn->Divide(colDet,rowDet);
  for (Int_t i=0; i<numDet; i++) {
    cRawXn->cd(i+1); 
    cRawXn->cd(i+1)->SetGrid();
    if( isLog ) cRawXn->cd(i+1)->SetLogy();
    hxn[i]->Draw("");
  }

  TCanvas *cxfxn = new TCanvas("cxfxn" + suffix,Form("XF vs. XN | %s", canvasTitle.Data()), 500 + 500 * aID, 500, canvasSize[0], canvasSize[1]);
  cxfxn->Clear(); cxfxn->Divide(colDet,rowDet);
  for (Int_t i=0;i<numDet;i++) {
    cxfxn->cd(i+1); 
    cxfxn->cd(i+1)->SetGrid(); 
    hxf_xn[i]->Draw("col");
  }

  TCanvas *cxfxne = new TCanvas("cxfxne" + suffix,Form("E - XF+XN | %s", canvasTitle.Data()), 600 + 500 * aID, 600, canvasSize[0], canvasSize[1]);
  cxfxne->Clear(); cxfxne->Divide(colDet,rowDet);
  TLine line(0,0, 4000, 4000); line.SetLineColor(2);
  for (Int_t i=0;i<numDet;i++) {
    cxfxne->cd(i+1); 
    cxfxne->cd(i+1)->SetGrid(); 
    he_xs[i]->Draw("col");
    line.Draw("same");
  }

}

void MonPlotter::PlotCal(){

  int padSize = 200;
  int canvasSize[2] = {padSize * colDet, padSize * rowDet};

  TCanvas *ceVx = new TCanvas("ceVx" + suffix, Form("E vs. X = (xf-xn)/e | %s", canvasTitle.Data()), 100 + 500 * aID, 100, canvasSize[0], canvasSize[1]);
  ceVx->Clear(); ceVx->Divide(colDet,rowDet);
  for (Int_t i=0;i<numDet;i++) {
    ceVx->cd(i+1); he_x[i]->Draw("col");
  }

  TCanvas *cxfxneC = new TCanvas("cxfxneC" + suffix,Form("Raw E - Corrected XF+XN | %s", canvasTitle.Data()), 200 + 500 * aID, 200, canvasSize[0], canvasSize[1]);
  cxfxneC->Clear(); cxfxneC->Divide(colDet,rowDet);
  TLine * line = new TLine(0,0, 4000, 4000); 
  line->SetLineColor(2);
  for (Int_t i=0;i<numDet;i++) {
    cxfxneC->cd(i+1); 
    cxfxneC->cd(i+1)->SetGrid(); 
    he_xsCal[i]->Draw("col");
    line->Draw("same");
  }

  TCanvas *cEC = new TCanvas("cEC" + suffix,Form("E corrected | %s", canvasTitle.Data()), 300 + 500 * aID, 300, canvasSize[0], canvasSize[1]);
  cEC->Clear();cEC->Divide(colDet,rowDet);
  for (Int_t i=0; i<numDet; i++) {
    cEC->cd(i+1); 
    cEC->cd(i+1)->SetGrid();
    heCal[i]->Draw("");
  }
  
  TCanvas *cEC2 = new TCanvas("cEC2" + suffix,Form("E corrected | %s", canvasTitle.Data()), 400 + 500 * aID, 400, canvasSize[0], canvasSize[1]);
  cEC2->Clear();
  heCal_ID->Draw("colz");

  TCanvas *cxfxnC = new TCanvas("cxfxnC" + suffix,Form("XF vs XN corrected | %s", canvasTitle.Data()), 500 + 500 * aID, 500, canvasSize[0], canvasSize[1]);
  cxfxnC->Clear(); cxfxnC->Divide(colDet,rowDet);
  for (Int_t i=0;i<numDet;i++) {
    cxfxnC->cd(i+1); 
    cxfxnC->cd(i+1)->SetGrid(); 
    hxfCal_xnCal[i]->Draw("col");
  }

}

void MonPlotter::PlotEZ(){
  TCanvas *cecalVz = new TCanvas("cevalVz" + suffix,Form("ECALVZ : %s", canvasTitle.Data()),1000, 650);
  cecalVz->Clear(); cecalVz->Divide(2,1);
  gStyle->SetOptStat("neiou");
  cecalVz->cd(1);heCal_z->Draw("col");
  cecalVz->cd(2);heCal_zGC->Draw("col");

}

void MonPlotter::PlotEx(){

  TCanvas *cExVxCal = new TCanvas("cExVxCal" + suffix,Form("EX | %s", canvasTitle.Data()), 200 + 1000 * aID, 200, 1600,1000);
  cExVxCal->Clear();
  gStyle->SetOptStat("neiou");
  cExVxCal->Divide(colDet,rowDet);
  for( int i = 0; i < numDet; i++){
    cExVxCal->cd(i+1); 
    hEx_xCal[i]->SetMarkerStyle(7);
    hEx_xCal[i]->Draw();
  }
  
  TCanvas *cexI = new TCanvas("cexI" + suffix,Form("EX : %s", canvasTitle.Data()),300 + 1000 * aID, 300, 1600,1000);
  cexI->Clear();cexI->Divide(colDet,rowDet);
  gStyle->SetOptStat("neiou");
  for( int i = 0; i < numDet; i++){
    cexI->cd(i+1); 
    hExi[i]->Draw("");
  }

  TCanvas *cExThetaCM = new TCanvas("cExThetaCM" + suffix,Form("EX - ThetaCM | %s", canvasTitle.Data()), 400 + 1000 * aID, 400, 650,650);
  cExThetaCM->Clear();
  gStyle->SetOptStat("neiou");
  hEx_ThetaCM->Draw("colz");

  TCanvas *cex = new TCanvas("cex" + suffix,Form("EX : %s", canvasTitle.Data()), 500 + 1000 * aID, 500, 1000,650);
  cex->Clear();
  gStyle->SetOptStat("neiou");
  hEx->Draw("");

}

void MonPlotter::PlotRDT(bool isLog){

  TCanvas *crdt = new TCanvas("crdt" + suffix,Form("raw RDT | %s", canvasTitle.Data()), 1000, 0, 1000,1000);
  crdt->Clear();crdt->Divide(numRDT/4,2);

  for( int i = 0; i < numRDT/2; i++){
    if( isLog ) crdt->cd(i+1)->SetLogz(); crdt->cd(i+1); hrdt2D[i]->Draw("col");  
  }
  
  TCanvas *crdtID = new TCanvas("crdtID" + suffix,Form("raw RDT ID | %s", canvasTitle.Data()),1100,1100, 500, 500);
  crdtID->Clear();
  if( isLog ) crdtID->SetLogz(); 
  hrdt_ID->Draw("colz");
  
  TCanvas *crdtS = new TCanvas("crdtS" + suffix,Form("raw RDT | %s", canvasTitle.Data()),1200, 1200, 1000, 1000);
  crdtS->Clear(); crdtS->Divide(2,numRDT/2);
  for( int i = 0; i < numRDT; i ++){
    crdtS->cd(i+1);
    if( isLog ) crdtS->cd(i+1)->SetLogy(); 
    hrdt[i]->Draw("");
  }

}

#endif