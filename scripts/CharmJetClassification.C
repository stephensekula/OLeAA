#include "TROOT.h"
#include "TChain.h"
#include "TFile.h"
#include "TEfficiency.h"
#include "TH1.h"
#include "TGraphErrors.h"
#include "TCut.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TMath.h"
#include "TLine.h"
#include "TLatex.h"
#include "TRatioPlot.h"

#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"

#include <glob.h>
#include <iostream>
#include <iomanip>
#include <vector>

#include "PlotFunctions.h"


void CharmJetClassification(TString dir, TString input, TString filePattern = "*/out.root", TString taggers = "ip3dtagger")
{
  // Global options
  gStyle->SetOptStat(0);

  // Create the TCanvas
  TCanvas *pad = new TCanvas("pad",
                             "",
                             800,
                             600);
  TLegend *legend    = nullptr;
  TH1F    *htemplate = nullptr;

  auto default_data = new TChain("tree");
  default_data->SetTitle(input.Data());
  auto files = fileVector(Form("%s/%s/%s", dir.Data(), input.Data(), filePattern.Data()));

  for (auto file : files)
  {
    default_data->Add(file.c_str());
  }

  // Create the signal and background trees

  auto signal_train = default_data->CopyTree("jet_flavor==4 && jet_n>0", "", TMath::Floor(default_data->GetEntries() / 1.0));
  std::cout << "Signal Tree (Training): " << signal_train->GetEntries() << std::endl;

  auto background_train = default_data->CopyTree("(jet_flavor<4||jet_flavor==21) && jet_n>0", "", TMath::Floor(default_data->GetEntries() / 1.0));
  std::cout << "Background Tree (Training): " << background_train->GetEntries() << std::endl;

  // auto u_background_train = default_data->CopyTree("(jet_flavor==2)", "",
  // TMath::Floor(default_data->GetEntries()/1.0));
  // std::cout << "u Background Tree (Training): " <<
  // u_background_train->GetEntries() << std::endl;

  // auto d_background_train = default_data->CopyTree("(jet_flavor==1)", "",
  // TMath::Floor(default_data->GetEntries()/1.0));
  // std::cout << "d Background Tree (Training): " <<
  // d_background_train->GetEntries() << std::endl;

  // auto g_background_train = default_data->CopyTree("(jet_flavor==21)", "",
  // TMath::Floor(default_data->GetEntries()/1.0));
  // std::cout << "g Background Tree (Training): " <<
  // g_background_train->GetEntries() << std::endl;

  // Create the TMVA tools
  TMVA::Tools::Instance();

  auto outputFile = TFile::Open("CharmJetClassification_Results.root", "RECREATE");

  TMVA::Factory factory("TMVAClassification",
                        outputFile,
                        "!V:ROC:!Correlations:!Silent:Color:DrawProgressBar:AnalysisType=Classification");

  TObjArray* tagger_list = taggers.Tokenize(",");

  for (Int_t itagger = 0; itagger < tagger_list->GetEntries(); itagger++) {

    std::string tagger_name = static_cast<TObjString*>(tagger_list->At(itagger))->GetString().Data();

    // TMVA::DataLoader loader_ktagger("dataset_ktagger");
    // TMVA::DataLoader loader_etagger("dataset_etagger");
    // TMVA::DataLoader loader_mutagger("dataset_mutagger");
    // TMVA::DataLoader loader_ip3dtagger("dataset_ip3dtagger");
    
    TMVA::DataLoader loader(Form("dataset_%s", tagger_name.c_str()));

    // loader_ktagger.AddVariable("jet_pt",  "Jet p_{T}",          "GeV", 'F',
    // 0.0,
    // 1000.0);
    // loader_ktagger.AddVariable("jet_eta", "Jet Pseudorapidity", "",    'F',
    // -5.0, 5.0);
    
    TCut charmjet_cut("jet_pt>5.0 && TMath::Abs(jet_eta) < 3.0 && jet_flavor==4 && met_et > 10");
    TCut lightjet_cut("jet_pt>5.0 && TMath::Abs(jet_eta) < 3.0 && (jet_flavor<4||jet_flavor==21) && met_et > 10");

    if (tagger_name == "ktagger") {

      loader.AddSpectator("jet_pt");
      loader.AddSpectator("jet_eta");
      loader.AddVariable("jet_k1_pt");
      loader.AddVariable("jet_k1_sIP3D");
      loader.AddVariable("jet_k2_pt");
      loader.AddVariable("jet_k2_sIP3D");
      loader.AddSpectator("jet_flavor");
      loader.AddSpectator("met_et");
      loader.AddSignalTree(signal_train, 1.0);
      loader.AddBackgroundTree(background_train, 1.0);

      // loader.AddVariable("jet_nconstituents");
      loader.PrepareTrainingAndTestTree(charmjet_cut,
					lightjet_cut,
					"nTrain_Signal=50000:nTrain_Background=500000:nTest_Signal=50000:nTest_Background=500000:SplitMode=Random:NormMode=NumEvents:!V");
      
      factory.BookMethod(&loader,    TMVA::Types::kMLP, "CharmKTagger",
			 "!H:!V:NeuronType=ReLU:VarTransform=Norm:NCycles=1000:HiddenLayers=N+12:TestRate=5:!UseRegulator");
    } else if (tagger_name == "etagger") {
      loader.AddSpectator("jet_pt");
      loader.AddSpectator("jet_eta");
      loader.AddVariable("jet_e1_pt");
      loader.AddVariable("jet_e1_sIP3D");
      loader.AddVariable("jet_e2_pt");
      loader.AddVariable("jet_e2_sIP3D");
      loader.AddSpectator("jet_flavor");
      loader.AddSpectator("met_et");
      loader.AddSignalTree(signal_train, 1.0);
      loader.AddBackgroundTree(background_train, 1.0);


      loader.PrepareTrainingAndTestTree(charmjet_cut,
					lightjet_cut,
					"nTrain_Signal=100000:nTrain_Background=1000000:nTest_Signal=100000:nTest_Background=1000000:SplitMode=Random:NormMode=NumEvents:!V");

      factory.BookMethod(&loader,    
			 TMVA::Types::kMLP, "CharmETagger",
			 "!H:!V:NeuronType=ReLU:VarTransform=Norm:NCycles=1000:HiddenLayers=N+12:TestRate=5:!UseRegulator");
    } else if (tagger_name == "mutagger") {
      loader.AddSpectator("jet_pt");
      loader.AddSpectator("jet_eta");
      loader.AddVariable("jet_mu1_pt");
      loader.AddVariable("jet_mu1_sIP3D");
      loader.AddVariable("jet_mu2_pt");
      loader.AddVariable("jet_mu2_sIP3D");
      loader.AddSpectator("jet_flavor");
      loader.AddSpectator("met_et");
      loader.AddSignalTree(signal_train, 1.0);
      loader.AddBackgroundTree(background_train, 1.0);

      loader.PrepareTrainingAndTestTree(charmjet_cut,
						 lightjet_cut,
						 "nTrain_Signal=100000:nTrain_Background=1000000:nTest_Signal=100000:nTest_Background=1000000:SplitMode=Random:NormMode=NumEvents:!V");

      factory.BookMethod(&loader,   
			 TMVA::Types::kMLP, "CharmMuTagger",
			 "!H:!V:NeuronType=ReLU:VarTransform=Norm:NCycles=1000:HiddenLayers=N+12:TestRate=5:!UseRegulator");

    } else if (tagger_name == "ip3dtagger") {

      loader.AddSpectator("jet_pt");
      loader.AddSpectator("jet_eta");
      loader.AddVariable("jet_t1_pt");
      loader.AddVariable("jet_t1_sIP3D");
      loader.AddVariable("jet_t2_pt");
      loader.AddVariable("jet_t2_sIP3D");
      loader.AddVariable("jet_t3_pt");
      loader.AddVariable("jet_t3_sIP3D");
      loader.AddVariable("jet_t4_pt");
      loader.AddVariable("jet_t4_sIP3D");
      loader.AddSpectator("jet_flavor");
      loader.AddSpectator("met_et");
      loader.AddSignalTree(signal_train, 1.0);
      loader.AddBackgroundTree(background_train, 1.0);

      loader.PrepareTrainingAndTestTree(charmjet_cut,
					lightjet_cut,
					"nTrain_Signal=10000:nTrain_Background=100000:nTest_Signal=10000:nTest_Background=100000:SplitMode=Random:NormMode=NumEvents:!V");

      factory.BookMethod(&loader, 
			 TMVA::Types::kMLP, "CharmIP3DTagger",
			 "!H:!V:NeuronType=ReLU:VarTransform=Norm:NCycles=1000:HiddenLayers=N+16:TestRate=5:!UseRegulator");
    } else if (tagger_name == "jb_jet_tagger") {

      loader.AddVariable("jet.PT");
      loader.AddVariable("jet.Eta");
      loader.AddVariable("jet.Phi");
      loader.AddVariable("jet.Mass");
      loader.AddVariable("jet.t1.PT");
      loader.AddVariable("jet.t1.sIP3D");
      loader.AddVariable("jet.t2.PT");
      loader.AddVariable("jet.t2.sIP3D");
      loader.AddVariable("jet.t3.PT");
      loader.AddVariable("jet.t3.sIP3D");
      loader.AddVariable("jet.t4.PT");
      loader.AddVariable("jet,t4.sIP3D");
      loader.AddSpectator("jet.flavor");
      loader.AddSignalTree(signal_train, 1.0);
      loader.AddBackgroundTree(background_train, 1.0);

      loader.PrepareTrainingAndTestTree(charmjet_cut,
					lightjet_cut,
					"nTrain_Signal=10000:nTrain_Background=100000:nTest_Signal=10000:nTest_Background=100000:SplitMode=Random:NormMode=NumEvents:!V");

      factory.BookMethod(&loader, 
			 TMVA::Types::kMLP, "JBJetTagger",
			 "!H:!V:NeuronType=ReLU:VarTransform=Norm:NCycles=1000:HiddenLayers=100:TestRate=5:!UseRegulator");
    }

    //  loader.AddVariable("jet_sip3dtag", "sIP3D Jet-Level Tag", "", 'B', -10,
    // 10);
    // loader.AddVariable("jet_charge");
    
    // loader.AddVariable("jet_ehadoveremratio");
    
    
    // loader.AddTree( signal_train, "strange_jets" );
    // loader.AddTree( u_background_train, "up jets" );
    // loader.AddTree( d_background_train, "down jets" );
    // loader.AddTree( g_background_train, "gluon jets" );
    

    // Declare the classification method(s)
    // factory.BookMethod(&loader,TMVA::Types::kBDT, "BDT",
    //
    //
    //  "!V:NTrees=1000:MinNodeSize=2.5%:MaxDepth=4:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20"
    // );
    
    // factory.BookMethod(&loader, TMVA::Types::kMLP,
    // "CharmETagger","!H:!V:NeuronType=ReLU:VarTransform=Norm:NCycles=600:HiddenLayers=N+8:TestRate=5:!UseRegulator");
    // factory.BookMethod(&loader, TMVA::Types::kMLP,
    // "CharmMuTagger","!H:!V:NeuronType=ReLU:VarTransform=Norm:NCycles=600:HiddenLayers=N+8:TestRate=5:!UseRegulator");
    
  // Train
    factory.TrainAllMethods();
    
    // Test
    factory.TestAllMethods();
    factory.EvaluateAllMethods();
    
    // Plot a ROC Curve
    pad->cd();
    pad = factory.GetROCCurve(&loader);
    //  pad = factory.GetROCCurve(&loader_ktagger);
    // pad = factory.GetROCCurve(&loader_etagger);
    // pad = factory.GetROCCurve(&loader_mutagger);
    // pad = factory.GetROCCurve(&loader_ip3dtagger);
    pad->Draw();
    
    pad->SaveAs(Form("CharmJetClassification_%s_ROC.pdf", tagger_name.c_str()));
  }
}
