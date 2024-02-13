#ifndef ClassReactionConfig_H
#define ClassReactionConfig_H

#include <stdio.h> /// for FILE
#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>

#include "TMath.h"
#include "TString.h"
#include "TMacro.h"

#include "AnalysisLib.h"

class ReactionConfig {

public:
  ReactionConfig(){}
  ~ReactionConfig(){}

  int beamA, beamZ;       
  int targetA, targetZ;     
  int recoilLightA, recoilLightZ;
  int recoilHeavyA, recoilHeavyZ;

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

  void SetReactionSimple(int beamA, int beamZ,
                   int targetA, int targetZ,
                   int recoilA, int recoilZ, float beamEnergy_AMeV);

  bool LoadReactionConfig(TString fileName);
  bool LoadReactionConfig(TMacro * macro);

  void PrintReactionConfig() const;
  
private:

};

inline void ReactionConfig::SetReactionSimple(int beamA, int beamZ,
                   int targetA, int targetZ,
                   int recoilA, int recoilZ, float beamEnergy_AMeV){

  this->beamA = beamA;
  this->beamZ = beamZ;
  this->targetA = targetA;
  this->targetZ = targetZ;
  this->recoilLightA = recoilA;
  this->recoilLightZ = recoilZ;
  recoilHeavyA = this->beamA + this->targetA - recoilLightA;
  recoilHeavyZ = this->beamZ + this->targetZ - recoilLightZ;

  beamEnergy = beamEnergy_AMeV;
  beamEnergySigma = 0;
  beamAngle = 0;    
  beamAngleSigma = 0;   
  beamX = 0;            
  beamY = 0;            

}

inline bool ReactionConfig::LoadReactionConfig(TString fileName){
  TMacro * haha = new TMacro();
  if( haha->ReadFile(fileName) > 0 ) {
    if( LoadReactionConfig(haha) ){
      return true;
    }else{
      return false;
    }
  }else{
    return false;
  }
}

inline bool ReactionConfig::LoadReactionConfig(TMacro * macro){

  if( macro == NULL ) return false;

  int numLine = macro->GetListOfLines()->GetSize();

  for( int i = 0; i < numLine; i ++){

    std::vector<std::string> str =AnalysisLib::SplitStr(macro->GetListOfLines()->At(i)->GetName(), " ");

    ///printf("%d | %s\n", i,  str[0].c_str());

    if( str[0].find_first_of("#") == 0 ) break;

    if( i == 0 ) beamA           = atoi(str[0].c_str());
    if( i == 1 ) beamZ           = atoi(str[0].c_str());
    if( i == 2 ) targetA         = atoi(str[0].c_str());
    if( i == 3 ) targetZ         = atoi(str[0].c_str());
    if( i == 4 ) recoilLightA    = atoi(str[0].c_str());
    if( i == 5 ) recoilLightZ    = atoi(str[0].c_str());
    if( i == 6 ) beamEnergy      = atof(str[0].c_str());
    if( i == 7 ) beamEnergySigma = atof(str[0].c_str());
    if( i == 8 ) beamAngle       = atof(str[0].c_str());
    if( i == 9 ) beamAngleSigma  = atof(str[0].c_str());
    if( i == 10 ) beamX = atof(str[0].c_str());
    if( i == 11 ) beamY = atof(str[0].c_str());
    if( i == 12 ) numEvents = atoi(str[0].c_str());
    if( i == 13 ) {
      if( str[0].compare("false") == 0 ) isTargetScattering = false;
      if( str[0].compare("true")  == 0 ) isTargetScattering = true;
    }
    if( i == 14 ) targetDensity   = atof(str[0].c_str());
    if( i == 15 ) targetThickness = atof(str[0].c_str());
    if( i == 16 ) beamStoppingPowerFile = str[0];
    if( i == 17 ) recoilLightStoppingPowerFile = str[0];
    if( i == 18 ) recoilHeavyStoppingPowerFile = str[0];
    if( i == 19 ) {
      if( str[0].compare("false") == 0 ) isDecay = false;
      if( str[0].compare("true")  == 0 ) isDecay = true;
    }
    if( i == 20 ) heavyDecayA = atoi(str[0].c_str());
    if( i == 21 ) heavyDecayZ = atoi(str[0].c_str());
    if( i == 22 ) {
      if( str[0].compare("false") == 0 ) isRedo = false;
      if( str[0].compare("true" ) == 0 ) isRedo = true;
    }
    
    if( i >= 23) {
      beamEx.push_back( atof(str[0].c_str()) );
    }
  }

  recoilHeavyA = beamA + targetA - recoilLightA;
  recoilHeavyZ = beamZ + targetZ - recoilLightZ;

  return true;
}

inline void ReactionConfig::PrintReactionConfig() const{

  printf("=====================================================\n");
  printf("   beam : A = %3d, Z = %2d \n", beamA, beamZ);
  printf(" target : A = %3d, Z = %2d \n", targetA, targetZ);
  printf("  light : A = %3d, Z = %2d \n", recoilLightA, recoilLightZ);

  printf(" beam Energy : %.2f +- %.2f MeV/u, dE/E = %5.2f %%\n", beamEnergy, beamEnergySigma, beamEnergySigma/beamEnergy);
  printf("       Angle : %.2f +- %.2f mrad\n", beamAngle, beamAngleSigma);
  printf("      offset : (x,y) = (%.2f, %.2f) mmm \n", beamX, beamY);

  printf("##### number of Simulation Events : %d \n", numEvents);
  
  printf("    is target scattering : %s \n", isTargetScattering ? "Yes" : "No");

  if(isTargetScattering){
    printf(" target density : %.f g/cm3\n", targetDensity);
    printf("      thickness : %.f cm\n", targetThickness);
    printf("         beam stopping file : %s \n", beamStoppingPowerFile.c_str());
    printf(" recoil light stopping file : %s \n", recoilLightStoppingPowerFile.c_str());
    printf(" recoil heavy stopping file : %s \n", recoilHeavyStoppingPowerFile.c_str());
  }
  
  printf("       is simulate decay : %s \n", isDecay ? "Yes" : "No");
  if( isDecay ){
    printf(" heavy decay : A = %d, Z = %d \n", heavyDecayA, heavyDecayZ);
  }
  printf(" is Redo until hit array : %s \n", isRedo ? "Yes" : "No");

  printf(" beam Ex : %.2f MeV \n", beamEx[0]);
  for( int i = 1; i < (int) beamEx.size(); i++){
    printf("         %.2f MeV \n", beamEx[i]);
  }
  
  printf("=====================================================\n");
}

#endif