#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TArc.h"
#include "TLine.h"
#include "TLatex.h"
#include "TMacro.h"

#include "../Armory/AnalysisLib.h"
#include "../Armory/ClassDetGeo.h"
#include "../Armory/ClassReactionConfig.h"
#include "../Cleopatra/ClassIsotope.h"
#include "../Cleopatra/ClassTransfer.h"

void SimulationChecker(TString transferRoot = "transfer.root"){

  gStyle->SetOptStat("");
  gStyle->SetStatY(0.9);
  gStyle->SetStatX(0.9);
  gStyle->SetStatW(0.4);
  gStyle->SetStatH(0.2);
  gStyle->SetLabelSize(0.05, "XY");
  gStyle->SetTitleFontSize(0.1);

  TFile * file = new TFile(transferRoot, "read");
  TTree * tree = (TTree*) file->Get("tree");

  TObjArray * fxList = (TObjArray *) file->FindObjectAny("EZCurve");  

  //*================================== List Of Reactions
  TMacro * listOfReaction = (TMacro *) file->FindObjectAny("ListOfReactions");

  int numReaction = listOfReaction->GetListOfLines()->GetSize();
  
  std::vector<int> reactionIDList(numReaction, -1);
  std::vector<TString> reactionNameList(numReaction, "");
  for( int i = 0; i < numReaction; i++){
    std::vector<std::string> tempStr = AnalysisLib::SplitStr(listOfReaction->GetListOfLines()->At(i)->GetName(), "|");
    reactionIDList[i] = atoi(tempStr[0].c_str());
    reactionNameList[i] = tempStr[1];
  }

  // for( int i = 0; i < numReaction; i++) printf("%2d | %s\n", reactionIDList[i], reactionNameList[i].Data());

  //*================================== DWBA, if any
  TMacro * dwbaReaction  = (TMacro *) file->FindObjectAny("DWBA_ReactionList");
  TMacro * dwbaExList  = (TMacro *) file->FindObjectAny("DWBA_ExList");
  // dwbaReaction->Print();
  // dwbaExList->Print();


  //*================================== reactionConfig
  TMacro * reactionConfigTxt = (TMacro *) file->FindObjectAny("reactionConfig");

  ReactionConfig reactionConfig(reactionConfigTxt);
 
  int nEvent = reactionConfig.numEvents;
  printf("number of events generated : %d \n", nEvent);

  //=== clear the Ex in reactionCondig if DWBA_ReactionList
  if( dwbaReaction || listOfReaction ){
    for( int i = 0; i < numReaction; i ++){
      reactionConfig.exList[reactionIDList[i]].Clear();

      for( int j = 0; j < dwbaReaction->GetListOfLines()->GetSize(); j++){
        std::vector<std::string> haha = AnalysisLib::SplitStr(dwbaReaction->GetListOfLines()->At(j)->GetName(), "|");

        if( atoi(haha[1].c_str()) == reactionIDList[i] ) {
          std::vector<std::string> dudu = AnalysisLib::SplitStr(dwbaExList->GetListOfLines()->At(j+1)->GetName(), " ");

          reactionConfig.exList[reactionIDList[i]].Add(atof(dudu[0].c_str()), atof(dudu[1].c_str()), 1.0, 0.00);

        }

      }

      reactionConfig.Print(reactionIDList[i]);
    }
  }



  //*================================== detector Geometry
  printf("=================================\n");
  printf(" loading detector Geometry.\n");
  TMacro * detGeoTxt = (TMacro *) file->FindObjectAny("detGeo");  

  DetGeo detGeo(detGeoTxt);

  detGeo.Print(true);

  printf("%f %f \n", detGeo.array[0].zMin, detGeo.array[0].zMax);

  printf("=================================\n");

  //*==================================

  int padSize = 500;
  int Div[2] = {5, numReaction};
  Int_t size[2] = {padSize,padSize}; ///x,y, single Canvas size
  TCanvas * cCheck = new TCanvas("cCheck", "Check For Simulation", 0, 0, size[0]*Div[0], size[1]*Div[1]);
  if(cCheck->GetShowEditor() )cCheck->ToggleEditor();
  if(cCheck->GetShowToolBar() )cCheck->ToggleToolBar();
  cCheck->Divide(Div[0],Div[1]);

  bool shownKELines = true;
  TString gate = "hit == 1 && loop <= 1 && thetaCM > 10";

  //^-------------------------------
  cCheck->cd(1);
  {
    TH2F * hez = new TH2F("hez", Form("e-z [gated] @ %5.0f mm; z [mm]; e [MeV]", detGeo.array[0].firstPos), 
                                          400, detGeo.array[0].zMin, detGeo.array[0].zMax, 
                                          400, 0, 10);
  
    tree->Draw("e:z>>hez", gate + " && rID == 0", "colz");
  }

  //^-------------------------------
  cCheck->cd(2);  
  {
    TH2F * hRecoilXY = new TH2F("hRecoilXY", Form("RecoilXY [gated] @ %4.0f mm; X [mm]; Y [mm]", detGeo.aux[0].detPos ), 
                                  400, -detGeo.aux[0].outerRadius, detGeo.aux[0].outerRadius, 
                                  400, -detGeo.aux[0].outerRadius, detGeo.aux[0].outerRadius);
    tree->Draw("yRecoil:xRecoil>>hRecoilXY", gate + " && rID == 0", "colz");
    TArc * detArc1 = new TArc(0,0, detGeo.aux[0].outerRadius);
    detArc1->SetLineColor(kBlue-8);
    detArc1->SetFillStyle(0);
    detArc1->Draw("same");  
    TArc * detArc2 = new TArc(0,0, detGeo.aux[0].innerRadius);
    detArc2->SetLineColor(kBlue-8);
    detArc2->SetFillStyle(0);
    detArc2->Draw("same");  
    
    if( reactionConfig.beamX != 0. || reactionConfig.beamY != 0. ){
      TArc * arc = new TArc(reactionConfig.beamX, reactionConfig.beamY, 1);
      arc->SetLineColor(2);
      detArc1->SetFillStyle(0);
      arc->Draw("same");
    }
  }
  //^-------------------------------
  cCheck->cd(3);  
  {
    // TH1F * hThetaCM[numEx];
    // TLegend * legend = new TLegend(0.8,0.2,0.99,0.8);
    // double maxCount = 0;
    // int startID = 0; // set the start ExID
    // for( int i = startID; i < numEx; i++){
    //   hThetaCM[i] = new TH1F(Form("hThetaCM%d", i), Form("thetaCM [gated] (ExID=%d); thetaCM [deg]; count", i), 200, thetaCMRange[0], thetaCMRange[1]);
    //   hThetaCM[i]->SetLineColor(i+1-startID);
    //   hThetaCM[i]->SetFillColor(i+1-startID);
    //   hThetaCM[i]->SetFillStyle(3000+i-startID);
    //   tree->Draw(Form("thetaCM>>hThetaCM%d", i), gate + Form("&& ExID==%d", i), "");
    //   legend->AddEntry(hThetaCM[i], Form("Ex=%5.1f MeV", exList.ExList[i].Ex));
    //   double max = hThetaCM[i]->GetMaximum();
    //   if( max > maxCount ) maxCount = max;
    // }

    // for( int i = startID; i < numEx; i++){
    //   hThetaCM[i]->GetYaxis()->SetRangeUser(1, maxCount * 1.2);
    //   if( i == startID ) {
    //     hThetaCM[i]->Draw();
    //   }else{
    //     hThetaCM[i]->Draw("same");
    //   }
    // }   
    // legend->Draw();
  }
  
  //^-------------------------------
  cCheck->cd(4);  
  {
    TLatex text;
    text.SetNDC();
    text.SetTextFont(82);
    text.SetTextSize(0.06);
    text.SetTextColor(2);

    text.DrawLatex(0., 0.9, reactionNameList[0]);
    text.DrawLatex(0., 0.8, detGeo.Bfield > 0 ? "out of plan" : "into plan");
    text.SetTextColor(1);
    text.DrawLatex(0., 0.7, "gate:");
    
    text.SetTextColor(2);
    //check gate text length, if > 30, break by "&&" 
    int ll = gate.Length();
    if( ll > 30 ) {
      std::vector<string> strList = AnalysisLib::SplitStr( (std::string) gate.Data(), "&&");
      for( int i = 0; i < strList.size(); i++){
        text.DrawLatex(0., 0.6 - 0.05*i, (TString) strList[i]);
      }
    }else{
      text.DrawLatex(0., 0.6, gate);
    }

    if( reactionConfig.beamX != 0.0 || reactionConfig.beamY != 0.0 ){
      text.DrawLatex(0.0, 0.1, Form("Bema pos: (%4.1f, %4.1f) mm", reactionConfig.beamX, reactionConfig.beamY));
    }
  }
  //^-------------------------------
  cCheck->cd(5); 
  {
    TH1F * hExCal = new TH1F("hExCal", Form("calculated Ex [gated]; Ex [MeV]; count / %.2f keV",  10.),  400, -1, 3);
    tree->Draw("ExCal>>hExCal", gate + " && rID == 0", "");
    Isotope hRecoil(reactionConfig.recoil[0].heavyA, reactionConfig.recoil[0].heavyZ);
    double Sn = hRecoil.CalSp(0,1);
    double Sp = hRecoil.CalSp(1,0);
    double Sa = hRecoil.CalSp2(4,2);
    double S2n = hRecoil.CalSp(0, 2);

    printf("Heavy recoil: %s \n", hRecoil.Name.c_str());
    printf("Sn  : %f MeV/u \n", Sn);
    printf("Sp  : %f MeV/u \n", Sp);
    printf("Sa  : %f MeV/u \n", Sa);
    printf("S2n : %f MeV/u \n", S2n);

    double yMax = hExCal->GetMaximum();
    TLine * lineSn  = new TLine(Sn,  0,  Sn, yMax); lineSn->SetLineColor(2); lineSn->Draw("");
    TLine * lineSp  = new TLine(Sp,  0,  Sp, yMax); lineSp->SetLineColor(4); lineSp->Draw("same");
    TLine * lineSa  = new TLine(Sa,  0,  Sa, yMax); lineSa->SetLineColor(6); lineSa->Draw("same");
    TLine * lineS2n = new TLine(S2n, 0, S2n, yMax); lineS2n->SetLineColor(8); lineS2n->Draw("same");
  
    TLatex * text = new TLatex();
    text->SetTextFont(82);
    text->SetTextSize(0.06);
    text->SetTextColor(2); text->DrawLatex(Sn,  yMax*0.9, "S_{n}");
    text->SetTextColor(4); text->DrawLatex(Sp,  yMax*0.9, "S_{p}");
    text->SetTextColor(6); text->DrawLatex(Sa,  yMax*0.9, "S_{a}");
    text->SetTextColor(8); text->DrawLatex(S2n, yMax*0.9, "S_{2n}");
  
  }

  cCheck->Modified();
  cCheck->Update();

}