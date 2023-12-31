#ifndef Utilities
#define Utilities

#include <TMath.h>
#include <TCanvas.h>

//This file runs after on Monitor.C
//This file is parasite on Monitor.C

int canvasSize[2] = {2000, 1200};

void listDraws(void) {
  printf("------------------- List of Plots -------------------\n");
  printf("  newCanvas() - Create a new Canvas\n");
  printf("-----------------------------------------------------\n");
  printf("      rawID() - Raw \033[0;31me\033[0m, \033[0;31mring\033[0m, \033[0;31mxf\033[0m, \033[0;31mxn\033[0m vs detID\n");
  printf("       rawe() - Raw \033[0;31me\033[0m for all %d detectors\n", numDet);
  printf("      rawxf() - Raw \033[0;31mxf\033[0m for all %d detectors\n", numDet);
  printf("      rawxn() - Raw \033[0;31mxn\033[0m for all %d detectors\n", numDet);
  printf("   rawxfVxn() - Raw \033[0;31mxf\033[0m vs. \033[0;31mxn\033[0m for all %d detectors\n", numDet);
  printf("    raweVxs() - Raw \033[0;31me\033[0m vs. Raw \033[0;31mxs = xf + xn\033[0m for all %d detectors\n", numDet);
  printf("     raweVx() - Raw \033[0;31me\033[0m vs. RAW \033[0;31mx\033[0m for all %d detectors\n", numDet);
  printf("-----------------------------------------------------\n");
  printf("    eVxsCal() - Raw \033[0;31me\033[0m vs. Corrected \033[0;31mxs\033[0m for all %d detectors\n", numDet);
  printf("       ecal() - Calibrated \033[0;31me\033[0m for all %d detectors\n", numDet);
  printf("xfCalVxnCal() - Calibrated \033[0;31mxf\033[0m vs. \033[0;31mxn\033[0m for all %d detectors\n", numDet);
  printf("  eCalVxCal() - Cal \033[0;31me\033[0m vs. \033[0;31mx\033[0m for all %d detectors\n", numDet);
  printf("-----------------------------------------------------\n");
  printf("    recoils() - Raw DE vs. E Recoil spectra\n");
  //printf("       elum() - Luminosity Energy Spectra\n");
  //printf("         ic() - Ionization Chamber Spectra\n");
  printf("-----------------------------------------------------\n");
  printf("     eCalVz() - Energy vs. Z\n");
  printf("  eCalVzRow() - Energy vs. Z for each row\n");
  printf("     excite() - Excitation Energy\n");
  printf("  ExThetaCM() - Ex vs ThetaCM\n");
  printf("    ExVxCal() - Ex vs X for all %d detectors\n", numDet);
  printf("-----------------------------------------------------\n");
  printf("   ShowFitMethod() - Shows various fitting methods \n");
  printf("   RDTCutCreator() - Create RDT Cuts [May need to edit]\n");
  printf("   Check_rdtGate() - Check RDT Cuts.  \n");
  printf("       readTrace() - read trace from gen_runXXX.root \n");
  printf("    readRawTrace() - read trace from runXXX.root \n");
  printf("         Check1D() - Count Integral within a range\n");
  printf("-----------------------------------------------------\n");
  printf("   %s\n", canvasTitle.Data());
  printf("-----------------------------------------------------\n");
}

int xD, yD;
void FindBesCanvasDivision(int nPad){
  for( int i = TMath::Sqrt(nPad); i >= 2 ; i--){
    if( nPad % i == 0 ) {
      yD = i;
      xD = nPad/i;      
      break;
    }
  }
}

int nCanvas=0;
void newCanvas(int sizeX = 800, int sizeY = 600, int posX = 0, int posY = 0){
  TString name; name.Form("cNewCanvas%d | %s", nCanvas, canvasTitle.Data());
  TCanvas * cNewCanvas = new TCanvas(name, name, posX, posY, sizeX, sizeY);
  nCanvas++;
  cNewCanvas->cd();
}

void rawID(){
  TCanvas * cRawID = (TCanvas *) gROOT->FindObjectAny("cRawID");
  if( cRawID == NULL ) cRawID = new TCanvas("cRawID", Form("Raw e, Ring, xf, xn vs ID | %s", canvasTitle.Data()), canvasSize[0], canvasSize[1]);
  cRawID->Clear();cRawID->Divide(2,2);
  cRawID->cd(1);  cRawID->cd(1)->SetGrid(); heVID->Draw("colz");
  cRawID->cd(2);  cRawID->cd(2)->SetGrid(); hMultiHit->Draw();
  cRawID->cd(3);  cRawID->cd(3)->SetGrid(); hxfVID->Draw("colz");
  cRawID->cd(4);  cRawID->cd(4)->SetGrid(); hxnVID->Draw("colz");
}

void rawe(Bool_t isLogy = false) {
  TCanvas *cRawE = (TCanvas *) gROOT->FindObjectAny("cRawE");
  if( cRawE == NULL ) cRawE = new TCanvas("cRawE",Form("E raw | %s", canvasTitle.Data()),canvasSize[0], canvasSize[1]);
  cRawE->Clear();cRawE->Divide(numCol,numRow);
  for (Int_t i=0; i < numDet; i++) {
    cRawE->cd(i+1); 
    cRawE->cd(i+1)->SetGrid();
    if( isLogy ) cRawE->cd(i+1)->SetLogy();
    he[i]->Draw("");
  }
}

void rawxf(Bool_t isLogy = false) {
  TCanvas *cRawXf = (TCanvas *) gROOT->FindObjectAny("cRawXf");
  if( cRawXf == NULL ) cRawXf = new TCanvas("cRawXf",Form("Xf raw | %s", canvasTitle.Data()),canvasSize[0], canvasSize[1]);
  cRawXf->Clear();cRawXf->Divide(numCol,numRow);
  for (Int_t i=0; i<numDet; i++) {
    cRawXf->cd(i+1); 
    cRawXf->cd(i+1)->SetGrid();
    if( isLogy ) cRawXf->cd(i+1)->SetLogy();
    hxf[i]->Draw("");
  }
}

void rawxn(Bool_t isLogy = false) {
  TCanvas *cRawXn = (TCanvas *) gROOT->FindObjectAny("cRawXn");
  if( cRawXn == NULL ) cRawXn = new TCanvas("cRawXn",Form("Xn raw | %s", canvasTitle.Data()),canvasSize[0], canvasSize[1]);
  cRawXn->Clear();cRawXn->Divide(numCol,numRow);
  for (Int_t i=0; i<numDet; i++) {
    cRawXn->cd(i+1); 
    cRawXn->cd(i+1)->SetGrid();
    if( isLogy ) cRawXn->cd(i+1)->SetLogy();
    hxn[i]->Draw("");
  }
}

void rawxfVxn(void) {
  TCanvas *cxfxn = (TCanvas *) gROOT->FindObjectAny("cxfxn");
  if( cxfxn == NULL ) cxfxn = new TCanvas("cxfxn",Form("XF vs. XN | %s", canvasTitle.Data()),canvasSize[0], canvasSize[1]);
  cxfxn->Clear(); cxfxn->Divide(numCol,numRow);
  for (Int_t i=0;i<numDet;i++) {
    cxfxn->cd(i+1); 
    cxfxn->cd(i+1)->SetGrid(); 
    hxfVxn[i]->Draw("col");
  }
}

void raweVxs(void) {
  TCanvas *cxfxne = (TCanvas *) gROOT->FindObjectAny("cxfxne");
  if( cxfxne == NULL ) cxfxne = new TCanvas("cxfxne",Form("E - XF+XN | %s", canvasTitle.Data()),canvasSize[0], canvasSize[1]);
  cxfxne->Clear(); cxfxne->Divide(numCol,numRow);
  TLine line(0,0, 4000, 4000); line.SetLineColor(2);
  for (Int_t i=0;i<numDet;i++) {
    cxfxne->cd(i+1); 
    cxfxne->cd(i+1)->SetGrid(); 
    heVxs[i]->Draw("col");
    line.Draw("same");
  }
}

void raweVx(void) {
  TCanvas *ceVx = (TCanvas *) gROOT->FindObjectAny("ceVx");
  if(ceVx == NULL) ceVx = new TCanvas("ceVx",Form("E vs. X = (xf-xn)/e | %s", canvasTitle.Data()),canvasSize[0], canvasSize[1]);
  ceVx->Clear(); ceVx->Divide(numCol,numRow);
  for (Int_t i=0;i<numDet;i++) {
    ceVx->cd(i+1); heVx[i]->Draw("col");
  }
}

void eVxsCal(void) {
  TCanvas *cxfxneC = (TCanvas *) gROOT->FindObjectAny("cxfxneC");
  if(cxfxneC == NULL)cxfxneC = new TCanvas("cxfxneC",Form("Raw E - Corrected XF+XN | %s", canvasTitle.Data()),canvasSize[0], canvasSize[1]);
  cxfxneC->Clear(); cxfxneC->Divide(numCol,numRow);
  TLine line(0,0, 4000, 4000); line.SetLineColor(2);
  for (Int_t i=0;i<numDet;i++) {
    cxfxneC->cd(i+1); 
    cxfxneC->cd(i+1)->SetGrid(); 
    heVxsCal[i]->Draw("col");
    line.Draw("same");
  }
}

void ecal(void) {
  TCanvas *cEC = (TCanvas *) gROOT->FindObjectAny("cEC");
  if(cEC == NULL) cEC = new TCanvas("cEC",Form("E corrected | %s", canvasTitle.Data()),canvasSize[0], canvasSize[1]);
  cEC->Clear();cEC->Divide(numCol,numRow);
  for (Int_t i=0; i<numDet; i++) {
    cEC->cd(i+1); 
    cEC->cd(i+1)->SetGrid();
    heCal[i]->Draw("");
  }
  
  TCanvas *cEC2 = (TCanvas *) gROOT->FindObjectAny("cEC2");
  if(cEC2 == NULL) cEC2 = new TCanvas("cEC2",Form("E corrected | %s", canvasTitle.Data()),canvasSize[0], canvasSize[1]);
  cEC2->Clear();
  heCalID->Draw("colz");
}

void xfCalVxnCal(void) {
  TCanvas *cxfxnC = (TCanvas *) gROOT->FindObjectAny("cxfxnC");
  if(cxfxnC == NULL) cxfxnC = new TCanvas("cxfxnC",Form("XF vs XN corrected | %s", canvasTitle.Data()),canvasSize[0], canvasSize[1]);
  cxfxnC->Clear(); cxfxnC->Divide(numCol,numRow);
  for (Int_t i=0;i<numDet;i++) {
    cxfxnC->cd(i+1); 
    cxfxnC->cd(i+1)->SetGrid(); 
    hxfCalVxnCal[i]->Draw("col");
  }
}

void eCalVxCal(void) {
  TCanvas *cecalVxcal =  (TCanvas *) gROOT->FindObjectAny("cecalVxcal");
  if( cecalVxcal == NULL ) cecalVxcal = new TCanvas("cecalVxcal",Form("ECALVXCAL | %s",canvasTitle.Data()),canvasSize[0], canvasSize[1]);
  cecalVxcal->Clear(); cecalVxcal->Divide(numCol,numRow);
  for (Int_t i=0;i<numDet;i++) {
    cecalVxcal->cd(i+1); 
    heCalVxCal[i]->SetMarkerStyle(7);
    heCalVxCal[i]->Draw("");
  } 
}

void recoils(bool isLogz = false) {
  TCanvas *crdt =  (TCanvas *) gROOT->FindObjectAny("crdt");
  if( crdt == NULL ) crdt = new TCanvas("crdt",Form("raw RDT | %s", canvasTitle.Data()),1700, 0, 1000,1000);
  crdt->Clear();crdt->Divide(2,2);

  if( isLogz ) crdt->cd(1)->SetLogz(); crdt->cd(1); hrdt2D[0]->Draw("col");  
  if( isLogz ) crdt->cd(2)->SetLogz(); crdt->cd(2); hrdt2D[1]->Draw("col");  
  if( isLogz ) crdt->cd(3)->SetLogz(); crdt->cd(3); hrdt2D[3]->Draw("col");  
  if( isLogz ) crdt->cd(4)->SetLogz(); crdt->cd(4); hrdt2D[2]->Draw("col");   
  
  TCanvas *crdtID =  (TCanvas *) gROOT->FindObjectAny("crdtID");
  if( crdtID == NULL ) crdtID = new TCanvas("crdtID",Form("raw RDT ID | %s", canvasTitle.Data()),0,0, 500, 500);
  crdtID->Clear();
  if( isLogz ) crdtID->SetLogz(); 
  hrdtID->Draw("colz");
  
  TCanvas *crdtS =  (TCanvas *) gROOT->FindObjectAny("crdtS");
  if( crdtS == NULL ) crdtS = new TCanvas("crdtS",Form("raw RDT | %s", canvasTitle.Data()),600, 0, 1000, 1000);
  crdtS->Clear(); crdtS->Divide(2,4);
  for( int i = 0; i < 8; i ++){
    crdtS->cd(i+1);
    if( isLogz ) crdtS->cd(i+1)->SetLogy(); 
    hrdt[i]->Draw("");
  }
}

// void elum(void) {
//   TCanvas *celum = (TCanvas *) gROOT->FindObjectAny("celum");
//   if( celum == NULL )  celum = new TCanvas("celum",Form("ELUM | %s", canvasTitle.Data()),1000,1000);
//   celum->Clear(); celum->Divide(4,4);
//   for( int i = 0 ; i < 16 ; i++){
//     celum->cd(i+1);
//     helum[i]->Draw("");
//   }
  
//   TCanvas *celumID = (TCanvas *) gROOT->FindObjectAny("celumID");
//   if( celumID == NULL )  celumID = new TCanvas("celumID",Form("ELUM-ID | %s", canvasTitle.Data()),1100, 0, 500,500);
//   celumID->Clear();
//   helumID->Draw("colz");
  
// }

  
// void ic(){  
//   TCanvas *cic =  (TCanvas *) gROOT->FindObjectAny("cic");
//   if( cic == NULL ) cic = new TCanvas("cic",Form("Ionization Chamber | %s", canvasTitle.Data() ),1200,800);
  
//   cic->Clear(); cic->SetGrid(0); cic->Divide(3,2);
//   gStyle->SetOptStat("neiou");
  
//   cic->cd(1); hic0->Draw();
//   cic->cd(2); hic1->Draw();
//   cic->cd(3); hic2->Draw();
//   cic->cd(4); hic01->Draw("colz");
//   cic->cd(5); hic02->Draw("colz");
//   cic->cd(6); hic12->Draw("colz");  
// }

void eCalVz(void) {
  TCanvas *cecalVz =  (TCanvas *) gROOT->FindObjectAny("cecalVz");
  if( cecalVz == NULL ) cecalVz = new TCanvas("cevalVz",Form("ECALVZ : %s", canvasTitle.Data()),1000,650);
  cecalVz->Clear(); cecalVz->Divide(2,1);
  gStyle->SetOptStat("neiou");
  cecalVz->cd(1);heCalVz->Draw("col");
  cecalVz->cd(2);heCalVzGC->Draw("col");
}

void eCalVzRow() {
  TCanvas *cecalVzRow =  (TCanvas *) gROOT->FindObjectAny("cecalVzRow");
  if( cecalVzRow == NULL ) cecalVzRow = new TCanvas("cevalVzRow",Form("eCal - Z : %s", canvasTitle.Data()),canvasSize[0], canvasSize[1]);
  FindBesCanvasDivision(numRow);
  cecalVzRow->Clear(); cecalVzRow->Divide(xD,yD);
  gStyle->SetOptStat("neiou");
  
  for(int row = 0; row < numRow; row ++){
     cecalVzRow->cd(row+1);
     cecalVzRow->cd(row+1)->SetGrid();
     hecalVzRow[row]->Draw("colz");
  }
}

void excite(void) {
  TCanvas *cex =  (TCanvas *) gROOT->FindObjectAny("cex");
  if( cex == NULL ) cex = new TCanvas("cex",Form("EX : %s", canvasTitle.Data()),0, 0, 1000,650);
  cex->Clear();
  gStyle->SetOptStat("neiou");
  hEx->Draw("");
  
  
  TCanvas *cexI =  (TCanvas *) gROOT->FindObjectAny("cexI");
  if( cexI == NULL ) cexI = new TCanvas("cexI",Form("EX : %s", canvasTitle.Data()),500, 0, 1600,1000);
  cexI->Clear();cexI->Divide(numCol,numRow);
  gStyle->SetOptStat("neiou");
  for( int i = 0; i < numDet; i++){
    cexI->cd(i+1); 
    hExi[i]->Draw("");
  }
}

void ExThetaCM(void) {
  TCanvas *cExThetaCM =  (TCanvas *) gROOT->FindObjectAny("cExThetaCM");
  if( cExThetaCM == NULL ) cExThetaCM = new TCanvas("cExThetaCM",Form("EX - ThetaCM | %s", canvasTitle.Data()),650,650);
  cExThetaCM->Clear();
  gStyle->SetOptStat("neiou");
  hExThetaCM->Draw("colz");
}

void ExVxCal(TString drawOpt = "") {
  TCanvas *cExVxCal =  (TCanvas *) gROOT->FindObjectAny("cExVxCal");
  if( cExVxCal == NULL ) cExVxCal = new TCanvas("cExVxCal",Form("EX | %s", canvasTitle.Data()),1600,1000);
  cExVxCal->Clear();
  gStyle->SetOptStat("neiou");
  
  cExVxCal->Divide(numCol,numRow);
  for( int i = 0; i < numDet; i++){
    cExVxCal->cd(i+1); 
    if( drawOpt == "" )hExVxCal[i]->SetMarkerStyle(7);
    hExVxCal[i]->Draw(drawOpt);
  }
}

void Count1DH(TString name, TH1F * hist, TCanvas * canvas, int padID,  double x1, double x2, Color_t color){
   
   int k1 = hist->FindBin(x1);
   int k2 = hist->FindBin(x2);

   int hight = 0 ;
   for( int i = k1; i < k2 ; i ++){
    int temp = hist->GetBinContent(i);
    if( temp > hight ) hight = temp;
   }
   hight = hight * 1.2;
   int max = hist->GetMaximum();
   
   canvas->cd(padID);
   
   if( color != 0 ){ 
     TBox box;
     box.SetFillColorAlpha(color, 0.1);
     box.DrawBox(x1, 0, x2, hight);
   }

   int count = hist->Integral(k1, k2);

   TLatex text;
   text.SetTextFont(82);
   text.SetTextSize(0.06);
   if( color != 0 ){
     text.SetTextColor(color);
     text.DrawLatex(x1, hight, Form("%d", count));
   }else{
     text.DrawLatex((x1+x2)/2., max, Form("%d", count));
   }

   printf(" %s  : %d \n", name.Data(),  count);
   
}



#endif 
