#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TObjArray.h>
#include <TStyle.h>
#include <TH2F.h>
#include <TH1F.h>
#include <TF1.h>
#include <TArc.h>
#include <TMath.h>
#include <TLine.h>
#include <TSpectrum.h>
#include <TGraph.h>
#include <TLegend.h>
#include <TLatex.h>
#include <TMacro.h>
#include <TObjArray.h>
#include <fstream>
#include <TCutG.h>
#include "../Armory/AnalysisLib.h"
#include "../Armory/ClassDetGeo.h"
#include "../Armory/ClassReactionConfig.h"
#include "../Cleopatra/ClassIsotope.h"
#include "../Cleopatra/ClassTransfer.h"

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
              pHitID,            /// 13
              pElum1XY,          /// 14
              pEElum1R,          /// 15
              pElum1RThetaCM,    /// 16
              pEmpty };          /// 17
plotID StringToPlotID(TString str);

void SimChecker(TString filename = "transfer.root",
                TString configFile = "../working/SimChecker_Config.txt",
                Int_t padSize = 500,
                bool outputCanvas = false){

  printf("=================================================================\n");
  printf("====================       Simulate Checker    ==================\n");
  printf("=================================================================\n");   

  TFile * file = new TFile(filename, "read");
  TTree * tree = (TTree*) file->Get("tree");

  //*================= Get reactions and Ex
  TMacro * ListOfReactions = (TMacro *) file->FindObjectAny("ListOfReactions");
  const short numReact = ListOfReactions->GetListOfLines()->GetEntries();

  printf(">>>>> %d reactions found.\n", numReact);

  std::string reactionList[numReact];
  for( int i = 0; i < numReact; i++ ){
    std::string haha = ListOfReactions->GetListOfLines()->At(i)->GetName();
    std::vector<std::string> kaka = AnalysisLib::SplitStr(haha, "|");
    reactionList[i]= kaka[1];
  }
  
  ExcitedEnergies reactEx[numReact]; //2-D array [i][j] = i-reaction, j-Ex
  TMacro * AllExList = (TMacro *) file->FindObjectAny("AllExList");

  TMacro * ExID_ReactID_List = (TMacro *) file->FindObjectAny("ExID_ReactID_List");
  const short numEx = ExID_ReactID_List->GetListOfLines()->GetEntries()-1;
  
  for( int i = 1; i <= numEx; i++){
    std::string haha = ExID_ReactID_List->GetListOfLines()->At(i)->GetName();
    std::vector<std::string> kaka = AnalysisLib::SplitStr(haha, " ");
    
    std::string dudu = AllExList->GetListOfLines()->At(i)->GetName();
    std::vector<std::string> dada = AnalysisLib::SplitStr(dudu, " ");
    
    short rID = atoi(kaka[1].c_str());
    reactEx[rID].Add(atof(dada[0].c_str()), 
                     atof(dada[1].c_str()), 
                     atof(dada[2].c_str()),
                     atof(dada[3].c_str()));
  }

  for( int i = 0; i < numReact; i++ ){
    printf("=========== %s\n", reactionList[i].c_str());
    reactEx[i].Print();
  }

  //*================== detGeoID
  TMacro * detGeotxt = (TMacro *) file->FindObjectAny("detGeo");
  DetGeo detGeo(detGeotxt);
  detGeo.Print(true);

  //*================== Get EZ-curve
  TObjArray * ezList = (TObjArray *) file->FindObjectAny("EZCurve");

  //*================== Get thetaCM = 0
  TObjArray * thetaCM0List = (TObjArray *) file->FindObjectAny("thetaCM_Z");

  //^################################################ Find e-range, z-range

  double zRange[numReact][2];
  double eMax[numReact];
  
  int count = 0;
  for( int i = 0; i < numReact; i++ ){
    zRange[i][0] = detGeo.array[i].zMin-50;
    zRange[i][1] = detGeo.array[i].zMax+50;

    eMax[i] = -1;
    for( size_t j = 0; j < reactEx[i].ExList.size() ; j ++){
      TGraph * func = (TGraph *) ezList->At(count);
      double aaa = func->Eval(zRange[i][1]);
      // printf(" xxxxxxxxxxxx %d, %d | %d %.3f\n", i, j, count, aaa);
      if( aaa > eMax[i] ) eMax[i] = aaa; 
      count++;
    }

    eMax[i] = TMath::Ceil( eMax[i] * 1.1 );
  }

  // for( int i = 0; i < numReact; i++ ){
  //   printf(" %d | eMax : %.2f, zRange : %.2f, %.2f \n", i, eMax[i], zRange[i][0], zRange[i][1]);
  // }

  // //^################################################

  TMacro * config = new TMacro(configFile);
  int numLine = config->GetListOfLines()->GetSize();

  TString gate;
  std::vector<plotID> padPlotID;

  float elumMax = 60;
  float thetaCMMax = 60; //TODO add thetaCM curves in transfer, so that it can be determinated automatically

  int rowCount = 0;
  int colCount = 0;

  bool startCanvasConfig = false;
  bool startGateConfig = false;
  bool startExtra = false;
  for( int i = 0; i < numLine ; i++){
    std::string haha = config->GetListOfLines()->At(i)->GetName();
    std::vector<std::string> dudu = AnalysisLib::SplitStr(haha, ",");
    TString lala = haha;
    lala.Remove(3);
    if( (lala == "   " || lala == "// " || lala == "//=") && dudu.size() == 0) continue;
    if( lala == "//#" ) break;
    if( lala == "//*" ) {
      startCanvasConfig = true;
      // rowCount ++;
      continue;
    }
    if( lala == "//^" ) {
      startCanvasConfig = false;
      startGateConfig = true;
      continue;
    }
    if( lala == "//@" ) {
      startGateConfig = false;
      startExtra = true;
    }

    if( startCanvasConfig ){
      rowCount ++;
      // printf("|%s|\n", haha.c_str());
      if( dudu.size() > colCount ) colCount = dudu.size();

      for( size_t k = 0; k < dudu.size() ; k++){
        padPlotID.push_back(StringToPlotID(dudu[k]));
      }
    }

    if( startGateConfig ){
      gate = haha;
    }

    if( startExtra ){
      if( dudu[0] == "elum_Max" ) elumMax = atof(dudu[2].c_str());
      if( dudu[0] == "thetaCM_Max" ) thetaCMMax = atof(dudu[2].c_str());
    }

  }
  
  int Div[2] = {colCount, rowCount};

  gStyle->SetOptStat("");
  gStyle->SetStatY(0.9);
  gStyle->SetStatX(0.9);
  gStyle->SetStatW(0.4);
  gStyle->SetStatH(0.2);
  gStyle->SetLabelSize(0.05, "XY");
  gStyle->SetTitleFontSize(0.1);

  printf(" Canvas division | col : %d, row : %d \n", colCount, rowCount);
  count = 0;
  for( int i = 0; i < rowCount; i++){
    for( int j = 0; j < colCount; j++){
      printf("%6d", padPlotID[count]);
      count++;
    }
    printf("\n");
  }

  printf("Gate : %s \n", gate.Data());
  printf("elum Max : %.2f mm\n", elumMax);
  printf("thetaCM Max : %.2f deg\n", thetaCMMax);

  printf("#####################################################\n");

  // //^################################################
  Int_t size[2] = {padSize,padSize}; ///x,y, single Canvas size

  TCanvas * cCheck[numReact];

  TH2F * hez[numReact];
  TH2F * hArrayXY[numReact];
  TH2F * hRecoilXY[numReact];
  TH2F * hElum1XY[numReact];
  TH1F ** hThetaCM[numReact];
  TH1F * hExCal[numReact];
  TH1F * hHit[numReact];

  for( int i = 0; i < numReact; i++ ){

    TString gateThis = gate + Form(" && rID == %d", i);
  
    cCheck[i] = new TCanvas(Form("cCheck-%d", i), Form("Simulation Checker (%d)", i), 200 * i , 200 * i, size[0]*Div[0], size[1]*Div[1]);
    if(cCheck[i]->GetShowEditor() ) cCheck[i]->ToggleEditor();
    if(cCheck[i]->GetShowToolBar() )cCheck[i]->ToggleToolBar();
    cCheck[i]->Divide(Div[0],Div[1]);

    for( int j = 1; j <= Div[0]*Div[1]; j ++){
      cCheck[i]->cd(j);

      if( padPlotID[i-1] == pThetaCM ) {
        cCheck[i]->cd(j)->SetGrid(0,0);
        cCheck[i]->cd(j)->SetLogy();
      }

      plotID pID = padPlotID[j-1];

      ///########################################
      if( pID == pEZ){
        hez[i] = new TH2F(Form("hez%d", i), 
                          Form("e-z [gated] @ %5.0f mm; z [mm]; e [MeV]", detGeo.array[i].firstPos), 
                          200, zRange[i][0], zRange[i][1], 
                          200, 0, eMax[i]);

        tree->Draw(Form("e:z>>hez%d", i), gateThis , "colz");
      }

      
      if( pID == pRecoilXY  ){
        hRecoilXY[i] = new TH2F(Form("hRecoilXY%d",i), 
                                Form("RecoilXY [gated] @ %4.0f mm; X [mm]; Y [mm]", detGeo.aux[i].detPos ), 
                                200, -detGeo.aux[i].outerRadius, detGeo.aux[i].outerRadius, 
                                200, -detGeo.aux[i].outerRadius, detGeo.aux[i].outerRadius);
        tree->Draw(Form("yRecoil:xRecoil>>hRecoilXY%d", i), gateThis, "colz");
        TArc * detArc1 = new TArc(0,0, detGeo.aux[i].outerRadius);
        detArc1->SetLineColor(kBlue-8);
        detArc1->SetFillStyle(0);
        detArc1->Draw("same");  
        TArc * detArc2 = new TArc(0,0, detGeo.aux[i].innerRadius);
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

      /*
      if( pID == pRecoilXY1       ){
        TH2F * hRecoilXY1 = new TH2F("hRecoilXY1", Form("RecoilXY-1 [gated] @ %4.0f mm; X [mm]; Y [mm]", detGeo.aux[detGeoID].detPos1 ), 
                                        400, -detGeo.aux[detGeoID].outerRadius, detGeo.aux[detGeoID].outerRadius, 
                                        400, -detGeo.aux[detGeoID].outerRadius, detGeo.aux[detGeoID].outerRadius);
        tree->Draw("yRecoil1:xRecoil1>>hRecoilXY1", gate, "colz");
      }

      if( pID == pRecoilXY2       ){
        TH2F * hRecoilXY2 = new TH2F("hRecoilXY2", Form("RecoilXY-2 [gated] @ %4.0f mm; X [mm]; Y [mm]", detGeo.aux[detGeoID].detPos2 ), 
                                        400, -detGeo.aux[detGeoID].outerRadius, detGeo.aux[detGeoID].outerRadius, 
                                        400, -detGeo.aux[detGeoID].outerRadius, detGeo.aux[detGeoID].outerRadius);
        tree->Draw("yRecoil2:xRecoil2>>hRecoilXY2", gate, "colz");
      }

      if( pID == pRecoilRZ       ){
        TH2F * hRecoilRZ = new TH2F("hRecoilRZ", "RecoilR - Z [gated]; z [mm]; RecoilR [mm]",  zRange[0], zRange[1], zRange[2], 400,0, detGeo.aux[detGeoID].outerRadius);
        tree->Draw("rhoRecoil:z>>hRecoilRZ", gate, "colz");
      }

      if( pID == pRecoilRTR      ){
        FindRange("TB", gate, tree, recoilERange);
        TH2F * hRecoilRTR = new TH2F("hRecoilRTR", "RecoilR - recoilE [gated]; recoil Energy [MeV]; RecoilR [mm]", 500, recoilERange[0], recoilERange[1], 500, 0, detGeo.aux[detGeoID].outerRadius);
        tree->Draw("rhoRecoil:TB>>hRecoilRTR", gate, "colz");
      }
      
      if( pID == pTDiffZ         ){
        double tDiffRange [2];
        FindRange("t-tB", gate, tree, tDiffRange);
        TH2F * hTDiffZ = new TH2F("hTDiffZ", "time(Array) - time(Recoil) vs Z [gated]; z [mm]; time diff [ns]", zRange[0], zRange[1], zRange[2],  500, tDiffRange[0], tDiffRange[1]);
        tree->Draw("t - tB : z >> hTDiffZ", gate, "colz");
      }
      */
     
      if( pID == pThetaCM        ){
        int numExTemp = reactEx[i].ExList.size();
        hThetaCM[i] = new TH1F *[numExTemp]; 
        TLegend * legend = new TLegend(0.8,0.2,0.99,0.8);
        double maxCount = 0;
        for( int h = 0; h < numExTemp; h++){
          hThetaCM[i][h] = new TH1F(Form("hThetaCM%d-%d", i, h), Form("thetaCM [gated] (ExID=%d); thetaCM [deg]; count", h), 200, 0, thetaCMMax);
          hThetaCM[i][h]->SetLineColor(h+1);
          hThetaCM[i][h]->SetFillColor(h+1);
          hThetaCM[i][h]->SetFillStyle(3000+h);
          tree->Draw(Form("thetaCM>>hThetaCM%d-%d", i, h), gateThis + Form("&& ExID==%d", h), "");
          legend->AddEntry(hThetaCM[i][h], Form("Ex=%5.1f MeV", reactEx[i].ExList[h].Ex));
          double max = hThetaCM[i][h]->GetMaximum();
          if( max > maxCount ) maxCount = max;
        }

        for( int h = 0; h < numExTemp; h++){
          hThetaCM[i][h]->GetYaxis()->SetRangeUser(1, maxCount * 1.2);
          if( h == 0 ) {
            hThetaCM[i][h]->Draw();
          }else{
            hThetaCM[i][h]->Draw("same");
          }
        }   
        legend->Draw();
      }
      /*
      if( pID == pThetaCM_Z      ){
        TH2F *hThetaCM_Z = new TH2F("hThetaCM_Z","ThetaCM vs Z ; Z [mm]; thetaCM [deg]",zRange[0], zRange[1], zRange[2], 200, thetaCMRange[0], thetaCMRange[1]);
        tree->Draw("thetaCM:z>>hThetaCM_Z",gate,"col");
        if( shownKELines){
          for( int i = 0; i < numEx ; i++){
            txList->At(i)->Draw("same");
          }
        }
      }
      */

      if( pID == pExCal          ){

          double exMin = 9999;
          double exMax = -1;
          for( size_t k = 0 ; k < reactEx[i].ExList.size(); k++){
            double kuku = reactEx[i].ExList[k].Ex;
            if( kuku > exMax ) exMax = kuku;
            if( kuku < exMin ) exMin = kuku;
          }

          double exPadding = (exMax - exMin) < 1 ? 1 : (exMax - exMin) * 0.3;

          exMin = exMin - exPadding ;
          exMax = exMax + exPadding ;

          hExCal[i] = new TH1F(Form("hExCal%d",i), 
                                Form("calculated Ex [gated]; Ex [MeV]; count / %.2f keV",  (exMax-exMin)/400.*1000),  
                                400, exMin, exMax);
          tree->Draw(Form("ExCal>>hExCal%d", i), gateThis, "");
          // Isotope hRecoil(recoil.heavyA, recoil.heavyZ);
          // double Sn = hRecoil.CalSp(0,1);
          // double Sp = hRecoil.CalSp(1,0);
          // double Sa = hRecoil.CalSp2(4,2);
          // double S2n = hRecoil.CalSp(0, 2);

          // printf("Heavy recoil: %s \n", hRecoil.Name.c_str());
          // printf("Sn  : %f MeV/u \n", Sn);
          // printf("Sp  : %f MeV/u \n", Sp);
          // printf("Sa  : %f MeV/u \n", Sa);
          // printf("S2n : %f MeV/u \n", S2n);

          // double yMax = hExCal->GetMaximum();
          // TLine * lineSn  = new TLine(Sn,  0,  Sn, yMax); lineSn->SetLineColor(2); lineSn->Draw("");
          // TLine * lineSp  = new TLine(Sp,  0,  Sp, yMax); lineSp->SetLineColor(4); lineSp->Draw("same");
          // TLine * lineSa  = new TLine(Sa,  0,  Sa, yMax); lineSa->SetLineColor(6); lineSa->Draw("same");
          // TLine * lineS2n = new TLine(S2n, 0, S2n, yMax); lineS2n->SetLineColor(8); lineS2n->Draw("same");

          // TLatex * text = new TLatex();
          // text->SetTextFont(82);
          // text->SetTextSize(0.06);
          // text->SetTextColor(2); text->DrawLatex(Sn,  yMax*0.9, "S_{n}");
          // text->SetTextColor(4); text->DrawLatex(Sp,  yMax*0.9, "S_{p}");
          // text->SetTextColor(6); text->DrawLatex(Sa,  yMax*0.9, "S_{a}");
          // text->SetTextColor(8); text->DrawLatex(S2n, yMax*0.9, "S_{2n}");
          
      }
      /*
      if( pID == pRecoilRThetaCM ){
        TH2F * hRecoilRThetaCM = new TH2F("hRecoilRThetaCM", "RecoilR - thetaCM [gated]; thetaCM [deg]; RecoilR [mm]", 400, 0, 60, 400,0, detGeo.aux[detGeoID].outerRadius);
        tree->Draw("rhoRecoil:thetaCM>>hRecoilRThetaCM", gate, "colz");
      }
      */
      if( pID == pArrayXY ){

        hArrayXY[i] = new TH2F(Form("hArrayXY%d", i), 
                               "Array-XY [gated]; X [mm]; Y [mm]", 
                               400, -detGeo.array[i].detPerpDist*1.5, detGeo.array[i].detPerpDist*1.5, 
                               400, -detGeo.array[i].detPerpDist*1.5, detGeo.array[i].detPerpDist*1.5);
        tree->Draw(Form("yArray:xArray>>hArrayXY%d", i), gateThis, "colz");
      }

      if( pID == pInfo ){
        TLatex text;
        text.SetNDC();
        text.SetTextFont(82);
        text.SetTextSize(0.06);
        text.SetTextColor(2);

        text.DrawLatex(0., 0.9, reactionList[i].c_str());
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
        
      
      if( pID == pElum1XY ){
        hElum1XY[i] = new TH2F(Form("hElum1XY%d", i), 
                                Form("Elum-1 XY [gated] @ %.0f mm ; X [mm]; Y [mm]", detGeo.aux[i].elumPos1),  
                                400, -elumMax, elumMax, 
                                400, -elumMax, elumMax);
        tree->Draw(Form("yElum1:xElum1>>hElum1XY%d", i), gateThis, "colz");
        
        double count = hElum1XY[i]->GetEntries();        
        if( count < 2000. ) {
          hElum1XY[i]->SetMarkerStyle(7);
          if( count < 500. ) hElum1XY[i]->SetMarkerStyle(3);
          hElum1XY[i]->Draw("scat");
        }
      }

      /*
      if( pID == pEElum1R ){
          TH2F * hEElum1Rho = new TH2F("hEElum1Rho", "Elum-1 E-R [gated]; R[mm]; Energy[MeV]",  400, 0, elumRange, 400, eRange[0], eRange[1]);
          tree->Draw("Tb:rhoElum1>>hEElum1Rho", gate, "colz");
      }
        
      if( pID == pElum1RThetaCM){
        int angBin = 400;
        
        TH2F * hElum1RThetaCM = new TH2F("hElum1RThetaCM", "Elum-1 rho vs ThetaCM [gated]; thatCM [deg]; Elum- rho [mm]", angBin, thetaCMRange[0], thetaCMRange[1],  400, 0, elumRange);
        tree->Draw("rhoElum1:thetaCM>>hElum1RThetaCM", gate, "colz");   
        
        TH1F * htemp = (TH1F *) hElum1RThetaCM->ProjectionX("htemp");  
        
        double rel = (thetaCMRange[1] - thetaCMRange[0])*1.0/angBin;
        printf("angular resolution : %f deg \n", rel);
        
        vector<double> xList;
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
          text.DrawLatex(xList[i], elumRange/2, Form("%.2f", xList[i]));
        }
        
        text.SetNDC();
        text.SetTextAngle(0);
        text.DrawLatex(0.15, 0.15, Form("accp. = %.2f(%.2f) msr", acceptance * 1000., (err1-err2)*1000./2));         

      }
      */
      if( pID == pHitID ){
          printf("=======================meaning of Hit ID\n");
          printf("   1 = light recoil hit array & heavy recoil hit recoil\n");
          printf("   0 = no detector\n");
          printf("  -1 = light recoil go opposite side of array\n");
          printf("  -2 = light recoil hit > det width\n");
          printf("  -3 = light recoil hit > array \n");
          printf("  -4 = light recoil hit blocker \n");
          printf(" -10 = light recoil orbit radius too big  \n");
          printf(" -11 = light recoil orbit radius too small\n");
          printf(" -12 = when reocol at the same side of array, light recoil blocked by recoil detector\n");
          printf(" -13 = more than 3 loops\n");
          printf(" -14 = heavy recoil did not hit recoil  \n");
          printf(" -15 = cannot find hit on array\n");
          printf(" -20 = unknown\n");
          printf("===========================================\n");
          
          hHit[i] = new TH1F(Form("hHit%d", i), "hit; hit-ID; count", 13, -11, 2);
          tree->Draw(Form("hit>>hHit%d", i), "", "");
      }

    }///===== end of pad loop

  }///===== end of reaction loop


}


plotID StringToPlotID(TString str){
   
  if( str == "pEZ") return plotID::pEZ;                      ///0
  if( str == "pRecoilXY") return plotID::pRecoilXY;                /// 1
  if( str == "pRecoilXY1" ) return plotID::pRecoilXY1;       /// 2
  if( str == "pRecoilXY2" ) return plotID::pRecoilXY2;       /// 3
  if( str == "pRecoilRZ" ) return plotID::pRecoilRZ;         /// 4
  if( str == "pRecoilRTR" ) return plotID::pRecoilRTR;       /// 5
  if( str == "pTDiffZ" ) return plotID::pTDiffZ;             /// 6
  if( str == "pThetaCM" ) return plotID::pThetaCM;           /// 7
  if( str == "pThetaCM_Z" ) return plotID::pThetaCM_Z;       /// 8
  if( str == "pExCal" ) return plotID::pExCal;               /// 9
  if( str == "pRecoilRThetaCM" ) return plotID::pRecoilRThetaCM;   /// 10
  if( str == "pArrayXY" ) return plotID::pArrayXY;           /// 11
  if( str == "pInfo" ) return plotID::pInfo;                 /// 12
  if( str == "pHitID" ) return plotID::pHitID;               /// 13
  if( str == "pElum1XY" ) return plotID::pElum1XY;           /// 14
  if( str == "pEElum1R" ) return plotID::pEElum1R;           /// 14
  if( str == "pElum1RThetaCM" ) return plotID::pElum1RThetaCM;    /// 15
  if( str == "pEmpty" ) return plotID::pEmpty ;              /// 16
  
  return plotID::pEmpty;
}