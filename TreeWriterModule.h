#ifndef TREEWRITERMODULE_HH
#define TREEWRITERMODULE_HH

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
#include "TDatabasePDG.h"
#include "TFile.h"
#include "TH1.h"
#include "TRandom.h"

// Other includes
#include "Module.h"
#include "AnalysisFunctions.cc"
#include "classes/DelphesClasses.h"
#include "JetTaggingTool.h"


class TreeWriterModule : public Module {
public:

  TreeWriterModule(ExRootTreeReader *data,
                   std::string       name);

  ~TreeWriterModule();

  void initialize() override;
  bool execute(std::map<std::string, std::any> *DataStore) override;
  void finalize() override;

private:

  float _met_et;

  // DIS variables
  float _bjorken_x;
  float _bjorken_Q2;
  float _bjorken_y;
  float _jb_x;
  float _jb_Q2;

  Double_t _mpi;
  Double_t _mK;
  Double_t _me;
  Double_t _mmu;
  Double_t _mp;


  std::map<TString, Double_t>_global_vars;
  std::map<TString, std::vector<Double_t> >_candidate_vars;


  // Internal correction of calorimeter energy distribution based on Full
  // Simulation
  std::map<SortableObject *, Double_t>_cache_emfrac;
  TFile *_emfrac_file = nullptr;

private:

  // Private methods
  Double_t kinVar(TString varName, TObject *obj) {
    if (obj->InheritsFrom("Jet")) {
      auto p = static_cast<Jet *>(obj);

      if (varName.Contains("_PT")) {
        return p->PT;
      } else if (varName.Contains("_Eta")) {
        return p->Eta;
      } else if (varName.Contains("_Phi")) {
        return p->Phi;
      } else if (varName.Contains("_M")) {
        return p->Mass;
      }
    } else if (obj->InheritsFrom("Track")) {
      auto p = static_cast<Track *>(obj);

      if (varName.Contains("_PT")) {
        return p->PT;
      } else if (varName.Contains("_Eta")) {
        return p->Eta;
      } else if (varName.Contains("_Phi")) {
        return p->Phi;
      } else if (varName.Contains("_M")) {
        return p->Mass;
      }
    } else if (obj->InheritsFrom("Electron")) {
      auto p = static_cast<Electron *>(obj);

      if (varName.Contains("_PT")) {
        return p->PT;
      } else if (varName.Contains("_Eta")) {
        return p->Eta;
      } else if (varName.Contains("_Phi")) {
        return p->Phi;
      } else if (varName.Contains("_M")) {
        return _me;
      }
    } else if (obj->InheritsFrom("Muon")) {
      auto p = static_cast<Muon *>(obj);

      if (varName.Contains("_PT")) {
        return p->PT;
      } else if (varName.Contains("_Eta")) {
        return p->Eta;
      } else if (varName.Contains("_Phi")) {
        return p->Phi;
      } else if (varName.Contains("_M")) {
        return _mmu;
      }
    }
    return 0.0;
  }

  Double_t truthID(TObject *obj) {
    if (obj->InheritsFrom("Jet")) {
      auto p = static_cast<Jet *>(obj);
      return p->Flavor;
    } else if (obj->InheritsFrom("Track")) {
      auto p = static_cast<Track *>(obj);
      return p->PID;
    } else if (obj->InheritsFrom("Electron")) {
      auto p = static_cast<Electron *>(obj);

      // Obtain truth from the GenParticle reference

      /* return -11*p->Charge; */
      if (p->Particle.GetObject() != nullptr) return static_cast<Candidate *>(p->Particle.GetObject())->PID;
      return 0.0;
    } else if (obj->InheritsFrom("Muon")) {
      auto p = static_cast<Track *>(obj);
      return -13 * p->Charge;
    }
    return 0.0;
  }

  Double_t caloVar(TString varName, TObject *obj, std::map<std::string, std::any> *DataStore) {
    if (obj->InheritsFrom("Electron")) {
      auto p = static_cast<Electron *>(obj);

      // Check if this particl has already been calorimeter-corrected
      Double_t emfrac = -1.0;

      if (_cache_emfrac.find(p) != _cache_emfrac.end()) {
        emfrac = _cache_emfrac[p];
      }

      if (emfrac < 0.0) {
        // Recompute the energy distribution from full simulation

        auto CaloTower = std::any_cast<TClonesArray *>((*DataStore)["Tower"]);
        Double_t CaloE = 0.0;
        Double_t CaloH = 0.0;

        std::vector<Tower *> track_towers;

        for (Int_t t = 0; t < CaloTower->GetEntries(); t++) {
          auto calotower       = static_cast<Tower *>(CaloTower->At(t));
          auto tower_particles = calotower->Particles;

          for (Int_t ref = 0; ref < tower_particles.GetEntries(); ref++) {
            TRef calo_ref     = tower_particles.At(ref);
            TObject *calo_obj = calo_ref.GetObject();

            if ((p->Particle.GetObject() == nullptr) || (calo_obj == nullptr)) continue;

            if (p->Particle.GetObject() == calo_ref.GetObject()) {
              track_towers.push_back(calotower);
            }
          }
        }


        for (auto track_tower : track_towers) {
          CaloE += track_tower->Eem;
          CaloH += track_tower->Ehad;
        }

        if (CaloE + CaloH > 0.0) {
          emfrac = CaloE / (CaloE + CaloH);
        } else {
          emfrac = -1.0;
        }

        if (emfrac >= 0.0) {
          // check if this is electron or pion
          Int_t pid = 0;

          if (p->Particle.GetObject() != nullptr) pid = static_cast<Candidate *>(p->Particle.GetObject())->PID;

          if ((TMath::Abs(pid) == 11) || (TMath::Abs(pid) == 211)) {
            // Correct using Full-Sim-based PDFs for calo. energy fractions
            TString PDFname = "EMEnergyRatios_%s" + TString(Form("_%d_pdf", TMath::Abs(pid)));

            if (p->Eta < -1.0) {
              // Backward PDF
              PDFname = Form(PDFname.Data(), "Backward");
            } else if (TMath::Abs(p->Eta) <= 1.0) {
              // Barrel PDF
              PDFname = Form(PDFname.Data(), "Barrel");
            } else if (TMath::Abs(p->Eta) > 1.0) {
              // Forward PDF
              PDFname = Form(PDFname.Data(), "Forward");
            }

            TH1D *pdf = static_cast<TH1D *>(_emfrac_file->Get(PDFname.Data()));

            if (pdf == nullptr) {
              std::stringstream message;
              message << "EM Ratio PDF " << PDFname.Data()
                      << " was not loaded correctly! [" << getName() << "::TreeWriterModule]" << std::endl;
              throw std::runtime_error(message.str());
            }

            // Use accept-reject to set the EM fraction
            Double_t max    = pdf->GetMaximum();
            Bool_t   accept = false;

            while (accept == false) {
              Double_t x = gRandom->Uniform(0, 1);
              Double_t y = gRandom->Uniform(0, max);

              Double_t y_lookup = pdf->GetBinContent(pdf->FindBin(x));

              if (y_lookup > y) {
                emfrac = x;
                accept = true;
              }
            }
          }
        }

        // Store the emfraction for this particle
        _cache_emfrac[p] = emfrac;
      }

      if (varName.Contains("_Eem")) {
        auto CaloTower = std::any_cast<TClonesArray *>((*DataStore)["Tower"]);
        Double_t CaloE = 0.0;
        Double_t CaloH = 0.0;

        std::vector<Tower *> track_towers;

        for (Int_t t = 0; t < CaloTower->GetEntries(); t++) {
          auto calotower       = static_cast<Tower *>(CaloTower->At(t));
          auto tower_particles = calotower->Particles;

          for (Int_t ref = 0; ref < tower_particles.GetEntries(); ref++) {
            TRef calo_ref     = tower_particles.At(ref);
            TObject *calo_obj = calo_ref.GetObject();

            if ((p->Particle.GetObject() == nullptr) || (calo_obj == nullptr)) continue;

            if (p->Particle.GetObject() == calo_ref.GetObject()) {
              track_towers.push_back(calotower);
            }
          }
        }


        for (auto track_tower : track_towers) {
          CaloE += track_tower->Eem;
          CaloH += track_tower->Ehad;
        }

        Double_t CaloTotal = CaloE + CaloH;

        return CaloTotal * _cache_emfrac[p];
      }

      if (varName.Contains("_Ehad")) {
        auto CaloTower = std::any_cast<TClonesArray *>((*DataStore)["Tower"]);
        Double_t CaloE = 0.0;
        Double_t CaloH = 0.0;

        std::vector<Tower *> track_towers;

        for (Int_t t = 0; t < CaloTower->GetEntries(); t++) {
          auto calotower       = static_cast<Tower *>(CaloTower->At(t));
          auto tower_particles = calotower->Particles;

          for (Int_t ref = 0; ref < tower_particles.GetEntries(); ref++) {
            TRef calo_ref     = tower_particles.At(ref);
            TObject *calo_obj = calo_ref.GetObject();

            if ((p->Particle.GetObject() == nullptr) || (calo_obj == nullptr)) continue;

            if (p->Particle.GetObject() == calo_ref.GetObject()) {
              track_towers.push_back(calotower);
            }
          }
        }


        for (auto track_tower : track_towers) {
          CaloE += track_tower->Eem;
          CaloH += track_tower->Ehad;
        }


        Double_t CaloTotal = CaloE + CaloH;

        return CaloTotal * (1.0 - _cache_emfrac[p]);
      }
      return 0.0;
    } else {
      return 0.0;
    }
  }

  Double_t jetTagging(TString varName, TObject *obj, std::map<std::string, std::any> *DataStore) {
    if (obj->InheritsFrom("Jet")) {
      auto p = static_cast<Jet *>(obj);

      JetTaggingTool *jet_tagger = jet_tagger->getInstance(getData());
      jet_tagger->execute(obj, DataStore);

      if (varName.Contains("jet_charge_05")) {
        return jet_tagger->getJetTaggingInfo(obj).jet_charge_05;
      }

      if (varName.Contains("sIP3DTagger")) {
        return jet_tagger->getJetTaggingInfo(obj).sIP3DTagged;
      }

      if (varName.Contains("kTagger")) {
        return jet_tagger->getJetTaggingInfo(obj).kTagged;
      }

      if (varName.Contains("t1_PT")) {
        return jet_tagger->getJetTaggingInfo(obj).t1_pt;
      }

      if (varName.Contains("t1_d0")) {
        return jet_tagger->getJetTaggingInfo(obj).t1_d0;
      }

      if (varName.Contains("t1_d0err")) {
        return jet_tagger->getJetTaggingInfo(obj).t1_d0err;
      }

      if (varName.Contains("t1_z0")) {
        return jet_tagger->getJetTaggingInfo(obj).t1_z0;
      }

      if (varName.Contains("t1_z0err")) {
        return jet_tagger->getJetTaggingInfo(obj).t1_z0err;
      }

      if (varName.Contains("t1_sIP3D")) {
        return jet_tagger->getJetTaggingInfo(obj).t1_sIP3D;
      }

      if (varName.Contains("t1_IP3D")) {
        return jet_tagger->getJetTaggingInfo(obj).t1_IP3D;
      }

      if (varName.Contains("t1_IP2D")) {
        return jet_tagger->getJetTaggingInfo(obj).t1_IP2D;
      }

      if (varName.Contains("t2_PT")) {
        return jet_tagger->getJetTaggingInfo(obj).t2_pt;
      }

      if (varName.Contains("t2_d0")) {
        return jet_tagger->getJetTaggingInfo(obj).t2_d0;
      }

      if (varName.Contains("t2_d0err")) {
        return jet_tagger->getJetTaggingInfo(obj).t2_d0err;
      }

      if (varName.Contains("t2_z0")) {
        return jet_tagger->getJetTaggingInfo(obj).t2_z0;
      }

      if (varName.Contains("t2_z0err")) {
        return jet_tagger->getJetTaggingInfo(obj).t2_z0err;
      }

      if (varName.Contains("t2_sIP3D")) {
        return jet_tagger->getJetTaggingInfo(obj).t2_sIP3D;
      }

      if (varName.Contains("t2_IP3D")) {
        return jet_tagger->getJetTaggingInfo(obj).t2_IP3D;
      }

      if (varName.Contains("t2_IP2D")) {
        return jet_tagger->getJetTaggingInfo(obj).t2_IP2D;
      }

      if (varName.Contains("t3_PT")) {
        return jet_tagger->getJetTaggingInfo(obj).t3_pt;
      }

      if (varName.Contains("t3_d0")) {
        return jet_tagger->getJetTaggingInfo(obj).t3_d0;
      }

      if (varName.Contains("t3_d0err")) {
        return jet_tagger->getJetTaggingInfo(obj).t3_d0err;
      }

      if (varName.Contains("t3_z0")) {
        return jet_tagger->getJetTaggingInfo(obj).t3_z0;
      }

      if (varName.Contains("t3_z0err")) {
        return jet_tagger->getJetTaggingInfo(obj).t3_z0err;
      }

      if (varName.Contains("t3_sIP3D")) {
        return jet_tagger->getJetTaggingInfo(obj).t3_sIP3D;
      }

      if (varName.Contains("t3_IP3D")) {
        return jet_tagger->getJetTaggingInfo(obj).t3_IP3D;
      }

      if (varName.Contains("t3_IP2D")) {
        return jet_tagger->getJetTaggingInfo(obj).t3_IP2D;
      }

      if (varName.Contains("t4_PT")) {
        return jet_tagger->getJetTaggingInfo(obj).t4_pt;
      }

      if (varName.Contains("t4_d0")) {
        return jet_tagger->getJetTaggingInfo(obj).t4_d0;
      }

      if (varName.Contains("t4_d0err")) {
        return jet_tagger->getJetTaggingInfo(obj).t4_d0err;
      }

      if (varName.Contains("t4_z0")) {
        return jet_tagger->getJetTaggingInfo(obj).t4_z0;
      }

      if (varName.Contains("t4_z0err")) {
        return jet_tagger->getJetTaggingInfo(obj).t4_z0err;
      }

      if (varName.Contains("t4_sIP3D")) {
        return jet_tagger->getJetTaggingInfo(obj).t4_sIP3D;
      }

      if (varName.Contains("t4_IP3D")) {
        return jet_tagger->getJetTaggingInfo(obj).t4_IP3D;
      }

      if (varName.Contains("t4_IP2D")) {
        return jet_tagger->getJetTaggingInfo(obj).t4_IP2D;
      }

      if (varName.Contains("k1_PT")) {
        return jet_tagger->getJetTaggingInfo(obj).k1_pt;
      }

      if (varName.Contains("k1_q")) {
        return jet_tagger->getJetTaggingInfo(obj).k1_q;
      }

      if (varName.Contains("k1_sIP3D")) {
        return jet_tagger->getJetTaggingInfo(obj).k1_sIP3D;
      }

      if (varName.Contains("k2_PT")) {
        return jet_tagger->getJetTaggingInfo(obj).k2_pt;
      }

      if (varName.Contains("k2_q")) {
        return jet_tagger->getJetTaggingInfo(obj).k2_q;
      }

      if (varName.Contains("k2_sIP3D")) {
        return jet_tagger->getJetTaggingInfo(obj).k2_sIP3D;
      }

      if (varName.Contains("e1_PT")) {
        return jet_tagger->getJetTaggingInfo(obj).e1_pt;
      }

      if (varName.Contains("e1_sIP3D")) {
        return jet_tagger->getJetTaggingInfo(obj).e1_sIP3D;
      }

      if (varName.Contains("e2_PT")) {
        return jet_tagger->getJetTaggingInfo(obj).e2_pt;
      }

      if (varName.Contains("e2_sIP3D")) {
        return jet_tagger->getJetTaggingInfo(obj).e2_sIP3D;
      }
    }
    return 0.0;
  }
};

#endif // ifndef TREEWRITERMODULE_HH
