#include "../armory/SolReader.h"
#include "TH1.h"
#include "TMath.h"
#include "TH2.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TGraph.h"


void script_multi(std::string run = "033"){
  

  SolReader* reader0 = new SolReader("../data_raw/*_" + run + "_00_21245_000.sol");
  //SolReader* reader1 = new SolReader("/home/ryan/analysis/data_raw/test_" + run + "_01_21233_000.sol");

  Hit * evt0 = reader0->hit;
  //Event * evt1 = reader1->evt;

  printf("----------file size: %u Byte\n", reader0->GetFileSize());
  //printf("----------file size: %u Byte\n", reader1->GetFileSize());

  reader0->ScanNumBlock();
  //reader1->ScanNumBlock();
  unsigned long startTime, endTime;

  if( reader0->GetTotalNumBlock() == 0 ) return;
  reader0->ReadBlock(0);
  startTime = evt0->timestamp;
  reader0->ReadBlock(reader0->GetTotalNumBlock() - 1);
  endTime = evt0->timestamp;

  double duration = double(endTime - startTime)*8./1e9;
  printf("============== %lu ns = %.4f sec.\n", (endTime - startTime)*8, duration);
  reader0->RewindFile();

  // if( reader1->GetTotalNumBlock() == 0 ) return;
  // reader1->ReadBlock(0);
  // startTime = evt1->timestamp;
  // reader1->ReadBlock(reader1->GetTotalNumBlock() - 1);
  // endTime = evt1->timestamp;

  // duration = double(endTime - startTime)*8./1e9;
  // printf("============== %lu ns = %.4f sec.\n", (endTime - startTime)*8, duration);
  // reader1->RewindFile();
  
  //int minBlock = std::min(reader0->GetTotalNumBlock(), reader1->GetTotalNumBlock());
  int minBlock = reader0->GetTotalNumBlock();

  TH1F * hID = new TH1F("hID", "hID", 64, 0, 64);
  TH1F * hTdiff = new TH1F("hTdiff", "tdiff", 300, -100, 200);
  TH2F * hTdiff2 = new TH2F("hTdiff2", "tdiff vs evt", 400, 0, minBlock, 100, 0, 100);

  TH1F * hRate0 = new TH1F("hRate0", "Rate", 20, 0, 20);
  TH1F * hRate1 = new TH1F("hRate1", "Rate", 20, 0, 20); hRate1->SetLineColor(2);

  TH1F * hE0 = new TH1F("hE0", "Energy", 400, 0, 30000);
  TH1F * hE1 = new TH1F("hE1", "Energy", 400, 0, 30000); hE1->SetLineColor(2);

  TH1F *hMulti = new TH1F("hMulti", "Multiplicy", 10, 0, 10);

  std::vector<std::pair<int, uint64_t>> ts ;

  for( int i = 0; i < minBlock; i++){


    reader0->ReadNextBlock();
    //reader1->ReadNextBlock();

    if( i < 10 ){
      printf("#################################################\n");
      evt0->PrintAll();
    }

    ts.push_back(std::pair(evt0->channel,evt0->timestamp));
    // printf("---------------------------------------------\n");
    // evt1->PrintAll();

    //if( evt0->channel == 30 ) evt0->PrintAll();

    hID->Fill(evt0->channel);

    if( evt0->channel == 0 ) {
      hRate0->Fill( evt0->timestamp * 8 / 1e9);
      hE0->Fill(evt0->energy);
    }
    if( evt0->channel == 30 ) {
      hRate1->Fill( evt0->timestamp * 8 / 1e9);
      hE1->Fill(evt0->energy);
    }
    //if( i < 10 ) printf("t0 : %10lu, t1 : %10lu, %10lu \n", evt0->timestamp, evt1->timestamp,
    //  evt0->timestamp > evt1->timestamp ? evt0->timestamp - evt1->timestamp : evt1->timestamp - evt0->timestamp);  
//
    //hTdiff->Fill(evt0->timestamp > evt1->timestamp ? evt0->timestamp - evt1->timestamp : evt1->timestamp - evt0->timestamp);
    //hTdiff2->Fill(i, evt0->timestamp > evt1->timestamp ? evt0->timestamp - evt1->timestamp : evt1->timestamp - evt0->timestamp);

    //if( i > 10 ) break;

  }

  delete reader0;
  //delete reader1;

  //build event
  int coinWin = 200;

  std::vector<std::vector<std::pair<int, uint64_t>>> events;
  std::vector<int> multi;
  events.push_back({ts[0]});
  multi.push_back(1);

  // Iterate through the vector starting from the second pair
  for (size_t i = 1; i < ts.size(); ++i) {
      uint64_t currentTimestamp = ts[i].second;
      uint64_t previousTimestamp = ts[i - 1].second;

      // Check if the timestamp difference is within coinWin
      if (currentTimestamp - previousTimestamp <= coinWin) {
          events.back().push_back(ts[i]); // Add to the current event
          multi.back() += 1;
      } else {
          // Create a new event
          events.push_back({ts[i]});
          multi.push_back(1);
      }
  }

  // Print the events

  double maxTD = -999, minTD = 999;

  for (size_t i = 0; i < events.size(); ++i) {
    hMulti->Fill(multi[i]);
    if( multi[i] > 1 ) {
      printf("Event %zu, Multi : %d\n", i, multi[i]);
      double haha[2];
      for (size_t j = 0; j < events[i].size(); ++j) {
          printf("Channel: %2d, Timestamp: %lu\n", events[i][j].first, events[i][j].second);
          if( events[i][j].first == 0 ) haha[0] = static_cast<double> ( events[i][j].second );
          if( events[i][j].first == 30 ) haha[1] = static_cast<double> ( events[i][j].second );
      }

      double TD = haha[1]-haha[0];

      if(TD > maxTD && TD < 105) maxTD = TD;
      if( TD < minTD && TD > -105) minTD = TD;
      hTdiff->Fill( TD );
      printf("-------------------\n");
    }
  }

  printf(" max TD : %f \n", maxTD);
  printf(" min TD : %f \n", minTD);
  printf(" spn TD : %f \n", maxTD - minTD);

  TCanvas * canvas = new TCanvas("canvas", "canvas", 1200, 800);

  gStyle->SetOptStat("neiou");
  canvas->Divide(3,2);
  canvas->cd(1);hID->Draw();
  canvas->cd(2);hE0->Draw(); hE1->Draw("same");
  canvas->cd(3);hRate0->Draw(); hRate1->Draw("same");
  canvas->cd(4);hTdiff->Draw();
  canvas->cd(5); hMulti->Draw();
  canvas->cd(5)->SetLogy(true);
  canvas->cd(6); hE1->Draw();
  //canvas->cd(2);hTdiff2->Draw();


}