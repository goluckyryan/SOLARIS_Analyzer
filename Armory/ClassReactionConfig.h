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

struct EnergyLevel{

  float Ex, xsec, SF, sigma;

  EnergyLevel(float Ex, float xsec, float SF, float sigma) {
    this->Ex = Ex;
    this->xsec = xsec;
    this->SF = SF;
    this->sigma = sigma;
  }

  void Print(std::string str) const {
    printf("%11.3f  %8.1f  %5.1f  %5.3f%s", Ex, xsec, SF, sigma, str.c_str() );
  }

};


struct ExcitedEnergies {

  std::vector<EnergyLevel> ExList;

  void Clear(){
    ExList.clear();
  }

  void Add(float Ex, float xsec, float SF, float sigma){
    ExList.push_back( EnergyLevel(Ex, xsec, SF, sigma));
  }

  void Print() const {
    printf("Energy[MeV]  Rel.Xsec     SF  sigma\n");
    for( size_t i = 0; i < ExList.size(); i++){
      ExList[i].Print("\n");
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
  float beamTheta;          ///beam-angle_in_mrad
  float beamThetaSigma;     ///beam-emittance_in_mrad
  float beamX;              ///x_offset_of_Beam_in_mm
  float beamY;              ///y_offset_of_Beam_in_mm

  unsigned short targetA, targetZ;     
  bool isTargetScattering;  ///isTargetScattering
  float targetDensity;      ///target_density_in_g/cm3
  float targetThickness;    ///targetThickness_in_cm
  std::string beamStoppingPowerFile;         ///stopping_power_for_beam

  std::vector<Recoil> recoil;
  std::vector<ExcitedEnergies> exList;

  int numEvents;            ///number_of_Event_being_generated
  bool isRedo;         ///isReDo

  void SetReactionSimple(int beamA, int beamZ,
                         int targetA, int targetZ,
                         int recoilA, int recoilZ, 
                         float beamEnergy_AMeV);

  bool LoadReactionConfig(TString fileName);
  bool LoadReactionConfig(TMacro * macro);

  void Print(int ID = -1, bool withEx = true) const;
  
private:

};

inline void ReactionConfig::SetReactionSimple(int beamA, int beamZ,
                                              int targetA, int targetZ,
                                              int recoilA, int recoilZ, 
                                              float beamEnergy_AMeV){

  this->beamA = beamA;
  this->beamZ = beamZ;
  this->targetA = targetA;
  this->targetZ = targetZ;

  this->recoil.push_back(Recoil());

  this->recoil.back().lightA = recoilA;
  this->recoil.back().lightZ = recoilZ;
  recoil.back().heavyA = this->beamA + this->targetA - recoil.back().lightA;
  recoil.back().heavyZ = this->beamZ + this->targetZ - recoil.back().lightZ;

  beamEnergy = beamEnergy_AMeV;
  beamEnergySigma = 0;
  beamTheta = 0;    
  beamThetaSigma = 0;   
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
    if( str[0].find("#---") != std::string::npos ) continue;
    if( str[0].find("#===") != std::string::npos ) {
      recoilFlag ++;
      recoilLine = 0;
      recoil.push_back(Recoil());
      exList.push_back(ExcitedEnergies());
      continue;
    }

    if( recoilFlag == 0 ){
      if( recoilLine == 0 ) beamA           = atoi(str[0].c_str());
      if( recoilLine == 1 ) beamZ           = atoi(str[0].c_str());
      if( recoilLine == 2 ) beamEx          = atoi(str[0].c_str());

      if( recoilLine == 3 ) beamEnergy      = atof(str[0].c_str());
      if( recoilLine == 4 ) beamEnergySigma = atof(str[0].c_str());
      if( recoilLine == 5 ) beamTheta       = atof(str[0].c_str());
      if( recoilLine == 6 ) beamThetaSigma  = atof(str[0].c_str());
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

      if( recoilLine > 6 && str.size() == 4) {
        if( str[0].find('#') != std::string::npos) continue;
        if( str[0] == "IAEA"){
                    
          recoil[ID].heavyA = beamA + targetA - recoil[ID].lightA;
          recoil[ID].heavyZ = beamZ + targetZ - recoil[ID].lightZ;
          printf(">>>>>>>>>>>>> Retrieving Ex data from IAEA website....\n");
          std::string scriptPath = "../WebSimHelper/getEx.py " + std::to_string(recoil[ID].heavyA) + " " + std::to_string(recoil[ID].heavyZ) + " " + str[2];
          std::vector<std::string> output = AnalysisLib::executePythonScript(scriptPath);

          if( output.size() > 1 ){
            for( size_t dudu = 1 ; dudu < output.size(); dudu ++ ){
              printf("%s", output[dudu].c_str());
              std::vector<std::string> dondon = AnalysisLib::SplitStr(output[dudu], " ");

              if( str[1].find("all") == std::string::npos){ // only comfirm states
                if(dondon[2].find(')') != std::string::npos  ) continue;
                if(dondon[2].find('N') != std::string::npos  ) continue;
                // printf("kdlsakdas ---- %s\n", str[1].c_str());

                if(str[1] == "+" && dondon[2].find('+') != std::string::npos  ){
                  // printf(" only comfim + states\n");
                  exList[ID].Add( atoi(dondon[1].c_str()), 1.0, 1.0, atoi(str[3].c_str()));
                }
                if(str[1] == "-" && dondon[2].find('-') != std::string::npos  ){
                  // printf(" only comfim - states\n");
                  exList[ID].Add( atoi(dondon[1].c_str()), 1.0, 1.0, atoi(str[3].c_str()));
                }
                if( str[1] == "known"  ){
                  // printf(" All comfim state\n");
                  exList[ID].Add( atoi(dondon[1].c_str()), 1.0, 1.0, atoi(str[3].c_str()));
                }

              }else{
                if(str[1] == "+all" && dondon[2].find('+') != std::string::npos  ){
                  // printf(" All state : %s\n", str[1].c_str());
                  exList[ID].Add( atoi(dondon[1].c_str()), 1.0, 1.0, atoi(str[3].c_str()));
                }
                if(str[1] == "-all" && dondon[2].find('-') != std::string::npos  ){
                  // printf(" All state : %s\n", str[1].c_str());
                  exList[ID].Add( atoi(dondon[1].c_str()), 1.0, 1.0, atoi(str[3].c_str()));
                }
                if( str[1] == "all" ){
                  // printf(" All state \n");
                  exList[ID].Add( atoi(dondon[1].c_str()), 1.0, 1.0, atoi(str[3].c_str()));
                }

              }
            }
          }else{
            printf(" No states found from IAEA database, assume ground state.");
            exList[ID].Add( 0, 1.0, 1.0, 0.01);
          }
        }else{
          exList[ID].Add( atoi(str[0].c_str()), atoi(str[1].c_str()), atoi(str[2].c_str()), atoi(str[3].c_str()));
        }
      }

    }

    recoilLine ++;
    
  }

  for( size_t i = 0; i < recoil.size(); i++){
    recoil[i].heavyA = beamA + targetA - recoil[i].lightA;
    recoil[i].heavyZ = beamZ + targetZ - recoil[i].lightZ;
  }
  return true;
}

inline void ReactionConfig::Print(int ID, bool withEx) const{

  printf("#####################################################\n");

  printf("number of Simulation Events : %d \n", numEvents);
  printf("    is Redo until hit array : %s \n", isRedo ? "Yes" : "No");
  
  printf("================================= Beam\n");
  printf("   beam : A = %3d, Z = %2d, Ex = %.2f MeV\n", beamA, beamZ, beamEx);
  printf(" beam Energy : %.2f +- %.2f MeV/u, dE/E = %5.2f %%\n", beamEnergy, beamEnergySigma, beamEnergySigma/beamEnergy);
  printf("       Angle : %.2f +- %.2f mrad\n", beamTheta, beamThetaSigma);
  printf("      offset : (x,y) = (%.2f, %.2f) mmm \n", beamX, beamY);

  printf("================================= Target\n");
  printf("            target : A = %3d, Z = %2d \n", targetA, targetZ);
  printf(" enable scattering : %s \n", isTargetScattering ? "Yes" : "No");
  if(isTargetScattering){
    printf(" target density : %.f g/cm3\n", targetDensity);
    printf("      thickness : %.f cm\n", targetThickness);
    printf("         beam stopping file : %s \n", beamStoppingPowerFile.c_str());
  }
  
  printf("================================= Number of recoil reactions : %zu\n", recoil.size());
  for( int i = 0; i < (int)recoil.size(); i ++ ){
    if( ID == i || ID < 0  ){
      printf("------------------------------------------ Recoil-%d\n", i); 
      recoil[i].Print();
      if( withEx ) exList[i].Print();
    }
  }
  
  
  printf("#####################################################\n");
}

#endif