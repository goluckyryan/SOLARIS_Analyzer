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

  TString saveFileName;
  TFile * saveFile; //!
  TProofOutputFile * proofFile; //!
  TTree * newTree; //!

  //TODO ---- 2D array
  Float_t   *eE; //!
  ULong64_t *eT; //!

  Float_t   *xf ; //!
  ULong64_t *xfT; //!

  Float_t   *xn ; //!
  ULong64_t *xnT; //!

  Float_t   *rdt ; //!
  ULong64_t *rdtT; //!

  //trace
  TClonesArray * arr ;//!
  TGraph * gTrace; //!
  TClonesArray * arrTrapezoid ;//!   
  TGraph * gTrapezoid; //!

  Float_t   *teE; //!  
  Float_t   *teT; //! 
  Float_t   *teR; //!

  Float_t   *trdt ; //!
  Float_t   *trdtT ; //!
  Float_t   *trdtR ; //!

};

#endif

#ifdef GeneralSort_cxx

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
  TString option = GetOption();
  TObjArray * tokens = option.Tokenize(",");
  traceMethod = ((TObjString*) tokens->At(0))->String().Atoi();
  saveFileName = ((TObjString*) tokens->At(1))->String();

  if( isTraceExist ){
    PrintTraceMethod();
  }else{
    printf("++++++++ no Trace found\n");
  }

  proofFile = new TProofOutputFile(saveFileName, "M");
  saveFile = proofFile->OpenFile("RECREATE");
   
  newTree = new TTree("gen_tree", "Tree After GeneralSort");
  newTree->SetDirectory(saveFile);
  newTree->AutoSave();

  detNum = ExtractDetNum(mapping, detName, detMaxID);

  eE  = new Float_t[detNum[0]];
  xf  = new Float_t[detNum[0]];
  xn  = new Float_t[detNum[0]];
  eT  = new ULong64_t[detNum[0]];
  xfT = new ULong64_t[detNum[0]];
  xnT = new ULong64_t[detNum[0]];

  rdt  = new Float_t[detNum[1]];
  rdtT = new ULong64_t[detNum[1]];

  newTree->Branch("e",      eE, Form("Energy[%d]/F", detNum[0]));
  newTree->Branch("e_t",    eT, Form("Energy_Timestamp[%d]/l", detNum[0]));

  newTree->Branch("xf",     xf, Form("XF[%d]/F", detNum[0]));
  newTree->Branch("xf_t",  xfT, Form("XF_Timestamp[%d]/l", detNum[0]));

  newTree->Branch("xn",     xn, Form("XN[%d]/F", detNum[0]));
  newTree->Branch("xn_t",  xnT, Form("XN_Timestamp[%d]/l", detNum[0]));

  newTree->Branch("rdt",     rdt, Form("Recoil[%d]/F", detNum[1]));
  newTree->Branch("rdt_t",  rdtT, Form("Recoil_Timestamp[%d]/l", detNum[1]));

  if( isTraceExist && traceMethod >= 0){

    arr = new TClonesArray("TGraph");

    newTree->Branch("trace", arr, 256000);
    arr->BypassStreamer();

    if( traceMethod > 0 ){

      teE = new Float_t[detNum[0]];
      teT = new Float_t[detNum[0]];
      teR = new Float_t[detNum[0]];

      trdt  = new Float_t[detNum[1]];
      trdtT = new Float_t[detNum[1]];
      trdtR = new Float_t[detNum[1]];

      newTree->Branch("te",    teE, Form("Trace_Energy[%d]/F", detNum[0]));
      newTree->Branch("te_t",  teT, Form("Trace_Energy_Time[%d]/F", detNum[0]));
      newTree->Branch("te_r",  teR, Form("Trace_Energy_RiseTime[%d]/F", detNum[0]));

      newTree->Branch("trdt",    trdt , Form("Trace_RDT[%d]/F", detNum[1]));
      newTree->Branch("trdt_t",  trdtT, Form("Trace_RDT_Time[%d]/F", detNum[1]));
      newTree->Branch("trdt_r",  trdtR, Form("Trace_RDT_RiseTime[%d]/F", detNum[1]));
    }

  }


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
