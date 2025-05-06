#ifndef knockout_h
#define knockout_h


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