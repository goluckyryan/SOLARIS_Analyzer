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
*   TH2D is always using "V" to seperate 2 variables, like eVx    *
*                                                                 *
*  histogram with TCutG, add suffix "GC" for Graphical-Cut.       *
*                                                                 *
*******************************************************************/

class MonPlotter{
public:

  MonPlotter(unsigned short arrayID, DetGeo * detGeo);
  ~MonPlotter();

  void SetUpCanvas(TString title, int sizeX, int sizeY, int divX, int divY);

  void SetUpHistograms(int * energyRange, double * exRange, int * thetaCMRange, int numRDT);

  void Plot();
  
  TCanvas * canvas;

  //===== eCal V z
  TH2F  * heCalVz;
  TH2F  * heCalVzGC;

  //====== Ex data
  TH1F  * hEx;
  TH1F ** hExi;
  TH2F ** hExVxCal;

  TH2F * hExThetaCM;

  TH1F * hExCut1;
  TH1F * hExCut2;


private:

  unsigned short aID;
  int numDet, colDet, rowDet; //array
  int numRDT;
  // DetGeo detGeo;
  double zRange[2] ; // zMin, zMax
  
  TString suffix;

}

MonPlotter::MonPlotter(unsigned short arrayID, DetGeo * detGeo){
  aID = arrayID;
  numDet = detGeo->array[aID].numDet;
  colDet = detGeo->array[aID].colDet;
  rowDet - numDet/colDet;

  suffix = Form("_%d", arrayID);

  zRange[0] = detGeo.array[aID].zMin - 50;
  zRange[1] = detGeo.array[aID].zMax + 50;

  canvas = nullptr;

}

MonPlotter::~MonPlotter(){

  delete canvas;

  delete [] heCalVz;
  delete [] heCalVzGC;
  delete [] hEx;
  for( int i = 0; i < detGeo.array[aID].numDet ; i++ ){
    delete [] hExi[i];
    delete [] hExVxCal[i];
  }
  delete [] hExi;
  delete [] hExVxCal;
  delete [] hExThetaCM;
  delete [] hExCut1;
  delete [] hExCut2;


}

void MonPlotter::SetUpCanvas(TString title, int sizeX, int sizeY, int divX, int divY){

  canvas = new TCanvas("canavs" + suffix, title, 200 * aID, 200 * aID, sizeX, sizeY);
  canvas->Divide(divX, divY);

}

void MonPlotter::SetUpHistograms(int * energyRange, double * exRange, int * thetaCMRange){

  this->numRDT = numRDT;

  //====================== E-Z plot
  heCalVz   = new TH2F("heCalVz" + suffix ,  "E vs. Z;Z (mm);E (MeV)"      , 400, zRange[0], zRange[1], 400, energyRange[0], energyRange[1]);
  heCalVzGC = new TH2F("heCalVzGC" + suffix ,"E vs. Z gated;Z (mm);E (MeV)", 400, zRange[0], zRange[1], 400, energyRange[0], energyRange[1]);

    //===================== energy spectrum
  hEx    = new TH1F("hEx",Form("excitation spectrum w/ goodFlag; Ex [MeV] ; Count / %4.0f keV", exRange[0]), (int) (exRange[2]-exRange[1])/exRange[0]*1000, exRange[1], exRange[2]);
  
  TString haha = "Ex (det=%i) w/goodFlag; Ex [MeV]; Count / " +std::to_string(exRange[0]) + "keV";

  hExi = new TH1F * [numDet];
  hExVxCal = new TH2F * [numDet];
  for(int i = 0; i < numDet; i++ ){
    hExi[i] = new TH1F(Form("hExi%d", i) + suffix, haha, (int) (exRange[2]-exRange[1])/exRange[0]*1000, exRange[1], exRange[2]);

    hExVxCal[i] = new TH2F(Form("hExVxCal%d", i) + suffix,
                           Form("Ex vs X (ch=%d); X (cm); Ex (MeV)", i),
                           500,   -0.1,  1.1, 
                           (int) (exRange[2]-exRange[1])/exRange[0]*1000, exRange[1], exRange[2]);
  }

  hExCut1  = new TH1F("hExCut1" + suffix,Form("excitation spectrum w/ goodFlag; Ex [MeV] ; Count / %4.0f keV", exRange[0]), (int) (exRange[2]-exRange[1])/exRange[0]*1000, exRange[1], exRange[2]);
  hExCut2  = new TH1F("hExCut2" + suffix,Form("excitation spectrum w/ goodFlag; Ex [MeV] ; Count / %4.0f keV", exRange[0]), (int) (exRange[2]-exRange[1])/exRange[0]*1000, exRange[1], exRange[2]);
  hExCut1->SetLineColor(2);
  hExCut2->SetLineColor(4);

  hExThetaCM = new TH2F("hExThetaCM" + suffix, "Ex vs ThetaCM; ThetaCM [deg]; Ex [MeV]", 200, thetaCMRange[0], thetaCMRange[1],  (int) (exRange[2]-exRange[1])/exRange[0]*1000, exRange[1], exRange[2]);

}


#endif