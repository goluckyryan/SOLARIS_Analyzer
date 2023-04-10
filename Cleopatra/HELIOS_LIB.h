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

#include "Isotope.h"
#include "../armory/AnalysisLib.h"

//=======================================================
//#######################################################
// Class for Transfer Reaction
// reaction notation A(a,b)B
// A = incident particle
// a = target
// b = light scattered particle
// B = heavy scattered particle
//======================================================= 
class TransferReaction {
public:
   TransferReaction();
   ~TransferReaction();
  
   void SetA(int A, int Z, double Ex);
   void Seta(int A, int Z);
   void Setb(int A, int Z);
   void SetB(int A, int Z);
   void SetIncidentEnergyAngle(double KEA, double theta, double phi);
   void SetExA(double Ex);
   void SetExB(double Ex);
   void SetReactionFromFile(string settingFile);
   
   TString GetReactionName();
   TString GetReactionName_Latex();
   
   int GetAtomicNumber_A(){return AA;}
   int GetAtomicNumber_a(){return Aa;}
   int GetAtomicNumber_b(){return Ab;}
   int GetAtomicNumber_B(){return AB;}
   
   double GetMass_A(){return mA + ExA;}
   double GetMass_a(){return ma;}
   double GetMass_b(){return mb;}
   double GetMass_B(){return mB + ExB;}
   
   int GetCharge_A(){return zA;}
   int GetCharge_a(){return za;}
   int GetCharge_b(){return zb;}
   int GetCharge_B(){return zB;}
   
   double GetCMTotalKE() {return Etot - mA - ma;}
   double GetQValue()    {return mA + ExA + ma - mb - mB - ExB;}
   double GetMaxExB()    {return Etot - mb - mB;}
   
   TLorentzVector GetPA(){return PA;}
   TLorentzVector GetPa(){return Pa;}
   TLorentzVector GetPb(){return Pb;}
   TLorentzVector GetPB(){return PB;}
   
   void CalReactionConstant();
   int CalExThetaCM(double e, double z, double Bfield, double a); // return 0 for no-result, 1 for OK.
   
   TLorentzVector * Event(double thetaCM, double phiCM);
   
   double GetEx(){return Ex;}
   double GetThetaCM(){return thetaCM;}
   
   double GetMomentumbCM()   {return p;}
   double GetReactionBeta()  {return beta;}
   double GetReactionGamma() {return gamma;}
   double GetCMTotalEnergy() {return Etot;}

   AnalysisLib::ReactionConfig GetRectionConfig() { return reaction;}
   
private:

   AnalysisLib::ReactionConfig reaction;

   string nameA, namea, nameb, nameB;
   double thetaIN, phiIN;
   double mA, ma, mb, mB;
   int AA, Aa, Ab, AB;
   int zA, za, zb, zB;
   double TA, T; // TA = KE of A pre u, T = total energy
   double ExA, ExB;
   double Ex, thetaCM; //calculated Ex using inverse mapping from e and z to thetaCM
   
   bool isReady;
   bool isBSet;
   
   double k; // CM Boost momentum
   double beta, gamma; //CM boost beta
   double Etot;
   double p; // CM frame momentum of b, B
   
   TLorentzVector PA, Pa, Pb, PB;

   TString format(TString name);
   
};

TransferReaction::TransferReaction(){
  
   thetaIN = 0.;
   phiIN = 0.;
   SetA(12, 6, 0);
   Seta(2,1);
   Setb(1,1);
   SetB(13,6);
   TA = 6;
   T = TA * AA;
   
   ExA = 0;
   ExB = 0;
   
   Ex = TMath::QuietNaN();
   thetaCM = TMath::QuietNaN();
   
   CalReactionConstant();
   
   TLorentzVector temp (0,0,0,0);
   PA = temp;
   Pa = temp;
   Pb = temp;
   PB = temp;
   
}

TransferReaction::~TransferReaction(){

}

void TransferReaction::SetA(int A, int Z, double Ex = 0){
  Isotope temp (A, Z);
  mA = temp.Mass;
  AA = A;
  zA = Z;
  ExA = Ex;
  nameA = temp.Name;
  isReady = false;
  isBSet = true;
  
}

void TransferReaction::Seta(int A, int Z){
  Isotope temp (A, Z);
  ma = temp.Mass;
  Aa = A;
  za = Z;
  namea = temp.Name;
  isReady = false;
  isBSet = false;
}

void TransferReaction::Setb(int A, int Z){
  Isotope temp (A, Z);
  mb = temp.Mass;
  Ab = A;
  zb = Z;
  nameb = temp.Name;
  isReady = false;
  isBSet = false;
}
void TransferReaction::SetB(int A, int Z){
  Isotope temp (A, Z);
  mB = temp.Mass;
  AB = A;
  zB = Z;
  nameB = temp.Name;
  isReady = false;
  isBSet = true;
}
void TransferReaction::SetIncidentEnergyAngle(double KEA, double theta, double phi){
  this->TA = KEA;
  this->T = TA * AA;
  this->thetaIN = theta;
  this->phiIN = phi;
  isReady = false;
}

void TransferReaction::SetExA(double Ex){
  this->ExA = Ex;
  isReady = false;
}

void TransferReaction::SetExB(double Ex){
  this->ExB = Ex;
  isReady = false;
}

void TransferReaction::SetReactionFromFile(string settingFile){

   TMacro * haha = new TMacro();
   if( haha->ReadFile(settingFile.c_str()) > 0 ) {
      reaction = AnalysisLib::LoadReactionConfig(haha);

     SetA(reaction.beamA, reaction.beamZ);
     Seta(reaction.targetA, reaction.targetZ);
     Setb(reaction.recoilLightA, reaction.recoilLightZ);
     SetB(reaction.recoilHeavyA, reaction.recoilHeavyZ);

     SetIncidentEnergyAngle(reaction.beamEnergy, 0, 0);
     CalReactionConstant();
   }else{
     
     printf("cannot read file %s.\n", settingFile.c_str());
     isReady = false;
   }
   
}

TString TransferReaction::GetReactionName(){
  TString rName;
  rName.Form("%s(%s,%s)%s", nameA.c_str(), namea.c_str(), nameb.c_str(), nameB.c_str()); 
  return rName;
}

TString TransferReaction::format(TString name){
  if( name.IsAlpha() ) return name;
  int len = name.Length();
  TString temp = name;
  TString temp2 = name;
  if( temp.Remove(0, len-2).IsAlpha()){
     temp2.Remove(len-2);
  }else{
     temp = name;
     temp.Remove(0, len-1);
     temp2.Remove(len-1);
  }
  return "^{"+temp2+"}"+temp;
}
TString TransferReaction::GetReactionName_Latex(){
  TString rName;
  rName.Form("%s(%s,%s)%s", format(nameA).Data(), format(namea).Data(), format(nameb).Data(), format(nameB).Data()); 
  return rName;
}

void TransferReaction::CalReactionConstant(){
   if( !isBSet){
      AB = AA + Aa - Ab;
      zB = zA + za - zb;
      Isotope temp (AB, zB);
      mB = temp.Mass;
      isBSet = true;
   }
   
   k = TMath::Sqrt(TMath::Power(mA + ExA + T, 2) - (mA + ExA) * (mA + ExA)); 
   beta = k / (mA + ExA + ma + T);
   gamma = 1 / TMath::Sqrt(1- beta * beta);   
   Etot = TMath::Sqrt(TMath::Power(mA + ExA + ma + T,2) - k * k);
   p = TMath::Sqrt( (Etot*Etot - TMath::Power(mb + mB + ExB,2)) * (Etot*Etot - TMath::Power(mb - mB - ExB,2)) ) / 2 / Etot;
   
   PA.SetXYZM(0, 0, k, mA + ExA);
   PA.RotateY(thetaIN);
   PA.RotateZ(phiIN);
   
   Pa.SetXYZM(0,0,0,ma);
   
   isReady = true;
}

TLorentzVector * TransferReaction::Event(double thetaCM, double phiCM)
{
   if( isReady == false ){
      CalReactionConstant();
   }

   //TLorentzVector Pa(0, 0, 0, ma);
   
   //---- to CM frame
   TLorentzVector Pc = PA + Pa;
   TVector3 b = Pc.BoostVector();
   
   TVector3 vb(0,0,0);
   
   if( b.Mag() > 0 ){
     TVector3 v0 (0,0,0);
     TVector3 nb = v0 - b;
     
     TLorentzVector PAc = PA; 
     PAc.Boost(nb);
     TVector3 vA = PAc.Vect();
     
     TLorentzVector Pac = Pa;
     Pac.Boost(nb);
     TVector3 va = Pac.Vect();
     
     //--- construct vb
     vb = va;
     vb.SetMag(p);

     TVector3 ub = vb.Orthogonal();
     vb.Rotate(thetaCM, ub);
     vb.Rotate(phiCM + TMath::PiOver2(), va); // somehow, the calculation turn the vector 90 degree.
     //vb.Rotate(phiCM , va); // somehow, the calculation turn the vector 90 degree.
   }
   
   //--- from Pb
   TLorentzVector Pbc;
   Pbc.SetVectM(vb, mb);
   
   //--- from PB
   TLorentzVector PBc;
   //PBc.SetVectM(vB, mB + ExB);
   PBc.SetVectM(-vb, mB + ExB);
   
   //---- to Lab Frame
   TLorentzVector Pb = Pbc;
   Pb.Boost(b);
   TLorentzVector PB = PBc;
   PB.Boost(b);
   
   TLorentzVector * output = new TLorentzVector[4];
   output[0] = PA;
   output[1] = Pa;
   output[2] = Pb;
   output[3] = PB;
   
   this->Pb = Pb;
   this->PB = PB;
   
   
   return output;   
}


int TransferReaction::CalExThetaCM(double e, double z, double Bfield, double a){
   double mass = mb;
   double massB = mB;
   double y = e + mass;
   double slope = 299.792458 * zb * abs(Bfield) / TMath::TwoPi() * beta / 1000.; // MeV/mm;
   double alpha = slope/beta;
   double G =  alpha * gamma * beta * a ;
   double Z = alpha * gamma * beta * z;
   double H = TMath::Sqrt(TMath::Power(gamma * beta,2) * (y*y - mass * mass) ) ;
   double Et = Etot;

   if( TMath::Abs(Z) < H ) {            
      //using Newton's method to solve 0 ==  H * sin(phi) - G * tan(phi) - Z = f(phi) 
      double tolerrence = 0.001;
      double phi = 0; //initial phi = 0 -> ensure the solution has f'(phi) > 0
      double nPhi = 0; // new phi

      int iter = 0;
      do{
         phi = nPhi;
         nPhi = phi - (H * TMath::Sin(phi) - G * TMath::Tan(phi) - Z) / (H * TMath::Cos(phi) - G /TMath::Power( TMath::Cos(phi), 2));               
         iter ++;
         if( iter > 10 || TMath::Abs(nPhi) > TMath::PiOver2()) break;
      }while( TMath::Abs(phi - nPhi ) > tolerrence);

      phi = nPhi;

      // check f'(phi) > 0
      double Df = H * TMath::Cos(phi) - G / TMath::Power( TMath::Cos(phi),2);
      if( Df > 0 && TMath::Abs(phi) < TMath::PiOver2()  ){
         double K = H * TMath::Sin(phi);
         double x = TMath::ACos( mass / ( y * gamma - K));
         double k = mass * TMath::Tan(x); // momentum of particel b or B in CM frame
         double EB = TMath::Sqrt(mass*mass + Et*Et - 2*Et*TMath::Sqrt(k*k + mass * mass));
         Ex = EB - massB;

         double hahaha1 = gamma* TMath::Sqrt(mass * mass + k * k) - y;
         double hahaha2 = gamma* beta * k;
         thetaCM = TMath::ACos(hahaha1/hahaha2) * TMath::RadToDeg();

         //double pt = k * TMath::Sin(thetaCM * TMath::DegToRad());
         //double pp = gamma*beta*TMath::Sqrt(mass*mass + k*k) - gamma * k * TMath::Cos(thetaCM * TMath::DegToRad());
         //thetaLab = TMath::ATan(pt/pp) * TMath::RadToDeg();

      }else{
         Ex = TMath::QuietNaN();
         thetaCM = TMath::QuietNaN();
         //thetaLab = TMath::QuietNaN();
         
         return 0;
      }

   }else{
      Ex = TMath::QuietNaN();
      thetaCM = TMath::QuietNaN();  
      //thetaLab = TMath::QuietNaN();
      return 0;
   }
   
   return 1; 
}
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
   bool SetDetectorGeometry(string filename);
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

   AnalysisLib::DetGeo GetDetectorGeometry() {return detGeo;}
   
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

   AnalysisLib::DetGeo detGeo;
   AnalysisLib::Array array;
   
   trajectory orbitb, orbitB;
   
   double e,detX ; ///energy of light recoil, position X
   double rhoHit;  /// radius of particle-b hit on recoil detector
   
   double eB;  ///energy of heavy recoil
   
   bool isDetReady;
   
   double xOff, yOff; // beam position
   
   //detetcor Geometry
   //int sign ; // sign of B-field
   // double Bfield; // T
   // double BfieldTheta; // rad, 0 = z-axis, pi/2 = y axis, pi = -z axis
   // double bore; // bore , mm
   // double perpDist; // distance from axis
   // double width; // width   
   // double posRecoil; // recoil, downstream
   // double rhoRecoilin; // radius recoil inner
   // double rhoRecoilout; // radius recoil outter
   // double length; // length
   // double blocker;
   // double firstPos; // m 
   // vector<double> pos; // near position in m
   // int nDet, mDet; // nDet = number of different pos, mDet, number of same pos   
   // bool isFromOutSide;

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

bool HELIOS::SetDetectorGeometry(string filename){

   TMacro * haha = new TMacro();
   if( haha->ReadFile(filename.c_str()) > 0 ) {
    detGeo = AnalysisLib::LoadDetectorGeo(haha);

    if( detGeo.use2ndArray ){
      array = detGeo.array2;
    }else{
      array = detGeo.array1;
    }

    PrintDetGeo(detGeo, false);

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

   vector<double> zPossible;
   vector<int> dID; //detRowID
   
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

//=======================================================
//#######################################################
// Class for multi-scattering of the beam inside target
// using SRIM to generate stopping power
// input : TLorentzVector, data_files
// output : scattered TLorentzVector
//=======================================================
class TargetScattering{

public:
   TargetScattering();
   ~TargetScattering();
   
   double GetKE0(){return KE0;}
   double GetKE() {return KE;}
   double GetKELoss() {return KE0-KE;}
   double GetDepth() {return depth;}
   double GetPathLength() {return length;}
   
   vector<string> SplitStr(string tempLine, string splitter, int shift = 0);
   void LoadStoppingPower(string file);
   
   void SetTarget(double density, double depth){
      this->density = density;
      this->depth = depth;
      isTargetSet = true;
      //printf("===== Target, density: %f g/cm3, depth: %f um \n", density, depth * 1e+4 );
   }
   
   TLorentzVector Scattering(TLorentzVector P){
      double mass = P.M();
      KE0 = (P.E() - mass);
      KE = KE0;
      double theta = P.Theta();   
      this->length = TMath::Abs(depth/TMath::Cos(theta));
      //printf("------- theta: %f deg, length: %f um, KE: %f MeV \n", theta * TMath::RadToDeg(), this->length * 1e+4, KE);
      
      //integrate the energy loss within the depth of A
      double dx = length/100.; // in cm
      double x = 0;
      double densityUse = density;
      if( unitID == 0 ) densityUse = 1.;
      do{
         //assume the particle will not be stopped
         //printf(" x: %f, KE:  %f, S: %f \n", x, KE, gA->Eval(KE));
         KE = KE - densityUse * gA->Eval(KE) * 10 * dx  ; // factor 10, convert MeV/cm -> MeV/cm
         
         //angular Straggling, assume (Lateral Straggling)/(Projected range)
         
         
         x = x + dx;
      }while(x < length && KE > 0 );
      
      //printf(" depth: %f cm = %f um, KE : %f -> %f MeV , dE = %f MeV \n", depth, depth * 1e+4, KE0, KE, KE0 - KE);
      double newk = 0;
      
      TLorentzVector Pnew;
      TVector3 vb(0,0,0);
      
      if( KE < 0 ) {
        KE = 0.0; // somehow, when KE == 0 , the program has problem to rotate the 4-vector
      }else{
        newk = TMath::Sqrt(TMath::Power(mass+KE,2) - mass * mass);
        vb = P.Vect();
        vb.SetMag(newk);      
      }
      Pnew.SetVectM(vb,mass);

      return Pnew;
   }
   
private:
   bool isTargetSet;
   double density; // density [mg/cm2]
   int unitID; // 0 = MeV /mm or keV/um , 1 = MeV / (ug/cm2) 
   
   double depth; // depth in target [cm]
   double length; // total path length in target [cm]
   double KE0, KE;
   
   TGraph * gA; // stopping power of A, b, B, in unit of MeV/(mg/cm2)
   TGraph * gB; // projection range [nm]
   TGraph * gC; // parallel Straggling [nm]
   TGraph * gD; // perpendicular Straggling [nm]
      
};

TargetScattering::TargetScattering(){
   isTargetSet = false;
   density = 1; // mg/cm2
   unitID = 0; 
   KE0 = 0;
   KE = 0;
   depth = 0;
   length = 0;
   gA = NULL;
   gB = NULL;
   gC = NULL;
   gD = NULL;
}

TargetScattering::~TargetScattering(){
   delete gA;
}

vector<string> TargetScattering::SplitStr(string tempLine, string splitter, int shift){

  vector<string> output;

  size_t pos;
  do{
    pos = tempLine.find(splitter); // fine splitter
    if( pos == 0 ){ //check if it is splitter again
      tempLine = tempLine.substr(pos+1);
      continue;
    }

    string secStr;
    if( pos == string::npos ){
      secStr = tempLine;
    }else{
      secStr = tempLine.substr(0, pos+shift);
      tempLine = tempLine.substr(pos+shift);
    }

    //check if secStr is begin with space
    while( secStr.substr(0, 1) == " "){
      secStr = secStr.substr(1);
    }
    
    if( !secStr.empty() ) output.push_back(secStr);
    //printf(" |%s---\n", secStr.c_str());
    
  }while(pos != string::npos );

  return output;
}

void TargetScattering::LoadStoppingPower(string filename){
   
   printf("loading Stopping power: %s.", filename.c_str());
   
   ifstream file;
   file.open(filename.c_str());
   
   vector<double> energy;
   vector<double> stopping;
   vector<double> projRange;
   vector<double> paraStraggling;
   vector<double> prepStraggling;
   
   if( file.is_open() ){
      printf("... OK\n");
      char lineChar[16635];
      string line;
      while( !file.eof() ){
         file.getline(lineChar, 16635, '\n');
         line = lineChar;

         size_t found = line.find("Target Density");
         if( found != string::npos ){
            printf("    %s\n", line.c_str());
         }
         
         found = line.find("Stopping Units =");
         if( found != string::npos){         
            printf("    %s\n", line.c_str());
            if( line.find("MeV / mm") != string::npos ) { 
               unitID = 0;
            }else if( line.find("keV / micron") != string::npos ){
               unitID = 0;
            }else if( line.find("MeV / (mg/cm2)") != string::npos ) {
               unitID = 1;
            }else{
               printf("please using MeV/mm, keV/um, or MeV/(mg/cm2) \n");  
            }
         }
         
         size_t foundkeV = line.find("keV   ");
         size_t foundMeV = line.find("MeV   ");
         size_t foundGeV = line.find("GeV   ");
         if ( foundkeV != string::npos || foundMeV != string::npos || foundGeV != string::npos ){
            vector<string> haha = SplitStr(line, "  ");
            //for( int i = 0 ; i < (int) haha.size()-1; i++){
            //   printf("%d,%s|", i, haha[i].c_str());
            //}
            //printf("\n");
         
            found = haha[0].find("keV"); if( found != string::npos ) energy.push_back(atof(haha[0].substr(0, found).c_str()) * 0.001);
            found = haha[0].find("MeV"); if( found != string::npos ) energy.push_back(atof(haha[0].substr(0, found).c_str()) * 1.);
            found = haha[0].find("GeV"); if( found != string::npos ) energy.push_back(atof(haha[0].substr(0, found).c_str()) * 1000.);
            
            double a = atof(haha[1].c_str());
            double b = atof(haha[2].c_str());
            stopping.push_back(a+b);
            
            found = haha[3].find("A") ; if( found != string::npos ) projRange.push_back(atof(haha[3].substr(0, found).c_str()) * 0.1);
            found = haha[3].find("um"); if( found != string::npos ) projRange.push_back(atof(haha[3].substr(0, found).c_str()) * 1000.);
            found = haha[3].find("mm"); if( found != string::npos ) projRange.push_back(atof(haha[3].substr(0, found).c_str()) * 1e6);
            
            found = haha[4].find("A") ; if( found != string::npos ) paraStraggling.push_back(atof(haha[4].substr(0, found).c_str()) * 0.1);
            found = haha[4].find("um"); if( found != string::npos ) paraStraggling.push_back(atof(haha[4].substr(0, found).c_str()) * 1e3);
            found = haha[4].find("mm"); if( found != string::npos ) paraStraggling.push_back(atof(haha[4].substr(0, found).c_str()) * 1e6);
            
            found = haha[5].find("A") ; if( found != string::npos ) prepStraggling.push_back(atof(haha[5].substr(0, found).c_str()) * 0.1);
            found = haha[5].find("um"); if( found != string::npos ) prepStraggling.push_back(atof(haha[5].substr(0, found).c_str()) * 1e3);
            found = haha[5].find("mm"); if( found != string::npos ) prepStraggling.push_back(atof(haha[5].substr(0, found).c_str()) * 1e6);
            
            //printf(" %f, %f, %f, %f, %f \n", energy.back(), stopping.back(), projRange.back(), paraStraggling.back(), prepStraggling.back());
         }
         
         
      }
      
   }else{
       printf("... fail\n");
   }
   
   gA = new TGraph(energy.size(), &energy[0], &stopping[0]);
   
   gB = new TGraph(energy.size(), &energy[0], &projRange[0]);
   gC = new TGraph(energy.size(), &energy[0], &paraStraggling[0]);
   gD = new TGraph(energy.size(), &energy[0], &prepStraggling[0]);
   
}
//=======================================================
//#######################################################
// Class for Particle Decay
// B --> d + D
//  input : TLorentzVector, emitting particle
// output : scattered TLorentzVector
//=======================================================
class Decay{
public:
   Decay();
   ~Decay();
   
   double GetQValue() { return Q;}
   
   double GetAngleChange(){
      TVector3 vD = PD.Vect();
      TVector3 vB = PB.Vect();
      vD.SetMag(1);
      vB.SetMag(1);
      double dot = vD.Dot(vB);
      return TMath::ACos(dot)*TMath::RadToDeg() ;
   }
   
   double GetThetaCM() { return theta * TMath::RadToDeg();}
    
   double GetCMMomentum(){ return k;}
   TLorentzVector GetDaugther_d() {return Pd;}
   TLorentzVector GetDaugther_D() {return PD;}
   
   void SetMotherDaugther(int AB, int zB, int AD, int zD){
      Isotope Mother(AB, zB);
      Isotope Daugther_D(AD, zD);
      Isotope Daugther_d(AB-AD, zB-zD);

      mB = Mother.Mass;
      mD = Daugther_D.Mass;
      md = Daugther_d.Mass;
      
      double Q = mB - mD - md;
      
      printf("====== decay mode : %s --> %s + %s, Q = %.3f MeV \n", Mother.Name.c_str(), Daugther_d.Name.c_str(), Daugther_D.Name.c_str(), Q);
      
      isMotherSet = true;
   }
   
   int CalDecay(TLorentzVector P_mother, double ExB, double ExD, double normOfReactionPlane = 0){
      if( !isMotherSet ) {
         return -1;
      }
      this->PB = P_mother;
      
      double MB = mB + ExB; ///mother
      double MD = mD + ExD; ///Big_Daugther
      Q = MB - MD - md;
      if( Q < 0 ) {
         this->PD = this->PB;
         dTheta = TMath::QuietNaN();
         k = TMath::QuietNaN();
         return -2;
      }
      
      //clear 
      TLorentzVector temp(0,0,0,0);
      PD = temp;
      Pd = temp;
      
      k = TMath::Sqrt((MB+MD+md)*(MB+MD-md)*(MB-MD+md)*(MB-MD-md))/2./MB;
      
      //in mother's frame, assume isotropic decay
      theta = TMath::ACos(2 * gRandom->Rndm() - 1) ; 
      
      //for non isotropic decay, edit f1. 
      //theta = TMath::ACos(f1->GetRandom());
      
      double phi = TMath::TwoPi() * gRandom->Rndm();
      PD.SetE(TMath::Sqrt(mD * mD + k * k ));
      PD.SetPz(k);
      PD.SetTheta(theta);
      PD.SetPhi(phi);
      
      Pd.SetE(TMath::Sqrt(md * md + k * k ));
      Pd.SetPz(k);
      Pd.SetTheta(theta + TMath::Pi());
      Pd.SetPhi(phi + TMath::Pi());
      
      PD.RotateY(TMath::Pi()/2.);
      PD.RotateZ(normOfReactionPlane);

      Pd.RotateY(TMath::Pi()/2.);
      Pd.RotateZ(normOfReactionPlane);
      
      //Transform to Lab frame;
      TVector3 boost = PB.BoostVector();
      
      PD.Boost(boost);
      Pd.Boost(boost);
      
      return 1;      
   }
   
private:
   TLorentzVector PB, Pd, PD;
   
   double mB, mD, md;
   double theta;
   
   TF1 * f1;
   
   bool isMotherSet;
   double Q;
   double k; // momentum in B-frame
   double dTheta; // change of angle
};

Decay::Decay(){
   TLorentzVector temp(0,0,0,0);
   PB = temp;
   Pd = temp;
   PD = temp;
  
   mB = TMath::QuietNaN();
   mD = TMath::QuietNaN();
   md = TMath::QuietNaN(); 
   theta = TMath::QuietNaN();
   
   k = TMath::QuietNaN();
   
   Q = TMath::QuietNaN();
   dTheta = TMath::QuietNaN();
   isMotherSet = false;
   
   f1 = new TF1("f1", "(1+ROOT::Math::legendre(2,x))/2.", -1, 1);
}

Decay::~Decay(){
   delete f1;
}

//=======================================================
//#######################################################
// Class for Knockout Reaction
// A(a,12)B, A = B + b, a->1, b->2
// incident particle is A
// the calculation: 1) go to A's rest frame
//                  2) calculate the b = A - B
//                  3) go to CM frame
//=======================================================
class Knockout{
public:
   Knockout();
   ~Knockout();
   
   void SetA(int A, int Z){
      Isotope temp(A,Z);
      mA = temp.Mass;
      AA = A;
      ZA = Z;
      nameA = temp.Name;
   }
   
   void SetExA(double Ex){
      this->ExA = Ex; 
   }
   
   void Seta(int A, int Z){
      Isotope temp(A,Z);
      ma = temp.Mass;
      Aa = A;
      Za = Z;
      m1 = ma;
      A1 = A;
      Z1 = Z;
      namea = temp.Name;
      name1 = temp.Name;
   }

   void Set2(int A, int Z){
      Isotope temp(A,Z);
      m2 = temp.Mass;
      A2 = A;
      Z2 = Z;
      name2 = temp.Name;
      
      AB = AA + Aa - A1 - A2;
      ZB = ZA + Za - Z1 - Z2;
      Isotope temp2(AB,ZB);
      mB0 = temp2.Mass;
      nameB = temp2.Name;
      
   }
   
   void SetBSpk(double S, double kb, double thetab, double phib){
      this->S = S;
      AB = AA + Aa - A1 - A2;
      ZB = ZA + Za - Z1 - Z2;
      Isotope temp(AB,ZB);
      mB0 = temp.Mass;
      nameB = temp.Name;
      this->kb = kb;
      this->thetab = thetab;
      this->phib = phib;
      
      mB = mA + ExA - m2 + S;
      
      ExB = mB - mB0;

      if( ExB < 0 && !isOverRideExNegative ){
         printf(" seperation energy is too small. \n");
      }
   }
   
   void OverRideExNegative(bool YN){
      isOverRideExNegative = YN;
   }   
   
   TString GetReactionName(){
      TString rName;
      
      TString normInv;
      if( isNormalKinematics ){
         normInv = "Normal Kinematics";
      }else{
         normInv = "Inverse Kinematics";
      }
      
      rName.Form("%s(%s,%s%s)%s, %s", nameA.c_str(), namea.c_str(), name1.c_str(), name2.c_str(), nameB.c_str(), normInv.Data()); 
   
      return rName;
   }
   
   void SetIncidentEnergyAngle(double KEA, double theta, double phi){
      this->KEA = KEA;
      this->thetaIN = theta;
      this->phiIN = phi;
   }
   
   void CalIncidentChannel(bool isNormalKinematics); 
   void CalReactionConstant(bool isNormalKinematics);
   void Event(double thetaCM, double phCM);
   
   double GetMass_A(){return mA;}
   double GetMass_a(){return ma;}
   double GetMass_b(){return mb;}
   double GetMass_B(){return mB;}
   double GetMass_Bgs(){return mB0;}
   double GetMass_1(){return m1;}
   double GetMass_2(){return m2;}
   
   TLorentzVector GetPA(){return PA;}
   TLorentzVector GetPa(){return Pa;}
   TLorentzVector GetPb(){return Pb;}
   TLorentzVector GetPB(){return PB;}
   TLorentzVector GetP1(){return P1;}
   TLorentzVector GetP2(){return P2;}
   
   double GetMomentumbNN(){return p;}
   double GetReactionBeta(){return beta;}
   double GetReactionGamma(){return gamma;}
   double GetNNTotalEnergy(){return Etot;}
   
   double GetNNTotalKE() {return Etot - mb - ma;}
   double GetQValue() {return mA + ExA - m2 - mB;}
   double GetMaxExB() {return Etot - mb - mB0;}
   
private:   
   int AA, Aa, A1, A2, AB;
   int ZA, Za, Z1, Z2, ZB;
   double mA, ma, m1, m2, mB, mB0, mb;
   string nameA, namea, name1, name2, nameB;
   
   double S; // separation energy
   double kb; // momentum of b
   double thetab, phib;// direction of b
   
   TLorentzVector PA, Pa, P1, P2, PB, Pb; // lab
   
   double KEA, thetaIN, phiIN;
   double T;
   
   double k, beta, gamma, Etot, p; // reaction constant, in NN frame
   double ExA, ExB;

   bool isNormalKinematics;
   bool isOverRideExNegative;
   
};

Knockout::Knockout(){
   TLorentzVector temp(0,0,0,0);
   
   PA = temp;
   Pa = temp;
   P1 = temp;
   P2 = temp;
   PB = temp;
   Pb = temp;
   
   SetA(12,6);
   Seta(1,1);
   Set2(1,1);
   
   SetBSpk(1000, 0, 0, 0);
   SetIncidentEnergyAngle(10, 0, 0);
   
   ExA = 0;
   
   isNormalKinematics = false;
   isOverRideExNegative = false;
}

Knockout::~Knockout(){

}

void Knockout::CalIncidentChannel(bool isNormalKinematics){
   if( ExB < 0 && !isOverRideExNegative) return;
   
   this->isNormalKinematics = isNormalKinematics;
   
   if(!isNormalKinematics){
      //===== construct Lab frame 4-momentum
      this->T = KEA * AA;
      double kA = TMath::Sqrt(TMath::Power(mA + ExA + T, 2) - (mA + ExA) * (mA + ExA)); 
      PA.SetXYZM(0, 0, kA, mA + ExA);
      PA.RotateY(thetaIN);
      PA.RotateZ(phiIN);
      
      Pa.SetXYZM(0,0,0,ma);
      
   }else{
      //===== construct 4-momentum
      this->T = KEA * Aa;
      double ka = TMath::Sqrt(TMath::Power(ma + T, 2) - (ma) * (ma)); 
      Pa.SetXYZM(0, 0, ka, ma);
      Pa.RotateY(thetaIN);
      Pa.RotateZ(phiIN);
      
      PA.SetXYZM(0, 0, 0, mA + ExA);  
   }

}

void Knockout::CalReactionConstant(bool isNormalKinematics){
   if( ExB < 0 && !isOverRideExNegative) return;
   
   this->isNormalKinematics = isNormalKinematics;
   
   CalIncidentChannel(isNormalKinematics);
   
   if(!isNormalKinematics){
      
      //===== change to A's rest frame
      TVector3 bA = PA.BoostVector();
      PA.Boost(-bA);
      
      //===== constructe bounded nucleus b
      PB.SetXYZM(0, 0, -kb, mB);
      PB.RotateY(thetab);
      PB.RotateZ(phib);
      
      Pb = PA - PB;
      mb = Pb.M();
   
      //===== change to Lab frame
      Pb.Boost(bA);
      PA.Boost(bA);
      PB.Boost(bA);
      
   }else{
      
      //===== constructe bounded nucleus b
      PB.SetXYZM(0, 0, -kb, mB);
      PB.RotateY(thetab);
      PB.RotateZ(phib);
      
      Pb = PA - PB;
      mb = Pb.M();
      
   }
   
   //====== reaction constant
   k = (Pa+Pb).P();
   double E = (Pa+Pb).E();
   beta = (Pa+Pb).Beta();
   gamma = 1 / TMath::Sqrt(1- beta * beta);   
   Etot = TMath::Sqrt(TMath::Power(E,2) - k * k);
   p = TMath::Sqrt( (Etot*Etot - TMath::Power(m1 + m2,2)) * (Etot*Etot - TMath::Power(m1 - m2 ,2)) ) / 2 / Etot;

	//if( TMath::IsNaN(p) ){
	//	printf(" Mc: %f, m1+m2: %f, kb:%f, thetab:%f, phib:%f\n", Etot, m1+m2, kb, thetab, phib); 
	//}

}

void Knockout::Event(double thetaCM, double phiCM){
   if( ExB < 0 && !isOverRideExNegative ) return;
   
   //===== construct Pcm
   TLorentzVector Pc = Pb + Pa;
   TVector3 bc = Pc.BoostVector();
   
   TLorentzVector Pac = Pa;
   Pac.Boost(-bc);
   TVector3 va = Pac.Vect();
   
   TLorentzVector Pbc = Pb;
   Pbc.Boost(-bc);
   TVector3 vb = Pbc.Vect();
   
   //--- from P1
   TVector3 v1 = va;
   v1.SetMag(p);
   
   TVector3 u1 = va.Orthogonal();
   v1.Rotate(thetaCM, u1);
   v1.Rotate(phiCM + TMath::PiOver2(), va); // somehow, the calculation turn the vector 90 degree.
   
   TLorentzVector P1c;
   P1c.SetVectM(v1, m1);
   
   //--- from P2
   TLorentzVector P2c;
   P2c.SetVectM(-v1, m2);
   
   //---- to Lab Frame
   P1 = P1c;
   P1.Boost(bc);
   P2 = P2c;
   P2.Boost(bc);
   
}


#endif 
