#ifndef Parameters_H
#define Parameters_H

// #include "ClassDetGeo.h"
// #include "ClassReactionConfig.h"

// DetGeo detGeo;
// ReactionConfig reactionConfig1;
// ReactionConfig reactionConfig2;

// void LoadDetGeoAndReactionConfigFile(std::string detGeoFileName = "detectorGeo.txt", 
//                                      std::string reactionConfigFileName1 = "reactionConfig1.txt", 
//                                      std::string reactionConfigFileName2 = "reactionConfig2.txt"){
//   printf("=====================================================\n");
//   printf(" loading detector geometery : %s.", detGeoFileName.c_str());
//   TMacro * haha = new TMacro();
//   if( haha->ReadFile(detGeoFileName.c_str()) > 0 ) {
//     detGeo = AnalysisLib::LoadDetectorGeo(haha);    
//     printf("... done.\n");
//     AnalysisLib::PrintDetGeo(detGeo);
//   }else{
//     printf("... fail\n");
//   }
//   delete haha;

//   printf("=====================================================\n");
//   printf(" loading reaction1 config : %s.", reactionConfigFileName1.c_str());
//   TMacro * kaka = new TMacro();
//   if( kaka->ReadFile(reactionConfigFileName1.c_str()) > 0 ) {
//     reactionConfig1  = AnalysisLib::LoadReactionConfig(kaka);
//     printf("..... done.\n");
//     AnalysisLib::PrintReactionConfig(reactionConfig1);
//   }else{
//     printf("..... fail\n");
//   }
//   delete kaka;

//   if( detGeo.use2ndArray){
//     printf("=====================================================\n");
//     printf(" loading reaction2 config : %s.", reactionConfigFileName2.c_str());
//     TMacro * jaja = new TMacro();
//     if( jaja->ReadFile(reactionConfigFileName2.c_str()) > 0 ) {
//       reactionConfig2  = AnalysisLib::LoadReactionConfig(kaka);
//       printf("..... done.\n");
//       AnalysisLib::PrintReactionConfig(reactionConfig2);
//     }else{
//       printf("..... fail\n");
//     }
//     delete jaja;
//   }

// }

//************************************** Correction parameters;
class CorrParas {

public:
  CorrParas(){
    xnCorr.clear();
    xScale.clear();
    xfxneCorr.clear();
    eCorr.clear();
    rdtCorr.clear();
  };

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

};

#endif