#include "Monitor.C+" // the plus sign mean compilation
#include "TObjArray.h"
#include "TFile.h"
#include "TMacro.h"


std::string create_range_string(const std::vector<int>& nums) {
    char range_str[100]; // buffer to hold the range string
    int pos = 0; // current position in the buffer
    int start = 0; // start of the current range
    int end = 0; // end of the current range
    for (int i = 1; i <= nums.size(); i++) {
        if (i == nums.size() || nums[i] != nums[i-1]+1) {
            end = i-1;
            if (start == end) {
                pos += std::sprintf(range_str+pos, "%d", nums[start]);
            } else if (end == start+1) {
                pos += std::sprintf(range_str+pos, "%d_%d", nums[start], nums[end]);
            } else {
                pos += std::sprintf(range_str+pos, "%d-%d", nums[start], nums[end]);
            }
            if (i < nums.size()) {
                pos += std::sprintf(range_str+pos, "_");
            }
            start = i;
        }
    }
    return std::string(range_str, pos);
}

void ChainMonitors(int RUNNUM = -1, int RUNNUM2 = -1) {


  ///default saveCanvas = false, no save Cavas
  ///                   = true, save Canvas
  
  TChain * chain = new TChain("gen_tree");

  if( RUNNUM == -1){
    /// this list only for manual Chain sort
    ///********** start Marker for AutoCalibration.

    chain->Add("../root_data/gen_run005.root");
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
  printf(" number of entries : %llu \n", chain->GetEntries());

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

  printf("%s\n", title.Data());
  
  //Some input to TSelector
  Monitor * selector = new Monitor();
  selector->SetCanvasTitle(title);
  chain->Process(selector, "");
  
}