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
#include "../Cleopatra/ClassSimPlotter.h"

plotID StringToPlotID(TString str);

void SimChecker(TString filename = "transfer.root",
                TString configFile = "../working/SimCheckerConfig.txt",
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

  Plotter * plotter[numReact];
  
  for( int i = 0; i < numReact; i++){
    plotter[i] = new Plotter(tree, i, reactionList[i], detGeo, reactEx[i], gate, padPlotID);
    plotter[i]->SetRanges(zRange[i][0], zRange[i][1], eMax[i], elumMax, thetaCMMax);
    plotter[i]->SetCanvas(colCount, rowCount, 500, padPlotID);
    plotter[i]->Plot();
  }

  return;

}


plotID StringToPlotID(TString str){
   
  if( str == "pEZ")              return plotID::pEZ;              /// 0
  if( str == "pRecoilXY")        return plotID::pRecoilXY;        /// 1
  if( str == "pThetaCM" )        return plotID::pThetaCM;         /// 2
  if( str == "pExCal" )          return plotID::pExCal;           /// 3
  if( str == "pArrayXY" )        return plotID::pArrayXY;         /// 4
  if( str == "pInfo" )           return plotID::pInfo;            /// 5
  if( str == "pElum1XY" )        return plotID::pElum1XY;         /// 6
  if( str == "pRecoilXY1" )      return plotID::pRecoilXY1;       /// 7
  if( str == "pRecoilXY2" )      return plotID::pRecoilXY2;       /// 8
  if( str == "pTDiffZ" )         return plotID::pTDiffZ;          /// 9
  if( str == "pRecoilRThetaCM" ) return plotID::pRecoilRThetaCM;  /// 10
  if( str == "pRecoilRZ" )       return plotID::pRecoilRZ;        /// 11
  if( str == "pEElum1R" )        return plotID::pEElum1R;         /// 12
  if( str == "pRecoilRTR" )      return plotID::pRecoilRTR;       /// 13
  if( str == "pThetaCM_Z" )      return plotID::pThetaCM_Z;       /// 14
  if( str == "pElum1RThetaCM" )  return plotID::pElum1RThetaCM;   /// 15
  if( str == "pEmpty" )          return plotID::pEmpty ;          /// 16
  
  return plotID::pEmpty;
}