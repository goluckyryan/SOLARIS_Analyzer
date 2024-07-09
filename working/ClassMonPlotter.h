#ifndef ClassMonitorPlotter_H
#define ClassMonitorPlotter_H

#include "../Armory/ClassDetGeo.h"
#include "../Armory/ClassReactionConfig.h"
#include "../Cleopatra/ClassTransfer.h"
#include "../Cleopatra/ClassIsotope.h"

#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"

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
  void SetUpHistograms(int * rawEnergyRange, int * energyRange, double * exRange, int * thetaCMRange, int * rdtDERange, int * rdtERange);

  void Plot();

  void PlotRawID();
  
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

  //===== eCal V z
  TH2F  * heCal_z;
  TH2F  * heCal_zGC;

  //======= Recoil
  TH2F * hrdt_ID;
  TH1F ** hrdt; // single recoil

  TH2F ** hrdt2D;
  TH2F ** hrdt2Dg;

  //====== Ex data
  TH1F  * hEx;
  TH1F ** hExi;
  TH2F ** hEx_xCal;

  TH1F * hExCut1;
  TH1F * hExCut2;

  TH2F * hEx_ThetaCM;
  //=======================

private:

  unsigned short aID;
  int numDet, colDet, rowDet; //array
  float detLength;
  int numRDT;
  float recoilOutter;
  double zRange[2] ; // zMin, zMax
  
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

  this->numRDT = numDet;
  recoilOutter = detGeo->aux[aID].outerRadius;

  zRange[0] = detGeo->array[aID].zMin - 50;
  zRange[1] = detGeo->array[aID].zMax + 50;

  canvas = nullptr;

}

MonPlotter::~MonPlotter(){

  printf("=============== %s\n", __func__);

  delete canvas;

  delete he_ID;
  delete hxf_ID;
  delete hxn_ID;
  delete hArrayMulti;

  delete heCal_z;
  delete hEx_ThetaCM;
  delete hExCut1;
  delete hExCut2;
  delete heCal_zGC;

  delete hrdt_ID;
  
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

}

void MonPlotter::SetUpCanvas(TString title, int padSize, int divX, int divY){

  canvas = new TCanvas("canavs" + suffix, title, 200 * aID, 200 * aID, divX * padSize, divY * padSize);
  canvas->Divide(divX, divY);
  numPad = divX * divY;

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

void MonPlotter::SetUpHistograms(int * rawEnergyRange, int * energyRange, double * exRange, int * thetaCMRange, int * rdtDERange, int * rdtERange){

  he_ID  = new TH2F("he_ID"  + suffix, "Raw e vs array ID; Array ID; Raw e",    numDet, 0, numDet,  200, rawEnergyRange[0], rawEnergyRange[1]);
  hxf_ID = new TH2F("hxf_ID" + suffix, "Raw xf vs array ID; Array ID; Raw xf",  numDet, 0, numDet,  200, rawEnergyRange[0], rawEnergyRange[1]);
  hxn_ID = new TH2F("hxn_ID" + suffix, "Raw xn vs array ID; Array ID; Raw xn",  numDet, 0, numDet,  200, rawEnergyRange[0], rawEnergyRange[1]);

  hArrayMulti = new TH1I("hArrayMulti", "Array Multiplicity", numDet, 0, numDet);

  CreateListOfHist1D(he,   numDet, "he",  "Raw e (ch=%d); e (channel); count",            200, rawEnergyRange[0], rawEnergyRange[1]);
  CreateListOfHist1D(hxf,  numDet, "hxf", "Raw xf (ch=%d); e (channel); count",           200, rawEnergyRange[0], rawEnergyRange[1]);
  CreateListOfHist1D(hxn,  numDet, "hxn", "Raw xn (ch=%d); e (channel); count",           200, rawEnergyRange[0], rawEnergyRange[1]);

  CreateListOfHist2D(hxf_xn, numDet, "hxf_xn", "Raw xf vs. xn (ch=%d);xf (channel);xn (channel)"     , 500, rawEnergyRange[0], rawEnergyRange[1], 500, rawEnergyRange[0], rawEnergyRange[1]);
  CreateListOfHist2D(he_xs,  numDet, "he_xs",  "Raw e vs xf+xn (ch=%d); xf+xn (channel); e (channel)", 500, rawEnergyRange[0], rawEnergyRange[1], 500, rawEnergyRange[0], rawEnergyRange[1]);

  CreateListOfHist1D(heCal, numDet, "heCal",       "Corrected e (ch=%d); e (MeV); count", 2000,    energyRange[0],    energyRange[1]);

  CreateListOfHist2D(hxfCal_xnCal, numDet, "hxfCal_xnCal", "Corrected XF vs. XN (ch=%d);XF (channel);XN (channel)",                         500,                 0, rawEnergyRange[1], 500,                 0, rawEnergyRange[1]);      
  CreateListOfHist2D(he_xsCal    , numDet, "he_xsCal",     "Raw e vs Corrected xf+xn (ch=%d); corrected xf+xn (channel); Raw e (channel)",  500, rawEnergyRange[0], rawEnergyRange[1], 500, rawEnergyRange[0], rawEnergyRange[1]);
  
  CreateListOfHist2D(he_x        , numDet, "he_x",     "Raw e vs x (ch=%d); x (mm); Raw e (channel)",  500, rawEnergyRange[0], rawEnergyRange[1], 500, -1, detLength +1);

  //====================== E-Z plot
  heCal_z   = new TH2F("heCal_z" + suffix ,  "E vs. Z;Z (mm);E (MeV)"      , 400, zRange[0], zRange[1], 400, energyRange[0], energyRange[1]);
  heCal_zGC = new TH2F("heCal_zGC" + suffix ,"E vs. Z gated;Z (mm);E (MeV)", 400, zRange[0], zRange[1], 400, energyRange[0], energyRange[1]);

  //===================== Recoil
  int rdtRange[2];
  rdtRange[0] = rdtDERange[0] < rdtERange[0] ? rdtDERange[0] : rdtERange[0];
  rdtRange[1] = rdtDERange[1] > rdtERange[1] ? rdtDERange[1] : rdtERange[1];

  hrdt_ID = new TH2F("hrdt_ID" + suffix, "Raw RDT vs ID; ID; Raw RDT", numRDT, 0, numRDT, 400, rdtRange[0], rdtRange[1]);

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

void MonPlotter::Plot(){

  for( int i = 0; i < numPad; i++ ){
    canvas->cd(i+1);
    switch (i){
      case 0: heCal_z->Draw("colz");break;
      case 1: heCal_zGC->Draw("colz");break;
      // case 2: hxn_ID->Draw("colz");break;
      default:break;
    }
  }
}

void MonPlotter::PlotRawID(){
  TCanvas * haha = new TCanvas("haha" + suffix, "Raw ID", 1200, 600);
  haha->Divide(2,2);

  haha->cd(1); he_ID->Draw("colz");
  haha->cd(2); hArrayMulti->Draw();
  haha->cd(3); hxf_ID->Draw("colz");
  haha->cd(4); hxn_ID->Draw("colz");
}

#endif