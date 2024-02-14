#ifndef Transfer_h
#define Transfer_h

#include "utility"

#include "ClassIsotope.h"
#include "../Armory/AnalysisLib.h"
#include "../Armory/ClassReactionConfig.h"

#include "TLorentzVector.h"
#include "TMath.h"

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

  void SetReactionSimple(int beamA, int beamZ,
                         int targetA, int targetZ,
                         int recoilA, int recoilZ, float beamEnergy_AMeV);

  void SetExA(double Ex);
  void SetExB(double Ex);
  void SetReactionFromFile(string reactionConfigFile);
  
  TString GetReactionName();
  TString GetReactionName_Latex();

  ReactionConfig GetRectionConfig() { return reaction;}

  double GetMass_A() const {return mA + ExA;}
  double GetMass_a() const {return ma;}
  double GetMass_b() const {return mb;}
  double GetMass_B() const {return mB + ExB;}

  int GetCharge_b() const {return reaction.recoilLightZ;}
  
  double GetCMTotalKE() {return Etot - mA - ma;}
  double GetQValue()    {return mA + ExA + ma - mb - mB - ExB;}
  double GetMaxExB()    {return Etot - mb - mB;}
  
  TLorentzVector GetPA(){return PA;}
  TLorentzVector GetPa(){return Pa;}
  TLorentzVector GetPb(){return Pb;}
  TLorentzVector GetPB(){return PB;}
  
  void CalReactionConstant();

  void Event(double thetaCM_rad, double phiCM_rad);
  
  double GetMomentumbCM()   {return p;}
  double GetReactionBeta()  {return beta;}
  double GetReactionGamma() {return gamma;}
  double GetCMTotalEnergy() {return Etot;}

  std::pair<double, double> CalExThetaCM(double e, double z, double Bfield, double a); 
  
private:

  ReactionConfig reaction;

  string nameA, namea, nameb, nameB;
  double thetaIN, phiIN;
  double mA, ma, mb, mB;

  double TA, T; // TA = KE of A pre u, T = total energy
  double ExA, ExB;
  
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
  T = TA * reaction.beamA;
  
  ExA = 0;
  ExB = 0;
  
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
  reaction.beamA = A;
  reaction.beamZ = Z;
  ExA = Ex;
  nameA = temp.Name;
  isReady = false;
  isBSet = true;
}

void TransferReaction::Seta(int A, int Z){
  Isotope temp (A, Z);
  ma = temp.Mass;
  reaction.targetA = A;
  reaction.targetZ = Z;
  namea = temp.Name;
  isReady = false;
  isBSet = false;
}
void TransferReaction::Setb(int A, int Z){
  Isotope temp (A, Z);
  mb = temp.Mass;
  reaction.recoilLightA = A;
  reaction.recoilLightZ = Z;
  nameb = temp.Name;
  isReady = false;
  isBSet = false;
}
void TransferReaction::SetB(int A, int Z){
  Isotope temp (A, Z);
  mB = temp.Mass;
  reaction.recoilHeavyA = A;
  reaction.recoilHeavyZ = Z;
  nameB = temp.Name;
  isReady = false;
  isBSet = true;
}

void TransferReaction::SetIncidentEnergyAngle(double KEA, double theta, double phi){
  this->TA = KEA;
  this->T = TA * reaction.beamA;
  this->thetaIN = theta;
  this->phiIN = phi;
  isReady = false;
}

void TransferReaction::SetReactionSimple(int beamA, int beamZ,
                   int targetA, int targetZ,
                   int recoilA, int recoilZ, float beamEnergy_AMeV){

  reaction.SetReactionSimple(beamA, beamZ,
                             targetA, targetZ,
                             recoilA, recoilZ, beamEnergy_AMeV);

  SetA(reaction.beamA, reaction.beamZ);
  Seta(reaction.targetA, reaction.targetZ);
  Setb(reaction.recoilLightA, reaction.recoilLightZ);
  SetB(reaction.recoilHeavyA, reaction.recoilHeavyZ);
  SetIncidentEnergyAngle(reaction.beamEnergy, 0, 0);

  CalReactionConstant();

}

void TransferReaction::SetExA(double Ex){
  this->ExA = Ex;
  isReady = false;
}

void TransferReaction::SetExB(double Ex){
  this->ExB = Ex;
  isReady = false;
}

void TransferReaction::SetReactionFromFile(string reactionConfigFile){

  if( reaction.LoadReactionConfig(reactionConfigFile) ){

    SetA(reaction.beamA, reaction.beamZ);
    Seta(reaction.targetA, reaction.targetZ);
    Setb(reaction.recoilLightA, reaction.recoilLightZ);
    SetB(reaction.recoilHeavyA, reaction.recoilHeavyZ);
    SetIncidentEnergyAngle(reaction.beamEnergy, 0, 0);

    CalReactionConstant();

  }else{     
    printf("cannot read file %s.\n", reactionConfigFile.c_str());
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
    reaction.recoilHeavyA = reaction.beamA + reaction.targetA - reaction.recoilLightA;
    reaction.recoilHeavyZ = reaction.beamZ + reaction.targetZ - reaction.recoilLightZ;
    Isotope temp (reaction.recoilHeavyA, reaction.recoilHeavyZ);
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

void TransferReaction::Event(double thetaCM_rad, double phiCM_rad){

  if( isReady == false ){
    CalReactionConstant();
  }

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
    vb.Rotate(thetaCM_rad, ub);
    vb.Rotate(phiCM_rad + TMath::PiOver2(), va); // somehow, the calculation turn the vector 90 degree.
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
  Pb = Pbc; Pb.Boost(b);
  PB = PBc; PB.Boost(b);
   
}


std::pair<double, double> TransferReaction::CalExThetaCM(double e, double z, double Bfield, double perpDist){

  double Ex = TMath::QuietNaN();
  double thetaCM = TMath::QuietNaN();

  double mass = mb;
  double massB = mB;
  double y = e + mass;
  double slope = 299.792458 * reaction.recoilLightZ * abs(Bfield) / TMath::TwoPi() * beta / 1000.; // MeV/mm;
  double alpha = slope/beta;
  double G =  alpha * gamma * beta * perpDist ;
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
    }

  }else{
    Ex = TMath::QuietNaN();
    thetaCM = TMath::QuietNaN();  
    //thetaLab = TMath::QuietNaN();
  }
  
  return std::make_pair(Ex, thetaCM); 
}


#endif