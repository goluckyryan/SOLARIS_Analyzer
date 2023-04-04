#include "SolReader.h"
#include <cstdio>
#include <cstdlib>

#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "TString.h"
#include "TMacro.h"
//#include "TClonesArray.h" // plan to save trace as TVector with TClonesArray
//#include "TVector.h"

#define MAX_ID 64
#define MAX_TRACE_LEN 2500

SolReader ** reader;
Event ** evt;

unsigned long totNumEvent = 0;

void findEarliestTime(const int &nFile, int &fileID){

  unsigned long firstTime = 0;
  for(int i = 0; i < nFile; i++){
    if( reader[i]->GetNumBlock() + 1 >= reader[i]->GetTotalNumBlock()  ) continue;
    if( firstTime == 0 ) {
      firstTime = evt[i]->timestamp;
      fileID = i;
      continue;
    }
    if( evt[i]->timestamp <= firstTime) {
      firstTime = evt[i]->timestamp;
      fileID = i;
    }
  }

}

unsigned long long          evID = 0;
int                        multi = 0;
int                   bd[MAX_ID] = {0}; 
int                   ch[MAX_ID] = {0};
int                    e[MAX_ID] = {0};  
unsigned long long   e_t[MAX_ID] = {0};
unsigned short   lowFlag[MAX_ID] = {0};
unsigned short  highFlag[MAX_ID] = {0};
int             traceLen[MAX_ID] = {0};
int trace[MAX_ID][MAX_TRACE_LEN] = {0};

void fillData(int &fileID, const bool &saveTrace){
  bd[multi] = fileID;
  ch[multi] = evt[fileID]->channel;
  e[multi] = evt[fileID]->energy;
  e_t[multi] = evt[fileID]->timestamp;
  lowFlag[multi] = evt[fileID]->flags_low_priority;
  highFlag[multi] = evt[fileID]->flags_high_priority;
  
  if( saveTrace ){
    traceLen[multi] = evt[fileID]->traceLenght;
    for( int i = 0; i < TMath::Min(traceLen[multi], MAX_TRACE_LEN); i++){
      trace[multi][i] = evt[fileID]->analog_probes[0][i];
    } 
  }

  multi++;
  reader[fileID]->ReadNextBlock();
}

void printEvent(){
  printf("==================== evID : %llu\n", evID);
  for( int i = 0; i < multi; i++){
    printf("    %2d | %d %d | %llu %d \n", i, bd[i], ch[i], e_t[i], e[i] );
  }
  printf("==========================================\n");
}

int main(int argc, char ** argv){

  printf("=====================================\n");
  printf("===        sol  --> root          ===\n");
  printf("=====================================\n");  

  if( argc <= 3){
    printf("%s [outfile] [timeWindow] [saveTrace] [sol-1] [sol-2] ... \n", argv[0]);
    printf("      outfile : output root file name\n");
    printf("   timeWindow : number of tick, 1 tick = %d ns.\n", tick2ns); 
    printf("    saveTrace : 1 = save trace, 0 = no trace\n");
    printf("        sol-X : the sol file(s)\n");
    return -1;
  }

  // for( int i = 0; i < argc; i++){
  //   printf("%d | %s\n", i, argv[i]);
  // }

  TString outFileName = argv[1];
  int timeWindow = atoi(argv[2]);
  const bool saveTrace = atoi(argv[3]);

  const int nFile = argc - 4;
  TString inFileName[nFile];
  for( int i = 0 ; i < nFile ; i++){
    inFileName[i] = argv[i+4];
  }

  printf(" out file : \033[1;33m%s\033[m\n", outFileName.Data());
  printf(" Event building time window : %d tics = %d nsec \n", timeWindow, timeWindow*tick2ns);
  printf(" Save Trace ? %s \n", saveTrace ? "Yes" : "No");
  printf(" Number of input file : %d \n", nFile);
  for( int i = 0; i < nFile; i++){
    printf("  %2d| %s \n", i, inFileName[i].Data());
  }
  printf("------------------------------------\n");

  TFile * outRootFile = new TFile(outFileName, "recreate");
  outRootFile->cd();

  TTree * tree = new TTree("tree", outFileName);

  tree->Branch("evID",         &evID, "event_ID/l"); 
  tree->Branch("multi",       &multi, "multi/I"); 
  tree->Branch("bd",              bd, "board[multi]/I");
  tree->Branch("ch",              ch, "channel[multi]/I");
  tree->Branch("e",                e, "energy[multi]/I");
  tree->Branch("e_t",            e_t, "energy_timestamp[multi]/l");
  tree->Branch("lowFlag",    lowFlag, "lowFlag[multi]/s");
  tree->Branch("highFlag",  highFlag, "highFlag[multi]/s");

  if( saveTrace){
    tree->Branch("tl",        traceLen, "traceLen[multi]/I");
    tree->Branch("trace",        trace, Form("trace[multi][%d]/I", MAX_TRACE_LEN));
  }

  reader = new SolReader*[nFile];
  evt = new Event *[nFile];

  for( int i = 0 ; i < nFile ; i++){
    reader[i] = new SolReader(inFileName[i].Data());
    evt[i] = reader[i]->evt;
    reader[i]->ScanNumBlock();

    totNumEvent += reader[i]->GetTotalNumBlock();
    reader[i]->ReadNextBlock(); // read the first block
  }

  //^=========================================== 

  printf("================================= \n");

  int fileID = 0;
  findEarliestTime(nFile, fileID);
  fillData(fileID, saveTrace);

  unsigned long firstTimeStamp = evt[fileID]->timestamp;
  unsigned long lastTimeStamp = 0;

  int last_precentage = 0;

  unsigned count = 1;
  while(count < totNumEvent){

    findEarliestTime(nFile, fileID);

    if( evt[fileID]->timestamp - e_t[0] < timeWindow ){

      fillData(fileID, saveTrace);

    }else{

      outRootFile->cd();
      tree->Fill();
      evID ++;

      multi = 0;
      fillData(fileID, saveTrace);
    }

    count ++;

    if( count == totNumEvent ) lastTimeStamp = evt[fileID]->timestamp;

    int percentage = count * 100/totNumEvent;

    if( percentage > last_precentage + 1.0 ) {
      printf("Processed : %u, %.0f%% \n\033[A\r", count, count*100./totNumEvent);
      last_precentage = percentage;
    }

  }
  
  outRootFile->cd();
  tree->Write();


  //======== Save timestamp as TMacro
  TMacro timeStamp;
  TString str;
  str.Form("%lu", firstTimeStamp); timeStamp.AddLine( str.Data() );
  str.Form("%lu", lastTimeStamp); timeStamp.AddLine( str.Data() );

  timeStamp.Write("timeStamp");


  printf("===================================== done. \n");
  printf("Number of Event Built : %lld\n", evID);
  printf("      first timestamp : %lu \n", firstTimeStamp);
  printf("       last timestamp : %lu \n", lastTimeStamp);
  unsigned long duration = lastTimeStamp - firstTimeStamp;
  printf("       total duration : %lu = %.2f sec \n", duration, duration * tick2ns * 1.0 / 1e9 );
  printf("===================================== end of summary. \n");


  //^############## delete new
  for( int i = 0; i < nFile; i++) delete reader[i];
  delete [] reader;
  outRootFile->Close();

  return 0;
}