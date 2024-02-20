#include "../Cleopatra/ClassHelios.h"
#include "TROOT.h"
#include "TBenchmark.h"
#include "TLorentzVector.h"
#include "TMath.h"
#include "TFile.h"
#include "TF1.h"
#include "TTree.h"
#include "TRandom.h"
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <TObjArray.h>

//----------- usage 
// $root transfer.C+ | tee output.txt
// this will same the massage to output.txt

const double ma = 3727.3792; // alpha mass

void alpha( int numEvent = 100000){

  //================================================= User Setting
  std::string heliosDetGeoFile = "detectorGeo.txt";
  int geoID = 0;
 
  //std::vector<double> energy = {3.18, 5.16, 5.49, 5.81};
  std::vector<double> energy = {5.34, 5.42, 5.68, 6.05, 6.23, 6.77, 8.78}; //228Th
 
  //---- Over-ride HELIOS detector geometry
  // double BField = 2.5; // T
  // double BFieldTheta = 0.; // direction of B-field
  // bool isCoincidentWithRecoil = false; 
  // double eSigma = 0.040 ; // detector energy sigma MeV
  // double zSigma = 0.500 ; // detector position sigma mm
  
  //---- save root file name
  TString saveFileName = "SimAlpha.root";
  
  //=============================================================
  //=============================================================
  
  printf("===================================================\n");
  printf("=============  Alpha source in HELIOS  ============\n");
  printf("===================================================\n");
  
  int numEnergy = energy.size();
  printf("========= Alpha Enegry : \n");
  for( int i = 0; i < numEnergy ; i++){
    printf("%2d | %6.2f MeV\n", i, energy[i]);
  }
   
  
  //======== Set HELIOS
  printf("############################################## HELIOS configuration\n");   
  HELIOS helios;
  // helios.OverrideMagneticFieldDirection(BFieldTheta);
  // helios.OverrideFirstPos(-700);
  //helios.OverrideDetectorDistance(5);
  // bool sethelios = helios.SetDetectorGeometry(heliosDetGeoFile, geoID);
  // if( !sethelios){
  //   helios.OverrideMagneticField(BField);
  //   printf("======== B-field : %5.2f T, Theta : %6.2f deg\n", BField, BFieldTheta);
  // }
  // helios.SetCoincidentWithRecoil(isCoincidentWithRecoil);
  // printf("========== energy resol.: %f MeV\n", eSigma);
  // printf("=========== pos-Z resol.: %f mm \n", zSigma);
  helios.SetDetectorGeometry(heliosDetGeoFile, geoID);
  helios.PrintGeometry();
  
  //====================== build tree
  TFile * saveFile = new TFile(saveFileName, "recreate");
  TTree * tree = new TTree("tree", "tree");
  
  double theta, phi, T;
  
  int hit; // the output of Helios.CalHit
  double e, z, x, t;
  int loop, detID;
  double dphi, rho; //rad of rotation, and radius
  int energyID;
  double xHit, yHit;

  tree->Branch("hit", &hit, "hit/I");
  tree->Branch("theta", &theta, "theta/D");
  tree->Branch("phi", &phi, "phi/D");
  tree->Branch("T", &T, "T/D");
  tree->Branch("energy", &energy, "energy/D");
  tree->Branch("energyID", &energyID, "energyID/I");
  
  tree->Branch("e", &e, "e/D");
  tree->Branch("z", &z, "z/D");
  tree->Branch("t", &t, "t/D");
  tree->Branch("detID", &detID, "detID/I");
  tree->Branch("loop", &loop, "loop/I");
  tree->Branch("dphi", &dphi, "dphi/D");
  tree->Branch("rho", &rho, "rho/D");
  tree->Branch("xHit", &xHit, "xHit/D");
  tree->Branch("yHit", &yHit, "yHit/D");
  
  //========timer
  TBenchmark clock;
  bool shown ;   
  clock.Reset();
  clock.Start("timer");
  shown = false;
  printf("############################################## generating %d events \n", numEvent);
  
  //====================================================== calculate 
  int count = 0;
  TLorentzVector P;
  TVector3 v;
  for( int i = 0; i < numEvent; i++){
    //==== generate alpha 
    theta = TMath::ACos(2 * gRandom->Rndm() - 1) ; 
    phi = TMath::TwoPi() * gRandom->Rndm(); 
    
    energyID = gRandom->Integer(numEnergy);
    T = energy[energyID];
    
    double p = TMath::Sqrt( ( ma + T )*(ma + T) - ma* ma);
    
    v.SetMagThetaPhi(p, theta, phi);
    
    P.SetVectM(v, ma);
    P.SetUniqueID(2); //alpha particle has charge 2
    
    //################################### tree branches
    
    //==== Helios
    helios.CalArrayHit(P);
    hit = helios.CheckDetAcceptance();
    
    e = helios.GetEnergy() + gRandom->Gaus(0, helios.GetDetectorGeometry().array[geoID].eSigma);

    trajectory orb = helios.GetTrajectory_b();

    z = orb.z + gRandom->Gaus(0,  helios.GetDetectorGeometry().array[geoID].zSigma);
    t = orb.t;
    loop = orb.effLoop;
    detID = orb.detID;
    dphi = orb.phi;
    rho = orb.rho;
    xHit = orb.x;
    yHit = orb.y;
    
    if( hit == 1) {
      count ++;
    }
    
    tree->Fill();
    
    //#################################################################### Timer  
    clock.Stop("timer");
    Double_t time = clock.GetRealTime("timer");
    clock.Start("timer");
    
    if ( !shown ) {
      if (fmod(time, 10) < 1 ){
        printf( "%10d[%2d%%]| %8.2f sec | expect: %5.1f min \n", i, TMath::Nint((i+1)*100./numEvent), time , numEvent*time/(i+1)/60);
        shown = 1;
      }
    }else{
      if (fmod(time, 10) > 9 ){
        shown = 0;
      }
    }
  }
  
  saveFile->Write();
  saveFile->Close();
  
  printf("=============== done. saved as %s. count(hit==1) : %d\n", saveFileName.Data(), count);
  gROOT->ProcessLine(".q");
}

int main(){

  alpha();
  return 0;
}