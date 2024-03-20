#include "../Armory/ClassDetGeo.h"
#include "../Armory/ClassReactionConfig.h"
#include "../Cleopatra/ClassHelios.h"
#include "../Cleopatra/ClassTransfer.h"

void test(){

  // DetGeo haha("detectorGeo.txt");
  // haha.Print(true);

  // ReactionConfig config("reactionConfig.txt");
  // config.Print();

  TransferReaction * transfer = new TransferReaction();
  // transfer->SetReactionSimple(32, 14, 2, 1, 1, 1, 8.8);

  int ID = 0;
  transfer->SetReactionFromFile("reactionConfig.txt", ID);
  
  transfer->PrintReaction();


  transfer->Event(25 * TMath::DegToRad(), 0 * TMath::DegToRad());
  transfer->PrintFourVectors();

  // ReactionConfig config2 = transfer->GetRectionConfig();

  // HELIOS * helios = new HELIOS();
  // helios->SetDetectorGeometry("detectorGeo.txt", 1);
  // helios->PrintGeometry();


  // TLorentzVector Pb = transfer->GetPb();
  // printf("Charge : %d\n", Pb.GetUniqueID());
  // int hit = helios->CalArrayHit(Pb, false);
  // helios->CheckDetAcceptance();
  
  // // //helios->CalTrajectoryPara(Pb, config.recoilLightZ, true);

  // printf("\n hit = %d | %s | %s\n", hit, helios->GetHitMessage().Data(), helios->GetAcceptanceMessage().Data());

  // trajectory orb = helios->GetTrajectory_b();

  // orb.PrintTrajectory();

  // delete helios;
  delete transfer;

}