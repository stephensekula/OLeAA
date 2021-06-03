#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TTreeFormula.h"
#include <iostream>
#include "TMath.h"

void CheckTree() {
  TFile *_file0 = TFile::Open("../CC_DIS_e10_p275_CT18NNLO/0/out.root");
  TTree* tree = static_cast<TTree*>(_file0->Get("tree"));

  TTreeFormula jet_t1_sIP3D("jet_t1_sIP3D","jet_t1_sIP3D",tree);
  TTreeFormula jet_pt("jet_pt","jet_pt",tree);

  for (UInt_t i = 0; i < tree->GetEntries(); i++) {

    tree->GetEntry(i);

    // std::cout << "Event: " << i << " " << jet_pt.GetNdata() << ", " << jet_t1_sIP3D.GetNdata() << std::endl;

    if (jet_t1_sIP3D.GetNdata() != jet_pt.GetNdata())
      std::cout << "Event: " << i << " mismatch in number of jets and jet_t1_sIP3D vector length!" << std::endl;


    if (jet_t1_sIP3D.GetNdata() > 0) {
      for (Int_t ijet = 0; ijet < jet_t1_sIP3D.GetNdata(); ijet++) {
	// std::cout << "          " << jet_t1_sIP3D.EvalInstance(ijet) << std::endl;
	if (TMath::IsNaN(jet_t1_sIP3D.EvalInstance(ijet))) {
	  std::cout << "Event: " << i << " has jet_t1_sIP3D that is NaN in value!" << std::endl;
	  std::cout << jet_t1_sIP3D.GetNdata() << std::endl;
	  std::cout << jet_t1_sIP3D.EvalInstance(ijet) << std::endl;
	}

	if (!TMath::Finite(jet_t1_sIP3D.EvalInstance(ijet))) {
	  std::cout << "Event: " << i << " has jet_t1_sIP3D that is Infinite in value!" << std::endl;
	  std::cout << jet_t1_sIP3D.GetNdata() << std::endl;
	  std::cout << jet_t1_sIP3D.EvalInstance(ijet) << std::endl;
	}
	
      }
    }

    
    
  } 
}
