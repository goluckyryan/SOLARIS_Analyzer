#ifndef Monitor_h
#define Monitor_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include <TH1.h>
#include <TMath.h>
#include <TH2.h>
#include <TStyle.h>
#include <TCutG.h>

#include "Mapping.h"
#include "../armory/AnalysisLib.h"

class Monitor : public TSelector {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain

   // Declaration of leaf types
   Float_t        * e; //!
   ULong64_t      * e_t; //!
   Float_t        * xf; //!
   ULong64_t      * xf_t; //!
   Float_t        * xn; //!
   ULong64_t      * xn_t; //!
   Float_t        * rdt; //!
   ULong64_t      * rdt_t; //!
   
   // List of branches
   TBranch        *b_Energy;   //!
   TBranch        *b_EnergyTimestamp;   //!
   TBranch        *b_XF;   //!
   TBranch        *b_XFTimestamp;   //!
   TBranch        *b_XN;   //!
   TBranch        *b_XNTimestamp;   //!
   TBranch        *b_RDT;   //!
   TBranch        *b_RDTTimestamp;   //!

   // trace analysis data
   // Float_t        * we; //!
   // Float_t        * weR; //!
   // Float_t        * weT; //!
   // Float_t        * wrdt; //!
   // Float_t        * wrdtT; //!
   // Float_t        * wrdtR; //!
   
   // TBranch        *b_Trace_Energy;   //!
   // TBranch        *b_Trace_Energy_RiseTime;   //!
   // TBranch        *b_Trace_Energy_Time;   //!
   // TBranch        *b_Trace_RDT;   //!
   // TBranch        *b_Trace_RDT_Time;   //!
   // TBranch        *b_Trace_RDT_RiseTime;   //!
   
   bool isArrayTraceExist;
   bool isRDTTraceExist;

   bool isRDTExist;

   bool isXNCorrOK;
   bool isXFXNCorrOK;
   bool isXScaleCorrOK;
   bool isECorrOK;
   bool isRDTCorrOK;

   //==== global variable
   float * x, * z;
   float * xCal, * xfCal, * xnCal, * eCal;
   
   std::vector<ULong64_t> startTime ;
   std::vector<ULong64_t> endTime ;

   double timeRangeInMin[2];
   ULong64_t baseTimeStamp;
   int treeID;

   int padID;

   Monitor(TTree * /*tree*/ =0) : fChain(0) {

        e   = new Float_t   [mapping::NARRAY];
       xf   = new Float_t   [mapping::NARRAY];
       xn   = new Float_t   [mapping::NARRAY];
      rdt   = new Float_t   [mapping::NRDT];
      e_t   = new ULong64_t [mapping::NARRAY];
      xf_t  = new ULong64_t [mapping::NARRAY];
      xn_t  = new ULong64_t [mapping::NARRAY];
      rdt_t = new ULong64_t [mapping::NRDT];

      x     = new float [mapping::NARRAY];
      z     = new float [mapping::NARRAY];
      xCal  = new float [mapping::NARRAY];
      xfCal = new float [mapping::NARRAY];
      xnCal = new float [mapping::NARRAY];
      eCal  = new float [mapping::NARRAY];

      isXNCorrOK = true;
      isXFXNCorrOK = true;
      isXScaleCorrOK = true;
      isECorrOK = true;
      isRDTCorrOK = true;

      padID = 0;

      timeRangeInMin[0] = 0;
      timeRangeInMin[1] = 100;

      startTime.clear();
      endTime.clear();

      baseTimeStamp = 0;
      treeID = -1;

   }
   virtual ~Monitor() {

      delete   e  ;
      delete  xf  ;
      delete  xn  ;
      delete rdt  ;
      delete e_t  ;
      delete xf_t ;
      delete xn_t ;
      delete rdt_t;

      delete z ;
      delete x ;
      delete xCal;
      delete xfCal;
      delete xnCal;
      delete eCal;

   }
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

   TString fCanvasTitle;
   void SetCanvasTitle(TString title) {fCanvasTitle = title;}
   TString GetCanvasTitle() const {return fCanvasTitle;}

   void SetStartStopTimes(std::vector<ULong64_t> t1, std::vector<ULong64_t> t2) {
      startTime = t1;
      endTime = t2;

      timeRangeInMin[0] = startTime[0] * tick2min;
      timeRangeInMin[1] = endTime[0] * tick2min;
      for( int i = 1; i < (int) endTime.size(); i++) timeRangeInMin[1] +=  ((endTime[i] - startTime[i]) * tick2min);  

      double duration = timeRangeInMin[1] - timeRangeInMin[0];

      timeRangeInMin[0] = TMath::Floor( timeRangeInMin[0] - duration * 0.1);
      timeRangeInMin[1] = TMath::Ceil( timeRangeInMin[1] + duration * 0.1);

   }
   
   void Draw2DHist(TH2F * hist);
   
   void PlotEZ(bool isRaw);
   void PlotTDiff(bool isGated, bool isLog);
   void PlotRDT(int id, bool isRaw);
   //void PlotCRDTPolar();

   template<typename T> void CreateListOfHist1D(T ** &histList, int size, const char * namePrefix, const char * TitleForm, int binX, float xMin, float xMax);
   template<typename T> void CreateListOfHist2D(T ** &histList, int size, const char * namePrefix, const char * TitleForm, int binX, float xMin, float xMax, int binY, float yMin, float yMax);

   ClassDef(Monitor,0);
};

#endif


#ifdef Monitor_cxx
void Monitor::Init(TTree *tree){

   printf("============================================ Branch Pointer Inititization. \n");

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("e",       e, &b_Energy);
   fChain->SetBranchAddress("e_t",   e_t, &b_EnergyTimestamp);
   fChain->SetBranchAddress("xf",     xf, &b_XF);
   fChain->SetBranchAddress("xf_t", xf_t, &b_XFTimestamp);
   fChain->SetBranchAddress("xn",     xn, &b_XN);
   fChain->SetBranchAddress("xn_t", xn_t, &b_XNTimestamp);

   TBranch * br = (TBranch *) fChain->GetListOfBranches()->FindObject("rdt");
   if( br == NULL ){
      printf(" ++++++++ no Recoil Branch.\n");
      isRDTExist = false;
   }else{
      printf(" ++++++++ Found Recoil Branch.\n");
      isRDTExist = true;
      fChain->SetBranchAddress("rdt"  , rdt,   &b_RDT);
      fChain->SetBranchAddress("rdt_t", rdt_t, &b_RDTTimestamp);
   }
   
   /*
   br = (TBranch *) fChain->GetListOfBranches()->FindObject("we");
   if( br == NULL ){
      printf(" ++++++++ no Array trace.\n");
      isArrayTraceExist = false;
   }else{
      isArrayTraceExist = true;
      if( isUseArrayTrace ){
         fChain->SetBranchAddress("te", e, &b_Energy);// replace  e with te
      }else{
         fChain->SetBranchAddress("te", te, &b_Trace_Energy);
      }
      fChain->SetBranchAddress("te_r", te_r, &b_Trace_Energy_RiseTime);
      fChain->SetBranchAddress("te_t", te_t, &b_Trace_Energy_Time);
   }
   
   br = (TBranch *) fChain->GetListOfBranches()->FindObject("wrdt");
   if( br == NULL ){
      printf(" ++++++++ no Recoil trace.\n");
      isRDTTraceExist = false;
   }else{
      isRDTTraceExist = true;
      if( isUseRDTTrace ) {
         fChain->SetBranchAddress("trdt", rdt, &b_RDT); // replace  rdt with trdt
         printf("************ using Trace in recoil \n");
      }else{
         fChain->SetBranchAddress("trdt", trdt, &b_Trace_RDT);
      }
      fChain->SetBranchAddress("trdt_t", trdt_t, &b_Trace_RDT_Time);
      fChain->SetBranchAddress("trdt_r", trdt_r, &b_Trace_RDT_RiseTime);
   }
   */
   
   printf("============================================ End of Branch Pointer Inititization. \n");
}

Bool_t Monitor::Notify(){

   return kTRUE;
}

void DrawLine(TH1 * hist, double pos){
   
   double yMax = hist->GetMaximum();
   TLine * line = new TLine(pos, 0, pos, yMax);
   line->SetLineColor(2);
   line->Draw("");
   
}
void Monitor::SlaveBegin(TTree * /*tree*/){
   /// not use, if use, place in Monitor.C
   TString option = GetOption();
}


void Monitor::SlaveTerminate(){
  /// not use, if use, place in Monitor.C
}

template<typename T> void Monitor::CreateListOfHist1D(T ** &histList, 
                                                    int size, 
                                                    const char * namePrefix, 
                                                    const char * TitleForm, 
                                                    int binX, float xMin, float xMax){

   //printf(" Making %d of %s.\n", size, namePrefix);
   histList = new T * [size];
   for(int i = 0; i < size; i++) histList[i] = new T(Form("%s%d", namePrefix, i), Form(TitleForm, i), binX, xMin, xMax);  
}

template<typename T> void Monitor::CreateListOfHist2D(T ** &histList, 
                                                    int size, 
                                                    const char * namePrefix, 
                                                    const char * TitleForm, 
                                                    int binX, float xMin, float xMax,
                                                    int binY, float yMin, float yMax){

   //printf(" Making %d of %s.\n", size, namePrefix);
   histList = new T * [size];
   for(int i = 0; i < size; i++) histList[i] = new T(Form("%s%d", namePrefix, i), Form(TitleForm, i), binX, xMin, xMax, binY, yMin, yMax);   
}

/*###########################################################
 * Plotting Function
###########################################################*/

void DrawBox(TH1* hist, double x1, double x2, Color_t color, float alpha){

   double yMax = hist->GetMaximum();
   TBox * box = new TBox (x1, 0, x2, yMax);
   box->SetFillColorAlpha(color, alpha);
   box->Draw();

}

void Monitor::Draw2DHist(TH2F * hist){
   
   if( hist->Integral() < 3000 ){
      hist->SetMarkerStyle(20);
      hist->SetMarkerSize(0.3);
      hist->Draw("");
   }else{
      hist->Draw("colz"); 
   }
}


void Monitor::PlotEZ(bool isRaw){
   padID++; cCanvas->cd(padID);

   if( isRaw ) {
      Draw2DHist(heCalVz);
      heCalVz->SetTitle("E vs Z | " + canvasTitle + " | " + rdtCutFile1);
      if( skipDetID.size() > 0 ){
         text.DrawLatex(0.15, 0.3, "skipped Detector:");
         for( int i = 0; i < (int) skipDetID.size(); i++){
            text.DrawLatex(0.15 + 0.1*i, 0.25, Form("%d", skipDetID[i]));
         }
      }

      text.DrawLatex(0.15, 0.8, Form("%.1f < eCal < %.1f MeV", eCalCut[0], eCalCut[1]));
      if( xGate < 1 ) text.DrawLatex(0.15, 0.75, Form("with |x-0.5|<%.4f", xGate/2.));

   }else{
      Draw2DHist(heCalVzGC);
   
      if( cutList1 ) text.DrawLatex(0.15, 0.8, "with Recoil gate");
      if( xGate < 1 ) text.DrawLatex(0.15, 0.75, Form("with |x-0.5|<%.4f", xGate/2.));
      //if( isTACGate ) text.DrawLatex(0.15, 0.7, Form("%d < TAC < %d", tacGate[0], tacGate[1]));
      if(isTimeGateOn)text.DrawLatex(0.15, 0.7, Form("%d < coinTime < %d", timeGate[0], timeGate[1]));

   }
   
   TFile * transfer = new TFile("transfer.root");
   TObjArray * gList  = NULL ;
   TObjArray * fxList = NULL ;
   int nGList = 0;
   int nFxList = 0;
   if( transfer->IsOpen() ) {
     gList  = (TObjArray *) transfer->FindObjectAny("gList");
     nGList = gList->GetLast() + 1;
     fxList = (TObjArray *) transfer->FindObjectAny("fxList");
     nFxList = fxList->GetLast() +1 ;
   }
   
   ///the constant thetaCM line
   if( transfer->IsOpen() ) gList->At(0)->Draw("same");
   ///the e-z line for excitation 
   if( transfer->IsOpen() ){
      for( int i = 0 ; i < nFxList ; i++){
         ((TF1*)fxList->At(i))->SetLineColor(6);
         fxList->At(i)->Draw("same");
      }
   }

}

void Monitor::PlotTDiff(bool isGated, bool isLog){
   padID++; cCanvas->cd(padID); 
   if( isLog ) cCanvas->cd(padID)->SetLogy(1);
   double yMax = 0;
   if( isGated ){
      yMax = htdiff->GetMaximum()*1.2;
      if( isLog ){
         htdiff->GetYaxis()->SetRangeUser(1, yMax);
      }else{
         htdiff->GetYaxis()->SetRangeUser(0, yMax);
      }
   }
   htdiff->Draw();
   if( isGated ){
      htdiffg->SetLineColor(2);
      htdiffg->Draw("same");
   }
   
   if( cutList1 ) text.DrawLatex(0.15, 0.8, "with Recoil gate");
   if(isTimeGateOn)text.DrawLatex(0.15, 0.7, Form("%d < coinTime < %d", timeGate[0], timeGate[1])); 
   DrawBox(htdiff, timeGate[0], timeGate[1], kGreen, 0.2);
}

void Monitor::PlotRDT(int id, bool isRaw){
   padID++; cCanvas->cd(padID);

   if( isRaw ){
      Draw2DHist(hrdt2D[id]);
   }else{
      Draw2DHist(hrdt2Dg[id]);
   }
   if(isTimeGateOn)text.DrawLatex(0.15, 0.8, Form("%d < coinTime < %d", timeGate[0], timeGate[1])); 
   //if( isTACGate ) text.DrawLatex(0.15, 0.7, Form("%d < TAC < %d", tacGate[0], tacGate[1]));
   if( cutList1 && cutList1->GetEntries() >  id ) {cutG = (TCutG *)cutList1->At(id) ; cutG->Draw("same");}
   if( cutList2 && cutList2->GetEntries() >  id ) {cutG = (TCutG *)cutList2->At(id) ; cutG->Draw("same");}

}

//void Monitor::PlotCRDTPolar(){
//  padID++; cCanvas->cd(padID);
//  cCanvas->cd(padID)->DrawFrame(-50, -50, 50, 50);
//  hcrdtPolar->Draw("same colz pol");
//}

#endif // #ifdef Monitor_cxx
