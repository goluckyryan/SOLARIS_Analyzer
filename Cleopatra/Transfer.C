/***********************************************************************
 * 
 *  This is Transfer.C for simulation of transfer reaction.
 * 
 * -----------------------------------------------------
 *  This program will call the root library and compile in g++
 *  compilation:
 *  g++ Transfer.C -o Transfer `root-config --cflags --glibs`
 *
 * ------------------------------------------------------
 *  created by Ryan (Tsz Leung) Tang, Feb-4, 2019
 *  email: goluckyryan@gmail.com
 * ********************************************************************/

#include <fstream>
#include <stdlib.h>
#include "Transfer.h"

using namespace std;

int main (int argc, char *argv[]) {
  
  printf("=================================================================\n");
  printf("==========     Simulate Transfer reaction in HELIOS    ==========\n");
  printf("=================================================================\n");
  
  if(argc == 2 || argc > 7) { 
    printf("Usage: ./Transfer [1] [2] [3] [4] [5] [6]\n");
    printf("       default file name \n");
    printf("   [1] reactionConfig.txt (input) reaction Setting \n");
    printf("   [2] detectorGeo.txt    (input) detector Setting \n");
    printf("   [3] ID                 (input) detector & reaction ID (default = 0 ) \n");
    printf("   [4] DWBA.root          (input) thetaCM distribution from DWBA \n");
    printf("   [5] transfer.root      (output) rootFile name for output \n");
    printf("   [6] plot               (input) will it plot stuffs [1/0] \n");

    printf("------------------------------------------------------\n");
    return 0 ; 
  }

  string       basicConfig = "reactionConfig.txt";
  string  heliosDetGeoFile = "detectorGeo.txt";
  int                   ID = 0;
  TString      ptolemyRoot = "DWBA.root"; // when no file, use isotropic distribution of thetaCM
  TString     saveFileName = "transfer.root";
  bool           isPlot = false;
   
  if( argc >= 2) basicConfig = argv[1];  
  if( argc >= 3) heliosDetGeoFile = argv[2];  
  if( argc >= 4) ID = atoi(argv[3]);  
  if( argc >= 5) ptolemyRoot = argv[4];
  if( argc >= 6) saveFileName = argv[5];
  if( argc >= 7) isPlot = atoi(argv[7]);
  
  Transfer( basicConfig, heliosDetGeoFile, ID, ptolemyRoot, saveFileName);

  //run Armory/Check_Simulation
  if( isPlot ){
    ifstream file_in;
    file_in.open("../Cleopatra/Check_Simulation.C", ios::in);
    if( file_in){
      printf("---- running ../Cleopatra/Check_Simulation.C on %s \n", saveFileName.Data());
      TString cmd;
      cmd.Form("root -l '../Cleopatra/Check_Simulation.C(\"%s\")'", saveFileName.Data());
      
      system(cmd.Data());
    }else{
      printf("cannot find ../Cleopatra/Check_Simulation.C \n");
    }
  }
  
}
