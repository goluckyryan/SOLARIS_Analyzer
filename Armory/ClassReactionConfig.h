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

struct Recoil {

  unsigned short lightA, lightZ; 
  unsigned short heavyA, heavyZ; 

  std::string lightStoppingPowerFile = "";  ///stopping_power_for_light_recoil
  std::string heavyStoppingPowerFile = "";  ///stopping_power_for_heavy_recoil

  bool isDecay = false;        ///isDacay
  unsigned short decayA = 0;     ///decayNucleus_A
  unsigned short decayZ = 0;     ///decayNucleus_Z

  void Print() const {
    printf("  light : A = %3d, Z = %2d \n", lightA, lightZ);
    printf("  heavy : A = %3d, Z = %2d \n", heavyA, heavyZ);

    printf(" light stopping file : %s \n", lightStoppingPowerFile.c_str());
    printf(" heavy stopping file : %s \n", heavyStoppingPowerFile.c_str());
    printf("   is simulate decay : %s \n", isDecay ? "Yes" : "No");
    if( isDecay ){
      printf("         heavy decay : A = %d, Z = %d \n", decayA, decayZ);
    }
  }

};


class ReactionConfig {

public:
  ReactionConfig(){}
  ReactionConfig(TString configTxt){ LoadReactionConfig(configTxt);}
  ReactionConfig(TMacro * macro){ LoadReactionConfig(macro);}
  ~ReactionConfig(){}

  unsigned short beamA, beamZ;       
  float beamEx;             ///excitation_energy_of_A[MeV]
  float beamEnergy;         ///MeV/u
  float beamEnergySigma;    ///beam-energy_sigma_in_MeV/u
  float beamAngle;          ///beam-angle_in_mrad
  float beamAngleSigma;     ///beam-emittance_in_mrad
  float beamX;              ///x_offset_of_Beam_in_mm
  float beamY;              ///y_offset_of_Beam_in_mm

  unsigned short targetA, targetZ;     
  bool isTargetScattering;  ///isTargetScattering
  float targetDensity;      ///target_density_in_g/cm3
  float targetThickness;    ///targetThickness_in_cm
  std::string beamStoppingPowerFile;         ///stopping_power_for_beam

  Recoil recoil[2];

  int numEvents;            ///number_of_Event_being_generated
  bool isRedo;         ///isReDo

  void SetReactionSimple(int beamA, int beamZ,
                   int targetA, int targetZ,
                   int recoilA, int recoilZ, float beamEnergy_AMeV, unsigned short ID);

  bool LoadReactionConfig(TString fileName);
  bool LoadReactionConfig(TMacro * macro);

  void Print() const;
  
private:

};

inline void ReactionConfig::SetReactionSimple(int beamA, int beamZ,
                   int targetA, int targetZ,
                   int recoilA, int recoilZ, float beamEnergy_AMeV, unsigned short ID){

  this->beamA = beamA;
  this->beamZ = beamZ;
  this->targetA = targetA;
  this->targetZ = targetZ;

  this->recoil[ID].lightA = recoilA;
  this->recoil[ID].lightZ = recoilZ;
  recoil[ID].heavyA = this->beamA + this->targetA - recoil[ID].lightA;
  recoil[ID].heavyZ = this->beamZ + this->targetZ - recoil[ID].lightZ;

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
      delete haha;
      return true;
    }else{
      delete haha;
      return false;
    }
  }else{
    delete haha;
    return false;
  }
}

inline bool ReactionConfig::LoadReactionConfig(TMacro * macro){

  if( macro == NULL ) return false;

  int recoilFlag = 0;
  int recoilLine = 0;

  int numLine = macro->GetListOfLines()->GetSize();

  for( int i = 0; i < numLine; i ++){

    std::string line = macro->GetListOfLines()->At(i)->GetName();
    if( AnalysisLib::isEmptyOrSpaces(line) ) continue;

    std::vector<std::string> str =AnalysisLib::SplitStr(line, " ");
    // printf("%d |%s|%d|%d\n", i,  str[0].c_str(), recoilFlag, recoilLine);

    if( str[0].find("####") != std::string::npos ) break;
    if( str[0].find("#===") != std::string::npos ) {
      recoilFlag ++;
      recoilLine = 0;
      continue;
    }

    if( recoilFlag == 0 ){
      if( recoilLine == 0 ) beamA           = atoi(str[0].c_str());
      if( recoilLine == 1 ) beamZ           = atoi(str[0].c_str());
      if( recoilLine == 2 ) beamEx          = atoi(str[0].c_str());

      if( recoilLine == 3 ) beamEnergy      = atof(str[0].c_str());
      if( recoilLine == 4 ) beamEnergySigma = atof(str[0].c_str());
      if( recoilLine == 5 ) beamAngle       = atof(str[0].c_str());
      if( recoilLine == 6 ) beamAngleSigma  = atof(str[0].c_str());
      if( recoilLine == 7 ) beamX = atof(str[0].c_str());
      if( recoilLine == 8 ) beamY = atof(str[0].c_str());

      if( recoilLine ==  9 ) targetA         = atoi(str[0].c_str());
      if( recoilLine == 10 ) targetZ         = atoi(str[0].c_str());

      if( recoilLine == 11 ) isTargetScattering    = str[0].compare("true")  == 0 ? true: false;
      if( recoilLine == 12 ) targetDensity         = atof(str[0].c_str());
      if( recoilLine == 13 ) targetThickness       = atof(str[0].c_str());
      if( recoilLine == 14 ) beamStoppingPowerFile = str[0];

      if( recoilLine == 15 ) numEvents = atoi(str[0].c_str());
      if( recoilLine == 16 ) isRedo    = str[0].compare("true" ) == 0 ? true : false;
    }

    if( recoilFlag > 0 ){

      unsigned ID = recoilFlag - 1;
      if( recoilLine == 0 ) recoil[ID].lightA  = atoi(str[0].c_str());
      if( recoilLine == 1 ) recoil[ID].lightZ  = atoi(str[0].c_str());
      if( recoilLine == 2 ) recoil[ID].lightStoppingPowerFile = str[0];
      if( recoilLine == 3 ) recoil[ID].heavyStoppingPowerFile = str[0];
      if( recoilLine == 4 ) recoil[ID].isDecay = str[0].compare("true")  == 0 ? true : false;
      if( recoilLine == 5 ) recoil[ID].decayA  = atoi(str[0].c_str());
      if( recoilLine == 6 ) recoil[ID].decayZ  = atoi(str[0].c_str());

    }

    recoilLine ++;
    
  }

  for( int i = 0; i < 2; i++){
    recoil[i].heavyA = beamA + targetA - recoil[i].lightA;
    recoil[i].heavyZ = beamZ + targetZ - recoil[i].lightZ;
  }
  return true;
}

inline void ReactionConfig::Print() const{

  printf("=====================================================\n");

  printf("number of Simulation Events : %d \n", numEvents);
  printf("    is Redo until hit array : %s \n", isRedo ? "Yes" : "No");
  
  printf("------------------------------ Beam\n");
  printf("   beam : A = %3d, Z = %2d, Ex = %.2f MeV\n", beamA, beamZ, beamEx);
  printf(" beam Energy : %.2f +- %.2f MeV/u, dE/E = %5.2f %%\n", beamEnergy, beamEnergySigma, beamEnergySigma/beamEnergy);
  printf("       Angle : %.2f +- %.2f mrad\n", beamAngle, beamAngleSigma);
  printf("      offset : (x,y) = (%.2f, %.2f) mmm \n", beamX, beamY);

  printf("------------------------------ Target\n");
  printf(" target : A = %3d, Z = %2d \n", targetA, targetZ);
  printf("    is target scattering : %s \n", isTargetScattering ? "Yes" : "No");
  if(isTargetScattering){
    printf(" target density : %.f g/cm3\n", targetDensity);
    printf("      thickness : %.f cm\n", targetThickness);
    printf("         beam stopping file : %s \n", beamStoppingPowerFile.c_str());
  }
  
  for( int i = 0; i < 2; i ++ ){
    printf("------------------------------ Recoil-%d\n", i); recoil[i].Print();
  }
  
  
  printf("=====================================================\n");
}

#endif