#ifndef SimPlotter_h
#define SimPlotter_h

#include <TTree.h>
#include <TH2F.h>
#include <TH1F.h>
#include <TString.h>
#include <TCanvas.h>
#include <TArc.h>
#include <TLegend.h>
#include <TLine.h>
#include <TLatex.h>
#include "../Armory/ClassDetGeo.h"
#include "../Armory/ClassReactionConfig.h"
#include "../Cleopatra/ClassIsotope.h"

enum plotID { pEZ,               /// 0
              pRecoilXY,         /// 1
              pRecoilXY1,        /// 2
              pRecoilXY2,        /// 3
              pRecoilRZ,         /// 4
              pRecoilRTR,        /// 5
              pTDiffZ,           /// 6
              pThetaCM,          /// 7
              pThetaCM_Z,        /// 8
              pExCal,            /// 9
              pRecoilRThetaCM,   /// 10
              pArrayXY,          /// 11
              pInfo,             /// 12
              pElum1XY,          /// 13
              pEElum1R,          /// 14
              pElum1RThetaCM,    /// 15
              pEmpty };          /// 16

class Plotter{
public:
  Plotter(TTree * tree, short reactionID, std::string reactionName, DetGeo detGeo, ExcitedEnergies exList, TString gate, std::vector<plotID> padPlotID);
  ~Plotter();

  void SetRanges(double zMin, double zMax, double eMax, double elumMax, double thetaCM_Max){
    this->zRange[0] = zMin;
    this->zRange[1] = zMax;
    this->eMax = eMax;
    this->elumMax = elumMax;
    this->thetaCM_Max = thetaCM_Max;
  } 
  void SetCanvas( int colSize, int rowSize , int padSize, std::vector<plotID> padPlotID ){    
    cCheck = new TCanvas(Form("SimChecker-%d", rID), 
                         "Simulation Checker", 
                         200 * rID, 
                         200 * rID, 
                         padSize*colSize, 
                         padSize*rowSize);

    if(cCheck->GetShowEditor() ) cCheck->ToggleEditor();
    if(cCheck->GetShowToolBar() )cCheck->ToggleToolBar();
    cCheck->Divide(colSize,rowSize);

    numPad = colSize * rowSize;
  }

  void Plot();

private:

  TTree * tree;
  DetGeo detGeo;
  std::string reactionName;
  short rID;
  ExcitedEnergies exList;
  int numEx;
  TString gate;
  TString gateTrue;
  std::vector<plotID> padPlotID;

  double zRange[2];
  double eMax;
  double recoilOut;
  double recoilIn;
  double elumMax;
  double thetaCM_Max;

  int numPad;
  TCanvas * cCheck;
  TH2F * hez;
  TH2F * hArrayXY;
  TH2F * heavyXY;
  TH2F * hElum1XY;
  TH1F ** hThetaCM;
  TH1F * hExCal;
  TH1F * hHit;
  TH2F * hRecoilXY1;
  TH2F * hRecoilXY2;
  TH2F * hTDiffZ;
  TH2F * hRecoilRThetaCM;
  TH2F * hRecoilRZ;
  TH2F * hEElum1Rho;
  TH2F * hRecoilRTR;
  TH2F * hElum1RThetaCM;
  TH2F * hThetaCM_Z;
  
  Isotope heavy;

  std::vector<double> FindRange(TString branch);

}; 

Plotter::Plotter(TTree * tree, short reactionID, std::string reactionName, DetGeo detGeo, ExcitedEnergies exList, TString gate, std::vector<plotID> padPlotID){
  this->tree = tree;
  this->rID = reactionID;
  this->reactionName = reactionName;
  this->exList = exList;
  numEx = this->exList.ExList.size();

  this->detGeo = detGeo;

  this->recoilOut = detGeo.aux[rID].outerRadius;
  this->recoilIn = detGeo.aux[rID].innerRadius;

  this->gate = gate;
  this->gateTrue = gate + Form(" && rID == %d", rID);

  this->padPlotID = padPlotID;

  cCheck = nullptr;
  hez = nullptr;
  hArrayXY = nullptr;
  heavyXY = nullptr;
  hElum1XY = nullptr;
  hThetaCM = nullptr;
  hExCal = nullptr;
  hHit = nullptr;
  hRecoilXY1 = nullptr;
  hRecoilXY2 = nullptr;
  hTDiffZ = nullptr;
  hRecoilRThetaCM = nullptr;
  hRecoilRZ = nullptr;
  hEElum1Rho = nullptr;
  hRecoilRTR = nullptr;
  hElum1RThetaCM = nullptr;
  hThetaCM_Z = nullptr;

  std::size_t pos1 = reactionName.find(")") + 1;
  std::size_t pos2 = reactionName.find(" ", pos1);
  std::string result = reactionName.substr(pos1, pos2 - pos1);

  pos1 = result.find("{") + 1;
  pos2 = result.find("}", pos1);

  std::string massHeavy = result.substr(pos1, pos2-pos1);
  std::string symHeavy = result.substr(pos2+1);

  heavy.SetIsoByName(massHeavy + symHeavy);

}

Plotter::~Plotter(){

  if( cCheck ) delete cCheck;
  if( hez ) delete hez;
  if( hArrayXY ) delete hArrayXY;
  if( heavyXY ) delete heavyXY;
  if( hElum1XY ) delete hElum1XY;
  if( hExCal ) delete hExCal;
  if( hRecoilXY1 ) delete hRecoilXY1;
  if( hRecoilXY2 ) delete hRecoilXY2;
  if( hTDiffZ ) delete hTDiffZ;
  if( hRecoilRThetaCM ) delete hRecoilRThetaCM;
  if( hRecoilRZ ) delete hRecoilRZ;
  if( hEElum1Rho ) delete hEElum1Rho;
  if( hRecoilRTR ) delete hRecoilRTR;
  if( hElum1RThetaCM ) delete hElum1RThetaCM;
  if( hThetaCM_Z ) delete hThetaCM_Z;

  if(hThetaCM ){
    for( int i = 0; i < numEx; i++) delete [] hThetaCM[i];
    delete [] hThetaCM;
  }

}

inline void Plotter::Plot(){

  for( int i = 1; i <= numPad; i++ ){
    cCheck->cd(i);
    plotID  pID = padPlotID[i-1];

    //&======================================= 0 
    if( pID == pEZ){
      hez = new TH2F(Form("hez%d",rID), //need unqie name in root.
                     Form("e-z [gated] @ %5.0f mm; z [mm]; e [MeV]", detGeo.array[rID].firstPos), 
                     400, zRange[0], zRange[1], 
                     400, 0, eMax);

      tree->Draw(Form("e:z>>hez%d", rID), gateTrue , "colz");
    }

    //&======================================= 1
    if( pID == pRecoilXY  ){
      heavyXY = new TH2F(Form("heavyXY%d",rID), 
                           Form("RecoilXY [gated] @ %4.0f mm; X [mm]; Y [mm]", detGeo.aux[rID].detPos ), 
                           400, -recoilOut, recoilOut, 
                           400, -recoilOut, recoilOut);
      tree->Draw(Form("yRecoil:xRecoil>>heavyXY%d", rID), gateTrue, "colz");
      
      // printf("%f, %f\n", recoilOut, recoilIn);
      TArc * detArc1 = new TArc(0, 0, recoilOut);
      detArc1->SetLineColor(kBlue-8);
      detArc1->SetFillStyle(0);
      detArc1->Draw("same");  
      TArc * detArc2 = new TArc(0, 0, recoilIn);
      detArc2->SetLineColor(kBlue-8);
      detArc2->SetFillStyle(0);
      detArc2->Draw("same");  
        
      // if( reactionConfig.beamX != 0. || reactionConfig.beamY != 0. ){
      //   TArc * arc = new TArc(reactionConfig.beamX, reactionConfig.beamY, 1);
      //   arc->SetLineColor(2);
      //   detArc1->SetFillStyle(0);
      //   arc->Draw("same");
      // }
    }

    //&======================================= 2
    if( pID == pThetaCM   ){
      hThetaCM = new TH1F *[numEx]; 
      TLegend * legend = new TLegend(0.8,0.2,0.99,0.8);
      double maxCount = 0;
      for( int h = 0; h < numEx; h++){
        hThetaCM[h] = new TH1F(Form("hThetaCM%d-%d", rID, h), Form("thetaCM [gated] (ExID=%d); thetaCM [deg]; count", h), 200, 0, thetaCM_Max);
        hThetaCM[h]->SetLineColor(h+1);
        hThetaCM[h]->SetFillColor(h+1);
        hThetaCM[h]->SetFillStyle(3000+h);
        tree->Draw(Form("thetaCM>>hThetaCM%d-%d", rID, h), gateTrue + Form("&& ExID==%d", h), "");
        legend->AddEntry(hThetaCM[h], Form("Ex=%5.1f MeV", exList.ExList[h].Ex));
        double max = hThetaCM[h]->GetMaximum();
        if( max > maxCount ) maxCount = max;
      }

      for( int h = 0; h < numEx; h++){
        hThetaCM[h]->GetYaxis()->SetRangeUser(1, maxCount * 1.2);
        if( h == 0 ) {
          hThetaCM[h]->Draw();
        }else{
          hThetaCM[h]->Draw("same");
        }
      }   
      legend->Draw();
    }

    //&======================================= 3
    if( pID == pExCal  ){
      double exMin = 9999;
      double exMax = -1;
      for( size_t k = 0 ; k < exList.ExList.size(); k++){
        double kuku = exList.ExList[k].Ex;
        if( kuku > exMax ) exMax = kuku;
        if( kuku < exMin ) exMin = kuku;
      }

      double exPadding = (exMax - exMin) < 1 ? 1 : (exMax - exMin) * 0.3;

      exMin = exMin - exPadding ;
      exMax = exMax + exPadding ;

      hExCal = new TH1F(Form("hExCal%d",rID), 
                            Form("calculated Ex [gated]; Ex [MeV]; count / %.2f keV",  (exMax-exMin)/400.*1000),  
                            400, exMin, exMax);
      tree->Draw(Form("ExCal>>hExCal%d", rID), gateTrue, "");


      double Sn = heavy.CalSp(0,1);
      double Sp = heavy.CalSp(1,0);
      double Sa = heavy.CalSp2(4,2);
      double S2n = heavy.CalSp(0, 2);

      printf("======= Heavy recoil: %s \n", heavy.Name.c_str());
      printf("Sn  : %6.2f MeV/u \n", Sn);
      printf("Sp  : %6.2f MeV/u \n", Sp);
      printf("Sa  : %6.2f MeV/u \n", Sa);
      printf("S2n : %6.2f MeV/u \n", S2n);

      double yMax = hExCal->GetMaximum();
      TLatex * text = new TLatex();
      text->SetTextFont(82);
      text->SetTextSize(0.06);
      
      if(  Sn < exMax ) {TLine * lineSn  = new TLine(Sn,  0,  Sn, yMax); lineSn->SetLineColor(2);  lineSn->Draw("");      text->SetTextColor(2); text->DrawLatex(Sn,  yMax*0.9, "S_{n}");}
      if(  Sp < exMax ) {TLine * lineSp  = new TLine(Sp,  0,  Sp, yMax); lineSp->SetLineColor(4);  lineSp->Draw("same");  text->SetTextColor(4); text->DrawLatex(Sp,  yMax*0.9, "S_{p}");}
      if(  Sa < exMax ) {TLine * lineSa  = new TLine(Sa,  0,  Sa, yMax); lineSa->SetLineColor(6);  lineSa->Draw("same");  text->SetTextColor(6); text->DrawLatex(Sa,  yMax*0.9, "S_{a}");}
      if( S2n < exMax ) {TLine * lineS2n = new TLine(S2n, 0, S2n, yMax); lineS2n->SetLineColor(8); lineS2n->Draw("same"); text->SetTextColor(8); text->DrawLatex(S2n, yMax*0.9, "S_{2n}");}
      
    }

    //&======================================= 4
    if( pID == pArrayXY ){

      hArrayXY = new TH2F(Form("hArrayXY%d", rID), 
                              "Array-XY [gated]; X [mm]; Y [mm]", 
                              400, -detGeo.array[rID].detPerpDist*1.5, detGeo.array[rID].detPerpDist*1.5, 
                              400, -detGeo.array[rID].detPerpDist*1.5, detGeo.array[rID].detPerpDist*1.5);
      tree->Draw(Form("yArray:xArray>>hArrayXY%d", rID), gateTrue, "colz");
    }

    //&======================================= 5
    if( pID == pInfo ){
      TLatex text;
      text.SetNDC();
      text.SetTextFont(82);
      text.SetTextSize(0.06);
      text.SetTextColor(2);

      text.DrawLatex(0., 0.9, reactionName.c_str());
      text.DrawLatex(0., 0.8, detGeo.Bfield > 0 ? "out of plan" : "into plan");
      text.SetTextColor(1);
      text.DrawLatex(0., 0.7, "gate:");
      
      text.SetTextColor(2);
      //check gate text length, if > 30, break by "&&" 
      int ll = gate.Length();
      if( ll > 30 ) {
        std::vector<std::string> strList = AnalysisLib::SplitStr( (std::string) gate.Data(), "&&");
        for( int i = 0; i < strList.size(); i++){
          text.DrawLatex(0., 0.6 - 0.05*i, (TString) strList[i]);
        }
      }else{
        text.DrawLatex(0., 0.6, gate);
      }

      // if( reactionConfig.beamX != 0.0 || reactionConfig.beamY != 0.0 ){
      //   text.DrawLatex(0.0, 0.1, Form("Bema pos: (%4.1f, %4.1f) mm", reactionConfig.beamX, reactionConfig.beamY));
      // }
    }

    //&======================================= 6
    if( pID == pElum1XY ){
      hElum1XY = new TH2F(Form("hElum1XY%d", rID), 
                                Form("Elum-1 XY [gated] @ %.0f mm ; X [mm]; Y [mm]", detGeo.aux[rID].elumPos1),  
                                400, -elumMax, elumMax, 
                                400, -elumMax, elumMax);
      tree->Draw(Form("yElum1:xElum1>>hElum1XY%d", rID), gateTrue, "colz");
        
      double count = hElum1XY->GetEntries();        
      if( count < 2000. ) {
        hElum1XY->SetMarkerStyle(7);
        if( count < 500. ) hElum1XY->SetMarkerStyle(3);
        hElum1XY->Draw("scat");
      }
    }

    //&======================================= 7
    if( pID == pRecoilXY1       ){
      TH2F * hRecoilXY1 = new TH2F(Form("hRecoilXY1-%d", rID), 
                                   Form("RecoilXY-1 [gated] @ %4.0f mm; X [mm]; Y [mm]", detGeo.aux[rID].detPos1 ), 
                                   400, -detGeo.aux[rID].outerRadius, detGeo.aux[rID].outerRadius, 
                                   400, -detGeo.aux[rID].outerRadius, detGeo.aux[rID].outerRadius);
      tree->Draw(Form("yRecoil1:xRecoil1>>hRecoilXY1-%d", rID), gateTrue, "colz");
    }

    //&======================================= 8
    if( pID == pRecoilXY2       ){
      hRecoilXY2 = new TH2F(Form("hRecoilXY2=%d", rID),
                                   Form("RecoilXY-2 [gated] @ %4.0f mm; X [mm]; Y [mm]", detGeo.aux[rID].detPos2 ), 
                                  400, -detGeo.aux[rID].outerRadius, detGeo.aux[rID].outerRadius, 
                                  400, -detGeo.aux[rID].outerRadius, detGeo.aux[rID].outerRadius);
      tree->Draw(Form("yRecoil2:xRecoil2>>hRecoilXY2-%d", rID), gate, "colz");
    }

    //&======================================= 9
    if( pID == pTDiffZ         ){
      std::vector<double> tDiffRange = FindRange("t-tB");
      hTDiffZ = new TH2F(Form("hTDiffZ%d", rID), 
                         "time(Array) - time(Recoil) vs Z [gated]; z [mm]; time diff [ns]", 
                         400, zRange[0], zRange[1],  
                         400, tDiffRange[0], tDiffRange[1]);
      tree->Draw(Form("t - tB : z >> hTDiffZ%d", rID), gateTrue, "colz");
    }

    //&======================================= 10
    if( pID == pRecoilRThetaCM ){
      hRecoilRThetaCM = new TH2F(Form("hRecoilRThetaCM%d", rID), 
                                 "RecoilR - thetaCM [gated]; thetaCM [deg]; RecoilR [mm]", 
                                 400, 0, 60, 
                                 400, 0, detGeo.aux[rID].outerRadius);
      tree->Draw(Form("rhoRecoil:thetaCM>>hRecoilRThetaCM%d", rID), gateTrue, "colz");
    }

    //&======================================= 11
    if( pID == pRecoilRZ       ){
      hRecoilRZ = new TH2F(Form("hRecoilRZ%d", rID), 
                           "RecoilR - Z [gated]; z [mm]; RecoilR [mm]",  
                           400, zRange[0], zRange[1], 
                           400,0, detGeo.aux[rID].outerRadius);
      tree->Draw(Form("rhoRecoil:z>>hRecoilRZ%d", rID), gateTrue, "colz");
    }

    //&======================================= 12
    if( pID == pEElum1R ){
      hEElum1Rho = new TH2F(Form("hEElum1Rho%d", rID), 
                            "Elum-1 E-R [gated]; R[mm]; Energy[MeV]",  
                            400, 0, elumMax, 
                            400, 0, eMax);
      tree->Draw(Form("Tb:rhoElum1>>hEElum1Rho%d", rID), gateTrue, "colz");
    }

    //&======================================= 13
    if( pID == pRecoilRTR      ){
      std::vector<double> recoilERange =  FindRange("TB");
      hRecoilRTR = new TH2F(Form("hRecoilRTR%d", rID), 
                            "RecoilR - recoilE [gated]; recoil Energy [MeV]; RecoilR [mm]", 
                            500, recoilERange[0], recoilERange[1], 
                            500, 0, detGeo.aux[rID].outerRadius);
      tree->Draw(Form("rhoRecoil:TB>>hRecoilRTR%d", rID), gateTrue, "colz");
    }

    //&======================================= 14
    if( pID == pThetaCM_Z      ){
      hThetaCM_Z = new TH2F(Form("hThetaCM_Z%d", rID),
                            "ThetaCM vs Z ; Z [mm]; thetaCM [deg]",
                            400, zRange[0], zRange[1], 
                            400, 0, thetaCM_Max);
      tree->Draw(Form("thetaCM:z>>hThetaCM_Z%d", rID), gateTrue,"col");
    }

    //&======================================= 15
    if( pID == pElum1RThetaCM){
      int angBin = 400;
      
      hElum1RThetaCM = new TH2F(Form("hElum1RThetaCM%d", rID), 
                                "Elum-1 rho vs ThetaCM [gated]; thatCM [deg]; Elum- rho [mm]", 
                                angBin, 0, thetaCM_Max,  
                                400, 0, elumMax);
      tree->Draw(Form("rhoElum1:thetaCM>>hElum1RThetaCM%d", rID), gateTrue, "colz");   
      
      TH1F * htemp = (TH1F *) hElum1RThetaCM->ProjectionX("htemp");  
      
      double rel = (thetaCM_Max)*1.0/angBin;
      printf("angular resolution : %f deg \n", rel);
      
      std::vector<double> xList;
      double old_y = 0;
      for( int i = 1; i <= angBin; i++){
        double y = htemp->GetBinContent(i);
        if( old_y == 0 && y > 0) xList.push_back(htemp->GetBinCenter(i));
        if( old_y > 0 && y == 0 ) xList.push_back(htemp->GetBinCenter(i));
        old_y = y;
      }
              
      printf("list of gaps :\n");
      for( int i = 0; i < (int) xList.size(); i+=2){
        printf("%d | %.3f - %.3f deg\n", i, xList[i], xList[i+1]);
      }
      
      TF1 f1("f1", "sin(x)");
      double acceptance = 0;
      double err1 = 0;
      double err2 = 0;
      for( int i = 0; i < (int) xList.size(); i += 2 ){
        acceptance += f1.Integral(xList[i] * TMath::DegToRad(), xList[i+1] * TMath::DegToRad() ) * TMath::TwoPi();
        err1 += f1.Integral((xList[i]-rel) * TMath::DegToRad(), (xList[i+1] + rel) * TMath::DegToRad() ) * TMath::TwoPi();
        err2 += f1.Integral((xList[i]+rel) * TMath::DegToRad(), (xList[i+1] - rel) * TMath::DegToRad() ) * TMath::TwoPi();
      }
      printf("acceptance = %f sr +- %f \n", acceptance, (err1-err2)/2);
      
      TLatex text;
      text.SetTextFont(82);
      text.SetTextSize(0.06);
      text.SetTextColor(2);
      text.SetTextAngle(90);

      for( int i = 0; i < (int) xList.size(); i++){
        text.DrawLatex(xList[i], elumMax/2, Form("%.2f", xList[i]));
      }
      
      text.SetNDC();
      text.SetTextAngle(0);
      text.DrawLatex(0.15, 0.15, Form("accp. = %.2f(%.2f) msr", acceptance * 1000., (err1-err2)*1000./2));         

      delete htemp;

    }

  }///===== end of pad loop

}

std::vector<double> Plotter::FindRange(TString branch){

  tree->Draw(Form("%s>>temp1", branch.Data()), gateTrue);
  TH1F * temp1 = (TH1F *) gROOT->FindObjectAny("temp1");

  std::vector<double> output;
  if( temp1 != nullptr){
    output.push_back( temp1->GetXaxis()->GetXmax() );
    output.push_back( temp1->GetXaxis()->GetXmin() );
    delete temp1;
  }
  return output;
}


#endif