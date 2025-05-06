#include "Monitor.C+" // the plus sign mean compilation
#include "TObjArray.h"
#include "TFile.h"
#include "TMacro.h"
#include "TChain.h"

TChain *gen_tree = nullptr;

void ChainMonitors(int RUNNUM = -1, int RUNNUM2 = -1) {
  ///default saveCanvas = false, no save Cavas
  ///                   = true, save Canvas
  
  gen_tree = new TChain("gen_tree");
  if( RUNNUM == -1){
    /// this list only for manual Chain sort
    ///********** start Marker for AutoCalibration.

    gen_tree->Add("../root_data/trace_run033.root");
  
    ///********** end Marker for AutoCalibration.
    
  }else{
    
    TString fileName;
    int endRUNNUM = RUNNUM2;
    if( RUNNUM2 == -1) endRUNNUM = RUNNUM;

    for( int i = RUNNUM ; i <= endRUNNUM ; i++){
      fileName.Form("../root_data/gen_run%03d.root", i);
      gen_tree->Add(fileName);
    }
  }
  
  //^============== should have other things, like calibrations. 
  Monitor(gen_tree);

}
