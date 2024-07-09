#ifndef Parameters_H
#define Parameters_H

//************************************** Correction parameters;
class CorrParas {
private:

  int defaultSize;

public:
  CorrParas(int defaultSize = 100){
    is_xn_OK = false;
    is_xfxne_OK = false;
    is_e_OK = false;
    is_xScale_OK = false;
    is_rdt_OK = false;

    this->defaultSize = defaultSize;
  };

  void LoadAllCorrections(){
    LoadXNCorr();
    LoadXFXN2ECorr();
    LoadECorr();
    LoadXScaleCorr();
    LoadRDTCorr();
  }

  void CheckCorrParasSize(int arraySize, int rdtSize){
    printf("------------ Check Correction parameter sizes\n");

    if( is_xn_OK && xnCorr.size() < arraySize ) {
      printf("    xnCorr [%zu] < array size %d. Set     xnCorr[1..99] = 1.0 \n", xnCorr.size(), arraySize);
      for( int i = 0; i < defaultSize; i++ ) xnCorr.push_back(1.0);
      is_xn_OK = false;
    }
    if( is_xfxne_OK && xfxneCorr.size() < arraySize ) {
      printf(" xfxneCorr [%zu] < array size %d. Set  xfxneCorr[1..99] = (0.0, 1.0) \n", xfxneCorr.size(), arraySize);
      for( int i = 0; i < defaultSize; i++ ) xfxneCorr.push_back({0.0, 1.0});
      is_xScale_OK = false;
    }
    if( is_e_OK && eCorr.size() < arraySize ) {
      printf("     eCorr [%zu] < array size %d. Set      eCorr[1..99] = (1.0, 0.0) \n", xnCorr.size(), arraySize);
      for( int i = 0; i < defaultSize; i++ ) eCorr.push_back({1.0, 0.0});
      is_e_OK = false;
    }
    if( is_xScale_OK && xScale.size() < arraySize ) {
      printf("    xScale [%zu] < array size %d. Set     xScale[1..99] = 1.0 \n", xScale.size(), arraySize);
      for( int i = 0; i < defaultSize; i++ ) xScale.push_back(1.0);
      is_xScale_OK = false;
    }
    if( is_rdt_OK && rdtCorr.size() < rdtSize ) {
      printf("   rdtCorr [%zu] < array size %d. Set   rdtScale[1..99] = (0.0, 1.0) \n", rdtCorr.size(), arraySize);
      for( int i = 0; i < defaultSize; i++ ) rdtCorr.push_back({0.0, 1.0});
      is_rdt_OK = false;
    }

    printf("------------ Done Check Corr. Para. Size.\n");
  }

  bool is_xn_OK;
  bool is_xfxne_OK;
  bool is_e_OK;
  bool is_xScale_OK;
  bool is_rdt_OK;

  std::vector<float> xnCorr;                 //correction of xn to match xf
  std::vector<std::vector<float>> xfxneCorr; //correction of xn and xf to match e
  std::vector<std::vector<float>> eCorr;     // correction to e, ch -> MeV
  std::vector<float> xScale;                 // correction of x to be (0,1)
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
      is_xn_OK = true;
    }else{
      for( int i = 0; i < defaultSize; i++ ) xnCorr.push_back(1.0);
      printf(".......... fail.    xnCorr[1..99] = 1.0\n");
      is_xn_OK = false;
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
      float a;
      while( file >> a ) xScale.push_back(a);  
      printf("........ done.\n");
      is_xScale_OK = true;
    }else{
      for( int i = 0; i < defaultSize; i++ ) xScale.push_back(1.0);
      printf("........ fail.    xScale[1..99] = 1.0\n");
      is_xScale_OK = false;
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
      is_xfxne_OK = true;
    }else{
      for( int i = 0; i < defaultSize; i++ ) xfxneCorr.push_back({0.0, 1.0});
      printf("........ fail. xfxneCorr[1..99] = (0.0, 1.0)\n");
      is_xfxne_OK = false;
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
      is_e_OK = true;
    }else{
      for( int i = 0; i < defaultSize; i++ ) eCorr.push_back( {1.0, 0.0} );
      printf(".............. fail.     eCorr[1..99] = (1.0, 0.0) \n");
      is_e_OK = false;
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
      is_rdt_OK = true;
    }else{
      for( int i = 0; i < defaultSize; i++ ) rdtCorr.push_back( {0.0, 1.0} );
      printf("............ fail.   rdtCorr[1..99] = (0.0, 1.0)\n");
      is_rdt_OK = false;
    }
    file.close();
    if( verbose ) for(int i = 0; i < (int) rdtCorr.size(); i++) printf("%2d | %10.3f, %10.3f\n", i, rdtCorr[i][0], rdtCorr[i][1]);
  }

};

#endif