#include "../armory/SolReader.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"

void script_single(std::string fileName){ 

  SolReader * reader = new SolReader(fileName);
  Hit * hit = reader->hit;

  reader->ScanNumBlock();

  long numBlock = reader->GetTotalNumBlock();

  for( int i = 1; i < 10; i ++ ){
    reader->ReadBlock(numBlock-i);
    hit->PrintEnergyTimeStamp();
  }

  /*
  SolReader * reader = new SolReader("../data_raw/Master_003_00_21245_000.sol");
  Event * hit = reader->hit;

  printf("========== file size: %u Byte\n", reader->GetFileSize());

  reader->ScanNumBlock();

  if( reader->GetTotalNumBlock() == 0 ) return;
  
  unsigned long startTime, endTime;
  reader->ReadBlock(0);
  startTime = hit->timestamp;
  reader->ReadBlock(reader->GetTotalNumBlock() - 1);
  endTime = hit->timestamp;

  double duration = double(endTime - startTime)*8./1e9;
  printf("============== %lu ns = %.4f sec.\n", (endTime - startTime)*8, duration);
  printf(" avarge rate (16ch): %f Hz\n", reader->GetTotalNumBlock()/duration/16);
  reader->RewindFile();


  TCanvas * canvas = new TCanvas("c1", "c1", 600, 600);

  TH1F * h1 = new TH1F("h1", "h1", 1000, startTime, endTime);
  TH2F * h2 = new TH2F("h2", "h2", 1000, startTime, endTime, 1000, 0, reader->GetTotalNumBlock());

  uint64_t tOld = startTime;

  for( int i = 0; i < reader->GetTotalNumBlock() ; i++){
  //for( int i = 0; i < 8 ; i++){
    
    reader->ReadNextBlock();
    
    if( i < 8 ){
      printf("########################## nBlock : %u, %u/%u\n", reader->GetBlockID(), 
                                                                reader->GetFilePos(), 
                                                                reader->GetFileSize());
      hit->PrintAll();
      //hit->PrintAllTrace();
    }

    h1->Fill(hit->timestamp);
    h2->Fill(hit->timestamp, i);
    
    if( i > 0 ){
      if( hit->timestamp < tOld) printf("-------- time not sorted.");
      tOld = hit->timestamp;
    }
    
  }


  h2->Draw();
  */

  //printf("reader traceLength : %lu \n", hit->traceLenght);

  /*
  for( int i = 0; i < hit->traceLenght; i++){

    printf("%4d| %d\n", i, hit->analog_probes[0][i]);

  }
  */

  hit = NULL;
  delete reader;

}