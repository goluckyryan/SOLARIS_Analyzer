//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Jul  8 13:26:58 2024 by ROOT version 6.32.02
// from TTree gen_tree/Tree After GeneralSort
// found on file: ../root_data/gen_run043.root
//////////////////////////////////////////////////////////

#ifndef MonAnalyzer_h
#define MonAnalyzer_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>

// Headers needed by this particular selector
#include "TClonesArray.h"



class MonAnalyzer : public TSelector {
public :
   TTreeReader     fReader;  //!the tree reader
   TTree          *fChain = 0;   //!pointer to the analyzed TTree or TChain

   // Readers to access the data (delete the ones you do not need).
   TTreeReaderValue<ULong64_t> EventID = {fReader, "evID"};
   TTreeReaderArray<Float_t> e = {fReader, "e"};
   TTreeReaderArray<ULong64_t> e_Timestamp = {fReader, "e_t"};
   TTreeReaderArray<Float_t> xf = {fReader, "xf"};
   TTreeReaderArray<ULong64_t> xf_Timestamp = {fReader, "xf_t"};
   TTreeReaderArray<Float_t> xn = {fReader, "xn"};
   TTreeReaderArray<ULong64_t> xn_Timestamp = {fReader, "xn_t"};
   TTreeReaderArray<Float_t> rdt = {fReader, "rdt"};
   TTreeReaderArray<ULong64_t> rdt_Timestamp = {fReader, "rdt_t"};
   TTreeReaderArray<unsigned int> trace_fUniqueID = {fReader, "trace.fUniqueID"};
   TTreeReaderArray<unsigned int> trace_fBits = {fReader, "trace.fBits"};
   TTreeReaderArray<TString> trace_fName = {fReader, "trace.fName"};
   TTreeReaderArray<TString> trace_fTitle = {fReader, "trace.fTitle"};
   TTreeReaderArray<Short_t> trace_fLineColor = {fReader, "trace.fLineColor"};
   TTreeReaderArray<Short_t> trace_fLineStyle = {fReader, "trace.fLineStyle"};
   TTreeReaderArray<Short_t> trace_fLineWidth = {fReader, "trace.fLineWidth"};
   TTreeReaderArray<Short_t> trace_fFillColor = {fReader, "trace.fFillColor"};
   TTreeReaderArray<Short_t> trace_fFillStyle = {fReader, "trace.fFillStyle"};
   TTreeReaderArray<Short_t> trace_fMarkerColor = {fReader, "trace.fMarkerColor"};
   TTreeReaderArray<Short_t> trace_fMarkerStyle = {fReader, "trace.fMarkerStyle"};
   TTreeReaderArray<Float_t> trace_fMarkerSize = {fReader, "trace.fMarkerSize"};
   TTreeReaderArray<Double_t> trace_fMinimum = {fReader, "trace.fMinimum"};
   TTreeReaderArray<Double_t> trace_fMaximum = {fReader, "trace.fMaximum"};
   TTreeReaderArray<TString> trace_fOption = {fReader, "trace.fOption"};
   TTreeReaderArray<Float_t> trace_e = {fReader, "we"};
   TTreeReaderArray<Float_t> trace_e_time = {fReader, "weT"};
   TTreeReaderArray<Float_t> trace_e_rise = {fReader, "weR"};
   TTreeReaderArray<Float_t> trace_xf = {fReader, "wxf"};
   TTreeReaderArray<Float_t> trace_xf_time = {fReader, "wxfT"};
   TTreeReaderArray<Float_t> trace_xf_rise = {fReader, "wxfR"};
   TTreeReaderArray<Float_t> trace_xn = {fReader, "wxn"};
   TTreeReaderArray<Float_t> trace_xn_time = {fReader, "wxnT"};
   TTreeReaderArray<Float_t> trace_xn_rise = {fReader, "wxnR"};
   TTreeReaderArray<Float_t> trace_rdt = {fReader, "wrdt"};
   TTreeReaderArray<Float_t> trace_rdt_time = {fReader, "wrdtT"};
   TTreeReaderArray<Float_t> trace_rdt_rise = {fReader, "wrdtR"};


   MonAnalyzer(TTree * /*tree*/ =0) { }
   ~MonAnalyzer() override { }
   Int_t   Version() const override { return 2; }
   void    Begin(TTree *tree) override;
   void    SlaveBegin(TTree *tree) override;
   void    Init(TTree *tree) override;
   bool    Notify() override;
   bool    Process(Long64_t entry) override;
   Int_t   GetEntry(Long64_t entry, Int_t getall = 0) override { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
   void    SetOption(const char *option) override { fOption = option; }
   void    SetObject(TObject *obj) override { fObject = obj; }
   void    SetInputList(TList *input) override { fInput = input; }
   TList  *GetOutputList() const override { return fOutput; }
   void    SlaveTerminate() override;
   void    Terminate() override;

   ClassDefOverride(MonAnalyzer,0);

};

#endif

#ifdef MonAnalyzer_cxx
void MonAnalyzer::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the reader is initialized.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   fReader.SetTree(tree);
}

bool MonAnalyzer::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return true;
}


#endif // #ifdef MonAnalyzer_cxx
