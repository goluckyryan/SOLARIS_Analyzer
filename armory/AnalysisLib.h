#ifndef ANALYSIS_LIB_H
#define ANALYSIS_LIB_H

#include <cstdio>
#include <vector>
#include <string>

#include <TMacro.h>
#include <TList.h>

namespace AnalysisLib {

std::vector<std::string> SplitStr(std::string tempLine, std::string splitter, int shift = 0){

  std::vector<std::string> output;

  size_t pos;
  do{
    pos = tempLine.find(splitter); /// fine splitter
    if( pos == 0 ){ ///check if it is splitter again
      tempLine = tempLine.substr(pos+1);
      continue;
    }

    std::string secStr;
    if( pos == std::string::npos ){
      secStr = tempLine;
    }else{
      secStr = tempLine.substr(0, pos+shift);
      tempLine = tempLine.substr(pos+shift);
    }

    ///check if secStr is begin with space
    while( secStr.substr(0, 1) == " ") secStr = secStr.substr(1);
    
    ///check if secStr is end with space
    while( secStr.back() == ' ') secStr = secStr.substr(0, secStr.size()-1);

    output.push_back(secStr);
    ///printf(" |%s---\n", secStr.c_str());
    
  }while(pos != std::string::npos );

  return output;
}


struct DetGeo{
   
  double Bfield;      /// T
  int BfieldSign ;    /// sign of B-field
  double BfieldTheta; /// rad, 0 = z-axis, pi/2 = y axis, pi = -z axis
  double bore;        /// bore , mm
  double detPerpDist;    /// distance from axis
  double detWidth;       /// width   
  double detLength;         /// length
  
  double recoilPos;      /// recoil, downstream
  double recoilInnerRadius;    /// radius recoil inner
  double recoilOuterRadius;   /// radius recoil outter

  double recoilPos1, recoilPos2; /// imaginary recoils

  double elumPos1, elumPos2; /// imaginary elum, only sensitive to light recoil

  double blocker;
  double firstPos;     /// meter

  double eSigma;    /// intrinsic energy resolution MeV
  double zSigma;    /// intrinsic position resolution mm
  
  std::vector<double> pos;  /// near position in meter
  int nDet, mDet;      /// nDet = number of different pos, mDet, number of same pos
  double zMin, zMax;   /// range of detectors

  std::vector<double> detPos; ///absolute position of detector

  bool isCoincidentWithRecoil;

  bool  detFaceOut;    ///detector_facing_Out_or_In

};

struct ReactionConfig{

  int beamA;       
  int beamZ;       
  int targetA;     
  int targetZ;     
  int recoilLightA;
  int recoilLightZ;
  int recoilHeavyA;
  int recoilHeavyZ;
  float beamEnergy;         ///MeV/u
  float beamEnergySigma;    ///beam-energy_sigma_in_MeV/u
  float beamAngle;          ///beam-angle_in_mrad
  float beamAngleSigma;     ///beam-emittance_in_mrad
  float beamX;              ///x_offset_of_Beam_in_mm
  float beamY;              ///y_offset_of_Beam_in_mm
  int numEvents;            ///number_of_Event_being_generated
  bool isTargetScattering;  ///isTargetScattering
  float targetDensity;      ///target_density_in_g/cm3
  float targetThickness;    ///targetThickness_in_cm
  std::string beamStoppingPowerFile;         ///stopping_power_for_beam
  std::string recoilLightStoppingPowerFile;  ///stopping_power_for_light_recoil
  std::string recoilHeavyStoppingPowerFile;  ///stopping_power_for_heavy_recoil
  bool isDecay;        ///isDacay
  int heavyDecayA;     ///decayNucleus_A
  int heavyDecayZ;     ///decayNucleus_Z
  bool isRedo;         ///isReDo
  std::vector<float> beamEx;        ///excitation_energy_of_A[MeV]
  

};

///Using TMacro to load the detectorGeo frist,
///this indrect method is good for loading detectorGeo from TMacro in root file
DetGeo LoadDetectorGeo(TMacro * macro){

  DetGeo detGeo;

  if( macro == NULL ) return detGeo;

  TList * haha = macro->GetListOfLines();
  int numLine = (haha)->GetSize();

  detGeo.pos.clear();

  for( int i = 0 ; i < numLine; i++){

    std::vector<std::string> str = SplitStr(macro->GetListOfLines()->At(i)->GetName(), " ");

    ///printf("%d | %s\n", i,  str[0].c_str());

    if( str[0].find_first_of("#") == 0 ) break;

    if ( i == 0 ) {
      detGeo.Bfield = atof(str[0].c_str());
      detGeo.BfieldSign = detGeo.Bfield > 0 ? 1: -1;
    }
    if ( i ==  1 ) detGeo.BfieldTheta       = atof(str[0].c_str());
    if ( i ==  2 ) detGeo.bore              = atof(str[0].c_str());
    if ( i ==  3 ) detGeo.detPerpDist       = atof(str[0].c_str());
    if ( i ==  4 ) detGeo.detWidth          = atof(str[0].c_str());
    if ( i ==  5 ) detGeo.detLength         = atof(str[0].c_str());
    if ( i ==  6 ) detGeo.recoilPos         = atof(str[0].c_str());
    if ( i ==  7 ) detGeo.recoilInnerRadius = atof(str[0].c_str());
    if ( i ==  8 ) detGeo.recoilOuterRadius = atof(str[0].c_str());
    if ( i ==  9 ) detGeo.isCoincidentWithRecoil = str[0] == "false" ? false: true;
    if ( i == 10 ) detGeo.recoilPos1        = atof(str[0].c_str());
    if ( i == 11 ) detGeo.recoilPos2        = atof(str[0].c_str());
    if ( i == 12 ) detGeo.elumPos1          = atof(str[0].c_str());
    if ( i == 13 ) detGeo.elumPos2          = atof(str[0].c_str());
    if ( i == 14 ) detGeo.blocker           = atof(str[0].c_str());
    if ( i == 15 ) detGeo.firstPos          = atof(str[0].c_str());
    if ( i == 16 ) detGeo.eSigma            = atof(str[0].c_str());
    if ( i == 17 ) detGeo.zSigma            = atof(str[0].c_str());
    if ( i == 18 ) detGeo.detFaceOut        = str[0] == "Out" ? true : false;
    if ( i == 19 ) detGeo.mDet              = atoi(str[0].c_str());
    if ( i >= 20 ) (detGeo.pos).push_back(atof(str[0].c_str()));
  }

  detGeo.nDet = (detGeo.pos).size();
  (detGeo.detPos).clear();
  
  for(int id = 0; id < detGeo.nDet; id++){
    if( detGeo.firstPos > 0 ) detGeo.detPos.push_back(detGeo.firstPos + detGeo.pos[id]);
    if( detGeo.firstPos < 0 ) detGeo.detPos.push_back(detGeo.firstPos - detGeo.pos[detGeo.nDet - 1 - id]);
    ///printf("%d | %f, %f \n", id, detGeo.pos[id], detGeo.detPos[id]);
  }

  detGeo.zMin = TMath::Min(detGeo.detPos.front(), detGeo.detPos.back()) - (detGeo.firstPos < 0 ? detGeo.detLength : 0);
  detGeo.zMax = TMath::Max(detGeo.detPos.front(), detGeo.detPos.back()) + (detGeo.firstPos > 0 ? detGeo.detLength : 0);
  
  return detGeo;
}

void PrintDetGeo(DetGeo detGeo){

  printf("=====================================================\n");
  printf("                 B-field: %8.2f  T, Theta : %6.2f deg \n", detGeo.Bfield, detGeo.BfieldTheta);
  if( detGeo.BfieldTheta != 0.0 ) {
    printf("                                      +---- field angle != 0 is not supported!!! \n");
  }
  printf("     Recoil detector pos: %8.2f mm, radius: %6.2f - %6.2f mm \n", detGeo.recoilPos, detGeo.recoilInnerRadius, detGeo.recoilOuterRadius);
  printf("        Blocker Position: %8.2f mm \n", detGeo.firstPos > 0 ? detGeo.firstPos - detGeo.blocker : detGeo.firstPos + detGeo.blocker );
  printf("          First Position: %8.2f mm \n", detGeo.firstPos);
  printf("------------------------------------- Detector Position \n");
  for(int i = 0; i < detGeo.nDet ; i++){
     if( detGeo.firstPos > 0 ){
        printf("%d, %8.2f mm - %8.2f mm \n", i, detGeo.detPos[i], detGeo.detPos[i] + detGeo.detLength);
     }else{
        printf("%d, %8.2f mm - %8.2f mm \n", i, detGeo.detPos[i] - detGeo.detLength , detGeo.detPos[i]);
     }
  }

  printf("   number of det : %d x %d \n", detGeo.mDet, detGeo.nDet);
  printf(" detector facing : %s\n", detGeo.detFaceOut ? "Out" : "In");
  printf("    energy resol.: %f MeV\n", detGeo.eSigma);
  printf("     pos-Z resol.: %f mm \n", detGeo.zSigma);

  if( detGeo.elumPos1 != 0 || detGeo.elumPos2 != 0 || detGeo.recoilPos1 != 0 || detGeo.recoilPos2 != 0){
    printf("=================================== Auxillary/Imaginary Detectors\n");
  }
  if( detGeo.elumPos1 != 0 )   printf("   Elum 1 pos.: %f mm \n", detGeo.elumPos1);
  if( detGeo.elumPos2 != 0 )   printf("   Elum 2 pos.: %f mm \n", detGeo.elumPos2);
  if( detGeo.recoilPos1 != 0 ) printf(" Recoil 1 pos.: %f mm \n", detGeo.recoilPos1);
  if( detGeo.recoilPos2 != 0 ) printf(" Recoil 2 pos.: %f mm \n", detGeo.recoilPos2);
  printf("=====================================================\n");

}

ReactionConfig LoadReactionConfig(TMacro * macro){

  ReactionConfig reaction;

  if( macro == NULL ) return reaction;

  int numLine = macro->GetListOfLines()->GetSize();

  for( int i = 0; i < numLine; i ++){

    std::vector<std::string> str = SplitStr(macro->GetListOfLines()->At(i)->GetName(), " ");

    ///printf("%d | %s\n", i,  str[0].c_str());

    if( str[0].find_first_of("#") == 0 ) break;

    if( i == 0 ) reaction.beamA           = atoi(str[0].c_str());
    if( i == 1 ) reaction.beamZ           = atoi(str[0].c_str());
    if( i == 2 ) reaction.targetA         = atoi(str[0].c_str());
    if( i == 3 ) reaction.targetZ         = atoi(str[0].c_str());
    if( i == 4 ) reaction.recoilLightA    = atoi(str[0].c_str());
    if( i == 5 ) reaction.recoilLightZ    = atoi(str[0].c_str());
    if( i == 6 ) reaction.beamEnergy      = atof(str[0].c_str());
    if( i == 7 ) reaction.beamEnergySigma = atof(str[0].c_str());
    if( i == 8 ) reaction.beamAngle       = atof(str[0].c_str());
    if( i == 9 ) reaction.beamAngleSigma  = atof(str[0].c_str());
    if( i == 10 ) reaction.beamX = atof(str[0].c_str());
    if( i == 11 ) reaction.beamY = atof(str[0].c_str());
    if( i == 12 ) reaction.numEvents = atoi(str[0].c_str());
    if( i == 13 ) {
      if( str[0].compare("false") == 0 ) reaction.isTargetScattering = false;
      if( str[0].compare("true")  == 0 ) reaction.isTargetScattering = true;
    }
    if( i == 14 ) reaction.targetDensity   = atof(str[0].c_str());
    if( i == 15 ) reaction.targetThickness = atof(str[0].c_str());
    if( i == 16 ) reaction.beamStoppingPowerFile = str[0];
    if( i == 17 ) reaction.recoilLightStoppingPowerFile = str[0];
    if( i == 18 ) reaction.recoilHeavyStoppingPowerFile = str[0];
    if( i == 19 ) {
      if( str[0].compare("false") == 0 ) reaction.isDecay = false;
      if( str[0].compare("true")  == 0 ) reaction.isDecay = true;
    }
    if( i == 20 ) reaction.heavyDecayA = atoi(str[0].c_str());
    if( i == 21 ) reaction.heavyDecayZ = atoi(str[0].c_str());
    if( i == 22 ) {
      if( str[0].compare("false") == 0 ) reaction.isRedo = false;
      if( str[0].compare("true" ) == 0 ) reaction.isRedo = true;
    }
    
    if( i >= 23) {
      reaction.beamEx.push_back( atof(str[0].c_str()) );
    }
  }

  reaction.recoilHeavyA = reaction.beamA + reaction.targetA - reaction.recoilLightA;
  reaction.recoilHeavyZ = reaction.beamZ + reaction.targetZ - reaction.recoilLightZ;
  
  return reaction;

}

void PrintReactionConfig(ReactionConfig reaction){

  printf("=====================================================\n");
  printf("   beam : A = %3d, Z = %2d \n", reaction.beamA, reaction.beamZ);
  printf(" target : A = %3d, Z = %2d \n", reaction.targetA, reaction.targetZ);
  printf("  light : A = %3d, Z = %2d \n", reaction.recoilLightA, reaction.recoilLightZ);

  printf(" beam Energy : %.2f +- %.2f MeV/u, dE/E = %5.2f %%\n", reaction.beamEnergy, reaction.beamEnergySigma, reaction.beamEnergySigma/reaction.beamEnergy);
  printf("       Angle : %.2f +- %.2f mrad\n", reaction.beamAngle, reaction.beamAngleSigma);
  printf("      offset : (x,y) = (%.2f, %.2f) mmm \n", reaction.beamX, reaction.beamY);

  printf("##### number of Simulation Events : %d \n", reaction.numEvents);
  
  printf("    is target scattering : %s \n", reaction.isTargetScattering ? "Yes" : "No");

  if(reaction.isTargetScattering){
    printf(" target density : %.f g/cm3\n", reaction.targetDensity);
    printf("      thickness : %.f cm\n", reaction.targetThickness);
    printf("         beam stopping file : %s \n", reaction.beamStoppingPowerFile.c_str());
    printf(" recoil light stopping file : %s \n", reaction.recoilLightStoppingPowerFile.c_str());
    printf(" recoil heavy stopping file : %s \n", reaction.recoilHeavyStoppingPowerFile.c_str());
  }
  
  printf("       is simulate decay : %s \n", reaction.isDecay ? "Yes" : "No");
  if( reaction.isDecay ){
    printf(" heavy decay : A = %d, Z = %d \n", reaction.heavyDecayA, reaction.heavyDecayZ);
  }
  printf(" is Redo until hit array : %s \n", reaction.isRedo ? "Yes" : "No");

  printf(" beam Ex : %.2f MeV \n", reaction.beamEx[0]);
  for( int i = 1; i < (int) reaction.beamEx.size(); i++){
    printf("         %.2f MeV \n", reaction.beamEx[i]);
  }
  
  printf("=====================================================\n");

}

DetGeo detGeo;
ReactionConfig reactionConfig;

void LoadDetGeoAndReactionConfigFile(string detGeoFileName = "detectorGeo.txt", string reactionConfigFileName = "reactionConfig.txt"){
  printf("=======================\n");
  printf(" loading detector geometery : %s.", detGeoFileName.c_str());
  TMacro * haha = new TMacro();
  if( haha->ReadFile(detGeoFileName.c_str()) > 0 ) {
    detGeo = AnalysisLib::LoadDetectorGeo(haha);    
    printf("... done.\n");
    AnalysisLib::PrintDetGeo(detGeo);
  }else{
    printf("... fail\n");
  }

  printf("=======================\n");
  printf(" loading reaction config : %s.", reactionConfigFileName.c_str());
  TMacro * kaka = new TMacro();
  if( kaka->ReadFile(reactionConfigFileName.c_str()) > 0 ) {
    reactionConfig  = AnalysisLib::LoadReactionConfig(kaka);
    printf("..... done.\n");
    AnalysisLib::PrintReactionConfig(reactionConfig);
  }else{
    printf("..... fail\n");
  }

  delete haha;
  delete kaka;

}

//************************************** Correction parameters;

std::vector<float> xnCorr;                 //correction of xn to match xf
std::vector<float> xScale;                 // correction of x to be (0,1)
std::vector<std::vector<float>> xfxneCorr; //correction of xn and xf to match e
std::vector<std::vector<float>> eCorr;     // correction to e, ch -> MeV
std::vector<std::vector<float>> rdtCorr;   // correction of rdt, ch -> MeV

//~========================================= xf = xn correction
void LoadXNCorr(bool verbose = false, const char * fileName = "correction_xf_xn.dat"){
  printf(" loading xf-xn correction.");
  xnCorr.clear();
  ifstream file;
  file.open(fileName);
  if( file.is_open() ){
    float a;
    while( file >> a ) xnCorr.push_back(a);
    printf(".......... done.\n");
  }else{
    printf(".......... fail.\n");
  }
  file.close();
  if( verbose ) for(int i = 0; i < (int) xnCorr.size(); i++) printf("%2d | %10.3f\n", i, xnCorr[i]);
}


//~========================================= X-Scale correction
void LoadXScaleCorr(bool verbose = false, const char * fileName = "correction_scaleX.dat"){
  printf(" loading x-Scale correction.");
  xScale.clear();
  ifstream file;
  file.open(fileName);
  if( file.is_open() ){
    float a, b;
    while( file >> a ) xScale.push_back(a);  
    printf("........ done.\n");
  }else{
    printf("........ fail.\n");
  }
  file.close();  
  if( verbose ) for(int i = 0; i < (int) xScale.size(); i++) printf("%2d | %10.3f\n", i, xnCorr[i]);
}

//~========================================= e = xf + xn correction
void LoadXFXN2ECorr(bool verbose = false, const char * fileName = "correction_xfxn_e.dat"){
  printf(" loading xf/xn-e correction.");
  xfxneCorr.clear();
  ifstream file;
  file.open(fileName);
  if( file.is_open() ){
    float a, b;
    while( file >> a >> b) xfxneCorr.push_back({a, b});
    printf("........ done.\n");
  }else{
    printf("........ fail.\n");
  }
  file.close();
  if( verbose ) for(int i = 0; i < (int) xfxneCorr.size(); i++) printf("%2d | %10.3f, %10.3f\n", i, xfxneCorr[i][0], xfxneCorr[i][1]);
} 

//~========================================= e correction
void LoadECorr(bool verbose = false, const char * fileName = "correction_e.dat"){
  printf(" loading e correction.");
  eCorr.clear();
  ifstream file;
  file.open(fileName);
  if( file.is_open() ){
    float a, b;
    while( file >> a >> b) eCorr.push_back( {a, b} );  // 1/a1,  a0 , e' = e * a1 + a0
    printf(".............. done.\n");
  }else{
    printf(".............. fail.\n");
  }
  file.close();
  if( verbose ) for(int i = 0; i < (int) eCorr.size(); i++) printf("%2d | %10.3f, %10.3f\n", i, eCorr[i][0], eCorr[i][1]);
}

//~========================================= rdt correction
void LoadRDTCorr(bool verbose = false, const char * fileName = "correction_rdt.dat"){
  printf(" loading rdt correction.");
  rdtCorr.clear();
  ifstream file;
  file.open(fileName);
  if( file.is_open() ){
    float a, b;
    while( file >> a >> b) rdtCorr.push_back({a, b});
    printf("............ done.\n");
  }else{
    printf("............ fail.\n");
  }
  file.close();
  if( verbose ) for(int i = 0; i < (int) rdtCorr.size(); i++) printf("%2d | %10.3f, %10.3f\n", i, rdtCorr[i][0], rdtCorr[i][1]);
}

double q, alpha, Et, betRel, gamm, G, massB, mass; //variables for Ex calculation
bool hasReactionPara = false;

//~========================================= reaction parameters
void LoadReactionParas(bool verbose = false){
   
  //check is the transfer.root is using the latest reactionConfig.txt   
  //sicne reaction.dat is generated as a by-product of transfer.root
  //TFile * transfer = new TFile("transfer.root");
  //TString aaa1 = "";
  //TString aaa2 = "";
  //if( transfer->IsOpen() ){
  //  TMacro * reactionConfig = (TMacro *) transfer->FindObjectAny("reactionConfig");
  //  TMacro presentReactionConfig ("reactionConfig.txt");
  //  aaa1 = ((TMD5*) reactionConfig->Checksum())->AsString();
  //  aaa2 = ((TMD5*) presentReactionConfig.Checksum())->AsString();
  //}
  //printf("%s\n", aaa1.Data());
  //printf("%s\n", aaa2.Data());

  //if( aaa1 != aaa2 ) {
  //  printf("########################## recalculate transfer.root \n");
  //  system("../Cleopatra/Transfer");
  //  printf("########################## transfer.root updated\n");
  //}
  printf(" loading reaction parameters");
  ifstream file;
  file.open("reaction.dat");
  hasReactionPara = false;
  if( file.is_open() ){
    string x;
    int i = 0;
    while( file >> x ){
      if( x.substr(0,2) == "//" )  continue;
      if( i == 0 ) mass = atof(x.c_str());
      if( i == 1 ) q    = atof(x.c_str());
      if( i == 2 ) betRel = atof(x.c_str()); 
      if( i == 3 ) Et   = atof(x.c_str()); 
      if( i == 4 ) massB = atof(x.c_str()); 
      i = i + 1;
    }
    printf("........ done.\n");

    hasReactionPara = true;
    alpha = 299.792458 * abs(detGeo.Bfield) * q / TMath::TwoPi()/1000.; //MeV/mm
    gamm = 1./TMath::Sqrt(1-betRel*betRel);
    G = alpha * gamm * betRel * detGeo.detPerpDist ;

    if( verbose ){
      printf("\tmass-b    : %f MeV/c2 \n", mass);
      printf("\tcharge-b  : %f \n", q);
      printf("\tE-total   : %f MeV \n", Et);
      printf("\tmass-B    : %f MeV/c2 \n", massB);
      printf("\tbeta      : %f \n", betRel);
      printf("\tB-field   : %f T \n", detGeo.Bfield);
      printf("\tslope     : %f MeV/mm \n", alpha * betRel);
      printf("\tdet radius: %f mm \n", detGeo.detPerpDist);
      printf("\tG-coeff   : %f MeV \n", G);
      printf("=================================\n");
    }

  }else{
    printf("........ fail.\n");
    hasReactionPara = false;
  }
  file.close();
   
}

std::vector<double> CalExTheta(double e, double z){
  if( !hasReactionPara) return {TMath::QuietNaN(), TMath::QuietNaN()};

  double y = e + mass; // to give the KE + mass of proton;
  double Z = alpha * gamm * betRel * z;
  double H = TMath::Sqrt(TMath::Power(gamm * betRel,2) * (y*y - mass * mass) ) ;
 
  if( TMath::Abs(Z) < H ) {
    ///using Newton's method to solve 0 ==	H * sin(phi) - G * tan(phi) - Z = f(phi) 
    double tolerrence = 0.001;
    double phi = 0;  ///initial phi = 0 -> ensure the solution has f'(phi) > 0
    double nPhi = 0; /// new phi

    int iter = 0;
    do{
      phi = nPhi;
      nPhi = phi - (H * TMath::Sin(phi) - G * TMath::Tan(phi) - Z) / (H * TMath::Cos(phi) - G /TMath::Power( TMath::Cos(phi), 2));					 
      iter ++;
      if( iter > 10 || TMath::Abs(nPhi) > TMath::PiOver2()) break;
    }while( TMath::Abs(phi - nPhi ) > tolerrence);
    phi = nPhi;

    /// check f'(phi) > 0
    double Df = H * TMath::Cos(phi) - G / TMath::Power( TMath::Cos(phi),2);
    if( Df > 0 && TMath::Abs(phi) < TMath::PiOver2()  ){
      double K = H * TMath::Sin(phi);
      double x = TMath::ACos( mass / ( y * gamm - K));
      double momt = mass * TMath::Tan(x); /// momentum of particel b or B in CM frame
      double EB = TMath::Sqrt(mass*mass + Et*Et - 2*Et*TMath::Sqrt(momt*momt + mass * mass));
      Ex = EB - massB;

      double hahaha1 = gamm* TMath::Sqrt(mass * mass + momt * momt) - y;
      double hahaha2 = gamm* betRel * momt;
      thetaCM = TMath::ACos(hahaha1/hahaha2) * TMath::RadToDeg();

    }else{
      Ex = TMath::QuietNaN();
      thetaCM = TMath::QuietNaN();
    }
  }else{
    Ex = TMath::QuietNaN();
    thetaCM = TMath::QuietNaN();
  }

  return {Ex, thetaCM};

}

//************************************** TCutG

TObjArray * LoadListOfTCut(TString fileName, TString cutName = "cutList"){

  TObjArray * cutList = nullptr;

  TFile * fCut = new TFile(fileName);
  bool isCutFileOpen = fCut->IsOpen();
  if(!isCutFileOpen) {
    printf( "Failed to open rdt-cutfile 1 : %s\n" , fileName.Data());
  }else{
    cutList = (TObjArray *) fCut->FindObjectAny(cutName);

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

  return cutList;
}

TCutG * LoadSingleTCut( TString fileName, TString cutName = "cutEZ"){
  
  TCutG * cut = nullptr;
  
  TFile * fCut = new TFile(fileName);
  bool isCutFileOpen = fCut->IsOpen(); 
  if( !isCutFileOpen)  {
    printf( "Failed to open E-Z cutfile : %s\n" , fileName.Data());
  }else{
    cut = (TCutG *) fCut->FindObjectAny(cutName);
    if( cut != NULL ) {
        printf("Found EZ cut| name : %s, VarX: %s, VarY: %s, numPoints: %d \n", 
          cut->GetName(),
          cut->GetVarX(),
          cut->GetVarY(),
          cut->GetN()
        );
    }
  }

  return cut;
}


//************************************** Others
std::vector<std::vector<double>> combination(std::vector<double> arr, int r){
  
  std::vector<std::vector<double>> output;
  
  int n = arr.size();
  std::vector<int> v(n);
  std::fill(v.begin(), v.begin()+r, 1);
  do {
    //for( int i = 0; i < n; i++) { printf("%d ", v[i]); }; printf("\n");
    
    std::vector<double> temp;
    for (int i = 0; i < n; ++i) { 
      if (v[i]) {
        //printf("%.1f, ", arr[i]); 
        temp.push_back(arr[i]);
      }
    }
    //printf("\n");
    
    output.push_back(temp);
    
  } while (std::prev_permutation(v.begin(), v.end()));
  
  return output;
}

double* sumMeanVar(std::vector<double> data){
  
  int n = data.size();
  double sum = 0;
  for( int k = 0; k < n; k++) sum += data[k];
  double mean = sum/n;
  double var = 0;
  for( int k = 0; k < n; k++) var += pow(data[k] - mean,2);
  
  static double output[3];
  output[0] = sum;
  output[1] = mean;
  output[2] = var;
  
  return output;
}

double*  fitSlopeIntercept(std::vector<double> dataX, std::vector<double> dataY){
  
  double * smvY = sumMeanVar(dataY);
  double sumY = smvY[0];
  double meanY = smvY[1];

  double * smvX = sumMeanVar(dataX);
  double sumX = smvX[0];
  double meanX = smvX[1];
  double varX = smvX[2];
  
  int n = dataX.size();
  double sumXY = 0;
  for( int j = 0; j < n; j++) sumXY += dataX[j] * dataY[j];

  double slope = ( sumXY - sumX * sumY/n ) / varX;
  double intercept = meanY - slope * meanX;
  
  static double output[2];
  output[0] = slope;
  output[1] = intercept;
  
  return output;
  
}

std::vector<std::vector<double>> FindMatchingPair(std::vector<double> enX, std::vector<double> enY){

   //output[0] = fitEnergy;
   //output[1] = refEnergy;

   int nX = enX.size();
   int nY = enY.size();
   
   std::vector<double> fitEnergy;
   std::vector<double> refEnergy;
   
   if( nX > nY ){
      
      std::vector<std::vector<double>> output = combination(enX, nY);
      
      double * smvY = sumMeanVar(enY);
      double sumY = smvY[0];
      double meanY = smvY[1];
      double varY = smvY[2];
      
      double optRSquared = 0;
      double absRSqMinusOne = 1;
      int maxID = 0;
      
      for( int k = 0; k < (int) output.size(); k++){
        
        double * smvX = sumMeanVar(output[k]);
        double sumX = smvX[0];
        double meanX = smvX[1];
        double varX = smvX[2];
        
        double sumXY = 0;
        for( int j = 0; j < nY; j++) sumXY += output[k][j] * enY[j];
        
        double rSq = abs(sumXY - sumX*sumY/nY)/sqrt(varX*varY);
        
        //for( int j = 0; j < nY ; j++){ printf("%.1f, ", output[k][j]); }; printf("| %.10f\n", rSq);
        
        if( abs(rSq-1) < absRSqMinusOne ) {
          absRSqMinusOne = abs(rSq-1);
          optRSquared = rSq;
          maxID = k;
        }
      }
      
      fitEnergy = output[maxID];
      refEnergy = enY;
      
      printf(" R^2 : %.20f\n", optRSquared);      
      
      //calculation fitting coefficient
      //double * si = fitSlopeIntercept(fitEnergy, refEnergy);
      //printf( " y = %.4f x + %.4f\n", si[0], si[1]);
      
    }else if( nX < nY ){
    
      std::vector<std::vector<double>> output = combination(enY, nX);
      
      
      double * smvX = sumMeanVar(enX);
      double sumX = smvX[0];
      double meanX = smvX[1];
      double varX = smvX[2];
      
      double optRSquared = 0;
      double absRSqMinusOne = 1;
      int maxID = 0;
      
      for( int k = 0; k < (int) output.size(); k++){
        
        double * smvY = sumMeanVar(output[k]);
        double sumY = smvY[0];
        double meanY = smvY[1];
        double varY = smvY[2];
        
        double sumXY = 0;
        for( int j = 0; j < nX; j++) sumXY += output[k][j] * enX[j];
        
        double rSq = abs(sumXY - sumX*sumY/nX)/sqrt(varX*varY);
        
        //for( int j = 0; j < nX ; j++){ printf("%.1f, ", output[k][j]); }; printf("| %.10f\n", rSq);
        
        if( abs(rSq-1) < absRSqMinusOne ) {
          absRSqMinusOne = abs(rSq-1);
          optRSquared = rSq;
          maxID = k;
        }
      }
      
      fitEnergy = enX;
      refEnergy = output[maxID];
      printf(" R^2 : %.20f\n", optRSquared);   
    
    }else{
      fitEnergy = enX;
      refEnergy = enY;
      
      //if nX == nY, ther could be cases that only partial enX and enY are matched. 
      
    }
    
    printf("fitEnergy = ");for( int k = 0; k < (int) fitEnergy.size() ; k++){ printf("%7.2f, ", fitEnergy[k]); }; printf("\n");
    printf("refEnergy = ");for( int k = 0; k < (int) refEnergy.size() ; k++){ printf("%7.2f, ", refEnergy[k]); }; printf("\n");
    
    std::vector<std::vector<double>> haha;
    haha.push_back(fitEnergy);
    haha.push_back(refEnergy);
    
    return haha;
   
}


}

#endif