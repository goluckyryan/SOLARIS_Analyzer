#ifndef ReactionParameters_H
#define ReactionParameters_H

#include "ClassDetGeo.h"

class ReactionParas{

public:
  ReactionParas();

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

  void LoadReactionParas(bool verbose = false);
  std::pair<double, double> CalExTheta(double e, double z)

};

ReactionParas::ReactionParas(){

}

//~========================================= reaction parameters
inline void ReactionParas::LoadReactionParas(bool verbose = false){
   
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

  std::string fileName;

  detPrepDist = Array::detPerpDist;

  printf(" loading reaction parameters");
  std::ifstream file;
  file.open(fileName.c_str());
  hasReactionPara = false;
  if( file.is_open() ){
    std::string x;
    int i = 0;
    while( file >> x ){
      if( x.substr(0,2) == "//" )  continue;
      if( i == 0 ) mass   = atof(x.c_str());
      if( i == 1 ) q      = atof(x.c_str());
      if( i == 2 ) beta = atof(x.c_str()); 
      if( i == 3 ) Et     = atof(x.c_str()); 
      if( i == 4 ) massB  = atof(x.c_str()); 
      i = i + 1;
    }
    printf("........ done.\n");

    hasReactionPara = true;
    alpha = 299.792458 * abs(detGeo.Bfield) * q / TMath::TwoPi()/1000.; //MeV/mm
    gamma = 1./TMath::Sqrt(1-beta * beta);
    G = alpha * gamma * beta * detPrepDist ;

    if( verbose ){
      printf("\tmass-b    : %f MeV/c2 \n", mass);
      printf("\tcharge-b  : %f \n", q);
      printf("\tE-total   : %f MeV \n", Et);
      printf("\tmass-B    : %f MeV/c2 \n", massB);
      printf("\tbeta      : %f \n", beta);
      printf("\tB-field   : %f T \n", detGeo.Bfield);
      printf("\tslope     : %f MeV/mm \n", alpha * beta);
      printf("\tdet radius: %f mm \n", detPrepDist);
      printf("\tG-coeff   : %f MeV \n", G);
      printf("=====================================================\n");
    }

  }else{
    printf("........ fail.\n");
  }
  file.close();
   
}

inline std::pair<double, double> ReactionParas::CalExTheta(double e, double z){

  ReactionParas * reactParas = nullptr;

  if( detGeo.array1.zMin <= z && z <= detGeo.array1.zMax ){
    reactParas = &reactParas1;
    if( !hasReactionPara) return {TMath::QuietNaN(), TMath::QuietNaN()};
  }
  
  if( detGeo.array2.zMin <= z && z <= detGeo.array2.zMax ){
    reactParas = &reactParas2;
    if( !hasReactionPara) return {TMath::QuietNaN(), TMath::QuietNaN()};
  }

  double Ex = TMath::QuietNaN();
  double thetaCM = TMath::QuietNaN();

  double y = e + mass; // to give the KE + mass of proton;
  double Z = alpha * gamma * beta * z;
  double H = TMath::Sqrt(TMath::Power(gamma * beta,2) * (y*y - mass * mass) ) ;
 
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
      double x = TMath::ACos( mass / ( y * gamma - K));
      double momt = mass * TMath::Tan(x); /// momentum of particel b or B in CM frame
      double EB = TMath::Sqrt(mass * mass + Et * Et - 2 * Et * TMath::Sqrt(momt*momt + mass * mass));
      Ex = EB - massB;

      double hahaha1 = gamma * TMath::Sqrt(mass * mass + momt * momt) - y;
      double hahaha2 = gamma * beta * momt;
      thetaCM = TMath::ACos(hahaha1/hahaha2) * TMath::RadToDeg();
    
    }
  }
  return std::make_pair(Ex, thetaCM);

}


#endif