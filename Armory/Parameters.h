#ifndef Parameters_H
#define Parameters_H

#include "ClassDetGeo.h"
#include "ClassReactionConfig.h"

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


#endif