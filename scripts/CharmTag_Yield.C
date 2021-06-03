#include "TROOT.h"
#include "TChain.h"
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

#include <glob.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>

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


void CharmTag_Yield()
{
  auto default_data = new TChain("tree");
  auto files = fileVector("../CC_DIS_e10_p275_CT18NNLO/0/out.root");
  
  for (auto file : files) 
    {
      default_data->Add(file.c_str());
    }

  TTree* default_data_selected = default_data->CopyTree("abs(jet_eta)<3.0 && met_et > 10.0");

  std::cout << default_data_selected->GetEntries() << std::endl;

  TCut cut_sip3d_tagged("jet_sip3dtag == 1");
  TCut cut_ktagged("jet_ktag == 1");
  TCut cut_etagged("jet_etag == 1");
  TCut cut_mutagged("jet_mutag == 1");

  TCut cut_lightjets("(jet_flavor < 4) | (jet_flavor == 21)");
  TCut cut_charmjets("jet_flavor == 4");

  std::map<std::string, TCut> cut_map;
  cut_map["sIP3D-tagged"] = cut_sip3d_tagged;
  cut_map["sIP3D-untagged"] = !cut_sip3d_tagged;
  cut_map["sIP3D-untagged, e-tagged"] = !cut_sip3d_tagged && cut_etagged;
  cut_map["sIP3D-untagged, mu-tagged"] = !cut_sip3d_tagged && cut_mutagged;
  cut_map["sIP3D-untagged, K-tagged"] = !cut_sip3d_tagged && cut_ktagged;
  cut_map["sIP3D-untagged, (e|mu|K)-tagged"] = !cut_sip3d_tagged && (cut_ktagged||cut_etagged||cut_mutagged);
  cut_map["Tagged by anything"] = (cut_sip3d_tagged||cut_ktagged||cut_etagged||cut_mutagged);


  std::vector<TCut> jet_types;
  jet_types.push_back(cut_lightjets);
  jet_types.push_back(cut_charmjets);

  for (auto jet_type : jet_types) {
    Float_t n_all = default_data_selected->Draw("jet_pt",jet_type.GetTitle());
    std::cout << std::fixed << n_all << std::endl;
    for (auto name : cut_map) {
      TCut tag_cat = name.second;
      Float_t n_select = default_data_selected->Draw("jet_pt", jet_type && tag_cat);
      std::cout << name.first << ": " << std::setprecision(0) << std::fixed << n_select << " (" << std::setprecision(3) << n_select/n_all*100 << "%)" << std::endl;
    }
  }
}
