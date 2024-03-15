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

double exDistFunc(Double_t *x, Double_t * par){
  return par[(int) x[0]];
}

void PrintEZPlotPara(TransferReaction tran, HELIOS helios){

  printf("==================================== E-Z plot slope\n");
  double  betaRect = tran.GetReactionBeta() ;
  double     gamma = tran.GetReactionGamma();
  double        mb = tran.GetMass_b();
  double       pCM = tran.GetMomentumbCM();
  double         q = TMath::Sqrt(mb*mb + pCM*pCM); ///energy of light recoil in center of mass
  double     slope = tran.GetEZSlope(helios.GetBField()); /// MeV/mm
  printf("                       e-z slope : %f MeV/mm\n", slope);   
  double intercept = q/gamma - mb; // MeV
  printf("    e-z intercept (ground state) : %f MeV\n", intercept); 

}

void Transfer(
         std::string      basicConfig = "reactionConfig.txt", 
         std::string heliosDetGeoFile = "detectorGeo.txt", 
         unsigned short            ID = 0,  // this is the ID for the array
         TString          ptolemyRoot = "DWBA.root",
         TString         saveFileName = "transfer.root"){

  //*############################################# Set Reaction
  TransferReaction transfer;
  HELIOS helios;
  Decay decay;

  std::vector<double> kbCM; /// momentum of b in CM frame
  TF1 * exDist = nullptr;

  transfer.SetReactionFromFile(basicConfig, ID);
  helios.SetDetectorGeometry(heliosDetGeoFile, ID);

  printf("*****************************************************************\n");
  printf("*\e[1m\e[33m        %27s                            \e[0m*\n", transfer.GetReactionName().Data());
  printf("*****************************************************************\n");
  printf("----- loading reaction setting from %s. \n", basicConfig.c_str());
  printf("----- loading geometry setting from %s. \n", heliosDetGeoFile.c_str());
 
  printf("\e[32m#################################### Reaction & HELIOS configuration\e[0m\n");   

  transfer.PrintReaction(false);

  if(transfer.GetRecoil().isDecay) {
    decay.SetMotherDaugther(transfer.GetRecoil());
  }

  helios.PrintGeometry();
  PrintEZPlotPara(transfer, helios);


  DetGeo detGeo = helios.GetDetectorGeometry();
  Array array = helios.GetArrayGeometry();
  ReactionConfig reactConfig = transfer.GetRectionConfig();
  Recoil recoil = transfer.GetRecoil();
  
  //*############################################# save reaction.dat
  // if( filename != "" ) {
  //   FILE * keyParaOut;
  //   keyParaOut = fopen (filename.Data(), "w+");

  //   printf("=========== save key reaction constants to %s \n", filename.Data());
  //   fprintf(keyParaOut, "%-15.4f  //%s\n", transfer.GetMass_b(), "mass_b");
  //   fprintf(keyParaOut, "%-15d  //%s\n",   reactConfig.recoilLightZ, "charge_b");
  //   fprintf(keyParaOut, "%-15.8f  //%s\n", transfer.GetReactionBeta(), "betaCM");
  //   fprintf(keyParaOut, "%-15.4f  //%s\n", transfer.GetCMTotalEnergy(), "Ecm");
  //   fprintf(keyParaOut, "%-15.4f  //%s\n", transfer.GetMass_B(), "mass_B");
  //   fprintf(keyParaOut, "%-15.4f  //%s\n", slope/betaRect, "alpha=slope/betaRect");

  //   fflush(keyParaOut);
  //   fclose(keyParaOut);
  // }
  
  //*############################################# Target scattering, only energy loss
  // bool isTargetScattering = reactConfig.isTargetScattering;
  // float           density = reactConfig.targetDensity;
  // float   targetThickness = reactConfig.targetThickness;
  
  // if(isTargetScattering) printf("\e[32m#################################### Target Scattering\e[0m\n");
  // TargetScattering msA;
  // TargetScattering msB;
  // TargetScattering msb;

  // if(reactConfig.isTargetScattering) printf("======== Target : (thickness : %6.2f um) x (density : %6.2f g/cm3) = %6.2f ug/cm2\n", 
  //                                              targetThickness * 1e+4, 
  //                                              density, 
  //                                              targetThickness * density * 1e+6);  

  // if( reactConfig.isTargetScattering ){
  //   msA.LoadStoppingPower(reactConfig.beamStoppingPowerFile);
  //   msb.LoadStoppingPower(reactConfig.recoilLightStoppingPowerFile);
  //   msB.LoadStoppingPower(reactConfig.recoilHeavyStoppingPowerFile);
  // }

  ExcitedEnergies exList = transfer.GetRectionConfig().exList[ID];
  
  //*############################################# Load DWBAroot for thetaCM distribution
  printf("\e[32m#################################### Load DWBA input : %s  \e[0m\n", ptolemyRoot.Data());
  TF1 * dist = NULL;
  TFile * distFile = new TFile(ptolemyRoot, "read");
  TObjArray * distList = nullptr;
  TMacro * dwbaExList = nullptr;
  if( distFile->IsOpen() ) {
    printf("--------- Found DWBA thetaCM distributions. Use the ExList from DWBA.\n"); 

    distList = (TObjArray *) distFile->FindObjectAny("thetaCM_TF1"); // the function List

    exList.Clear();

    dwbaExList = (TMacro *) distFile->FindObjectAny("ExList");   
    int numEx = dwbaExList->GetListOfLines()->GetSize() - 1 ;
    for(int i = 1; i <= numEx ; i++){
      std::string temp = dwbaExList->GetListOfLines()->At(i)->GetName();
      if( temp[0] == '/' ) continue;
      std::vector<std::string> tempStr = AnalysisLib::SplitStr(temp, " ");
      exList.Add( atof(tempStr[0].c_str()), atof(tempStr[1].c_str()), 1.0, 0.00);
    }

  }else{
    printf("------- no DWBA input. Use the ExList from %s\n", basicConfig.c_str());    
  }


  printf("------------------------------ Heavy Recoil excitation\n"); 
  printf("Energy[MeV]  Rel.Xsec     SF  sigma\n");

  int numEx = exList.ExList.size();

  for( int j = 0; j < numEx; j++){
    double ex = exList.ExList[j].Ex;
    kbCM.push_back(transfer.CalkCM(ex));
    int decayID = decay.CalDecay(TLorentzVector (0,0,0,0), ex, 0);
    exList.ExList[j].Print(decayID == 1 ? "-->Decay" : "\n");
  } 

  //---- create Ex-distribution
  if( exList.ExList.size() > 1 ) {
    printf("---- creating Ex-distribution \n");
    exDist = new TF1("exDist", exDistFunc, 0, numEx, numEx);
    for(int q = 0; q < numEx; q++){
      exDist->SetParameter(q, exList.ExList[q].xsec*exList.ExList[q].SF);
    }
  }

  //*############################################# build tree
  printf("\e[32m#################################### building Tree in %s\e[0m\n", saveFileName.Data());
  TFile * saveFile = new TFile(saveFileName, "recreate");
  TTree * tree = new TTree("tree", "tree");

  TMacro config(basicConfig.c_str());
  TMacro detGeoTxt(heliosDetGeoFile.c_str());
  config.SetName(transfer.GetReactionName_Latex().Data());
  config.Write("reactionConfig");
  detGeoTxt.Write("detGeo");

  if( distList != NULL ) distList->Write("DWBA", 1);
  if( dwbaExList != NULL ) dwbaExList->Write("DWBA_ExList", 1);

  TMacro idMacro;
  idMacro.AddLine(Form("%d", ID));
  idMacro.Write("detGeoID");
  
  TMacro hitMeaning;
  hitMeaning.AddLine("======================= meaning of Hit\n"); 
  for( int code = -15 ; code <= 1; code ++ ){
    hitMeaning.AddLine( Form( "%4d = %s", code, helios.AcceptanceCodeToMsg(code).Data() ));
  }
  hitMeaning.AddLine(" other = unknown\n");
  hitMeaning.AddLine("===========================================\n");  
  hitMeaning.Write("hitMeaning");

  int hit; /// the output of Helios.CalHit
  tree->Branch("hit", &hit, "hit/I");
  
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
  if( recoil.isDecay ) {
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
  if( detGeo.elumPos1 != 0 ) {
    tree->Branch("xElum1",     &xElum1, "xElum1/D");
    tree->Branch("yElum1",     &yElum1, "yElum1/D");
    tree->Branch("rhoElum1", &rhoElum1, "rhoElum1/D");
  }
  
  double xElum2, yElum2, rhoElum2;
  if( detGeo.elumPos2 != 0 ) {
    tree->Branch("xElum2",     &xElum2, "xElum2/D");
    tree->Branch("yElum2",     &yElum2, "yElum2/D");
    tree->Branch("rhoElum2", &rhoElum2, "rhoElum2/D");
  }
  
  ///in case need other recoil detector. 
  double xRecoil1, yRecoil1, rhoRecoil1;
  if( detGeo.recoilPos1 != 0 ){
    tree->Branch("xRecoil1",     &xRecoil1, "xRecoil1/D");
    tree->Branch("yRecoil1",     &yRecoil1, "yRecoil1/D");
    tree->Branch("rhoRecoil1", &rhoRecoil1, "rhoRecoil1/D");
  }
  double xRecoil2, yRecoil2, rhoRecoil2;
  if( detGeo.recoilPos2 != 0 ){
    tree->Branch("xRecoil2",     &xRecoil2, "xRecoil2/D");
    tree->Branch("yRecoil2",     &yRecoil2, "yRecoil2/D");
    tree->Branch("rhoRecoil2", &rhoRecoil2, "rhoRecoil2/D");
  }
  //======= function for e-z plot for ideal case
  printf("++++ generate functions\n");
  TObjArray * gList = new TObjArray();
  gList->SetName("Constant thetaCM lines");
  const int gxSize = 50;
  TF1 ** gx = new TF1*[gxSize];
  TString name;

  double mb = transfer.GetMass_b();
  double betaRect = transfer.GetReactionBeta();
  double gamma = transfer.GetReactionGamma();
  double slope = transfer.GetEZSlope(helios.GetBField()); /// MeV/mm

  for( int i = 0; i < gxSize; i++){
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

  //--- cal modified f
  TObjArray * fxList = new TObjArray();
  TGraph ** fx = new TGraph*[numEx];
  std::vector<double> px, py;
  int countfx = 0;
  for( int j = 0 ; j < numEx; j++){
    double a = helios.GetDetRadius();
    double q = TMath::Sqrt(mb*mb + kbCM[j] * kbCM[j] );
    px.clear();
    py.clear();
    countfx = 0;
    for(int i = 0; i < 100; i++){
      double thetacm = TMath::Pi()/TMath::Log(100) * (TMath::Log(100) - TMath::Log(100-i)) ;//using log scale, for more point in small angle.
      double temp = TMath::TwoPi() * slope / betaRect / kbCM[j] * a / TMath::Sin(thetacm); 
      double pxTemp = betaRect /slope * (gamma * betaRect * q - gamma * kbCM[j] * TMath::Cos(thetacm)) * (1 - TMath::ASin(temp)/TMath::TwoPi()) ;
      double pyTemp = gamma * q - mb - gamma * betaRect * kbCM[j] * TMath::Cos(thetacm);   
      if( TMath::IsNaN(pxTemp) || TMath::IsNaN(pyTemp) ) continue;
      px.push_back(pxTemp);
      py.push_back(pyTemp);
      countfx ++;
    }

    fx[j] = new TGraph(countfx, &px[0], &py[0]);
    name.Form("fx%d", j);
    fx[j]->SetName(name);
    fx[j]->SetLineColor(4);
    fxList->Add(fx[j]);
    printf(",");
  }
  fxList->Write("EZCurve", TObject::kSingleKey);
  printf(" %d e-z finite-size detector functions\n", numEx);

  //--- cal modified thetaCM vs z
  TObjArray * txList = new TObjArray();
  TGraph ** tx = new TGraph*[numEx];
  for( int j = 0 ; j < numEx; j++){
    double a = helios.GetDetRadius();
    double q = TMath::Sqrt(mb*mb + kbCM[j] * kbCM[j] );
    px.clear();
    py.clear();
    countfx = 0;
    for(int i = 0; i < 100; i++){
      double thetacm = (i + 8.) * TMath::DegToRad();
      double temp = TMath::TwoPi() * slope / betaRect / kbCM[j] * a / TMath::Sin(thetacm); 
      double pxTemp = betaRect /slope * (gamma * betaRect * q - gamma * kbCM[j] * TMath::Cos(thetacm)) * (1 - TMath::ASin(temp)/TMath::TwoPi());
      double pyTemp = thetacm * TMath::RadToDeg();   
      if( TMath::IsNaN(pxTemp) || TMath::IsNaN(pyTemp) ) continue;
      px.push_back(pxTemp);
      py.push_back(pyTemp);
      countfx ++;
    }

    tx[j] = new TGraph(countfx, &px[0], &py[0]);
    name.Form("tx%d", j);
    tx[j]->SetName(name);
    tx[j]->SetLineColor(4);
    txList->Add(tx[j]);
    printf("*");
  }
  txList->Write("thetaCM_Z", TObject::kSingleKey);
  printf(" %d thetaCM-z for finite-size detector functions\n", numEx);
  
  //========timer
  TBenchmark clock;
  bool shown ;   
  clock.Reset();
  clock.Start("timer");
  shown = false;
  
  //change the number of event into human easy-to-read form
  int numEvent = reactConfig.numEvents;
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

  double KEA = reactConfig.beamEnergy;
  double theta = reactConfig.beamTheta;
  double  phi = 0.0;
      
  //*====================================================== calculate event
  int count = 0;
  for( int i = 0; i < numEvent; i++){
    bool redoFlag = true;
    if( !reactConfig.isRedo ) redoFlag = false;
    do{
      
      //==== Set Ex of B
      if( numEx == 1 ) {
        ExID = 0;
        Ex = exList.ExList[0].Ex + (exList.ExList[0].sigma == 0 ? 0 : gRandom->Gaus(0, exList.ExList[0].sigma));
      }else{
        ExID = exDist->GetRandom();
        Ex = exList.ExList[ExID].Ex + (exList.ExList[ExID].sigma == 0 ? 0 : gRandom->Gaus(0, exList.ExList[ExID].sigma));
      }
      transfer.SetExB(Ex);
  
      //==== Set incident beam
      if( reactConfig.beamEnergySigma != 0 ){
        KEA = gRandom->Gaus(reactConfig.beamEnergy, reactConfig.beamEnergySigma);
      }
      if( reactConfig.beamThetaSigma != 0 ){
        theta = gRandom->Gaus(reactConfig.beamTheta, reactConfig.beamThetaSigma);
      }
  
      //==== for taregt scattering
      transfer.SetIncidentEnergyAngle(KEA, theta, 0.);
      transfer.CalReactionConstant();

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
        dist = (TF1 *) distList->At(ExID);
        thetaCM = dist->GetRandom() / 180. * TMath::Pi();
      }else{
        thetaCM = TMath::ACos(2 * gRandom->Rndm() - 1) ; 
      }

      double phiCM = TMath::TwoPi() * gRandom->Rndm(); 

      //==== Calculate reaction
      transfer.Event(thetaCM, phiCM);
      TLorentzVector Pb = transfer.GetPb();
      TLorentzVector PB = transfer.GetPB();

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
    if( recoil.isDecay){
      
      decayID = decay.CalDecay(PB, Ex, 0, phiCM + TMath::Pi()/2); // decay to ground state
      if( decayID == 1 ){
        PB = decay.GetDaugther_D();
        //decayTheta = decay.GetAngleChange();
        decayTheta = decay.GetThetaCM();
        PB.SetUniqueID(recoil.decayZ);
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

      helios.CalArrayHit(Pb);
      helios.CalRecoilHit(PB);
      hit = 2;
      while( hit > 1 ){  hit = helios.CheckDetAcceptance(); } /// while hit > 1, goto next loop;
      
      trajectory orb_b = helios.GetTrajectory_b();
      trajectory orb_B = helios.GetTrajectory_B();
      
      e = helios.GetEnergy() + gRandom->Gaus(0, array.eSigma );

      double ranX = gRandom->Gaus(0, array.zSigma);
      z = orb_b.z + ranX;
      detX = helios.GetDetX() + ranX;
 
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
      if( detGeo.elumPos1 != 0 ){
        xElum1   = helios.GetXPos(detGeo.elumPos1);
        yElum1   = helios.GetYPos(detGeo.elumPos1);
        rhoElum1 = helios.GetR(detGeo.elumPos1);
      }
      if( detGeo.elumPos2 != 0 ){
        xElum2   = helios.GetXPos(detGeo.elumPos2);
        yElum2   = helios.GetYPos(detGeo.elumPos2);
        rhoElum2 = helios.GetR(detGeo.elumPos2);
      }

      //Recoil
      rhoRecoil = orb_B.R;
      tB = orb_B.t;
      xRecoil = orb_B.x;
      yRecoil = orb_B.y;
      rhoB = orb_B.rho;

      //other recoil detectors
      if ( detGeo.recoilPos1 != 0 ){
        xRecoil1   = helios.GetRecoilXPos(detGeo.recoilPos1);
        yRecoil1   = helios.GetRecoilYPos(detGeo.recoilPos1);
        rhoRecoil1 = helios.GetRecoilR(detGeo.recoilPos1);
      }
      if ( detGeo.recoilPos2 != 0 ){
        xRecoil2   = helios.GetRecoilXPos(detGeo.recoilPos2);
        yRecoil2   = helios.GetRecoilYPos(detGeo.recoilPos2);
        rhoRecoil2 = helios.GetRecoilR(detGeo.recoilPos2);
      }
      
      std::pair<double,double> ExThetaCM = transfer.CalExThetaCM(e, z, helios.GetBField(), helios.GetDetRadius());
      ExCal = ExThetaCM.first;
      thetaCMCal = ExThetaCM.second;

      //change thetaCM into deg
      thetaCM = thetaCM * TMath::RadToDeg();
      
      //if decay, get the light decay particle on the recoil;
      if( recoil.isDecay ){
        if( decayID == 1 ){
          TLorentzVector Pd = decay.GetDaugther_d();
          
          Td = Pd.E() - Pd.M();
          
          helios.CalRecoilHit(Pd);
          
          trajectory orb_d = helios.GetTrajectory_B();
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

    if( reactConfig.isRedo ){
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
  delete exDist;

  printf("=============== done. saved as %s. count(hit==1) : %d\n", saveFileName.Data(), count);
  //gROOT->ProcessLine(".q");

  return;

}


int main (int argc, char *argv[]) {
  
  printf("=================================================================\n");
  printf("==========     Simulate Transfer reaction in HELIOS    ==========\n");
  printf("=================================================================\n");
  
  if(argc == 2 || argc > 7) { 
    printf("Usage: ./Transfer [1] [2] [3] [4] [5] [6]\n");
    printf("       default file name \n");
    printf("   [1] reactionConfig.txt (input) reaction Setting \n");
    printf("   [2] detectorGeo.txt    (input) detector Setting \n");
    printf("   [3] ID                 (input) detector & reaction ID (default = 0 ) \n");
    printf("   [4] DWBA.root          (input) thetaCM distribution from DWBA \n");
    printf("   [5] transfer.root      (output) rootFile name for output \n");
    printf("   [6] plot               (input) will it plot stuffs [1/0] \n");

    printf("------------------------------------------------------\n");
    return 0 ; 
  }

  std::string       basicConfig = "reactionConfig.txt";
  std::string  heliosDetGeoFile = "detectorGeo.txt";
  int                   ID = 0;
  TString      ptolemyRoot = "DWBA.root"; // when no file, use isotropic distribution of thetaCM
  TString     saveFileName = "transfer.root";
  bool           isPlot = false;
   
  if( argc >= 2) basicConfig = argv[1];  
  if( argc >= 3) heliosDetGeoFile = argv[2];  
  if( argc >= 4) ID = atoi(argv[3]);  
  if( argc >= 5) ptolemyRoot = argv[4];
  if( argc >= 6) saveFileName = argv[5];
  if( argc >= 7) isPlot = atoi(argv[7]);
  
  Transfer( basicConfig, heliosDetGeoFile, ID, ptolemyRoot, saveFileName);

  //run Armory/Check_Simulation
  if( isPlot ){
    std::ifstream file_in;
    file_in.open("../Cleopatra/Check_Simulation.C", std::ios::in);
    if( file_in){
      printf("---- running ../Cleopatra/Check_Simulation.C on %s \n", saveFileName.Data());
      TString cmd;
      cmd.Form("root -l '../Cleopatra/Check_Simulation.C(\"%s\")'", saveFileName.Data());
      
      system(cmd.Data());
    }else{
      printf("cannot find ../Cleopatra/Check_Simulation.C \n");
    }
  }
  
}

