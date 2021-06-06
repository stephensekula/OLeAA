#ifndef JETTAGGINGTOOL_HH
#define JETTAGGINGTOOL_HH


// C++ includes
#include <vector>
#include <map>
#include <utility>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>

// ROOT includes
#include "TString.h"
#include "TreeHandler.h"
#include "TSystem.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "TClonesArray.h"

// Other includes
#include "AnalysisFunctions.cc"
#include "classes/DelphesClasses.h"

using namespace std;

struct JetTaggingInfo {
  Double_t sIP3DTagged;
  Double_t t1_sIP3D;
  Double_t t1_pt;
  Double_t t2_sIP3D;
  Double_t t2_pt;
  Double_t t3_sIP3D;
  Double_t t3_pt;
  Double_t t4_sIP3D;
  Double_t t4_pt;
  Double_t k1_sIP3D;
  Double_t k1_pt;
  Double_t k2_sIP3D;
  Double_t k2_pt;
};


class JetTaggingTool {
  static JetTaggingTool *instance;
  
  // Private constructor so that no objects can be created.
  JetTaggingTool(ExRootTreeReader* data) {
    _jet_tagging_store = std::map<TObject*, JetTaggingInfo>();
    _data = data;

    _EFlowTrack = _data->UseBranch("EFlowTrack");
  }
  
 public:

  static JetTaggingTool *getInstance(ExRootTreeReader* data) {
    if (!instance)
      instance = new JetTaggingTool(data);
    return instance;
  }

  JetTaggingInfo getJetTaggingInfo(TObject* obj) {
    if (_jet_tagging_store.find(obj) != _jet_tagging_store.end()) {
      return _jet_tagging_store[obj];
    }

    JetTaggingInfo blank;
    return blank;
  }

  void execute(TObjArray* jets, std::map<std::string, std::any>* DataStore) {
    for (Int_t i = 0; i < jets->GetEntries(); i++) {
      execute(jets->At(i), DataStore);
    }

  }

  void execute(TObject* obj, std::map<std::string, std::any>* DataStore) {
    JetTaggingInfo j = {};
    _jet_tagging_store[obj] = j;

    auto jet = static_cast<Jet*>(obj);
    compute_sIP3DTagging( jet, DataStore );

  }

  void clear() {
    _jet_tagging_store.clear();
  }

  void compute_sIP3DTagging( Jet* jet, std::map<std::string, std::any>* DataStore ) {
    _jet_tagging_store[jet].sIP3DTagged = Tagged_sIP3D(jet, *_EFlowTrack, 3.00, 0.25, 2.0);

    // Retrieve information about the leading, subleading, etc. tracks 
    std::vector<Track*> jet_tracks;
    for (Int_t t = 0; t < _EFlowTrack->GetEntries(); t++) {
      auto trk = static_cast<Track*>(_EFlowTrack->At(t));
      if (jet->P4().DeltaR( trk->P4() ) < 0.5) {
	jet_tracks.push_back(trk);
      }
    }
    std::sort(jet_tracks.begin(), jet_tracks.end(), 
	      [](auto& lhs, const auto& rhs)
	      {
		return lhs->PT > rhs->PT;
	      }
	      );
    if (jet_tracks.size()>0) {
      _jet_tagging_store[jet].t1_pt = jet_tracks[0]->PT;
      _jet_tagging_store[jet].t1_sIP3D = sIP3D(jet, jet_tracks[0]);
    }
    if (jet_tracks.size()>1) {
      _jet_tagging_store[jet].t2_pt = jet_tracks[1]->PT;
      _jet_tagging_store[jet].t2_sIP3D = sIP3D(jet, jet_tracks[1]);
    }
    if (jet_tracks.size()>2) {
      _jet_tagging_store[jet].t3_pt = jet_tracks[2]->PT;
      _jet_tagging_store[jet].t3_sIP3D = sIP3D(jet, jet_tracks[2]);
    }
    if (jet_tracks.size()>3) {
      _jet_tagging_store[jet].t4_pt = jet_tracks[3]->PT;
      _jet_tagging_store[jet].t4_sIP3D = sIP3D(jet, jet_tracks[3]);
    }

    // Retrieve information about leading, subleading, etc. kaons
    if (DataStore->find("ChargedKaon") != DataStore->end()) {
      TObjArray* ChargedKaon = std::any_cast<TObjArray*>((*DataStore)["ChargedKaon"]);
      std::vector<Track*> kaon_tracks;
      for (Int_t t = 0; t < ChargedKaon->GetEntries(); t++) {
	auto kaon = static_cast<Track*>(ChargedKaon->At(t));

	// Find the matching EFlowTrack inside the jet
	Track* trk = nullptr;
	for (Int_t e = 0; e < _EFlowTrack->GetEntries(); e++) {
	  auto eflowtrack = static_cast<Track*>(_EFlowTrack->At(e));
	  if (kaon->Particle == eflowtrack->Particle) {
	    trk = eflowtrack;
	    break;
	  }
	}

	if (trk == nullptr)
	  continue;

	if (jet->P4().DeltaR( trk->P4() ) < 0.5) {
	  kaon_tracks.push_back(trk);
	} 
      }
      std::sort(kaon_tracks.begin(), kaon_tracks.end(), 
		[](auto& lhs, const auto& rhs)
		{
		  return lhs->PT > rhs->PT;
		}
		);
      if (kaon_tracks.size()>0) {
	_jet_tagging_store[jet].k1_pt = kaon_tracks[0]->PT;
	_jet_tagging_store[jet].k1_sIP3D = sIP3D(jet, kaon_tracks[0]);
      }
      if (kaon_tracks.size()>1) {
	_jet_tagging_store[jet].k2_pt = kaon_tracks[1]->PT;
	_jet_tagging_store[jet].k2_sIP3D = sIP3D(jet, kaon_tracks[1]);
      }
    }

  }

 private:
  ExRootTreeReader* _data = nullptr;
  std::map<TObject*, JetTaggingInfo> _jet_tagging_store;
  TClonesArray* _EFlowTrack;
};

#endif
