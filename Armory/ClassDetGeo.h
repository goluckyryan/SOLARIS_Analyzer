#ifndef ClassDetGeo_H
#define ClassDetGeo_H

#include <stdio.h> /// for FILE
#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>

#include "TMath.h"
#include "TString.h"
#include "TMacro.h"

#include "AnalysisLib.h"

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

  void PrintArray() const{
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

class DetGeo {

public:
  DetGeo();
  ~DetGeo();

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

  bool LoadDetectorGeo(TString fileName, bool verbose = true);
  bool LoadDetectorGeo(TMacro * macro, bool verbose = true);

  void PrintDetGeo( bool printAll = true) const;

private:

};

inline DetGeo::DetGeo(){

}

inline DetGeo::~DetGeo(){

}

inline bool DetGeo::LoadDetectorGeo(TString fileName, bool verbose){

  TMacro * haha = new TMacro();
  if( haha->ReadFile(fileName) > 0 ) {
    if( LoadDetectorGeo(haha, verbose) ){
      return true;
    }else{
      return false;
    }
  }else{
    return false;
  }
}

///Using TMacro to load the detectorGeo frist,
///this indrect method is good for loading detectorGeo from TMacro in root file
inline bool DetGeo::LoadDetectorGeo(TMacro * macro, bool verbose){

  if( macro == NULL ) return false;

  TList * haha = macro->GetListOfLines();
  int numLine = (haha)->GetSize();

  array1.pos.clear();
  array2.pos.clear();
  use2ndArray = false;

  int detFlag = 0;
  int detLine = 0;

  for( int i = 0 ; i < numLine; i++){

    std::vector<std::string> str = AnalysisLib::SplitStr(macro->GetListOfLines()->At(i)->GetName(), " ");

    //printf("%3d | %s\n", i,  str[0].c_str());

    if( str[0].find("####") != std::string::npos ) break;
    if( str[0].find("#===") != std::string::npos ) {
      detFlag ++;
      detLine = 0;
      continue;;
    }

    if( detFlag == 0 ){
      if ( i == 0 ) {
        Bfield = atof(str[0].c_str());
        BfieldSign = Bfield > 0 ? 1: -1;
      }
      if ( i ==  1 ) BfieldTheta       = atof(str[0].c_str());
      if ( i ==  2 ) bore              = atof(str[0].c_str());
      if ( i ==  3 ) recoilPos         = atof(str[0].c_str());
      if ( i ==  4 ) recoilInnerRadius = atof(str[0].c_str());
      if ( i ==  5 ) recoilOuterRadius = atof(str[0].c_str());
      if ( i ==  6 ) isCoincidentWithRecoil = str[0] == "false" ? false: true;
      if ( i ==  7 ) recoilPos1        = atof(str[0].c_str());
      if ( i ==  8 ) recoilPos2        = atof(str[0].c_str());
      if ( i ==  9 ) elumPos1          = atof(str[0].c_str());
      if ( i == 10 ) elumPos2          = atof(str[0].c_str());
    }

    if( detFlag == 1){
      if ( detLine == 0 ) array1.detPerpDist       = atof(str[0].c_str());
      if ( detLine == 1 ) array1.detWidth          = atof(str[0].c_str());
      if ( detLine == 2 ) array1.detLength         = atof(str[0].c_str());
      if ( detLine == 3 ) array1.blocker           = atof(str[0].c_str());
      if ( detLine == 4 ) array1.firstPos          = atof(str[0].c_str());
      if ( detLine == 5 ) array1.eSigma            = atof(str[0].c_str());
      if ( detLine == 6 ) array1.zSigma            = atof(str[0].c_str());
      if ( detLine == 7 ) array1.detFaceOut        = str[0] == "Out" ? true : false;
      if ( detLine == 8 ) array1.mDet              = atoi(str[0].c_str());
      if ( detLine >= 9 ) array1.pos.push_back(atof(str[0].c_str()));
      detLine ++;
    }

    if( detFlag == 2){
      if ( detLine ==  0 ) use2ndArray    = str[0] == "true" ? true : false;
      if ( detLine ==  1 ) array2.detPerpDist       = atof(str[0].c_str());
      if ( detLine ==  2 ) array2.detWidth          = atof(str[0].c_str());
      if ( detLine ==  3 ) array2.detLength         = atof(str[0].c_str());
      if ( detLine ==  4 ) array2.blocker           = atof(str[0].c_str());
      if ( detLine ==  5 ) array2.firstPos          = atof(str[0].c_str());
      if ( detLine ==  6 ) array2.eSigma            = atof(str[0].c_str());
      if ( detLine ==  7 ) array2.zSigma            = atof(str[0].c_str());
      if ( detLine ==  8 ) array2.detFaceOut        = str[0] == "Out" ? true : false;
      if ( detLine ==  9 ) array2.mDet              = atoi(str[0].c_str());
      if ( detLine >= 10 ) array2.pos.push_back(atof(str[0].c_str()));
      detLine ++;
    }

  }

  array1.DeduceAbsolutePos();

  zMin = array1.zMin;
  zMax = array1.zMax;

  if( use2ndArray) {
    array2.DeduceAbsolutePos();

    zMax = TMath::Min(array1.zMax, array2.zMax);
    zMin = TMath::Min(array1.zMin, array2.zMin);
  }

  if( verbose ) PrintDetGeo(false); 

  return true;

}

inline void DetGeo::PrintDetGeo(bool printAll) const{

  printf("=====================================================\n");
  printf("                 B-field: %8.2f  T, Theta : %6.2f deg \n", Bfield, BfieldTheta);
  if( BfieldTheta != 0.0 ) {
    printf("                                      +---- field angle != 0 is not supported!!! \n");
  }
  printf("     Recoil detector pos: %8.2f mm, radius: %6.2f - %6.2f mm \n", recoilPos, recoilInnerRadius, recoilOuterRadius);
  if( printAll ){
    printf("------------------------------------- Detector Position \n");
    array1.PrintArray();

    if( use2ndArray){
      printf("--------------------------------- 2nd Detector Position \n");
      array2.PrintArray();
    }
  }else{
    if( use2ndArray){
      printf("--------------------------------- 2nd Detector Position \n");
      array2.PrintArray();
    }else{
      printf("------------------------------------- Detector Position \n");
      array1.PrintArray();
    }
  }

  if( elumPos1 != 0 || elumPos2 != 0 || recoilPos1 != 0 || recoilPos2 != 0){
    printf("=================================== Auxillary/Imaginary Detectors\n");
  }
  if( elumPos1 != 0 )   printf("   Elum 1 pos.: %f mm \n", elumPos1);
  if( elumPos2 != 0 )   printf("   Elum 2 pos.: %f mm \n", elumPos2);
  if( recoilPos1 != 0 ) printf(" Recoil 1 pos.: %f mm \n", recoilPos1);
  if( recoilPos2 != 0 ) printf(" Recoil 2 pos.: %f mm \n", recoilPos2);
  printf("=====================================================\n");

}

#endif