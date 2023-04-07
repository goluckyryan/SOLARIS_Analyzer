#include "Monitor.C+" // the plus sign mean compilation
#include "TObjArray.h"
#include "TFile.h"
#include "TMacro.h"
std::string create_range_string(const std::vector<int>& nums);

void ChainMonitors(int RUNNUM = -1, int RUNNUM2 = -1) {
  ///default saveCanvas = false, no save Cavas
  ///                   = true, save Canvas
  
  TChain * chain = new TChain("gen_tree");
  if( RUNNUM == -1){
    /// this list only for manual Chain sort
    ///********** start Marker for AutoCalibration.

    chain->Add("../root_data/gen_run005.root");
    chain->Add("../root_data/gen_run003.root");
     ///chain->Add("../root_data/trace_run135.root");
  
    ///********** end Marker for AutoCalibration.
    
    
  }else{
    
    TString fileName;
    int endRUNNUM = RUNNUM2;
    if( RUNNUM2 == -1) endRUNNUM = RUNNUM;
    
    for( int i = RUNNUM ; i <= endRUNNUM ; i++){
      fileName.Form("../root_data/gen_run%03d.root", i);
      chain->Add(fileName);
    }
  }
  
  TObjArray * fileList = chain->GetListOfFiles();
  
  printf("\033[0;31m========================================== Number of Files : %2d\n",fileList->GetEntries());
  fileList->Print();
  printf("========================================== Number of Files : %2d\033[0m\n",fileList->GetEntries());
  printf("---------------------------------- Total Number of entries : %llu \n", chain->GetEntries());

  double totDuration = 0;
  std::vector<ULong64_t> startTime;
  std::vector<ULong64_t> stopTime;
  std::vector<int> runList;
  
  for( int i = 0; i < fileList->GetEntries(); i++){
    TString fileName = fileList->At(i)->GetTitle();
    TFile file(fileName);
    TMacro * timeStamp = (TMacro*) file.FindObjectAny("timeStamp");
    //timeStamp->Print();

    TString haha = timeStamp->GetListOfLines()->At(0)->GetName();
    ULong64_t t1 = haha.Atoll();

    haha = timeStamp->GetListOfLines()->At(1)->GetName();
    ULong64_t t2  = haha.Atoll();

    haha = timeStamp->GetListOfLines()->At(2)->GetName();
    int RunID =  haha.Atoi();

    totDuration += (t2-t1)*8./1e9;
    startTime.push_back(t1);
    stopTime.push_back(t2);
    runList.push_back(RunID);
  }

  //======== format CanvasTitle
  std::sort(runList.begin(), runList.end());
  TString title = "Run:" +  create_range_string(runList);
  title += Form(" | %.0f min", totDuration/60.) ;
  
  //Some input to TSelector
  Monitor * selector = new Monitor();
  selector->SetCanvasTitle(title);
  selector->SetStartStopTimes(startTime, stopTime);
  chain->Process(selector, "");

  delete chain;
  delete selector;
  
}


std::string create_range_string(const std::vector<int>& nums) {
  std::string range_str;
  int lastNum = nums[0];
  int rangeStart = lastNum;
  for (int i = 1; i < nums.size(); i++) {
    if (nums[i] == lastNum + 1) {
      lastNum = nums[i];
    } else {
      if (rangeStart == lastNum) {
        range_str += std::to_string(rangeStart) + "_";
      } else {
          range_str += std::to_string(rangeStart) + "-" + std::to_string(lastNum) + "_";
      }
      rangeStart = lastNum = nums[i];
    }
  }
  // Add the last range
  if (rangeStart == lastNum) {
    range_str += std::to_string(rangeStart);
  } else {
    range_str += std::to_string(rangeStart) + "-" + std::to_string(lastNum);
  }
  return range_str;
}