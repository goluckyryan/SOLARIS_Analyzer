#ifndef GeneralSort_h
#define GeneralSort_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include <TObjString.h>
#include <TGraph.h>
#include <TClonesArray.h>
#include <TF1.h>
#include <TProofOutputFile.h>

/*********************************=======

the sequence of each method 

1) Begin (master, stdout)
  2) SlaveBegin (slave)
    3) Init
    4) Notify
    5) Process (looping each event)
  6) SlaveTerminate
7) Terminate 

// variable in the Master will not pass to Slave

******************************************/


/// in Process_Sort, copy the GeneralSortMapping.h to ~/.proof/working/
#include "../working/Mapping.h"
#include "../armory/AnalysisLib.h"

double fitFunc(double * x, double * par){   
  /// par[0] = A
  /// par[1] = t0
  /// par[2] = rise time
  /// par[3] = baseline
  /// par[4] = decay
  /// par[5] = pre-rise slope
  
  if( x[0] < par[1] ) return  par[3] + par[5] * (x[0]-par[1]);
  
  return par[3] + par[0] * (1 - TMath::Exp(- (x[0] - par[1]) / par[2]) ) * TMath::Exp(- (x[0] - par[1]) / par[4]);
}

// Header file for the classes stored in the TTree if any.
class GeneralSort : public TSelector {
public :
  TTree          *fChain;   //!pointer to the analyzed TTree or TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

  // Declaration of leaf types
  ULong64_t       evID;
  Int_t           multi;
  Int_t           bd[100];   //[multi]
  Int_t           ch[100];   //[multi]
  Int_t           e[100];   //[multi]
  ULong64_t       e_t[100];   //[multi]
  UShort_t        lowFlag[100]; //[multi]
  UShort_t        highFlag[100]; //[multi]
  Int_t           tl[100];   //[multi]
  Int_t           trace[100][2500];   //[multi]

  // List of branches
  TBranch        *b_event_ID;   //!
  TBranch        *b_multi;   //!
  TBranch        *b_bd;   //!
  TBranch        *b_ch;   //!
  TBranch        *b_e;   //!
  TBranch        *b_e_t;   //!
  TBranch        *b_lowFlag;   //!
  TBranch        *b_highFlag;   //!
  TBranch        *b_tl;   //!
  TBranch        *b_trace;   //!

  GeneralSort(TTree * /*tree*/ =0) : fChain(0) { 
    printf("constructor :: %s\n", __func__);

    isTraceExist = false;
    traceMethod = 0; // -1 = ignore trace, 0 = no trace fit, 1 = fit, 2 = trapezoid
  
    isParallel = false;
    detNum.clear();
    nDetType = 0;
  }
  virtual ~GeneralSort() { }
  virtual Int_t   Version() const { return 2; }
  virtual void    Begin(TTree *tree);
  virtual void    SlaveBegin(TTree *tree);
  virtual void    Init(TTree *tree);
  virtual Bool_t  Notify();
  virtual Bool_t  Process(Long64_t entry);
  virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
  virtual void    SetOption(const char *option) { fOption = option; }
  virtual void    SetObject(TObject *obj) { fObject = obj; }
  virtual void    SetInputList(TList *input) { fInput = input; }
  virtual TList  *GetOutputList() const { return fOutput; }
  virtual void    SlaveTerminate();
  virtual void    Terminate();

  ClassDef(GeneralSort,0);

  bool isTraceExist;
  int traceMethod;

  void SetTraceMethod(int methodID) {traceMethod = methodID;}
  void PrintTraceMethod();

  std::vector<int> detNum;
  int nDetType;

  void SetUpTree();
  void DecodeOption();
  bool isParallel;

  TString saveFileName;
  TFile * saveFile; //!
  TProofOutputFile * proofFile; //!
  TTree * newTree; //!

  Float_t   ** eE; //!
  ULong64_t ** eT; //!

  //trace
  TClonesArray * arr ;//!
  TGraph * gTrace; //!
  TClonesArray * arrTrapezoid ;//!   
  TGraph * gTrapezoid; //!

  //trace energy, trigger time, rise time
  Float_t   **teE; //! 
  Float_t   **teT; //! 
  Float_t   **teR; //! 

};

#endif

#ifdef GeneralSort_cxx

//^##############################################################
void GeneralSort::SetUpTree(){

  printf("%s\n", __func__);

  if( isParallel){
    proofFile = new TProofOutputFile(saveFileName, "M");
    saveFile = proofFile->OpenFile("RECREATE");
  }else{
    saveFile = new TFile(saveFileName,"RECREATE");
  }

  newTree = new TTree("gen_tree", "Tree After GeneralSort");
  newTree->SetDirectory(saveFile);
  newTree->AutoSave();

  detNum = ExtractDetNum(mapping, detTypeName, detMaxID);

  nDetType = (int) detTypeName.size();

  eE = new Float_t * [nDetType];
  eT = new ULong64_t * [nDetType];

  for( int i = 0 ; i < nDetType; i++){
    eE[i] = new Float_t[detNum[i]];
    eT[i] = new ULong64_t[detNum[i]];

    for( int j = 0; j < detNum[i]; j++){
      eE[i][j] =  TMath::QuietNaN();
      eT[i][j] = 0;
    }

    newTree->Branch(  detTypeName[i].c_str(),        eE[i], Form("%s[%d]/F", detTypeName[i].c_str(), detNum[i]));
    newTree->Branch( (detTypeName[i]+"_t").c_str(),  eT[i], Form("%s_Timestamp[%d]/l", detTypeName[i].c_str(), detNum[i]));
  }


  if( isTraceExist && traceMethod >= 0){

    arr = new TClonesArray("TGraph");

    newTree->Branch("trace", arr, 256000);
    arr->BypassStreamer();

    if( traceMethod > 0 ){

      teE = new Float_t * [nDetType];
      teT = new Float_t * [nDetType];
      teR = new Float_t * [nDetType];

      for( int i = 0 ; i < nDetType; i++){
        teE[i] = new Float_t[detNum[i]];
        teT[i] = new Float_t[detNum[i]];
        teR[i] = new Float_t[detNum[i]];

        for( int j = 0; j < detNum[i]; j++){
          teE[i][j] =  TMath::QuietNaN();
          teT[i][j] =  TMath::QuietNaN();
          teR[i][j] =  TMath::QuietNaN();
        }

        newTree->Branch( ("t" + detTypeName[i]).c_str(),       teE[i], Form("trace_%s[%d]/F",      detTypeName[i].c_str(), detNum[i]));
        newTree->Branch( ("t" + detTypeName[i]+"_t").c_str(),  teT[i], Form("trace_%s_time[%d]/l", detTypeName[i].c_str(), detNum[i]));
        newTree->Branch( ("t" + detTypeName[i]+"_r").c_str(),  teR[i], Form("trace_%s_rise[%d]/l", detTypeName[i].c_str(), detNum[i]));
      }

    }

  }

}

//^##############################################################
void GeneralSort::DecodeOption(){
  TString option = GetOption();
  if( option != ""){
    TObjArray * tokens = option.Tokenize(",");
    traceMethod = ((TObjString*) tokens->At(0))->String().Atoi();
    saveFileName = ((TObjString*) tokens->At(1))->String();
    isParallel = ((TObjString*) tokens->At(2))->String().Atoi();
  }else{
    traceMethod = -1;
    saveFileName = "temp.root";
    isParallel = false;
  }

  printf("|%s| %d %s %d \n", option.Data(), traceMethod, saveFileName.Data(), isParallel);

}

//^##############################################################
void GeneralSort::Init(TTree *tree){

  printf("%s\n", __func__);

  // Set branch addresses and branch pointers
  if (!tree) return;
  fChain = tree;
  fChain->SetMakeClass(1);

  fChain->SetBranchAddress("evID",        &evID, &b_event_ID);
  fChain->SetBranchAddress("multi",      &multi, &b_multi);
  fChain->SetBranchAddress("bd",             bd, &b_bd);
  fChain->SetBranchAddress("ch",             ch, &b_ch);
  fChain->SetBranchAddress("e",               e, &b_e);
  fChain->SetBranchAddress("e_t",           e_t, &b_e_t);
  fChain->SetBranchAddress("lowFlag",   lowFlag, &b_lowFlag);
  fChain->SetBranchAddress("highFlag", highFlag, &b_highFlag);

  TBranch * br = (TBranch *) fChain->GetListOfBranches()->FindObject("tl");
  if( br == NULL ){
    printf(" ++++++++ no Trace.\n");
    isTraceExist = false;
  }else{
    isTraceExist = true;
    fChain->SetBranchAddress("tl", tl, &b_tl);
    fChain->SetBranchAddress("trace", trace, &b_trace);
  }

  unsigned long NumEntries = fChain->GetEntries();
  printf( "========== total Entry : %ld\n", NumEntries);

  //########################### Get Option 
  DecodeOption();

  if( isTraceExist ){
    PrintTraceMethod();
  }else{
    printf("++++++++ no Trace found\n");
  }

  SetUpTree();

  printf("---- end of Init %s\n ", __func__);

}

Bool_t GeneralSort::Notify(){
  printf("%s\n", __func__);
  return kTRUE;
}

void GeneralSort::PrintTraceMethod(){
  const char* traceMethodStr;
  switch(traceMethod) {
    case -1 : traceMethodStr = "Ignore Trace"; break;
    case  0 : traceMethodStr = "Copy"; break;
    case  1 : traceMethodStr = "Fit"; break;
    case  2 : traceMethodStr = "Trapezoid"; break;
    default:  traceMethodStr = "Unknown"; break;
  }
  printf("  Trace method ? %s \n", traceMethodStr);
}


#endif // #ifdef GeneralSort_cxx
