#include "TROOT.h"

#include "TChain.h"
#include "TFile.h"
#include "TEfficiency.h"
#include "TH1.h"
#include "TH2.h"
#include "TGraphErrors.h"
#include "TCut.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TMath.h"
#include "TLine.h"
#include "TLatex.h"
#include "TRatioPlot.h"

#include <glob.h>
#include <iostream>
#include <iomanip>
#include <vector>

std::vector<std::string> fileVector(const std::string& pattern){
  glob_t glob_result;
  glob(pattern.c_str(),GLOB_TILDE,NULL,&glob_result);
  std::vector<std::string> files;
  for(unsigned int i=0;i<glob_result.gl_pathc;++i){
    files.push_back(std::string(glob_result.gl_pathv[i]));
  }
  globfree(&glob_result);
  return files;
}

void CharmJetKinematics() {
  //Please label the plot with indicating the particle ID. Normalize your counts to an integrated luminosity of 10 fb-1

  TString title("Charm Jet Momentum vs. Theta (#intL=100 fb^{-1} )"); // Title of your plot

  TChain* tree = new TChain("Delphes");
  auto files = fileVector(Form("../../CC_DIS_e10_p275_CT18NNLO/*/out.root"));

  for (auto file : files) 
    {
      tree->Add(file.c_str());
    }


  //Please keep the following binning so that we can compare across different WG
  TH2D *h=new TH2D("h",title.Data(),36,0.,TMath::Pi(),50.,0.,100.);
  
  //Fill your 2D histogram with Theta (in deg) and Momenta (in GeV) for a given particle ID
  tree->Draw("GenJet.PT*TMath::CosH(GenJet.Eta):2*TMath::ATan(TMath::Exp(-GenJet.Eta))>>h","GenJet.Flavor==4");
 
  // --------- Plotting ---------------- //
  //     (code by R. Seidl) 

  gStyle->SetOptStat(0);
  gStyle->SetTitleX(.5);
  gStyle->SetTitleY(.87);

  TH2F *hdummy=new TH2F("hdummy",title.Data(),10,-100.,100., 10, 0., 120.);

  Double_t xtit = h->GetYaxis()->GetTitleSize();
  xtit=xtit*1.0;
  
  h->Scale(1.0/h->Integral());

  Double_t maxy = h->GetMaximum()*1.1;
  Double_t miny = h->GetMinimum(0.);
  
  hdummy->GetZaxis()->SetRangeUser(miny,maxy);
  hdummy->GetYaxis()->SetTitleSize(xtit);
  hdummy->GetXaxis()->SetTitleSize(xtit);
  
  hdummy->GetYaxis()->SetLabelSize(xtit);
  hdummy->GetXaxis()->SetLabelSize(xtit);
  
  // hdummy->GetYaxis()->SetTitle("Momentum (GeV)  ");
  hdummy->GetXaxis()->SetTitle("Momentum [GeV]  ");
  
  hdummy->GetXaxis()->SetTitleOffset(1.2);     
  hdummy->GetYaxis()->SetTitleOffset(1.0);
  hdummy->SetLineColor(1);
  hdummy->SetLineStyle(1);
  hdummy->SetLineWidth(2);
  
  hdummy->DrawCopy("AXIS");
  delete hdummy;

  gPad->SetRightMargin(0.15);
  gPad->SetBottomMargin(0.15);
  gPad->SetLogz();
 


  h->DrawCopy("POL COLZ SAME");
  //h->DrawNormalized("POL COLZ SAME");

  // ---- Output -------

  TFile *fout=new TFile("charm_jet_coverage_genjet_p_CC_DIS_e10_p275_CT18NNLO.root","recreate");
  h->Write(title.Data());
  gPad->Write(title.Data());
  title+=".pdf";
  gPad->Print("charm_jet_coverage_genjet_p_CC_DIS_e10_p275_CT18NNLO.pdf");




}
