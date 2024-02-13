#ifndef decay_h
#define decay_h

#include "TVector3.h"

#include "../Cleopatra/ClassIsotope.h"

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


#endif