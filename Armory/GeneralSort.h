#ifndef GeneralSort_h
#define GeneralSort_h

#include <TROOT.h>
#include <TChain.h>
#include <TTree.h>
#include <TFile.h>
#include <TSelector.h>
#include <TObjString.h>
#include <TGraph.h>
#include <TClonesArray.h>
#include <TF1.h>
#include <TStopwatch.h>
#include <TProofOutputFile.h>

//^######################################### Skip list for trace fitting
//TODO

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

/// in Process_Sort, copy the Mapping.h to ~/.proof/working/
#include "../working/Mapping.h"

//^######################################### FIT FUNCTION
const int numPara = 6;
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

TStopwatch stpWatch;

//^######################################### Class definition
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

    saveFile = nullptr;
    newSaveTree = nullptr;

    eE = nullptr;
    eT = nullptr;

    arr = nullptr;
    gTrace = nullptr;
    gFit = nullptr;
    arrTrapezoid = nullptr;
    gTrapezoid = nullptr;

    teE = nullptr;
    teT = nullptr;
    teR = nullptr;

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

  void SetUpTree();
  void DecodeOption();
  bool isParallel;

  unsigned long NumEntries;

  TString saveFileName;
  TFile * saveFile; //!
  TProofOutputFile * proofFile; //!
  TTree * newSaveTree; //!

  Float_t   ** eE; //!
  ULong64_t ** eT; //!

  //trace
  TClonesArray * arr ;//!
  TGraph * gTrace; //!
  TF1 * gFit; //!
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

  newSaveTree = new TTree("gen_tree", "Tree After GeneralSort");
  newSaveTree->SetDirectory(saveFile);
  newSaveTree->AutoSave();

  newSaveTree->Branch( "evID", &evID, "EventID/l"); // simply copy

  eE = new Float_t * [mapping::nDetType];
  eT = new ULong64_t * [mapping::nDetType];

  for( int i = 0 ; i < mapping::nDetType; i++){
    eE[i] = new Float_t[mapping::detNum[i]];
    eT[i] = new ULong64_t[mapping::detNum[i]];

    for( int j = 0; j < mapping::detNum[i]; j++){
      eE[i][j] =  TMath::QuietNaN();
      eT[i][j] = 0;
    }

    newSaveTree->Branch(  mapping::detTypeName[i].c_str(),        eE[i], Form("%s[%d]/F", mapping::detTypeName[i].c_str(), mapping::detNum[i]));
    newSaveTree->Branch( (mapping::detTypeName[i]+"_t").c_str(),  eT[i], Form("%s_Timestamp[%d]/l", mapping::detTypeName[i].c_str(), mapping::detNum[i]));
  }


  if( isTraceExist && traceMethod >= 0){

    arr = new TClonesArray("TGraph");

    newSaveTree->Branch("trace", arr, 256000);
    arr->BypassStreamer();

    if( traceMethod > 0 ){

      teE = new Float_t * [mapping::nDetType];
      teT = new Float_t * [mapping::nDetType];
      teR = new Float_t * [mapping::nDetType];

      for( int i = 0 ; i < mapping::nDetType; i++){
        teE[i] = new Float_t[mapping::detNum[i]];
        teT[i] = new Float_t[mapping::detNum[i]];
        teR[i] = new Float_t[mapping::detNum[i]];

        for( int j = 0; j <mapping::detNum[i]; j++){
          teE[i][j] =  TMath::QuietNaN();
          teT[i][j] =  TMath::QuietNaN();
          teR[i][j] =  TMath::QuietNaN();
        }

        //TODO use a blackList to skip some trace

        newSaveTree->Branch( ("w" + mapping::detTypeName[i]).c_str(),      teE[i], Form("trace_%s[%d]/F",      mapping::detTypeName[i].c_str(), mapping::detNum[i]));
        newSaveTree->Branch( ("w" + mapping::detTypeName[i]+"T").c_str(),  teT[i], Form("trace_%s_time[%d]/l", mapping::detTypeName[i].c_str(), mapping::detNum[i]));
        newSaveTree->Branch( ("w" + mapping::detTypeName[i]+"R").c_str(),  teR[i], Form("trace_%s_rise[%d]/l", mapping::detTypeName[i].c_str(), mapping::detNum[i]));
      }

    }

  }
  newSaveTree->Print("toponly"); //very important, otherwise the mac will blow up.
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
    printf(" ++++++++ Found Trace.\n");
    isTraceExist = true;
    fChain->SetBranchAddress("tl", tl, &b_tl);
    fChain->SetBranchAddress("trace", trace, &b_trace);
  }

  NumEntries = fChain->GetEntries();
  printf( " ========== total Entry : %ld\n", NumEntries);

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
  printf("\033[1;33m ===== Trace method ? %s \033[m\n", traceMethodStr);
}

#endif // #ifdef GeneralSort_cxx
