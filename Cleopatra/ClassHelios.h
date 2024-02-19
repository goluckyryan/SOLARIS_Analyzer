#ifndef HELIOS_Library_h
#define HELIOS_Library_h

#include "TBenchmark.h"
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom.h"
#include "TMacro.h"
#include "TGraph.h"
#include <vector>
#include <fstream>

#include "../Armory/AnalysisLib.h"
#include "../Armory/ClassDetGeo.h"
#include "../Armory/ClassReactionConfig.h"

//=======================================================
//#######################################################
//Class for HELIOS
//input Lorentz vector, detector configuration
//output e, z, Ex, thetaCM, etc
//======================================================= 

struct trajectory{
  double theta, phi;
  double vt, vp; // tranvser and perpendicular velocity
  double rho;    // orbit radius
  double z0, t0; // position cycle
  double x, y, z; // hit position
  double t; //actual orbit time;
  double R; //hit radius = sqrt(x^2+y^2);
  int detID, detRowID; 
  int loop;
  double effLoop;

  void PrintTrajectory(){
    printf("=====================\n");
    printf("    theta : %f deg\n", theta*TMath::RadToDeg());
    printf("      phi : %f deg\n", phi*TMath::RadToDeg());
    printf("       vt : %f mm/ns\n", vt);
    printf("       vp : %f mm/ns\n", vp);
    printf("      rho : %f mm\n", rho);
    printf("       z0 : %f mm\n", z0);
    printf("       t0 : %f ns\n", t0);
    printf("(x, y, z) : (%f, %f. %f) mm\n", x, y, z);
    printf("        R : %f mm\n", R);
    printf("        t : %f ns\n", t);
    printf("  effLoop : %f cycle\n", effLoop);
    printf("     Loop : %d cycle\n", loop);
    printf(" detRowID : %d \n", detRowID);
    printf("    detID : %d \n", detID);
    
  }

  void Clear(){
    theta = TMath::QuietNaN();
    phi = TMath::QuietNaN();
    vt = TMath::QuietNaN();
    vp = TMath::QuietNaN();
    rho = TMath::QuietNaN();
    z0 = TMath::QuietNaN();
    t0 = TMath::QuietNaN();
    x = TMath::QuietNaN();
    y = TMath::QuietNaN();
    z = TMath::QuietNaN();
    effLoop = TMath::QuietNaN();
    detID = -1;
    detRowID = -1;
    loop = -1;
  }
};

class HELIOS{
public:

  HELIOS();
  ~HELIOS();
  
  void SetCoincidentWithRecoil(bool TorF){ this->isCoincidentWithRecoil = TorF;}
  bool GetCoincidentWithRecoil(){return this->isCoincidentWithRecoil;}
  bool SetDetectorGeometry(std::string filename, unsigned short ID);
  void SetBeamPosition(double x, double y) { xOff = x; yOff = y;}
  
  void OverrideMagneticField(double BField);
  void OverrideMagneticFieldDirection(double BfieldThetaInDeg);
  void OverrideFirstPos(double firstPos);
  void OverrideDetectorDistance(double perpDist);
  void OverrideDetectorFacing(bool isOutside);
  
  int CheckDetAcceptance();
  int CalArrayHit(TLorentzVector Pb, bool debug = false);
  int CalRecoilHit(TLorentzVector PB);  
  void CalTrajectoryPara(TLorentzVector P, bool isLightRecoil);
  
  int GetNumberOfDetectorsInSamePos(){return array.mDet;}
  double GetEnergy()const {return e;}
  double GetDetX()  const {return detX;} // position in each detector, range from -1, 1
  
  /// clockwise rotation for B-field along the z-axis, sign = 1.
  double XPos(double Zpos, double theta, double phi, double rho, int sign){
    if( TMath::IsNaN(Zpos) ) return TMath::QuietNaN();
    return rho * ( TMath::Sin( TMath::Tan(theta) * Zpos / rho - sign * phi ) + sign * TMath::Sin(phi) ) + xOff;
  }
  double YPos(double Zpos, double theta, double phi, double rho, int sign){
    if( TMath::IsNaN(Zpos) ) return TMath::QuietNaN();
    return rho * sign * (TMath::Cos( TMath::Tan(theta) * Zpos / rho - sign * phi ) - TMath::Cos(phi)) + yOff;
  }
  double RPos(double Zpos, double theta, double phi, double rho, int sign){
    if( TMath::IsNaN(Zpos) ) return TMath::QuietNaN();
    double x =  XPos(Zpos, theta, phi, rho, sign) ;
    double y =  YPos(Zpos, theta, phi, rho, sign) ;
    return sqrt(x*x+y*y);
  }
  
  double GetXPos(double ZPos){ return TMath::IsNaN(ZPos) ? TMath::QuietNaN() : XPos( ZPos, orbitb.theta, orbitb.phi, orbitb.rho, detGeo.BfieldSign); }
  double GetYPos(double ZPos){ return TMath::IsNaN(ZPos) ? TMath::QuietNaN() : YPos( ZPos, orbitb.theta, orbitb.phi, orbitb.rho, detGeo.BfieldSign); }
  double GetR(double ZPos)   { return TMath::IsNaN(ZPos) ? TMath::QuietNaN() : RPos( ZPos, orbitb.theta, orbitb.phi, orbitb.rho, detGeo.BfieldSign); }
  
  double GetRecoilEnergy(){return eB;}
  double GetRecoilXPos(double ZPos){ return TMath::IsNaN(ZPos) ? TMath::QuietNaN() : XPos( ZPos, orbitB.theta, orbitB.phi, orbitB.rho, detGeo.BfieldSign); }   
  double GetRecoilYPos(double ZPos){ return TMath::IsNaN(ZPos) ? TMath::QuietNaN() : YPos( ZPos, orbitB.theta, orbitB.phi, orbitB.rho, detGeo.BfieldSign); }
  double GetRecoilR(double ZPos)   { return TMath::IsNaN(ZPos) ? TMath::QuietNaN() : RPos( ZPos, orbitB.theta, orbitB.phi, orbitB.rho, detGeo.BfieldSign); }
  
  void PrintGeometry() const;

  double GetBField()    const {return detGeo.Bfield;}
  double GetDetRadius() const {return array.detPerpDist;}

  trajectory GetTrajectory_b() const {return orbitb;}
  trajectory GetTrajectory_B() const {return orbitB;}
  
  DetGeo GetDetectorGeometry() const {return detGeo;}
  Array GetArrayGeometry()     const {return array;}
  
  TString GetHitMessage()         {return hitMessage;}
  TString GetAcceptanceMessage()  { AcceptanceCodeToMsg(acceptanceCode); return acceptanceMsg;}
  
  TString AcceptanceCodeToMsg(short code );

private:   
      
  DetGeo detGeo;
  Array array;
  
  trajectory orbitb, orbitB;
  
  double e,detX ; ///energy of light recoil, position X
  double rhoHit;  /// radius of particle-b hit on recoil detector
  
  double eB;  ///energy of heavy recoil
  
  bool isDetReady;

  TString hitMessage;
  TString acceptanceMsg; //acceptance check
  short acceptanceCode;
  
  double xOff, yOff; // beam position

  bool overrideDetDistance;
  bool overrideFirstPos;
  bool isCoincidentWithRecoil;
  
  const double c = 299.792458; //mm/ns


};

HELIOS::HELIOS(){
   
  orbitb.Clear();
  orbitB.Clear();
   
  e = TMath::QuietNaN();
  eB = TMath::QuietNaN();
  detX = TMath::QuietNaN();
  rhoHit = TMath::QuietNaN();
  
  xOff = 0.0;
  yOff = 0.0;
  
  isDetReady = false;

  hitMessage = "";
  acceptanceMsg = "";
  acceptanceCode = 0;

  overrideDetDistance = false;
  overrideFirstPos = false;
  isCoincidentWithRecoil = false;
   
}

HELIOS::~HELIOS(){
  
}

void HELIOS::OverrideMagneticField(double BField){ 
  this->detGeo.Bfield = BField; 
  this->detGeo.BfieldSign = BField > 0 ? 1: -1;
}

void HELIOS::OverrideMagneticFieldDirection(double BfieldThetaInDeg){ 
   this->detGeo.BfieldTheta = BfieldThetaInDeg;
}

void HELIOS::OverrideFirstPos(double firstPos){
   overrideFirstPos = true;
   printf("------ Overriding FirstPosition to : %8.2f mm \n", firstPos);
   this->array.firstPos = firstPos;
}

void HELIOS::OverrideDetectorDistance(double perpDist){
   overrideDetDistance = true;
   printf("------ Overriding Detector Distance to : %8.2f mm \n", perpDist);
   this->array.detPerpDist = perpDist;
}

void HELIOS::OverrideDetectorFacing(bool isOutside){
   this->array.detFaceOut = isOutside;
   printf(" Detectors are facing %s\n", array.detFaceOut ? "outside": "inside" );
}

bool HELIOS::SetDetectorGeometry(std::string filename, unsigned short ID){

  if( detGeo.LoadDetectorGeo(filename, false)) {

    array = detGeo.array[ID];
    isCoincidentWithRecoil = detGeo.isCoincidentWithRecoil;
    isDetReady = true;

  }else{
    printf("cannot read file %s.\n", filename.c_str());
    isDetReady = false;
  }
  
  return isDetReady;  
}

void HELIOS::PrintGeometry() const{

  printf("=====================================================\n");
  printf("                 B-field: %8.2f  T, Theta : %6.2f deg \n", detGeo.Bfield, detGeo.BfieldTheta);
  if( detGeo.BfieldTheta != 0.0 ) {
    printf("                                      +---- field angle != 0 is not supported!!! \n");
  }
  printf("     Recoil detector pos: %8.2f mm, radius: %6.2f - %6.2f mm \n", detGeo.recoilPos, detGeo.recoilInnerRadius, detGeo.recoilOuterRadius);

  printf("----------------------------------- Detector Position \n");
  array.PrintArray();

  if( detGeo.elumPos1 != 0 || detGeo.elumPos2 != 0 || detGeo.recoilPos1 != 0 || detGeo.recoilPos2 != 0){
    printf("=================================== Auxillary/Imaginary Detectors\n");
  }
  if( detGeo.elumPos1 != 0 )   printf("   Elum 1 pos.: %f mm \n", detGeo.elumPos1);
  if( detGeo.elumPos2 != 0 )   printf("   Elum 2 pos.: %f mm \n", detGeo.elumPos2);
  if( detGeo.recoilPos1 != 0 ) printf(" Recoil 1 pos.: %f mm \n", detGeo.recoilPos1);
  if( detGeo.recoilPos2 != 0 ) printf(" Recoil 2 pos.: %f mm \n", detGeo.recoilPos2);
  printf("=====================================================\n");


}

TString HELIOS::AcceptanceCodeToMsg(short code ){

   switch(code){
      case   3 : acceptanceMsg = "try one more loop"; break;
      case   2 : acceptanceMsg = "hit less than the nearest array. increase loop"; break;
      case   1 : acceptanceMsg = "GOOD!! hit Array"; break;

      case   0 : acceptanceMsg = "detector geometry incomplete."; break;
      case  -1 : acceptanceMsg = "array at upstream, z is downstream."; break;
      case  -2 : acceptanceMsg = "array at downstream, z is upstream."; break;
      case  -3 : acceptanceMsg = "hit at the XY gap."; break;
      case  -4 : acceptanceMsg = "hit more upstream than the array length"; break;
      case  -5 : acceptanceMsg = "hit more downstream than the array length"; break;
      case  -6 : acceptanceMsg = "hit blocker"; break;
      case  -7 : acceptanceMsg = "hit array Z-gap"; break;

      case -10 : acceptanceMsg = "rho is too big"; break;
      case -11 : acceptanceMsg = "rho is too small"; break;
      case -12 : acceptanceMsg = "light recoil blocked by recoil detector"; break;
      case -13 : acceptanceMsg = "more than 3 loops."; break;
      case -14 : acceptanceMsg = "heavy recoil does not hit recoil detector"; break;
      case -15 : acceptanceMsg = "det Row ID == -1"; break;
      default : acceptanceMsg = "unknown error."; break;
   }

   return acceptanceMsg;

}

int HELIOS::CheckDetAcceptance(){
   
  //CalArrayHit and CalRecoilHit must be done before.
  
  if( isDetReady == false ) { acceptanceCode = 0; return acceptanceCode; }

  // -1 ========= when recoil direction is not same side of array
  if( array.firstPos < 0 && orbitb.z > 0 ) {acceptanceCode = -1; return acceptanceCode;}
  
  // -2 ========= when recoil direction is not same side of array
  if( array.firstPos > 0 && orbitb.z < 0 ) {acceptanceCode = -2; return acceptanceCode;}

  // -11 ======== rho is too small
  if(  2 * orbitb.rho < array.detPerpDist ) { acceptanceCode = -11; return acceptanceCode;} 
  
  // -15 ========= if detRowID == -1, should be (2 * orbitb.rho < perpDist)
  if( orbitb.detRowID == -1 ) {acceptanceCode = -15; return acceptanceCode;}
  
  // -10 =========== when rho is too big . 
  if( detGeo.bore < 2 * orbitb.rho) { acceptanceCode = -10; return acceptanceCode;} 
  
  // -14 ========== check particle-B hit radius on recoil dectector
  if( isCoincidentWithRecoil && orbitB.R > detGeo.recoilOuterRadius  ) {acceptanceCode = -14; return acceptanceCode;} 

  //if( isCoincidentWithRecoil && (orbitB.R > rhoRecoilout || orbitB.R < rhoRecoilin) ) return -14;
  
  // -12 ========= check is particle-b was blocked by recoil detector
  rhoHit = GetR(detGeo.recoilPos);
  if( orbitb.z > 0 && detGeo.recoilPos > 0 && orbitb.z > detGeo.recoilPos && rhoHit < detGeo.recoilOuterRadius ) { acceptanceCode = -12; return acceptanceCode;}
  if( orbitb.z < 0 && detGeo.recoilPos < 0 && orbitb.z < detGeo.recoilPos && rhoHit < detGeo.recoilOuterRadius ) { acceptanceCode = -12; return acceptanceCode;}
  
  // -13 ========= not more than 3 loops
  if( orbitb.loop > 3 ) {acceptanceCode = -13; return acceptanceCode;}
  
  // -3 ========= calculate the "y"-distance from detector center
  if( sqrt(orbitb.R*orbitb.R - array.detPerpDist * array.detPerpDist)> array.detWidth/2 ) { acceptanceCode = -3; return acceptanceCode;} 
  
  // -4, -5 ==== when zPos further the range of whole array, more loop would not save
  if( array.firstPos < 0 && orbitb.z < array.detPos[0] - array.detLength )            { acceptanceCode = -4; return acceptanceCode;}
  if( array.firstPos > 0 && orbitb.z > array.detPos[array.nDet-1] + array.detLength ) { acceptanceCode = -5; return acceptanceCode;} 

  // -6 ======== Hit on blacker
  if( array.blocker != 0 && array.firstPos > 0 && array.detPos[0] - array.blocker  < orbitb.z && orbitb.z < array.detPos[0] ) {acceptanceCode = -6; return acceptanceCode;} 
  if( array.blocker != 0 && array.firstPos < 0 && array.detPos[array.nDet-1]  < orbitb.z && orbitb.z < array.detPos[array.nDet-1] + array.blocker ) { acceptanceCode = -6; return acceptanceCode;} 

  // 2 ======  when zPos less then the nearest position, more loop may hit
  int increaseLoopFlag = 0;
  if( array.firstPos < 0 && array.detPos[array.nDet-1] < orbitb.z ) increaseLoopFlag = 2; 
  if( array.firstPos > 0 && array.detPos[0] > orbitb.z ) increaseLoopFlag = 2; 
  if (increaseLoopFlag == 2 ) {
    orbitb.z += orbitb.z0;
    orbitb.effLoop += 1.0;
    orbitb.loop += 1;
    orbitb.t = orbitb.t0 * orbitb.effLoop;
    acceptanceCode = 2; 
    return acceptanceCode;
  }
   
  // 1 ======= check hit array z- position
  if( array.firstPos < 0 ){
    for( int i = 0; i < array.nDet; i++){
      if( array.detPos[i] - array.detLength <= orbitb.z && orbitb.z <= array.detPos[i]) {
        orbitb.detID = i;
        detX = ( orbitb.z - (array.detPos[i] + array.detLength/2 ))/ array.detLength * 2 ;// range from -1 , 1 
        acceptanceCode = 1; 
        return acceptanceCode;
      }
    }      
  }else{
    for( int i = 0; i < array.nDet ; i++){
      if( array.detPos[i] <= orbitb.z && orbitb.z <= array.detPos[i] + array.detLength)  {
        ///printf(" %d | %f < z = %f < %f \n", i,  array.detPos[i], orbitb.z, array.detPos[i]+length); 
        orbitb.detID = i;
        detX = ( orbitb.z - (array.detPos[i] - array.detLength/2 ))/ array.detLength*2 ;// range from -1 , 1 
        acceptanceCode = 1; 
        return acceptanceCode;
      }
    }
  }

   
  // -7 ======== check hit array gap
  if( array.firstPos < 0 ){
    for( int i = 0; i < array.nDet-1 ; i++){
        if( array.detPos[i] < orbitb.z && orbitb.z < array.detPos[i+1] - array.detLength ) { acceptanceCode = -7; return acceptanceCode; }//increaseLoopFlag = 3; 
    }      
  }else{
    for( int i = 0; i < array.nDet-1 ; i++){
        if( array.detPos[i] + array.detLength < orbitb.z && orbitb.z < array.detPos[i+1]  ) { acceptanceCode = -7; return acceptanceCode; }//increaseLoopFlag = 3;
    }
  }
  if (increaseLoopFlag == 3 ) {
    orbitb.z += orbitb.z0;
    orbitb.effLoop += 1.0;
    orbitb.loop += 1;
    orbitb.t = orbitb.t0 * orbitb.effLoop;
    acceptanceCode = 3;
    return acceptanceCode;
  }
   
   acceptanceCode = -20 ;
   return acceptanceCode; // for unknown reason
}

void HELIOS::CalTrajectoryPara(TLorentzVector P, bool isLightRecoil){
   
   if( isLightRecoil ){
      orbitb.theta = P.Theta();
      orbitb.phi = P.Phi();
      orbitb.rho = P.Pt() / abs(detGeo.Bfield) / P.GetUniqueID() / c * 1000; //mm
      orbitb.vt = P.Beta() * TMath::Sin(P.Theta()) * c ; // mm / nano-second  
      orbitb.vp = P.Beta() * TMath::Cos(P.Theta()) * c ; // mm / nano-second  
      orbitb.t0 = TMath::TwoPi() * orbitb.rho / orbitb.vt; // nano-second  
      orbitb.z0 = orbitb.vp * orbitb.t0;
      
      orbitb.detID = -1;
      orbitb.detRowID = -1;

   }else{
      orbitB.theta = P.Theta();
      orbitB.phi = P.Phi();
      orbitB.rho = P.Pt() / abs(detGeo.Bfield) / P.GetUniqueID() / c * 1000; //mm
      orbitB.vt = P.Beta() * TMath::Sin(P.Theta()) * c ; // mm / nano-second  
      orbitB.vp = P.Beta() * TMath::Cos(P.Theta()) * c ; // mm / nano-second  
      orbitB.t0 = TMath::TwoPi() * orbitB.rho / orbitB.vt; // nano-second  
      orbitB.z0 = orbitB.vp * orbitB.t0;
      
      orbitB.detID = -1;
      orbitB.detRowID = -1;
   }
}

int HELIOS::CalArrayHit(TLorentzVector Pb, bool debug){
   
   e = Pb.E() - Pb.M();
   detX = TMath::QuietNaN();
   rhoHit = TMath::QuietNaN();
   
   CalTrajectoryPara(Pb, true);
   
   int targetLoop = 1;
   int inOut = array.detFaceOut == true ? 1: 0; //1 = from Outside, 0 = from inside
   
   if( debug ) {
      printf("===================================\n");
      printf("theta : %f deg, phi : %f deg \n", orbitb.theta * TMath::RadToDeg(), orbitb.phi * TMath::RadToDeg());
      printf("z0: %f mm, rho : %f mm \n", orbitb.z0, orbitb.rho);
      printf("      inOut : %d = %s \n", inOut, inOut == 1 ? "Out" : "in");
      printf(" z range : %.2f - %.2f \n", detGeo.zMin, detGeo.zMax);
      printf(" B-field sign : %d\n", detGeo.BfieldSign);
      printf("-----------------------------------\n");
   }

   std::vector<double> zPossible;
   std::vector<int> dID; //detRowID
   
   int iStart = ( detGeo.BfieldSign == 1 ? 0 : -array.mDet );
   int iEnd = ( detGeo.BfieldSign == 1 ? 2 * array.mDet : array.mDet );   
   for( int i = iStart; i < iEnd ; i++){
      
      double phiD = TMath::TwoPi()/array.mDet * i ;
      double dphi = orbitb.phi - phiD;
      double aEff = array.detPerpDist - (xOff * TMath::Cos(phiD) + yOff * TMath::Sin(phiD)); 
      double hahaha = asin( aEff/ orbitb.rho - detGeo.BfieldSign * sin(dphi));
      
      int n = 2*targetLoop + inOut; 
      
      double zP = orbitb.z0 /TMath::TwoPi() * ( detGeo.BfieldSign * dphi + n * TMath::Pi() + pow(-1, n) * hahaha );
      
      if( debug ) {
         double xP = GetXPos(zP) ;
         double yP = GetYPos(zP) ;
         printf("phiD: %4.0f, dphi: %6.1f, mod(pi): %6.1f, Loop : %9.5f, zHit : %8.3f mm, (x,y) = (%7.2f, %7.2f) \n",
                  phiD * TMath::RadToDeg(), 
                  (orbitb.phi-phiD) * TMath::RadToDeg(), 
                  fmod(orbitb.phi-phiD, TMath::Pi())*TMath::RadToDeg(),
                  zP/orbitb.z0, zP, xP, yP );
      }
      
      ///Selection
      if( !TMath::IsNaN(zP) && 0 < zP/orbitb.z0 && TMath::Max(0, targetLoop-1) < zP/orbitb.z0 && zP/orbitb.z0 < targetLoop ) {
         zPossible.push_back(zP);
         dID.push_back(i);
      }
   }

   if( debug ) printf("-----------------------------------\n");
   double dMin = 1;   
   for( int i = 0; i < (int) zPossible.size(); i++){
      
      double dd = abs(zPossible[i]/orbitb.z0 - (targetLoop - (1-inOut)));
      
      if( debug ) printf(" %d | zP : %8.3f mm; loop : %9.5f ", i, zPossible[i], zPossible[i]/orbitb.z0);
      
      if( dd < dMin) {
         orbitb.z    = zPossible[i];
         dMin = dd;
         orbitb.effLoop = zPossible[i]/orbitb.z0;
         orbitb.loop = TMath::Ceil(orbitb.effLoop);
         orbitb.detRowID = (12+dID[i])%4;
         orbitb.t = orbitb.t0 * orbitb.effLoop;
         
         double phiD = TMath::TwoPi()/array.mDet * dID[i] ;
         double dphi = orbitb.phi - phiD ;
         
         if( debug ) {
            // Check is in or out 
            double hitDir = cos( orbitb.z/orbitb.z0 * TMath::TwoPi() - detGeo.BfieldSign * dphi );
            printf(" hitDir : %4.1f ", hitDir);
            if( ( inOut == 1 && hitDir > 0 ) || (inOut == 0 && hitDir < 0 ) ) {
                printf(" != %f ", array.detPerpDist);
                orbitb.z = TMath::QuietNaN();
                orbitb.loop = -1;
                orbitb.detRowID = -1;
                hitMessage = "wrong direction.";
                return - 2;
            }
         
            // this must be false, otherwise, calculation error
            double xPos = GetXPos(orbitb.z ) ;
            double yPos = GetYPos(orbitb.z ) ;
            double a = xPos * cos(phiD) + yPos * sin(phiD);
            printf(" a : %f ", a);
            if( abs(a - array.detPerpDist) > 0.01) { 
                printf(" != %f ", array.detPerpDist);
                orbitb.z = TMath::QuietNaN();
                orbitb.loop = -1;
                orbitb.detRowID = -1;
                hitMessage = "not on the detector plan.";
                return -3;
            }
         }
      }
      if(debug) printf("\n");
   }
   
   // calculate x, y, R
   orbitb.x = GetXPos(orbitb.z) ;
   orbitb.y = GetYPos(orbitb.z) ;
   orbitb.R = GetR(orbitb.z);
   
   hitMessage = "successful hit.";
   return 1; // return 1 when OK
}

int HELIOS::CalRecoilHit(TLorentzVector PB){
   
   CalTrajectoryPara(PB, false);
   
   orbitB.z = detGeo.recoilPos;
   orbitB.x = GetRecoilXPos(detGeo.recoilPos)  ;
   orbitB.y = GetRecoilYPos(detGeo.recoilPos)  ;
   orbitB.R = GetRecoilR(detGeo.recoilPos);
   orbitB.effLoop = orbitB.z/orbitB.z0;
   orbitB.t  = orbitB.t0 * orbitB.effLoop ;
   
   return 1;
}

#endif 
