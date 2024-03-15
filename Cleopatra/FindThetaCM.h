/***********************************************************************
 * 
 *  This is FindThetaCM.h, To calculate the thetaCM convrage for each detector
 *  
 *  This required two inout files: basicReactionConfig.txt
 *                                 detectorGeo.txt
 * 
 *-------------------------------------------------------
 *  created by Ryan (Tsz Leung) Tang, Nov-18, 2018
 *  email: goluckyryan@gmail.com
 * ********************************************************************/

#include "TFile.h"
#include "TTree.h"
#include "TF1.h"
#include "TMacro.h"
#include "TObjArray.h"
#include "TGraph.h"
#include "../Cleopatra/ClassHelios.h"
#include "../Cleopatra/ClassTransfer.h"

void FindThetaCM(double Ex, int nDivision=1, double XRATION = 0.95, 
            std::string reactionConfigFileName="reactionConfig.txt",  
            std::string detGeoFileName = "detectorGeo.txt", unsigned short ID = 0){


  ///========================================================= load files

  TransferReaction reaction(reactionConfigFileName, ID);
  reaction.SetExB(Ex);  
  reaction.CalReactionConstant();
  reaction.PrintReaction(false);
  ReactionConfig reConfig = reaction.GetRectionConfig();
  Recoil recoil = reaction.GetRecoil();

  HELIOS helios(detGeoFileName, ID);
  helios.PrintGeometry();
  DetGeo detGeo = helios.GetDetectorGeometry();
  Array array = helios.GetArrayGeometry();
   
  //calculate a TGraph for thetaCM vs z
  const int nData = 170;
  double px[nData];
  double py[nData];
  
  double mb = reaction.GetMass_b();
  double kCM = reaction.GetMomentumbCM();
  double q = TMath::Sqrt(mb*mb + kCM * kCM );
  double beta = reaction.GetReactionBeta() ;
  double BField = detGeo.Bfield;
  double slope = reaction.GetEZSlope(BField);
  double gamma = reaction.GetReactionGamma();
  double perpDist = array.detPerpDist;

  for(int i = 0; i < nData; i++){
    double thetacm = (i + 5.) * TMath::DegToRad();
    double temp = TMath::TwoPi() * slope / beta / kCM * perpDist / TMath::Sin(thetacm); 
    px[i] = beta /slope * (gamma * beta * q - gamma * kCM * TMath::Cos(thetacm)) * (1 - TMath::ASin(temp)/TMath::TwoPi());
    py[i] = thetacm * TMath::RadToDeg();   
  }

  //find minimum z position
  TGraph * xt = new TGraph(100, py, px);
  xt->SetName("xt");
  ///double zMin0 = xt->Eval(0);
  ///printf("z for thetaCM = 0 : %f mm \n", zMin0);
  
  ///xt->Draw("AC*");
  
  /// find the minimum z position and the corresponding theta
  double zMin0 = 99999999;
  double tMin0 = 99999999;
  for( double ttt = 3 ; ttt < 20 ; ttt += 0.1 ){
    double zzz = xt->Eval(ttt);
    if( zzz < zMin0 ) {
      zMin0 = zzz;
      tMin0 = ttt;
    }
  }   
  printf(" z min %f mm at thetaCM %f deg \n", zMin0, tMin0);

  TGraph * tx = new TGraph(nData, px, py);
  tx->SetName(Form("tx"));
  tx->SetLineColor(4);

  //Remove nan data
  for( int i = tx->GetN() -1 ;  i >= 0 ; i--){
    if( TMath::IsNaN(tx->GetPointX(i)) ) tx->RemovePoint(i);
  }
  
  // tx->Draw("AC");

  ///========================================================= result
    
  int iDet = array.nDet;
  double length = array.detLength;
  std::vector<double> midPos;
   
  for(int i = 0; i < iDet; i++){
    if( array.firstPos > 0 ){
      midPos.push_back(array.detPos[i]+length/2.);
    }else{
      midPos.push_back(array.detPos[i]-length/2.);
    }
    // printf("%2d | %f \n", i, midPos.back());
  }

  printf("==== ThetaCM in degree =================\n");
  printf(" x-ratio : %f, number of division : %d \n", XRATION, nDivision);
  printf("\n");
  for( int j = 0; j < nDivision + 1; j++) printf("%5.2f   ", -XRATION + 2*XRATION/nDivision*j);
  printf(" <<-- in X \n");
  for( int j = 0; j < nDivision + 1; j++) printf("%5s   ", "  |  ");
  printf("\n");
  for( int j = 0; j < nDivision + 1; j++) printf("%5.2f   ", length/2 -length*XRATION/2 + length*XRATION/nDivision*j);
  printf(" <<-- in mm \n\n");
  printf("=========================      Ex : %6.4f MeV\n", Ex);
  printf("            %6s - %6s |  %6s, %6s, %6s\n", "Min",  "Max", "Mean", "Dt", "sin(x)dx * 180/pi");
  printf("-------------------------------------------------\n");
  for( int i = 0; i < iDet; i++){
    double zMin = midPos[i]-length*XRATION/2.;
    double zMax = midPos[i]+length*XRATION/2.;
    double zLength = zMax - zMin; 
    double zStep = zLength/(nDivision);
    for( int j = 0 ; j < nDivision ; j++){
      
      double tMin =  (zMin +     j*zStep > zMin0) ? tx->Eval(zMin +     j*zStep) : TMath::QuietNaN();
      double tMax =  (zMin + (j+1)*zStep > zMin0) ? tx->Eval(zMin + (j+1)*zStep) : TMath::QuietNaN();
      
      double tMean = (tMax + tMin)/2.;
      double dt = (tMax - tMin);
      
      double sintdt = TMath::Sin(tMean * TMath::DegToRad()) * dt ;

        
      printf(" det-%d[%d]:  %6.2f - %6.2f |  %6.2f, %6.2f, %6.4f\n", i, j, tMin,  tMax, tMean, dt, sintdt);
      
    }
    if( nDivision > 0 ) printf("--------------\n");
  }
  printf("================================================= \n");

}
