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

  bool enable;

  double detPerpDist;    /// distance from axis
  double detWidth;       /// width   
  double detLength;      /// length
  double blocker;
  double firstPos;       /// meter
  double eSigma;         /// intrinsic energy resolution MeV
  double zSigma;         /// intrinsic position resolution mm  
  bool  detFaceOut;      ///detector_facing_Out_or_In
  std::vector<double> pos;  /// realtive position in meter
  int colDet, rowDet;      /// colDet = number of different pos, rowDet, number of same pos
  std::vector<double> detPos; ///absolute position of detector
  int numDet;  /// colDet * rowDet

  double zMin, zMax;
  
  void DeduceAbsolutePos(){
    
    colDet = pos.size();
    numDet = colDet * rowDet;
    detPos.clear();
    
    for(int id = 0; id < colDet; id++){
      if( firstPos > 0 ) detPos.push_back(firstPos + pos[id]);
      if( firstPos < 0 ) detPos.push_back(firstPos - pos[colDet - 1 - id]);
      // printf("%d | %f, %f \n", id, pos[id], detPos[id]);
    }

    zMin = TMath::Min(detPos.front(), detPos.back()) - (firstPos < 0 ? detLength : 0);
    zMax = TMath::Max(detPos.front(), detPos.back()) + (firstPos > 0 ? detLength : 0);
  }

  void Print() const{

    printf("------------------------------- Array\n");
    
    for(int i = 0; i < colDet ; i++){
      if( firstPos > 0 ){
        printf("%d, %8.2f mm - %8.2f mm \n", i, detPos[i], detPos[i] + detLength);
      }else{
        printf("%d, %8.2f mm - %8.2f mm \n", i, detPos[i] - detLength , detPos[i]);
      }
    }

    printf("   Blocker Position: %8.2f mm \n", firstPos > 0 ? firstPos - blocker : firstPos + blocker );
    printf("     First Position: %8.2f mm \n", firstPos);
    printf("     number of det : %d = %d x %d (side x col) \n", numDet, rowDet, colDet);
    printf("   detector facing : %s\n", detFaceOut ? "Out" : "In");
    printf("      energy resol.: %f MeV\n", eSigma);
    printf("       pos-Z resol.: %f mm \n", zSigma);
  }

};

struct Auxillary{

  //========== recoil
  double detPos; //
  double outerRadius;
  double innerRadius;

  bool isCoincident;

  double detPos1 = 0; // virtual recoil
  double detPos2 = 0; // virtual recoil

  //========== enum
  double elumPos1 = 0;
  double elumPos2 = 0;

  void Print() const {
    printf("------------------------------- Auxillary\n");
    printf("     Recoil detector pos: %8.2f mm, radius: %6.2f - %6.2f mm \n", detPos, innerRadius, outerRadius);

    if( elumPos1 != 0 || elumPos2 != 0 || detPos1 != 0 || detPos2 != 0){
      printf("=================================== Virtual Detectors\n");
      if( elumPos1 != 0 ) printf("   Elum 1 pos.: %f mm \n", elumPos1);
      if( elumPos2 != 0 ) printf("   Elum 2 pos.: %f mm \n", elumPos2);
      if( detPos1  != 0 ) printf(" Recoil 1 pos.: %f mm \n", detPos1);
      if( detPos2  != 0 ) printf(" Recoil 2 pos.: %f mm \n", detPos2);
      printf("=====================================================\n");
    }
  }

};

class DetGeo {

public:
  DetGeo(){};
  DetGeo(TString detGeoTxt){ LoadDetectorGeo(detGeoTxt, false);}
  DetGeo(TMacro * macro){ LoadDetectorGeo(macro, false);}
  ~DetGeo(){};

  double Bfield;      /// T
  int BfieldSign ;    /// sign of B-field
  double bore;        /// bore , mm

  unsigned short numGeo;
  double zMin, zMax;   /// total range span of all arrays

  bool LoadDetectorGeo(TString fileName, bool verbose = true);
  bool LoadDetectorGeo(TMacro * macro, bool verbose = true);

  //=================== array
  std::vector<Array> array;
  std::vector<Auxillary> aux;

  void Print( bool printArray = false) ;

  short GetArrayID(int id){
    int detCount = 0;
    for( int i = 0; i < numGeo; i ++ ){
      detCount += array[i].numDet;
      if( id < detCount ) return i;
    }
    return -1;
  }

private:

};

inline bool DetGeo::LoadDetectorGeo(TString fileName, bool verbose){

  TMacro * haha = new TMacro();
  if( haha->ReadFile(fileName) > 0 ) {
    if( LoadDetectorGeo(haha, verbose) ){
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

///Using TMacro to load the detectorGeo frist,
///this indrect method is good for loading detectorGeo from TMacro in root file
inline bool DetGeo::LoadDetectorGeo(TMacro * macro, bool verbose){

  if( macro == NULL ) return false;

  TList * haha = macro->GetListOfLines();
  int numLine = (haha)->GetSize();

  // for( int i = 0; i < 2 ; i++) array[i].pos.clear();

  int detFlag = 0;
  int detLine = 0;

  for( int i = 0 ; i < numLine; i++){

    std::string line = macro->GetListOfLines()->At(i)->GetName();

    if( AnalysisLib::isEmptyOrSpaces(line) ) continue;

    std::vector<std::string> str = AnalysisLib::SplitStr(line, " ");

    // printf("%3d | %s\n", i,  str[0].c_str());

    if( str[0].find("####") != std::string::npos ) break;
    if( str[0].find("#===") != std::string::npos ) {
      detFlag ++;
      array.push_back(Array());
      aux.push_back(Auxillary());
      detLine = 0;
      continue;;
    }

    if( detFlag == 0 ){
      if ( detLine == 0 ) {
        Bfield = atof(str[0].c_str());
        BfieldSign = Bfield > 0 ? 1: -1;
      }
      if ( detLine ==  1 ) bore              = atof(str[0].c_str());
    }

    if( detFlag >  0){
      unsigned short ID = detFlag - 1;
      if ( detLine ==  0 ) array[ID].enable       = str[0] == "true" ? true : false;
      if ( detLine ==  1 ) aux[ID].detPos         = atof(str[0].c_str());
      if ( detLine ==  2 ) aux[ID].innerRadius    = atof(str[0].c_str());
      if ( detLine ==  3 ) aux[ID].outerRadius    = atof(str[0].c_str());
      if ( detLine ==  4 ) aux[ID].isCoincident   = str[0] == "true" ? true : false;
      if ( detLine ==  5 ) aux[ID].detPos1        = atof(str[0].c_str());
      if ( detLine ==  6 ) aux[ID].detPos2        = atof(str[0].c_str());
      if ( detLine ==  7 ) aux[ID].elumPos1       = atof(str[0].c_str());
      if ( detLine ==  8 ) aux[ID].elumPos2       = atof(str[0].c_str());
      if ( detLine ==  9 ) array[ID].detPerpDist  = atof(str[0].c_str());
      if ( detLine == 10 ) array[ID].detWidth     = atof(str[0].c_str());
      if ( detLine == 11 ) array[ID].detLength    = atof(str[0].c_str());
      if ( detLine == 12 ) array[ID].blocker      = atof(str[0].c_str());
      if ( detLine == 13 ) array[ID].firstPos     = atof(str[0].c_str());
      if ( detLine == 14 ) array[ID].eSigma       = atof(str[0].c_str());
      if ( detLine == 15 ) array[ID].zSigma       = atof(str[0].c_str());
      if ( detLine == 16 ) array[ID].detFaceOut   = str[0] == "Out" ? true : false;
      if ( detLine == 17 ) array[ID].rowDet         = atoi(str[0].c_str());
      if ( detLine >= 18 ) array[ID].pos.push_back(atof(str[0].c_str()));
    }

    detLine ++;
  }

  zMin =  99999;
  zMax = -99999;
  numGeo = 0;

  for( int i = 0; i < detFlag; i ++ ){
    array[i].DeduceAbsolutePos();
    if (array[i].enable ) {
      numGeo ++;
      double zmax = TMath::Max(array[i].zMin, array[i].zMax);
      double zmin = TMath::Min(array[i].zMin, array[i].zMax);
      if( zmax > zMax ) zMax = zmax;
      if( zmin < zMin ) zMin = zmin;
    }
  }

  if( verbose ) Print(false); 

  return true;

}

inline void DetGeo::Print(bool printArray){

  printf("#####################################################\n");
  printf("           B-field : %8.2f T, %s\n", Bfield, Bfield > 0 ? "out of plan" : "into plan");
  printf("              Bore : %8.2f mm\n", bore);
  printf("  No. of det. Set. : %zu \n", array.size());
  
  printf("             z-Min : %8.2f mm\n", zMin);
  printf("             z-Max : %8.2f mm\n", zMax);
  if( printArray  ) {
    for( size_t i = 0; i < array.size() ; i++){
      printf("================================= %zu-th Detector Info (%s)\n", i, array[i].enable ? "enabled" : "disabled");
      array[i].Print();
      aux[i].Print();
    }
  }
  printf("#####################################################\n");

}

#endif