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
  Double_t jet_charge_05;
  Double_t sIP3DTagged;
  Double_t kTagged;
  Double_t t1_sIP3D;
  Double_t t1_IP3D;
  Double_t t1_IP2D;
  Double_t t1_d0;
  Double_t t1_d0err;
  Double_t t1_z0;
  Double_t t1_z0err;
  Double_t t1_pt;
  Double_t t2_sIP3D;
  Double_t t2_IP3D;
  Double_t t2_IP2D;
  Double_t t2_d0;
  Double_t t2_d0err;
  Double_t t2_z0;
  Double_t t2_z0err;
  Double_t t2_pt;
  Double_t t3_sIP3D;
  Double_t t3_IP3D;
  Double_t t3_IP2D;
  Double_t t3_d0;
  Double_t t3_d0err;
  Double_t t3_z0;
  Double_t t3_z0err;
  Double_t t3_pt;
  Double_t t4_sIP3D;
  Double_t t4_IP3D;
  Double_t t4_IP2D;
  Double_t t4_d0;
  Double_t t4_d0err;
  Double_t t4_z0;
  Double_t t4_z0err;
  Double_t t4_pt;
  Double_t k1_sIP3D;
  Double_t k1_pt;
  Double_t k1_q;
  Double_t k2_sIP3D;
  Double_t k2_pt;
  Double_t k2_q;
  Double_t e1_sIP3D;
  Double_t e1_pt;
  Double_t e2_sIP3D;
  Double_t e2_pt;
};


class JetTaggingTool {
  static JetTaggingTool *instance;

  // Private constructor so that no objects can be created.
  JetTaggingTool(ExRootTreeReader * data) {
    _jet_tagging_store = std::map < TObject *, JetTaggingInfo > ();
    _data              = data;
  }

public:

  static JetTaggingTool         * getInstance(ExRootTreeReader *data) {
    if (!instance) instance = new JetTaggingTool(data);
    return instance;
  }

  JetTaggingInfo getJetTaggingInfo(TObject *obj) {
    if (_jet_tagging_store.find(obj) != _jet_tagging_store.end()) {
      return _jet_tagging_store[obj];
    }

    JetTaggingInfo blank;
    return blank;
  }

  void execute(TObjArray *jets, std::map < std::string, std::any > *DataStore) {
    for (Int_t i = 0; i < jets->GetEntries(); i++) {
      execute(jets->At(i), DataStore);
    }
  }

  void execute(TObject *obj, std::map < std::string, std::any > *DataStore) {
    JetTaggingInfo j = {};

    _jet_tagging_store[obj] = j;

    auto jet = static_cast < Jet * > (obj);
    compute_sIP3DTagging(jet, DataStore);

    // Compute Jet-level variables
    _jet_tagging_store[jet].jet_charge_05 = JetCharge(jet,
                                                      std::any_cast < TClonesArray * > ((*DataStore)["EFlowTrack"]),
                                                      0.5);
  }

  void clear() {
    _jet_tagging_store.clear();
  }

  void compute_sIP3DTagging(Jet *jet, std::map < std::string, std::any > *DataStore) {
    TClonesArray *EFlowTrack = std::any_cast < TClonesArray * > ((*DataStore)["EFlowTrack"]);

    // retrieve the beam spot
    TClonesArray *BeamSpot = std::any_cast < TClonesArray * > ((*DataStore)["BeamSpot"]);
    GenParticle  *bs       = nullptr;

    if (BeamSpot != nullptr) {
      bs = static_cast < GenParticle * > (BeamSpot->At(0));
    }
    _jet_tagging_store[jet].sIP3DTagged = Tagged_sIP3D(jet, *EFlowTrack, 3.00, 0.25, 2.0, bs);

    // Retrieve information about the leading, subleading, etc. tracks
    std::vector < Track * > jet_tracks;

    for (Int_t t = 0; t < EFlowTrack->GetEntries(); t++) {
      auto trk = static_cast < Track * > (EFlowTrack->At(t));

      if (jet->P4().DeltaR(trk->P4()) < 0.5) {
        jet_tracks.push_back(trk);
      }
    }
    std::sort(jet_tracks.begin(), jet_tracks.end(),
              [] (auto& lhs, const auto& rhs)
    {
      return lhs->PT > rhs->PT;
    }
              );

    if (jet_tracks.size() > 0) {
      _jet_tagging_store[jet].t1_pt    = jet_tracks[0]->PT;
      _jet_tagging_store[jet].t1_d0    = jet_tracks[0]->D0;
      _jet_tagging_store[jet].t1_d0err = jet_tracks[0]->ErrorD0;
      _jet_tagging_store[jet].t1_z0    = jet_tracks[0]->DZ;
      _jet_tagging_store[jet].t1_z0err = jet_tracks[0]->ErrorDZ;
      _jet_tagging_store[jet].t1_sIP3D = sIP3D(jet, jet_tracks[0], bs);
      _jet_tagging_store[jet].t1_IP3D  = IP3D(jet_tracks[0]);
      _jet_tagging_store[jet].t1_IP2D  = IP2D(jet_tracks[0]);
    }

    if (jet_tracks.size() > 1) {
      _jet_tagging_store[jet].t2_pt    = jet_tracks[1]->PT;
      _jet_tagging_store[jet].t2_d0    = jet_tracks[1]->D0;
      _jet_tagging_store[jet].t2_d0err = jet_tracks[1]->ErrorD0;
      _jet_tagging_store[jet].t2_z0    = jet_tracks[1]->DZ;
      _jet_tagging_store[jet].t2_z0err = jet_tracks[1]->ErrorDZ;
      _jet_tagging_store[jet].t2_sIP3D = sIP3D(jet, jet_tracks[1], bs);
      _jet_tagging_store[jet].t2_IP3D  = IP3D(jet_tracks[1]);
      _jet_tagging_store[jet].t2_IP2D  = IP2D(jet_tracks[1]);
    }

    if (jet_tracks.size() > 2) {
      _jet_tagging_store[jet].t3_pt    = jet_tracks[2]->PT;
      _jet_tagging_store[jet].t3_d0    = jet_tracks[2]->D0;
      _jet_tagging_store[jet].t3_d0err = jet_tracks[2]->ErrorD0;
      _jet_tagging_store[jet].t3_z0    = jet_tracks[2]->DZ;
      _jet_tagging_store[jet].t3_z0err = jet_tracks[2]->ErrorDZ;
      _jet_tagging_store[jet].t3_sIP3D = sIP3D(jet, jet_tracks[2], bs);
      _jet_tagging_store[jet].t3_IP3D  = IP3D(jet_tracks[2]);
      _jet_tagging_store[jet].t3_IP2D  = IP2D(jet_tracks[2]);
    }

    if (jet_tracks.size() > 3) {
      _jet_tagging_store[jet].t4_pt    = jet_tracks[3]->PT;
      _jet_tagging_store[jet].t4_d0    = jet_tracks[3]->D0;
      _jet_tagging_store[jet].t4_d0err = jet_tracks[3]->ErrorD0;
      _jet_tagging_store[jet].t4_z0    = jet_tracks[3]->DZ;
      _jet_tagging_store[jet].t4_z0err = jet_tracks[3]->ErrorDZ;
      _jet_tagging_store[jet].t4_sIP3D = sIP3D(jet, jet_tracks[3], bs);
      _jet_tagging_store[jet].t4_IP3D  = IP3D(jet_tracks[3]);
      _jet_tagging_store[jet].t4_IP2D  = IP2D(jet_tracks[3]);
    }

    // Retrieve information about leading, subleading, etc. kaons
    if (DataStore->find("ChargedKaon") != DataStore->end()) {
      TObjArray *ChargedKaon = std::any_cast < TObjArray * > ((*DataStore)["ChargedKaon"]);
      std::vector < Track * > kaon_tracks;

      for (Int_t t = 0; t < ChargedKaon->GetEntries(); t++) {
        auto kaon = static_cast < Track * > (ChargedKaon->At(t));

        // Find the matching EFlowTrack inside the jet
        Track *trk = nullptr;

        for (Int_t e = 0; e < EFlowTrack->GetEntries(); e++) {
          auto eflowtrack = static_cast < Track * > (EFlowTrack->At(e));
          auto eflowcand  = static_cast < Candidate * > (EFlowTrack->At(e));

          if (eflowtrack == nullptr) continue;

          if (kaon->Particle.GetObject() == eflowtrack->Particle.GetObject()) {
            trk = eflowtrack;
            break;
          }
        }

        if (trk == nullptr) continue;

        if (jet->P4().DeltaR(trk->P4()) < 0.5) {
          kaon_tracks.push_back(trk);
        }
      }
      std::sort(kaon_tracks.begin(), kaon_tracks.end(),
                [] (auto& lhs, const auto& rhs)
      {
        return lhs->PT > rhs->PT;
      }
                );

      if (kaon_tracks.size() > 0) {
        _jet_tagging_store[jet].k1_pt    = kaon_tracks[0]->PT;
        _jet_tagging_store[jet].k1_q     = kaon_tracks[0]->Charge;
        _jet_tagging_store[jet].k1_sIP3D = sIP3D(jet, kaon_tracks[0], bs);
      } else {
        _jet_tagging_store[jet].k1_pt    = 0.0;
        _jet_tagging_store[jet].k1_q     = 0.0;
        _jet_tagging_store[jet].k1_sIP3D = -199.0;
      }

      if (kaon_tracks.size() > 1) {
        _jet_tagging_store[jet].k2_pt    = kaon_tracks[1]->PT;
        _jet_tagging_store[jet].k2_q     = kaon_tracks[1]->Charge;
        _jet_tagging_store[jet].k2_sIP3D = sIP3D(jet, kaon_tracks[1], bs);
      } else {
        _jet_tagging_store[jet].k2_pt    = 0.0;
        _jet_tagging_store[jet].k2_q     = 0.0;
        _jet_tagging_store[jet].k2_sIP3D = -199.0;
      }

      // Set the kTagged variable based on work by S. Gilchrist (SMU)
      Double_t kTagged = 0.0;

      if (kaon_tracks.size() > 0) {
        Double_t k1_pt    = _jet_tagging_store[jet].k1_pt;
        Double_t k1_q     = _jet_tagging_store[jet].k1_q;
        Double_t k1_sIP3D = _jet_tagging_store[jet].k1_sIP3D;

        Double_t k1_pt_ratio = k1_pt / jet->PT;

        kTagged = (0.1 < k1_pt_ratio && k1_pt_ratio < 0.4) && (TMath::Abs(k1_sIP3D) > 4.0) & (k1_q > 0.0);
      }
      _jet_tagging_store[jet].kTagged = kTagged;
    }


    // Retrieve information about leading, subleading, etc. electrons
    if (DataStore->find("ChargedElectron") != DataStore->end()) {
      auto ChargedElectron = std::any_cast < TObjArray * > ((*DataStore)["ChargedElectron"]);

      std::vector < Electron * > electrons;

      for (Int_t e = 0; e < ChargedElectron->GetEntries(); e++) {
        auto trk = static_cast < Electron * > (ChargedElectron->At(e));

        if (jet->P4().DeltaR(trk->P4()) < 0.5) {
          electrons.push_back(trk);
        }
      }
      std::sort(electrons.begin(), electrons.end(),
                [] (auto& lhs, const auto& rhs)
      {
        return lhs->PT > rhs->PT;
      }
                );

      if (electrons.size() > 0) {
        _jet_tagging_store[jet].e1_pt = electrons[0]->PT;

        for (Int_t e = 0; e < EFlowTrack->GetEntries(); e++) {
          auto eflowtrack = static_cast < Track * > (EFlowTrack->At(e));

          if (eflowtrack->Particle.GetObject() == electrons[0]->Particle.GetObject()) {
            _jet_tagging_store[jet].e1_sIP3D = sIP3D(jet, eflowtrack, bs);
            break;
          }
        }
      }

      if (electrons.size() > 1) {
        _jet_tagging_store[jet].e2_pt = electrons[1]->PT;

        for (Int_t e = 0; e < EFlowTrack->GetEntries(); e++) {
          auto eflowtrack = static_cast < Track * > (EFlowTrack->At(e));

          if (eflowtrack->Particle.GetObject() == electrons[1]->Particle.GetObject()) {
            _jet_tagging_store[jet].e2_sIP3D = sIP3D(jet, eflowtrack, bs);
            break;
          }
        }
      }
    }
  }

private:

  ExRootTreeReader *_data = nullptr;
  std::map < TObject *, JetTaggingInfo > _jet_tagging_store;
};

#endif /* ifndef JETTAGGINGTOOL_HH */
