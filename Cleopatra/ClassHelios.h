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
};

void PrintTrajectory(trajectory a){
   printf("=====================\n");
   printf("    theta : %f deg\n", a.theta*TMath::RadToDeg());
   printf("      phi : %f deg\n", a.phi*TMath::RadToDeg());
   printf("       vt : %f mm/ns\n", a.vt);
   printf("       vp : %f mm/ns\n", a.vp);
   printf("      rho : %f mm\n", a.rho);
   printf("       z0 : %f mm\n", a.z0);
   printf("       t0 : %f ns\n", a.t0);
   printf("(x, y, z) : (%f, %f. %f) mm\n", a.x, a.y, a.z);
   printf("        R : %f mm\n", a.R);
   printf("        t : %f ns\n", a.t);
   printf("  effLoop : %f cycle\n", a.effLoop);
   printf("     Loop : %d cycle\n", a.loop);
   printf(" detRowID : %d \n", a.detRowID);
   printf("    detID : %d \n", a.detID);
   
}

class HELIOS{
public:

   HELIOS();
   ~HELIOS();
   
   void SetCoincidentWithRecoil(bool TorF){ this->isCoincidentWithRecoil = TorF;}
   bool GetCoincidentWithRecoil(){return this->isCoincidentWithRecoil;}
   bool SetDetectorGeometry(std::string filename);
   void SetBeamPosition(double x, double y) { xOff = x; yOff = y;}
   
   void OverrideMagneticField(double BField){ this->detGeo.Bfield = BField; this->detGeo.BfieldSign = BField > 0 ? 1: -1;}
   void OverrideMagneticFieldDirection(double BfieldThetaInDeg){ this->detGeo.BfieldTheta = BfieldThetaInDeg;}
   void OverrideFirstPos(double firstPos){
      overrideFirstPos = true;
      printf("------ Overriding FirstPosition to : %8.2f mm \n", firstPos);
      this->array.firstPos = firstPos;
   }
   void OverrideDetectorDistance(double perpDist){
      overrideDetDistance = true;
      printf("------ Overriding Detector Distance to : %8.2f mm \n", perpDist);
      this->array.detPerpDist = perpDist;
   }
   
   void SetDetectorOutside(bool isOutside){
      this->array.detFaceOut = isOutside;
      printf(" Detectors are facing %s\n", array.detFaceOut ? "outside": "inside" );
   }
   
   int DetAcceptance();
   int CalArrayHit(TLorentzVector Pb, int Zb);
   int CalRecoilHit(TLorentzVector PB, int ZB);
   //int CalHit(TLorentzVector Pb, int Zb, TLorentzVector PB, int ZB, double xOff = 0, double yOff = 0 ); // return 0 for no hit, 1 for hit
   
   void CalTrajectoryPara(TLorentzVector P, int Z, int id); // id = 0 for Pb, id = 1 for PB.
   
   int GetNumberOfDetectorsInSamePos(){return array.mDet;}
   double GetEnergy(){return e;}
   double GetDetX(){return detX;} // position in each detector, range from -1, 1
   
   /// clockwise rotation for B-field along the z-axis, sign = 1.
   double XPos(double Zpos, double theta, double phi, double rho, int sign){
     return rho * ( TMath::Sin( TMath::Tan(theta) * Zpos / rho - sign * phi ) + sign * TMath::Sin(phi) ) + xOff;
   }
   double YPos(double Zpos, double theta, double phi, double rho, int sign){
     return rho * sign * (TMath::Cos( TMath::Tan(theta) * Zpos / rho - sign * phi ) - TMath::Cos(phi)) + yOff;
   }
   double RPos(double Zpos, double theta, double phi, double rho, int sign){
      double x =  XPos(Zpos, theta, phi, rho, sign) ;
      double y =  YPos(Zpos, theta, phi, rho, sign) ;
      return sqrt(x*x+y*y);
   }
   
   double GetXPos(double ZPos){ return XPos( ZPos, orbitb.theta, orbitb.phi, orbitb.rho, detGeo.BfieldSign); }
   double GetYPos(double ZPos){ return YPos( ZPos, orbitb.theta, orbitb.phi, orbitb.rho, detGeo.BfieldSign); }
   double GetR(double ZPos)   { return RPos( ZPos, orbitb.theta, orbitb.phi, orbitb.rho, detGeo.BfieldSign); }
   
   double GetRecoilEnergy(){return eB;}
   double GetRecoilXPos(double ZPos){ return XPos( ZPos, orbitB.theta, orbitB.phi, orbitB.rho, detGeo.BfieldSign); }   
   double GetRecoilYPos(double ZPos){ return YPos( ZPos, orbitB.theta, orbitB.phi, orbitB.rho, detGeo.BfieldSign); }
   double GetRecoilR(double ZPos)   { return RPos( ZPos, orbitB.theta, orbitB.phi, orbitB.rho, detGeo.BfieldSign); }
   
   double GetBField() {return detGeo.Bfield;}
   double GetDetRadius() {return array.detPerpDist;}

   trajectory GetTrajectory_b() {return orbitb;}
   trajectory GetTrajectory_B() {return orbitB;}

   DetGeo GetDetectorGeometry() {return detGeo;}
   
private:   
      
   void ClearTrajectory(trajectory t){
      t.theta = TMath::QuietNaN();
      t.phi = TMath::QuietNaN();
      t.vt = TMath::QuietNaN();
      t.vp = TMath::QuietNaN();
      t.rho = TMath::QuietNaN();
      t.z0 = TMath::QuietNaN();
      t.t0 = TMath::QuietNaN();
      t.x = TMath::QuietNaN();
      t.y = TMath::QuietNaN();
      t.z = TMath::QuietNaN();
      t.effLoop = TMath::QuietNaN();
      t.detID = -1;
      t.detRowID = -1;
      t.loop = -1;
   }

   DetGeo detGeo;
   Array array;
   
   trajectory orbitb, orbitB;
   
   double e,detX ; ///energy of light recoil, position X
   double rhoHit;  /// radius of particle-b hit on recoil detector
   
   double eB;  ///energy of heavy recoil
   
   bool isDetReady;
   
   double xOff, yOff; // beam position


   bool overrideDetDistance;
   bool overrideFirstPos;
   bool isCoincidentWithRecoil;
   
   const double c = 299.792458; //mm/ns
};

HELIOS::HELIOS(){
   
   ClearTrajectory(orbitb);
   ClearTrajectory(orbitB);
   
   e = TMath::QuietNaN();
   eB = TMath::QuietNaN();
   detX = TMath::QuietNaN();
   rhoHit = TMath::QuietNaN();
   
   xOff = 0.0;
   yOff = 0.0;
   
   isDetReady = false;

   overrideDetDistance = false;
   overrideFirstPos = false;
   isCoincidentWithRecoil = false;
   
}

HELIOS::~HELIOS(){
  
}

bool HELIOS::SetDetectorGeometry(std::string filename){

   if( detGeo.LoadDetectorGeo(filename)) {

    if( detGeo.use2ndArray ){
      array = detGeo.array2;
    }else{
      array = detGeo.array1;
    }

    isCoincidentWithRecoil = detGeo.isCoincidentWithRecoil;

    isDetReady = true;
   }else{
    printf("cannot read file %s.\n", filename.c_str());
    isDetReady = false;
   }
   
   return isDetReady;  
}

int HELIOS::DetAcceptance(){
   
   //CalArrayHit and CalRecoilHit must be done before.
   
   if( isDetReady == false ) return 0;

   // -1 ========= when recoil direction is not same side of array
   if( array.firstPos < 0 && orbitb.z > 0 ) return -1;
   if( array.firstPos > 0 && orbitb.z < 0 ) return -1;

   // -11 ======== rho is too small
   if(  2 * orbitb.rho < array.detPerpDist ) return -11; 
   
   // -15 ========= if detRowID == -1, should be (2 * orbitb.rho < perpDist)
   if( orbitb.detRowID == -1 ) return -15;
   
   // -10 =========== when rho is too big . 
   if( detGeo.bore < 2 * orbitb.rho) return -10; 
   
   // -14 ========== check particle-B hit radius on recoil dectector
   if( isCoincidentWithRecoil && orbitB.R > detGeo.recoilOuterRadius  ) return -14;
   //if( isCoincidentWithRecoil && (orbitB.R > rhoRecoilout || orbitB.R < rhoRecoilin) ) return -14;
   
   // -12 ========= check is particle-b was blocked by recoil detector
   rhoHit = GetR(detGeo.recoilPos);
   if( orbitb.z > 0 && detGeo.recoilPos > 0 && orbitb.z > detGeo.recoilPos && rhoHit < detGeo.recoilOuterRadius ) return -12;
   if( orbitb.z < 0 && detGeo.recoilPos < 0 && orbitb.z < detGeo.recoilPos && rhoHit < detGeo.recoilOuterRadius ) return -12;
   
   // -13 ========= not more than 3 loops
   if( orbitb.loop > 3 ) return -13;
   
   // -2 ========= calculate the "y"-distance from detector center
   if( sqrt(orbitb.R*orbitb.R - array.detPerpDist * array.detPerpDist)> array.detWidth/2 ) return -2; 
   
   // -3 ==== when zPos further the range of whole array, more loop would not save
   if( array.firstPos < 0 && orbitb.z < array.detPos[0] - array.detLength ) return -3; 
   if( array.firstPos > 0 && orbitb.z > array.detPos[array.nDet-1] + array.detLength ) return -3; 

   // -4 ======== Hit on blacker
   if( array.blocker != 0 && array.firstPos > 0 && array.detPos[0] - array.blocker  < orbitb.z && orbitb.z < array.detPos[0] ) return -4; 
   if( array.blocker != 0 && array.firstPos < 0 && array.detPos[array.nDet-1]  < orbitb.z && orbitb.z < array.detPos[array.nDet-1] + array.blocker ) return -4; 

   // 2 ======  when zPos less then the nearest position, more loop may hit
   int increaseLoopFlag = 0;
   if( array.firstPos < 0 && array.detPos[array.nDet-1] < orbitb.z ) increaseLoopFlag = 2; 
   if( array.firstPos > 0 && array.detPos[0] > orbitb.z ) increaseLoopFlag = 2; 
   if (increaseLoopFlag == 2 ) {
      orbitb.z += orbitb.z0;
      orbitb.effLoop += 1.0;
      orbitb.loop += 1;
      orbitb.t = orbitb.t0 * orbitb.effLoop;
      return 2;
   }
   
   // 1 ======= check hit array z- position
   if( array.firstPos < 0 ){
      for( int i = 0; i < array.nDet; i++){
         if( array.detPos[i] - array.detLength <= orbitb.z && orbitb.z <= array.detPos[i]) {
            orbitb.detID = i;
            detX = ( orbitb.z - (array.detPos[i] + array.detLength/2 ))/ array.detLength * 2 ;// range from -1 , 1 
            return 1;
         }
      }      
   }else{
      for( int i = 0; i < array.nDet ; i++){
         if( array.detPos[i] <= orbitb.z && orbitb.z <= array.detPos[i] + array.detLength)  {
            ///printf(" %d | %f < z = %f < %f \n", i,  array.detPos[i], orbitb.z, array.detPos[i]+length); 
            orbitb.detID = i;
            detX = ( orbitb.z - (array.detPos[i] - array.detLength/2 ))/ array.detLength*2 ;// range from -1 , 1 
            return 1;
         }
      }
   }

   
   // -5 ======== check hit array gap
   if( array.firstPos < 0 ){
      for( int i = 0; i < array.nDet-1 ; i++){
         if( array.detPos[i] < orbitb.z && orbitb.z < array.detPos[i+1] - array.detLength ) return -5; //increaseLoopFlag = 3; 
      }      
   }else{
      for( int i = 0; i < array.nDet-1 ; i++){
         if( array.detPos[i] + array.detLength < orbitb.z && orbitb.z < array.detPos[i+1]  ) return -5; //increaseLoopFlag = 3;
      }
   }
   if (increaseLoopFlag == 3 ) {
      orbitb.z += orbitb.z0;
      orbitb.effLoop += 1.0;
      orbitb.loop += 1;
      orbitb.t = orbitb.t0 * orbitb.effLoop;
      return 3;
   }
   
   
   return -20; // for unknown reason
}

void HELIOS::CalTrajectoryPara(TLorentzVector P, int Z, int id){
   
   if( id == 0 ){
      orbitb.theta = P.Theta();
      orbitb.phi = P.Phi();
      orbitb.rho = P.Pt() / abs(detGeo.Bfield) / Z / c * 1000; //mm
      orbitb.vt = P.Beta() * TMath::Sin(P.Theta()) * c ; // mm / nano-second  
      orbitb.vp = P.Beta() * TMath::Cos(P.Theta()) * c ; // mm / nano-second  
      orbitb.t0 = TMath::TwoPi() * orbitb.rho / orbitb.vt; // nano-second  
      orbitb.z0 = orbitb.vp * orbitb.t0;
      
      orbitb.detID = -1;
      orbitb.detRowID = -1;
   
   }
   
   if( id == 1 ){
      orbitB.theta = P.Theta();
      orbitB.phi = P.Phi();
      orbitB.rho = P.Pt() / abs(detGeo.Bfield) / Z / c * 1000; //mm
      orbitB.vt = P.Beta() * TMath::Sin(P.Theta()) * c ; // mm / nano-second  
      orbitB.vp = P.Beta() * TMath::Cos(P.Theta()) * c ; // mm / nano-second  
      orbitB.t0 = TMath::TwoPi() * orbitB.rho / orbitB.vt; // nano-second  
      orbitB.z0 = orbitB.vp * orbitB.t0;
      
      orbitB.detID = -1;
      orbitB.detRowID = -1;
   
   }
}

int HELIOS::CalArrayHit(TLorentzVector Pb, int Zb){
   
   e = Pb.E() - Pb.M();
   detX = TMath::QuietNaN();
   rhoHit = TMath::QuietNaN();
   
   CalTrajectoryPara(Pb, Zb, 0);
   
   int targetLoop = 1;
   int inOut = array.detFaceOut == true ? 1: 0; //1 = from Outside, 0 = from inside
   
   bool debug = false;
   
   if( debug ) {
      printf("===================================\n");
      printf("theta : %f deg, phi : %f deg \n", orbitb.theta * TMath::RadToDeg(), orbitb.phi * TMath::RadToDeg());
      printf("z0: %f mm, rho : %f mm \n", orbitb.z0, orbitb.rho);
      printf("      inOut : %d = %s \n", inOut, inOut == 1 ? "Out" : "in");
      printf(" z range : %.2f - %.2f \n", detGeo.zMin, detGeo.zMax);
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
      double hahaha = asin( aEff/ orbitb.rho - detGeo.BfieldTheta * sin(dphi));
      
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
      if( !TMath::IsNaN(zP) && 0< zP/orbitb.z0 && TMath::Max(0, targetLoop-1) < zP/orbitb.z0 && zP/orbitb.z0 < targetLoop ) {
         zPossible.push_back(zP);
         dID.push_back(i);
      }
   }
   /*
   if( zPossible.size() == 0 ){ // will not happen
      zHit = TMath::QuietNaN();
      xPos = TMath::QuietNaN();
      yPos = TMath::QuietNaN();
      loop = -1;
      detID = -1;
      detRowID = -1;
      return -1 ;
   }*/
   
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
   
   return 1; // return 1 when OK
}

int HELIOS::CalRecoilHit(TLorentzVector PB, int ZB){
   
   CalTrajectoryPara(PB, ZB, 1);
   
   orbitB.z = detGeo.recoilPos;
   orbitB.x = GetRecoilXPos(detGeo.recoilPos)  ;
   orbitB.y = GetRecoilYPos(detGeo.recoilPos)  ;
   orbitB.R = GetRecoilR(detGeo.recoilPos);
   orbitB.effLoop = orbitB.z/orbitB.z0;
   orbitB.t  = orbitB.t0 * orbitB.effLoop ;
   
   return 1;
}

#endif 