#ifndef ANALYSIS_LIB_H
#define ANALYSIS_LIB_H

#include <cstdio>
#include <vector>
#include <fstream>
#include <string>

#include <TMacro.h>
#include <TList.h>
#include <TFile.h>
#include <TMath.h>
#include <TObjArray.h>
#include <TCutG.h>

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
};

struct Array{

  double detPerpDist;    /// distance from axis
  double detWidth;       /// width   
  double detLength;      /// length
  double blocker;
  double firstPos;       /// meter
  double eSigma;         /// intrinsic energy resolution MeV
  double zSigma;         /// intrinsic position resolution mm  
  bool  detFaceOut;      ///detector_facing_Out_or_In
  std::vector<double> pos;  /// near position in meter
  int nDet, mDet;      /// nDet = number of different pos, mDet, number of same pos
  std::vector<double> detPos; ///absolute position of detector

  double zMin, zMax;
  
  void DeduceAbsolutePos(){
    nDet = pos.size();
    detPos.clear();
    
    for(int id = 0; id < nDet; id++){
      if( firstPos > 0 ) detPos.push_back(firstPos + pos[id]);
      if( firstPos < 0 ) detPos.push_back(firstPos - pos[nDet - 1 - id]);
      ///printf("%d | %f, %f \n", id, pos[id], detPos[id]);
    }

    zMin = TMath::Min(detPos.front(), detPos.back()) - (firstPos < 0 ? detLength : 0);
    zMax = TMath::Max(detPos.front(), detPos.back()) + (firstPos > 0 ? detLength : 0);
  }

  void PrintArray(){
    for(int i = 0; i < nDet ; i++){
      if( firstPos > 0 ){
        printf("%d, %8.2f mm - %8.2f mm \n", i, detPos[i], detPos[i] + detLength);
      }else{
        printf("%d, %8.2f mm - %8.2f mm \n", i, detPos[i] - detLength , detPos[i]);
      }
    }

    printf("   Blocker Position: %8.2f mm \n", firstPos > 0 ? firstPos - blocker : firstPos + blocker );
    printf("     First Position: %8.2f mm \n", firstPos);
    printf("     number of det : %d x %d \n", mDet, nDet);
    printf("   detector facing : %s\n", detFaceOut ? "Out" : "In");
    printf("      energy resol.: %f MeV\n", eSigma);
    printf("       pos-Z resol.: %f mm \n", zSigma);
  }

};

struct DetGeo{
   
  double Bfield;      /// T
  int BfieldSign ;    /// sign of B-field
  double BfieldTheta; /// rad, 0 = z-axis, pi/2 = y axis, pi = -z axis
  double bore;        /// bore , mm
  
  double recoilPos;      /// recoil, downstream
  double recoilInnerRadius;    /// radius recoil inner
  double recoilOuterRadius;   /// radius recoil outter

  double recoilPos1, recoilPos2; /// imaginary recoils

  double elumPos1, elumPos2; /// imaginary elum, only sensitive to light recoil

  //===================1st array
  Array array1;

  //==================2nd array
  bool   use2ndArray;
  Array array2;

  double zMin, zMax;   /// range of detectors
  bool isCoincidentWithRecoil;

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

  detGeo.array1.pos.clear();
  detGeo.array2.pos.clear();
  detGeo.use2ndArray = false;

  int detFlag = 0;
  int detLine = 0;

  for( int i = 0 ; i < numLine; i++){

    std::vector<std::string> str = SplitStr(macro->GetListOfLines()->At(i)->GetName(), " ");

    //printf("%3d | %s\n", i,  str[0].c_str());

    if( str[0].find("####") != std::string::npos ) break;
    if( str[0].find("#===") != std::string::npos ) {
      detFlag ++;
      detLine = 0;
      continue;;
    }

    if( detFlag == 0 ){
      if ( i == 0 ) {
        detGeo.Bfield = atof(str[0].c_str());
        detGeo.BfieldSign = detGeo.Bfield > 0 ? 1: -1;
      }
      if ( i ==  1 ) detGeo.BfieldTheta       = atof(str[0].c_str());
      if ( i ==  2 ) detGeo.bore              = atof(str[0].c_str());
      if ( i ==  3 ) detGeo.recoilPos         = atof(str[0].c_str());
      if ( i ==  4 ) detGeo.recoilInnerRadius = atof(str[0].c_str());
      if ( i ==  5 ) detGeo.recoilOuterRadius = atof(str[0].c_str());
      if ( i ==  6 ) detGeo.isCoincidentWithRecoil = str[0] == "false" ? false: true;
      if ( i ==  7 ) detGeo.recoilPos1        = atof(str[0].c_str());
      if ( i ==  8 ) detGeo.recoilPos2        = atof(str[0].c_str());
      if ( i ==  9 ) detGeo.elumPos1          = atof(str[0].c_str());
      if ( i == 10 ) detGeo.elumPos2          = atof(str[0].c_str());
    }

    if( detFlag == 1){
      if ( detLine == 0 ) detGeo.array1.detPerpDist       = atof(str[0].c_str());
      if ( detLine == 1 ) detGeo.array1.detWidth          = atof(str[0].c_str());
      if ( detLine == 2 ) detGeo.array1.detLength         = atof(str[0].c_str());
      if ( detLine == 3 ) detGeo.array1.blocker           = atof(str[0].c_str());
      if ( detLine == 4 ) detGeo.array1.firstPos          = atof(str[0].c_str());
      if ( detLine == 5 ) detGeo.array1.eSigma            = atof(str[0].c_str());
      if ( detLine == 6 ) detGeo.array1.zSigma            = atof(str[0].c_str());
      if ( detLine == 7 ) detGeo.array1.detFaceOut        = str[0] == "Out" ? true : false;
      if ( detLine == 8 ) detGeo.array1.mDet              = atoi(str[0].c_str());
      if ( detLine >= 9 ) (detGeo.array1.pos).push_back(atof(str[0].c_str()));
      detLine ++;
    }

    if( detFlag == 2){
      if ( detLine ==  0 ) detGeo.use2ndArray    = str[0] == "true" ? true : false;
      if ( detLine ==  1 ) detGeo.array2.detPerpDist       = atof(str[0].c_str());
      if ( detLine ==  2 ) detGeo.array2.detWidth          = atof(str[0].c_str());
      if ( detLine ==  3 ) detGeo.array2.detLength         = atof(str[0].c_str());
      if ( detLine ==  4 ) detGeo.array2.blocker           = atof(str[0].c_str());
      if ( detLine ==  5 ) detGeo.array2.firstPos          = atof(str[0].c_str());
      if ( detLine ==  6 ) detGeo.array2.eSigma            = atof(str[0].c_str());
      if ( detLine ==  7 ) detGeo.array2.zSigma            = atof(str[0].c_str());
      if ( detLine ==  8 ) detGeo.array2.detFaceOut        = str[0] == "Out" ? true : false;
      if ( detLine ==  9 ) detGeo.array2.mDet              = atoi(str[0].c_str());
      if ( detLine >= 10 ) (detGeo.array2.pos).push_back(atof(str[0].c_str()));
      detLine ++;
    }

  }

  detGeo.array1.DeduceAbsolutePos();

  detGeo.zMin = detGeo.array1.zMin;
  detGeo.zMax = detGeo.array1.zMax;

  if( detGeo.use2ndArray) {
    detGeo.array2.DeduceAbsolutePos();

    detGeo.zMin = TMath::Min(detGeo.array1.zMin, detGeo.array2.zMin);
    detGeo.zMax = TMath::Min(detGeo.array1.zMax, detGeo.array2.zMax);
  }
  
  return detGeo;
}

void PrintDetGeo(DetGeo detGeo, bool printAll = true){

  printf("=====================================================\n");
  printf("                 B-field: %8.2f  T, Theta : %6.2f deg \n", detGeo.Bfield, detGeo.BfieldTheta);
  if( detGeo.BfieldTheta != 0.0 ) {
    printf("                                      +---- field angle != 0 is not supported!!! \n");
  }
  printf("     Recoil detector pos: %8.2f mm, radius: %6.2f - %6.2f mm \n", detGeo.recoilPos, detGeo.recoilInnerRadius, detGeo.recoilOuterRadius);
  if( printAll ){
    printf("------------------------------------- Detector Position \n");
    detGeo.array1.PrintArray();

    if( detGeo.use2ndArray){
      printf("--------------------------------- 2nd Detector Position \n");
      detGeo.array2.PrintArray();
    }
  }else{
    if( detGeo.use2ndArray){
      printf("--------------------------------- 2nd Detector Position \n");
      detGeo.array2.PrintArray();
    }else{
      printf("------------------------------------- Detector Position \n");
      detGeo.array1.PrintArray();
    }
  }

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
ReactionConfig reactionConfig1;
ReactionConfig reactionConfig2;

void LoadDetGeoAndReactionConfigFile(std::string detGeoFileName = "detectorGeo.txt", 
                                     std::string reactionConfigFileName1 = "reactionConfig1.txt", 
                                     std::string reactionConfigFileName2 = "reactionConfig2.txt"){
  printf("=====================================================\n");
  printf(" loading detector geometery : %s.", detGeoFileName.c_str());
  TMacro * haha = new TMacro();
  if( haha->ReadFile(detGeoFileName.c_str()) > 0 ) {
    detGeo = AnalysisLib::LoadDetectorGeo(haha);    
    printf("... done.\n");
    AnalysisLib::PrintDetGeo(detGeo);
  }else{
    printf("... fail\n");
  }
  delete haha;

  printf("=====================================================\n");
  printf(" loading reaction1 config : %s.", reactionConfigFileName1.c_str());
  TMacro * kaka = new TMacro();
  if( kaka->ReadFile(reactionConfigFileName1.c_str()) > 0 ) {
    reactionConfig1  = AnalysisLib::LoadReactionConfig(kaka);
    printf("..... done.\n");
    AnalysisLib::PrintReactionConfig(reactionConfig1);
  }else{
    printf("..... fail\n");
  }
  delete kaka;

  if( detGeo.use2ndArray){
    printf("=====================================================\n");
    printf(" loading reaction2 config : %s.", reactionConfigFileName2.c_str());
    TMacro * jaja = new TMacro();
    if( jaja->ReadFile(reactionConfigFileName2.c_str()) > 0 ) {
      reactionConfig2  = AnalysisLib::LoadReactionConfig(kaka);
      printf("..... done.\n");
      AnalysisLib::PrintReactionConfig(reactionConfig2);
    }else{
      printf("..... fail\n");
    }
    delete jaja;
  }

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
  std::ifstream file;
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
  std::ifstream file;
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
  std::ifstream file;
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
  std::ifstream file;
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
  std::ifstream file;
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


struct ReactionParas{

  double Et; // total energy in CM frame
  double beta;  // Lorentz beta from Lab to CM
  double gamma; // Lorentz gamma from Lab to CM
  double alpha; // E-Z slope / beta
  double G;    //The G-coefficient....
  double massB; // heavy mass
  double q;  // charge of light particle
  double mass; //light mass
  bool hasReactionPara;

  double detPrepDist; 

};

ReactionParas reactParas1;
ReactionParas reactParas2;

//~========================================= reaction parameters
void LoadReactionParas(int arrayID, bool verbose = false){
   
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

  ReactionParas * reactParas = nullptr;

  std::string fileName;

  if( arrayID == 1){
    reactParas = &AnalysisLib::reactParas1;
    fileName = "reaction.dat";
  }else if( arrayID == 2){
    reactParas = &AnalysisLib::reactParas2;
    fileName = "reaction2.dat";
  }else{
    printf("arrayID must be either 1 or 2. Abort.\n");
    return;
  }
  reactParas->detPrepDist = AnalysisLib::detGeo.array1.detPerpDist;

  printf(" loading reaction parameters");
  std::ifstream file;
  file.open(fileName.c_str());
  reactParas->hasReactionPara = false;
  if( file.is_open() ){
    std::string x;
    int i = 0;
    while( file >> x ){
      if( x.substr(0,2) == "//" )  continue;
      if( i == 0 ) reactParas->mass   = atof(x.c_str());
      if( i == 1 ) reactParas->q      = atof(x.c_str());
      if( i == 2 ) reactParas->beta = atof(x.c_str()); 
      if( i == 3 ) reactParas->Et     = atof(x.c_str()); 
      if( i == 4 ) reactParas->massB  = atof(x.c_str()); 
      i = i + 1;
    }
    printf("........ done.\n");

    reactParas->hasReactionPara = true;
    reactParas->alpha = 299.792458 * abs(detGeo.Bfield) * reactParas->q / TMath::TwoPi()/1000.; //MeV/mm
    reactParas->gamma = 1./TMath::Sqrt(1-reactParas->beta * reactParas->beta);
    reactParas->G = reactParas->alpha * reactParas->gamma * reactParas->beta * reactParas->detPrepDist ;

    if( verbose ){
      printf("\tmass-b    : %f MeV/c2 \n", reactParas->mass);
      printf("\tcharge-b  : %f \n", reactParas->q);
      printf("\tE-total   : %f MeV \n", reactParas->Et);
      printf("\tmass-B    : %f MeV/c2 \n", reactParas->massB);
      printf("\tbeta      : %f \n", reactParas->beta);
      printf("\tB-field   : %f T \n", detGeo.Bfield);
      printf("\tslope     : %f MeV/mm \n", reactParas->alpha * reactParas->beta);
      printf("\tdet radius: %f mm \n", reactParas->detPrepDist);
      printf("\tG-coeff   : %f MeV \n", reactParas->G);
      printf("=====================================================\n");
    }

  }else{
    printf("........ fail.\n");
  }
  file.close();
   
}

std::vector<double> CalExTheta(double e, double z){

  ReactionParas * reactParas = nullptr;

  if( detGeo.array1.zMin <= z && z <= detGeo.array1.zMax ){
    reactParas = &reactParas1;
    if( !reactParas->hasReactionPara) return {TMath::QuietNaN(), TMath::QuietNaN()};
  }
  
  if( detGeo.array2.zMin <= z && z <= detGeo.array2.zMax ){
    reactParas = &reactParas2;
    if( !reactParas->hasReactionPara) return {TMath::QuietNaN(), TMath::QuietNaN()};
  }

  double Ex = TMath::QuietNaN();
  double thetaCM = TMath::QuietNaN();

  double y = e + reactParas->mass; // to give the KE + mass of proton;
  double Z = reactParas->alpha * reactParas->gamma * reactParas->beta * z;
  double H = TMath::Sqrt(TMath::Power(reactParas->gamma * reactParas->beta,2) * (y*y - reactParas->mass * reactParas->mass) ) ;
 
  if( TMath::Abs(Z) < H ) {
    ///using Newton's method to solve 0 ==	H * sin(phi) - G * tan(phi) - Z = f(phi) 
    double tolerrence = 0.001;
    double phi = 0;  ///initial phi = 0 -> ensure the solution has f'(phi) > 0
    double nPhi = 0; /// new phi

    int iter = 0;
    do{
      phi = nPhi;
      nPhi = phi - (H * TMath::Sin(phi) - reactParas->G * TMath::Tan(phi) - Z) / (H * TMath::Cos(phi) - reactParas->G /TMath::Power( TMath::Cos(phi), 2));					 
      iter ++;
      if( iter > 10 || TMath::Abs(nPhi) > TMath::PiOver2()) break;
    }while( TMath::Abs(phi - nPhi ) > tolerrence);
    phi = nPhi;

    /// check f'(phi) > 0
    double Df = H * TMath::Cos(phi) - reactParas->G / TMath::Power( TMath::Cos(phi),2);
    if( Df > 0 && TMath::Abs(phi) < TMath::PiOver2()  ){
      double K = H * TMath::Sin(phi);
      double x = TMath::ACos( reactParas->mass / ( y * reactParas->gamma - K));
      double momt = reactParas->mass * TMath::Tan(x); /// momentum of particel b or B in CM frame
      double EB = TMath::Sqrt(reactParas->mass * reactParas->mass + reactParas->Et * reactParas->Et - 2 * reactParas->Et * TMath::Sqrt(momt*momt + reactParas->mass * reactParas->mass));
      Ex = EB - reactParas->massB;

      double hahaha1 = reactParas->gamma * TMath::Sqrt(reactParas->mass * reactParas->mass + momt * momt) - y;
      double hahaha2 = reactParas->gamma * reactParas->beta * momt;
      thetaCM = TMath::ACos(hahaha1/hahaha2) * TMath::RadToDeg();
    
    }
  }
  return {Ex, thetaCM};

}

//************************************** TCutG

TObjArray * LoadListOfTCut(TString fileName, TString cutName = "cutList"){

  if( fileName == "" ) return nullptr;

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
  
  if( fileName == "" ) return nullptr;
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