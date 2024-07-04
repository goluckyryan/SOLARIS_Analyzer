#include "TROOT.h"
#include "TBenchmark.h"
#include "TLorentzVector.h"
#include "TMath.h"
#include "TFile.h"
#include "TF1.h"
#include "TTree.h"
#include "TRandom.h"
#include "TGraph.h"
#include "TMacro.h"
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <TObjArray.h>

#include "../Armory/ClassDetGeo.h"
#include "ClassTargetScattering.h"
#include "ClassDecay.h"
#include "ClassTransfer.h"
#include "ClassHelios.h"

void PrintEZPlotPara(TransferReaction tran, HELIOS helios){

  printf("==================================== E-Z plot slope\n");
  double  betaRect = tran.GetReactionBeta() ;
  double     gamma = tran.GetReactionGamma();
  double        mb = tran.GetMass_b();
  double       pCM = tran.GetMomentumbCM();
  double         q = TMath::Sqrt(mb*mb + pCM*pCM); ///energy of light recoil in center of mass
  double     slope = tran.GetEZSlope(helios.GetBField()); /// MeV/mm
  printf("                       e-z slope : %f MeV/mm\n", slope);   
  // double intercept = q/gamma - mb; // MeV
  // printf("    e-z intercept (ground state) : %f MeV\n", intercept); 
}

void Transfer(
         std::string  basicConfig = "reactionConfig.txt", 
         std::string   detGeoFile = "detectorGeo.txt", 
         TString      ptolemyRoot = "DWBA.root",
         TString     saveFileName = "transfer.root"){

  //*############################################# Set Reaction

  // std::vector<double> kbCM; /// momentum of b in CM frame
  // TF1 * exDistribution = nullptr;

  DetGeo detGeoConfig;
  ReactionConfig reactionConfig;

  detGeoConfig.LoadDetectorGeo(detGeoFile, false);
  reactionConfig.LoadReactionConfig(basicConfig);

  const unsigned short numDetGeo = detGeoConfig.array.size();
  const unsigned short numReact  = reactionConfig.recoil.size();

  if( numDetGeo != numReact ){
    printf("\e[31m !!!!!! number of array is not equal to number of reaction.!!! \e[0m\n");
    printf("Abort\n");
    return;
  }

  unsigned short numTransfer = 0;

  for( int i = 0; i < std::min(numDetGeo, numReact); i++){
    if( detGeoConfig.array[i].enable ) numTransfer ++;
  }

  TransferReaction * transfer = new TransferReaction[numTransfer];  
  Decay * decay = new Decay[numTransfer];
  HELIOS * helios = new HELIOS[numTransfer];

  int count = 0;
  for( unsigned short i = 0 ; i < numDetGeo; i++){
    if( detGeoConfig.array[i].enable ){
      transfer[count].SetReactionFromFile(basicConfig, i);
      if(transfer[count].GetRecoil().isDecay) {
        decay[count].SetMotherDaugther(transfer[count].GetRecoil());
      }
      helios[count].SetDetectorGeometry(detGeoFile, i);

      count ++;
    }
  }

  printf("----- loading reaction setting from %s. \n", basicConfig.c_str());
  printf("----- loading geometry setting from %s. \n", detGeoFile.c_str());
 
  printf("\e[32m#################################### Reaction & HELIOS configuration\e[0m\n");   

  //*############################################# Load DWBAroot for thetaCM distribution
  TFile * distFile = new TFile(ptolemyRoot, "read");
  TObjArray * distList = nullptr;
  TMacro * dwbaExList = nullptr;
  TMacro * dwbaReactList = nullptr;

  TMacro dwbaExList_Used;
  TMacro dwbaReactList_Used;
  
  if( distFile->IsOpen() ) {
    printf("\e[32m#################################### Load DWBA input : %s  \e[0m\n", ptolemyRoot.Data());
    printf("--------- Found DWBA thetaCM distributions. Use the ExList from DWBA.\n"); 

    distList = (TObjArray *) distFile->FindObjectAny("thetaCM_TF1"); // the function List
    dwbaExList = (TMacro *) distFile->FindObjectAny("ExList");   
    dwbaExList_Used.AddLine(dwbaExList->GetListOfLines()->At(0)->GetName());
    dwbaReactList = (TMacro *) distFile->FindObjectAny("ReactionList");   

    int numEx = dwbaExList->GetListOfLines()->GetSize() - 1 ;
    
    // for( int i = 0; i < numTransfer; i++){ transfer[i].GetExList()->Clear(); }
    ExcitedEnergies dwbaExTemp[numTransfer];

    for( int i = 1; i <= numEx ; i++){
      //Check DWBA reaction is same as transfer setting
      std::string reactionName = dwbaReactList->GetListOfLines()->At(i-1)->GetName();

      for( int j = 0; j < numTransfer; j++){
        if( reactionName.find( transfer[j].GetReactionName().Data() ) != std::string::npos) {
          std::string temp = dwbaExList->GetListOfLines()->At(i)->GetName();
          dwbaReactList_Used.AddLine((reactionName + " | " + std::to_string(j)).c_str());
          dwbaExList_Used.AddLine(temp.c_str());
          if( temp[0] == '/' ) continue;
          std::vector<std::string> tempStr = AnalysisLib::SplitStr(temp, " ");
          // transfer[j].GetExList()->Add( atof(tempStr[0].c_str()), atof(tempStr[1].c_str()), 1.0, 0.00);
          dwbaExTemp[j].Add( atof(tempStr[0].c_str()), atof(tempStr[1].c_str()), 1.0, 0.00);
        }
      }
    }

    for( int i = 0; i < numTransfer; i++ ){
      if( dwbaExTemp[i].ExList.size() > 0 ) {
        transfer[i].GetExList()->Clear();
        for( size_t j = 0 ; dwbaExTemp[i].ExList.size(); j ++ ){
          transfer[i].GetExList()->Add( dwbaExTemp[i].ExList[j].Ex, dwbaExTemp[i].ExList[j].xsec, 1.0, 0.00);
        }
      }else{
        printf("Cannot match %s with DWBA, use Reaction Ex List\n", transfer[i].GetReactionName().Data());
      }
    }

  }else{
    printf("------- no DWBA input. Use the ExList from %s\n", basicConfig.c_str()); 
  }

  std::vector<bool> listOfTransfer(numTransfer, false);
  
  for( int i = 0; i < numTransfer; i++){
    if( transfer[i].GetExList()->ExList.size() > 0 ){

      listOfTransfer[i] = true;

      transfer[i].PrintReaction(false);
      transfer[i].GetExList()->Print();
      helios[i].PrintGeometry();
      transfer[i].CreateExDistribution();
      // PrintEZPlotPara(transfer[i], helios[i]);
    }else{
      printf(" Reaction : %s has no excited energy. Skipped. \n", transfer[i].GetReactionName().Data());
    }
  }
  
  //*############################################# build tree
  printf("\e[32m#################################### building Tree in %s\e[0m\n", saveFileName.Data());
  TFile * saveFile = new TFile(saveFileName, "recreate");
  TTree * tree = new TTree("tree", "tree");

  TMacro config(basicConfig.c_str());
  TMacro detGeoTxt(detGeoFile.c_str());
  config.SetName("ReactionConfig");
  config.Write("reactionConfig");
  detGeoTxt.Write("detGeo");

  if( distList != NULL ) distList->Write("DWBA", 1);
  if( dwbaExList != NULL ) {
    dwbaExList_Used.Write("DWBA_ExList", 1);
    dwbaReactList_Used.Write("DWBA_ReactionList", 1);
  }

  
  TMacro hitMeaning;
  hitMeaning.AddLine("======================= meaning of Hit\n"); 
  for( int code = -15 ; code <= 1; code ++ ){
    hitMeaning.AddLine( Form( "%4d = %s", code, helios[0].AcceptanceCodeToMsg(code).Data() ));
  }
  hitMeaning.AddLine(" other = unknown\n");
  hitMeaning.AddLine("===========================================\n");  
  hitMeaning.Write("hitMeaning");

  int hit; /// the output of Helios.CalHit
  tree->Branch("hit", &hit, "hit/I");

  int rID; /// reaction ID
  tree->Branch("rID", &rID, "reactionID/I");
  
  double thetab, phib, Tb;
  double thetaB, phiB, TB;
  tree->Branch("thetab", &thetab, "thetab/D");
  tree->Branch("phib",     &phib, "phib/D");
  tree->Branch("Tb",         &Tb, "Tb/D");
  tree->Branch("thetaB", &thetaB, "thetaB/D");
  tree->Branch("phiB",     &phiB, "phiB/D");
  tree->Branch("TB",         &TB, "TB/D");
  
  double thetaCM;
  tree->Branch("thetaCM", &thetaCM, "thetaCM/D");
  
  double e, z, detX, t, z0, tB;
  tree->Branch("e",    &e, "energy_light/D");
  tree->Branch("x", &detX, "detector_x/D");
  tree->Branch("z",    &z, "array_hit_z/D");
  tree->Branch("z0",  &z0, "z-cycle/D");
  tree->Branch("t",    &t, "cycle_time_light/D");
  tree->Branch("tB",  &tB, "recoil_hit_time/D");   /// hit time for recoil on the recoil detector
  
  int loop, detID, detRowID;
  tree->Branch("detID",       &detID, "detID/I");
  tree->Branch("detRowID", &detRowID, "detRowID/I");
  tree->Branch("loop",         &loop, "loop/I");
  
  double rho, rhoB; ///orbit radius
  tree->Branch("rho",   &rho, "orbit_radius_light/D");
  tree->Branch("rhoB", &rhoB, "orbit_radius_heavy/D");

  int ExID;
  double Ex;
  tree->Branch("ExID", &ExID, "ExID/I");
  tree->Branch("Ex",     &Ex, "Ex/D");

  double ExCal, thetaCMCal;
  tree->Branch("ExCal",           &ExCal, "ExCal/D");
  tree->Branch("thetaCMCal", &thetaCMCal, "thetaCMCal/D");
  
  // double TbLoss; /// energy loss of particle-b from target scattering
  // double KEAnew; ///beam energy after target scattering
  // double depth;  /// reaction depth;
  // double Ecm;
  // if( reactConfig.isTargetScattering ){
  //   tree->Branch("depth",   &depth, "depth/D");
  //   tree->Branch("TbLoss", &TbLoss, "TbLoss/D");
  //   tree->Branch("KEAnew", &KEAnew, "KEAnew/D");
  //   tree->Branch("Ecm",       &Ecm, "Ecm/D");
  // }

  double decayTheta; /// the change of thetaB due to decay
  double xRecoil_d, yRecoil_d, rhoRecoil_d, Td;

  bool isAnyDecay = false;
  for( int i = 0; i < numTransfer; i++ ){
    if( !listOfTransfer[i] ) continue;
    isAnyDecay |= transfer[i].GetRecoil().isDecay;
  }

  if( isAnyDecay ) {
    tree->Branch("decayTheta",   &decayTheta, "decayTheta/D");
    tree->Branch("xRecoil_d",     &xRecoil_d, "xRecoil_d/D");
    tree->Branch("yRecoil_d",     &yRecoil_d, "yRecoil_d/D");
    tree->Branch("rhoRecoil_d", &rhoRecoil_d, "rhoRecoil_d/D");
    tree->Branch("Td",                   &Td, "Td/D");
  }
  
  double xArray, yArray, rhoArray; ///x, y, rho positon of particle-b on PSD
  tree->Branch("xArray",     &xArray, "xArray/D");
  tree->Branch("yArray",     &yArray, "yArray/D");
  tree->Branch("rhoArray", &rhoArray, "rhoArray/D");
  
  double xRecoil, yRecoil, rhoRecoil; /// x, y, rho position of particle-B on recoil-detector
  tree->Branch("xRecoil",     &xRecoil, "xRecoil/D");
  tree->Branch("yRecoil",     &yRecoil, "yRecoil/D");
  tree->Branch("rhoRecoil", &rhoRecoil, "rhoRecoil/D");
  

  ///in case need ELUM
  double xElum1, yElum1, rhoElum1;
  bool isAnyElum1 = false;
  for( int i = 0; i < numTransfer; i++ ){
    if( !listOfTransfer[i] ) continue;
    isAnyElum1 |=  (helios[i].GetAuxGeometry().elumPos1 != 0); 
  }
  if( isAnyElum1 ) {
    tree->Branch("xElum1",     &xElum1, "xElum1/D");
    tree->Branch("yElum1",     &yElum1, "yElum1/D");
    tree->Branch("rhoElum1", &rhoElum1, "rhoElum1/D");
  }
  
  double xElum2, yElum2, rhoElum2;
  bool isAnyElum2 = false;
  for( int i = 0; i < numTransfer; i++ ){
    if( !listOfTransfer[i] ) continue;
    isAnyElum2 |=  (helios[i].GetAuxGeometry().elumPos2 != 0); 
  }
  if( isAnyElum2 ) {
    tree->Branch("xElum2",     &xElum2, "xElum2/D");
    tree->Branch("yElum2",     &yElum2, "yElum2/D");
    tree->Branch("rhoElum2", &rhoElum2, "rhoElum2/D");
  }
  
  ///in case need other recoil detector. 
  double xRecoil1, yRecoil1, rhoRecoil1;
  bool isAnyRecoil1 = false;
  for( int i = 0; i < numTransfer; i++ ){
    if( !listOfTransfer[i] ) continue;
    isAnyRecoil1 |=  (helios[i].GetAuxGeometry().detPos1 != 0); 
  }
  if( isAnyRecoil1 != 0 ){
    tree->Branch("xRecoil1",     &xRecoil1, "xRecoil1/D");
    tree->Branch("yRecoil1",     &yRecoil1, "yRecoil1/D");
    tree->Branch("rhoRecoil1", &rhoRecoil1, "rhoRecoil1/D");
  }

  double xRecoil2, yRecoil2, rhoRecoil2;
  bool isAnyRecoil2 = false;
  for( int i = 0; i < numTransfer; i++ ){
    if( !listOfTransfer[i] ) continue;
    isAnyRecoil2 |=  (helios[i].GetAuxGeometry().detPos2 != 0); 
  }
  if( isAnyRecoil2 != 0 ){
    tree->Branch("xRecoil2",     &xRecoil2, "xRecoil2/D");
    tree->Branch("yRecoil2",     &yRecoil2, "yRecoil2/D");
    tree->Branch("rhoRecoil2", &rhoRecoil2, "rhoRecoil2/D");
  }

  //======= list of reaction used.
  TMacro listOfReaction;
  for( int i = 0; i < numTransfer ; i++){
    if( !listOfTransfer[i] ) continue;
    listOfReaction.AddLine(Form("%2d | %s", i, transfer[i].GetReactionName_Latex().Data()));
  }

  listOfReaction.Write("ListOfReactions");
  
  //======= function for e-z plot for ideal case
  printf("++++ generate functions\n");
  TObjArray * gList = new TObjArray();
  gList->SetName("Constant thetaCM = 0 lines");
  const int gxSize = numTransfer;
  TF1 ** gx = new TF1*[gxSize];
  TString name;

  for( int i = 0; i < gxSize; i++){
    double mb = transfer[i].GetMass_b();
    double betaRect = transfer[i].GetReactionBeta();
    double gamma = transfer[i].GetReactionGamma();
    double slope = transfer[i].GetEZSlope(helios[0].GetBField()); /// MeV/mm

    name.Form("g%d", i);     
    gx[i] = new TF1(name, "([0]*TMath::Sqrt([1]+[2]*x*x)+[5]*x)/([3]) - [4]", -1000, 1000);      
    double thetacm = i * TMath::DegToRad();
    double gS2 = TMath::Power(TMath::Sin(thetacm)*gamma,2);
    gx[i]->SetParameter(0, TMath::Cos(thetacm));
    gx[i]->SetParameter(1, mb*mb*(1-gS2));
    gx[i]->SetParameter(2, TMath::Power(slope/betaRect,2));
    gx[i]->SetParameter(3, 1-gS2);
    gx[i]->SetParameter(4, mb);
    gx[i]->SetParameter(5, -gS2*slope);
    gx[i]->SetNpx(1000);
    gList->Add(gx[i]);
    printf("/");
    if( i > 1 && i % 40 == 0 ) printf("\n");
  }
  gList->Write("EZ_thetaCM", TObject::kSingleKey);
  printf(" %d constant thetaCM functions\n", gxSize);

  for( int i = 0; i < gxSize; i++){
    delete gx[i];
  }
  delete [] gx;
  delete gList;

  //--- cal modified f
  int numEx = 0;
  for( int i = 0; i < numTransfer; i++){
    if( !listOfTransfer[i] ) continue;
    numEx += transfer[i].GetExList()->ExList.size();
  }

  TObjArray * fxList = new TObjArray();
  TGraph ** fx = new TGraph*[numEx];
  std::vector<double> px, py;
  int countfx = 0;

  count = 0;
  for( int i = 0; i < numTransfer; i++ ){
    if( !listOfTransfer[i] ) continue;
    double mb = transfer[i].GetMass_b();
    double betaRect = transfer[i].GetReactionBeta();
    double gamma = transfer[i].GetReactionGamma();
    double slope = transfer[i].GetEZSlope(helios[0].GetBField()); /// MeV/mm

    for( size_t j = 0 ; j < transfer[i].GetExList()->ExList.size(); j++){
      double Ex = transfer[i].GetExList()->ExList[j].Ex;
      double kbCM = transfer[i].CalkCM(Ex);
      double a = helios[i].GetDetRadius();
      double q = TMath::Sqrt(mb*mb + kbCM * kbCM );
      px.clear();
      py.clear();
      countfx = 0;
      for(int i = 0; i < 100; i++){
        double thetacm = TMath::Pi()/TMath::Log(100) * (TMath::Log(100) - TMath::Log(100-i)) ;//using log scale, for more point in small angle.
        double temp = TMath::TwoPi() * slope / betaRect / kbCM * a / TMath::Sin(thetacm); 
        double pxTemp = betaRect /slope * (gamma * betaRect * q - gamma * kbCM * TMath::Cos(thetacm)) * (1 - TMath::ASin(temp)/TMath::TwoPi()) ;
        double pyTemp = gamma * q - mb - gamma * betaRect * kbCM * TMath::Cos(thetacm);   
        if( TMath::IsNaN(pxTemp) || TMath::IsNaN(pyTemp) ) continue;
        px.push_back(pxTemp);
        py.push_back(pyTemp);
        countfx ++;
      }

      fx[count] = new TGraph(countfx, &px[0], &py[0]);
      name.Form("fx%d_%ld", i, j);
      fx[count]->SetName(name);
      fx[count]->SetLineColor(4);
      fxList->Add(fx[count]);
      printf(",");
      count ++;
    }
  }
  fxList->Write("EZCurve", TObject::kSingleKey);
  printf(" %d (%d) e-z finite-size detector functions\n", numEx, count);

  for( int i = 0 ; i < numEx; i++) delete fx[i];
  delete [] fx;
  delete fxList;

  // //--- cal modified thetaCM vs z
  // TObjArray * txList = new TObjArray();
  // TGraph ** tx = new TGraph*[numEx];
  // for( int j = 0 ; j < numEx; j++){
  //   double a = helios.GetDetRadius();
  //   double q = TMath::Sqrt(mb*mb + kbCM[j] * kbCM[j] );
  //   px.clear();
  //   py.clear();
  //   countfx = 0;
  //   for(int i = 0; i < 100; i++){
  //     double thetacm = (i + 8.) * TMath::DegToRad();
  //     double temp = TMath::TwoPi() * slope / betaRect / kbCM[j] * a / TMath::Sin(thetacm); 
  //     double pxTemp = betaRect /slope * (gamma * betaRect * q - gamma * kbCM[j] * TMath::Cos(thetacm)) * (1 - TMath::ASin(temp)/TMath::TwoPi());
  //     double pyTemp = thetacm * TMath::RadToDeg();   
  //     if( TMath::IsNaN(pxTemp) || TMath::IsNaN(pyTemp) ) continue;
  //     px.push_back(pxTemp);
  //     py.push_back(pyTemp);
  //     countfx ++;
  //   }

  //   tx[j] = new TGraph(countfx, &px[0], &py[0]);
  //   name.Form("tx%d", j);
  //   tx[j]->SetName(name);
  //   tx[j]->SetLineColor(4);
  //   txList->Add(tx[j]);
  //   printf("*");
  // }
  // txList->Write("thetaCM_Z", TObject::kSingleKey);
  // printf(" %d thetaCM-z for finite-size detector functions\n", numEx);
  
  //========timer
  TBenchmark clock;
  bool shown ;   
  clock.Reset();
  clock.Start("timer");
  shown = false;
  
  //change the number of event into human easy-to-read form
  int numEvent = reactionConfig.numEvents;
  int digitLen = TMath::Floor(TMath::Log10(numEvent));
  TString numEventStr;
  if( 3 <= digitLen && digitLen < 6 ){
    numEventStr.Form("%5.1f kilo", numEvent/1000.);
  }else if ( 6<= digitLen && digitLen < 9 ){
    numEventStr.Form("%6.2f million", numEvent/1e6);    
  }else if ( 9<= digitLen ){
    numEventStr.Form("%6.2f billion", numEvent/1e9);    
  }
  printf("\e[32m#################################### generating %s events \e[0m\n", numEventStr.Data());

  double KEA = reactionConfig.beamEnergy;
  double theta = reactionConfig.beamTheta;
  double  phi = 0.0;
  
  TF1 * angDist = nullptr;

  //*====================================================== calculate event
  count = 0;
  for( int i = 0; i < numEvent; i++){
    bool redoFlag = true;
    if( !reactionConfig.isRedo ) redoFlag = false;
    do{

      rID = gRandom->Integer( numTransfer );
      if( !listOfTransfer[rID] ) continue;
      
      //==== Set Ex of B
      ExID = transfer[rID].GetRandomExID();
      double sigma = transfer[rID].GetExList()->ExList[ExID].sigma;
      Ex = transfer[rID].GetExList()->ExList[ExID].Ex + gRandom->Gaus(0, sigma);

      transfer[rID].SetExB(Ex);
  
      //==== Set incident beam
      if( reactionConfig.beamEnergySigma != 0 ){
        KEA = gRandom->Gaus(reactionConfig.beamEnergy, reactionConfig.beamEnergySigma);
      }
      if( reactionConfig.beamThetaSigma != 0 ){
        theta = gRandom->Gaus(reactionConfig.beamTheta, reactionConfig.beamThetaSigma);
      }
  
      //==== for taregt scattering
      transfer[rID].SetIncidentEnergyAngle(KEA, theta, 0.);
      transfer[rID].CalReactionConstant();

      // TLorentzVector PA = transfer.GetPA();            
      //depth = 0;
      // if( isTargetScattering ){
      //   //==== Target scattering, only energy loss
      //   depth = targetThickness * gRandom->Rndm();
      //   msA.SetTarget(density, depth); 
      //   TLorentzVector PAnew = msA.Scattering(PA);
      //   KEAnew = msA.GetKE()/reactConfig.beamA;
      //   transfer.SetIncidentEnergyAngle(KEAnew, theta, phi);
      //   transfer.CalReactionConstant();
      //   Ecm = transfer.GetCMTotalKE();
      // }

      //==== Calculate thetaCM, phiCM
      if( distFile->IsOpen()){
        angDist = (TF1 *) distList->At(ExID);
        thetaCM = angDist->GetRandom() / 180. * TMath::Pi();
      }else{
        thetaCM = TMath::ACos(2 * gRandom->Rndm() - 1) ; 
      }

      double phiCM = TMath::TwoPi() * gRandom->Rndm(); 

      //==== Calculate reaction
      transfer[rID].Event(thetaCM, phiCM);
      TLorentzVector Pb = transfer[rID].GetPb();
      TLorentzVector PB = transfer[rID].GetPB();

      // //==== Calculate energy loss of scattered and recoil in target
      // if( isTargetScattering ){
      //   if( Pb.Theta() < TMath::PiOver2() ){
      //     msb.SetTarget(density, targetThickness - depth);
      //   }else{
      //     msb.SetTarget(density, depth);
      //   }
      //   Pb = msb.Scattering(Pb);
      //   TbLoss = msb.GetKELoss();
      //   msB.SetTarget(density, targetThickness - depth);
      //   PB = msB.Scattering(PB);
      // }else{
      //   TbLoss = 0;
      // }

    //======= Decay of particle-B
    int decayID = 0;
    if( transfer[rID].GetRecoil().isDecay){
      
      decayID = decay[rID].CalDecay(PB, Ex, 0, phiCM + TMath::Pi()/2); // decay to ground state
      if( decayID == 1 ){
        PB = decay[rID].GetDaugther_D();
        //decayTheta = decay.GetAngleChange();
        decayTheta = decay[rID].GetThetaCM();
        PB.SetUniqueID(transfer[rID].GetRecoil().decayZ);
      }else{
        decayTheta = TMath::QuietNaN();
      }
    }

    //################################### tree branches
    //===== reaction
    thetab = Pb.Theta() * TMath::RadToDeg();
    thetaB = PB.Theta() * TMath::RadToDeg();

    Tb = Pb.E() - Pb.M();
    TB = PB.E() - PB.M();

    phib = Pb.Phi() * TMath::RadToDeg();
    phiB = PB.Phi() * TMath::RadToDeg();

    //==== Helios
    
    // printf(" thetaCM : %f, Tb : %f\n", thetaCM * TMath::RadToDeg(), Pb.M());
    
    if( Tb > 0  || TB > 0 ){

      helios[rID].CalArrayHit(Pb);
      helios[rID].CalRecoilHit(PB);
      hit = 2;
      while( hit > 1 ){  hit = helios[rID].CheckDetAcceptance(); } /// while hit > 1, goto next loop;
      
      trajectory orb_b = helios[rID].GetTrajectory_b();
      trajectory orb_B = helios[rID].GetTrajectory_B();
      
      e = helios[rID].GetEnergy() + gRandom->Gaus(0, helios[rID].GetArrayGeometry().eSigma );

      double ranX = gRandom->Gaus(0, helios[rID].GetArrayGeometry().zSigma);
      z = orb_b.z + ranX;
      detX = helios[rID].GetDetX() + ranX;
 
      z0 = orb_b.z0;
      t = orb_b.t;
      loop = orb_b.loop;
      detID = orb_b.detID;
      detRowID = orb_b.detRowID;
      rho = orb_b.rho;
      rhoArray = orb_b.R;
      
      xArray = orb_b.x;
      yArray = orb_b.y;
      
      //ELUM
      double elumPos1 = helios[rID].GetAuxGeometry().elumPos1;
      if( elumPos1 != 0 ){
        xElum1   = helios[rID].GetXPos(elumPos1);
        yElum1   = helios[rID].GetYPos(elumPos1);
        rhoElum1 = helios[rID].GetR   (elumPos1);
      }
      double elumPos2 = helios[rID].GetAuxGeometry().elumPos2;
      if( elumPos2 ){
        xElum2   = helios[rID].GetXPos(elumPos2);
        yElum2   = helios[rID].GetYPos(elumPos2);
        rhoElum2 = helios[rID].GetR   (elumPos2);
      }

      //Recoil
      rhoRecoil = orb_B.R;
      tB = orb_B.t;
      xRecoil = orb_B.x;
      yRecoil = orb_B.y;
      rhoB = orb_B.rho;

      //other recoil detectors
      double recoilPos1 = helios[rID].GetAuxGeometry().detPos1;
      if ( recoilPos1 != 0 ){
        xRecoil1   = helios[rID].GetRecoilXPos(recoilPos1);
        yRecoil1   = helios[rID].GetRecoilYPos(recoilPos1);
        rhoRecoil1 = helios[rID].GetRecoilR   (recoilPos1);
      }
      double recoilPos2 = helios[rID].GetAuxGeometry().detPos2;
      if ( recoilPos2 != 0 ){
        xRecoil2   = helios[rID].GetRecoilXPos(recoilPos2);
        yRecoil2   = helios[rID].GetRecoilYPos(recoilPos2);
        rhoRecoil2 = helios[rID].GetRecoilR   (recoilPos2);
      }
      
      std::pair<double,double> ExThetaCM = transfer[rID].CalExThetaCM(e, z, helios[rID].GetBField(), helios[rID].GetDetRadius());
      ExCal = ExThetaCM.first;
      thetaCMCal = ExThetaCM.second;

      //change thetaCM into deg
      thetaCM = thetaCM * TMath::RadToDeg();
      
      //if decay, get the light decay particle on the recoil;
      if( transfer[rID].GetRecoil().isDecay ){
        if( decayID == 1 ){
          TLorentzVector Pd = decay[rID].GetDaugther_d();
          
          Td = Pd.E() - Pd.M();
          
          helios[rID].CalRecoilHit(Pd);
          
          trajectory orb_d = helios[rID].GetTrajectory_B();
          rhoRecoil_d = orb_d.R;
          xRecoil_d = orb_d.x;
          yRecoil_d = orb_d.y;
          
        }else{
          rhoRecoil_d = TMath::QuietNaN();
          xRecoil_d = TMath::QuietNaN();
          yRecoil_d = TMath::QuietNaN();
        }
      }
    
    }else{
      hit = -404;
    }

    if( hit == 1)  count ++;

    if( reactionConfig.isRedo ){
      if( hit == 1) {
        redoFlag = false;
      }else{
        redoFlag = true;
        //printf("%d, %2d, thetaCM : %f, theta : %f, z0: %f \n", i, hit, thetaCM * TMath::RadToDeg(), thetab, helios.GetZ0());
      }
    }else{
      redoFlag = false;
    }

    }while( redoFlag );
    tree->Fill();

    //#################################################################### Timer  
    clock.Stop("timer");
    Double_t time = clock.GetRealTime("timer");
    clock.Start("timer");

    if ( !shown ) {
      if (fmod(time, 10) < 1 ){
        printf( "%10d[%2d%%]| %8.2f sec | expect: %5.1f min \n", i, TMath::Nint((i+1)*100./numEvent), time , numEvent*time/(i+1)/60);
        shown = 1;
      }
    }else{
      if (fmod(time, 10) > 9 ){
        shown = 0;
      }
    }
    
  }

  saveFile->Write();
  saveFile->Close();
  
  distFile->Close();
  delete angDist;

  printf("=============== done. saved as %s. count(hit==1) : %d\n", saveFileName.Data(), count);
  //gROOT->ProcessLine(".q");

  delete [] transfer;
  delete [] decay;
  delete [] helios;

  distFile->Close();
  delete distFile;
  delete distList;
  delete dwbaExList;
  delete dwbaReactList;

  return;

}


int main (int argc, char *argv[]) {
  
  printf("=================================================================\n");
  printf("==========     Simulate Transfer reaction in HELIOS    ==========\n");
  printf("=================================================================\n");
  
  if(argc == 2 || argc > 5 ) { 
    printf("Usage: ./Transfer [1] [2] [3] [4]\n");
    printf("       default file name \n");
    printf("   [1] reactionConfig.txt (input) reaction Setting \n");
    printf("   [2] detectorGeo.txt    (input) detector Setting \n");
    printf("   [3] DWBA.root          (input) thetaCM distribution from DWBA \n");  
    printf("   [4] transfer.root      (output) rootFile name for output \n");

    printf("-----------------------------------------------------------------\n");
    printf(" When DWBA.root provided.\n");
    printf(" The excitation energies from the DWBA are used.\n");
    printf(" And the excitation energies in reactionConfig.txt will be ignored\n");
    printf("=================================================================\n");
    return 0 ; 
  }

  std::string  basicConfig = "reactionConfig.txt";
  std::string   detGeoFile = "detectorGeo.txt";
  TString      ptolemyRoot = "DWBA.root"; // when no file, use isotropic distribution of thetaCM
  TString     saveFileName = "transfer.root";
  bool           isPlot = false;
   
  if( argc >= 2) basicConfig = argv[1];  
  if( argc >= 3) detGeoFile = argv[2];  
  if( argc >= 4) ptolemyRoot = argv[3];
  if( argc >= 5) saveFileName = argv[4];
  
  Transfer( basicConfig, detGeoFile, ptolemyRoot, saveFileName);

  //run Armory/Check_Simulation
  // if( isPlot ){
  //   std::ifstream file_in;
  //   file_in.open("../Cleopatra/Check_Simulation.C", std::ios::in);
  //   if( file_in){
  //     printf("---- running ../Cleopatra/Check_Simulation.C on %s \n", saveFileName.Data());
  //     TString cmd;
  //     cmd.Form("root -l '../Cleopatra/Check_Simulation.C(\"%s\")'", saveFileName.Data());
      
  //     system(cmd.Data());
  //   }else{
  //     printf("cannot find ../Cleopatra/Check_Simulation.C \n");
  //   }
  // }
  
}

