#include "ElectronPIDModule.h"

#include "TClonesArray.h"
#include "TRandom3.h"
#include "TDatabasePDG.h"
#include "Math/PdfFuncMathCore.h"

#include "AnalysisFunctions.cc"

#include <iostream>

ElectronPIDModule::ElectronPIDModule(ExRootTreeReader *data, std::string name)
  : Module(data, name)
{
  _params = std::map<std::string, std::string>();
  _outputList    = new TObjArray();
  _electron_mass = TDatabasePDG().GetParticle(11)->Mass();
}

ElectronPIDModule::~ElectronPIDModule()
{}

void ElectronPIDModule::initialize()
{
  // Verify required parameters are specified
  std::vector<std::string> required;
  required.push_back("inputList");
  required.push_back("outputList");
  
  for (auto r : required) {

    ExRootConfParam p = getConfiguration()->GetParam(Form("%s::%s", getName().c_str(), r.c_str()));

    if (p.GetSize() > 0) {
      _params[r] = p.GetString();
    }

    if (_params.find(r) == _params.end()) {
      std::stringstream message;
      message << "Required parameter " << r << " not specified in module " << getName() << " of class RefinerModule!" << std::endl;
      throw std::runtime_error(message.str());
    } else {
      std::cout << getName() << "::" << r << ": value set to " << _params[r] << std::endl;
    }
  }
  
}

bool ElectronPIDModule::execute(std::map<std::string, std::any> *DataStore)
{
  auto data = getData();


  if (DataStore->find(_params["outputList"]) != DataStore->end()) {
    std::stringstream message;
    message << "Cannot overwrite existing list ChargedElectron [" << getName() << "::ElectronPIDModule]" << std::endl;
    throw std::runtime_error(message.str());
  }

  if ((_outputList != nullptr) && (_outputList->GetEntries() > 0)) {
    for (Int_t i = 0; i < _outputList->GetEntries(); i++) {
      delete _outputList->At(i);
    }
    _outputList->Clear();
  }


  if (((*DataStore).find("Tower") == (*DataStore).end()) ||
      ((*DataStore).find(_params["inputList"]) == (*DataStore).end())) {
    std::stringstream message;
    message << "Missing input lists requested from the DataStore! [" << getName() << "::ElectronPIDModule]" << std::endl;
    throw std::runtime_error(message.str());
  }

  auto CaloTower  = std::any_cast<TClonesArray *>((*DataStore)["Tower"]);
  TObjArray* EFlowTrack = nullptr;
  
  
  try {
    EFlowTrack = std::any_cast<TObjArray *>((*DataStore)[_params["inputList"]]);
  } catch (const std::bad_any_cast& e)
    {
      try {
	EFlowTrack = std::any_cast<TClonesArray *>((*DataStore)[_params["inputList"]]);
      } catch (const std::bad_any_cast& e)
        {
          std::stringstream message;
          message << "Input list " << _params["inputList"]
                  << " must be either TObjArray or TClonesArray! [" << getName() << "::execute]" << std::endl;
          throw std::runtime_error(message.str());
        }
    }
  

  // Loop over EFlowTracks. Store for each the EM and HAD energy in the
  // associated
  // Calorimeter tower.

  for (Int_t e = 0; e < EFlowTrack->GetEntries(); e++) {
    auto eflowtrack = static_cast<Track *>(EFlowTrack->At(e));

    if (eflowtrack->PT == 0.0) continue;

    // if (TMath::Abs(eflowtrack->PID)!=11)
    //   continue;
    std::vector<Tower *> track_towers;

    for (Int_t t = 0; t < CaloTower->GetEntries(); t++) {
      auto calotower       = static_cast<Tower *>(CaloTower->At(t));
      auto tower_particles = calotower->Particles;

      for (Int_t ref = 0; ref < tower_particles.GetEntries(); ref++) {
        TRef track_ref    = eflowtrack->Particle;
        TRef calo_ref     = tower_particles.At(ref);
        TObject *calo_obj = calo_ref.GetObject();

        if ((track_ref.GetObject() == nullptr) || (calo_obj == nullptr)) continue;

        if (track_ref.GetObject() == calo_ref.GetObject()) {
          track_towers.push_back(calotower);
        }
      }
    }


    Double_t Eem  = 0.0;
    Double_t Ehad = 0.0;

    for (auto track_tower : track_towers) {
      // std::cout << "E(EM, HAD) = " << track_tower->Eem << ", " <<
      // track_tower->Ehad << std::endl;
      Eem  += track_tower->Eem;
      Ehad += track_tower->Ehad;
    }

    // Establish the selection for electrons
    ExRootConfParam p = getConfiguration()->GetParam(Form("%s::%s", getName().c_str(), "fEM_min"));

    Double_t fEM     = Eem / (Eem + Ehad);
    
    auto EMFracMap = std::any_cast<std::map<TObject *, Double_t>* >((*DataStore)["EMFracMap"]);

    if (EMFracMap->find(eflowtrack->Particle.GetObject()) != EMFracMap->end()) {
      fEM = (*EMFracMap)[eflowtrack->Particle.GetObject()];
      Double_t Etotal = Eem + Ehad;
      Eem = Etotal * fEM;
      Ehad = Etotal * (1.0 - fEM);
   }

    Double_t fEM_min = 0.0;

    if (p.GetSize() > 0) {
      fEM_min = p.GetDouble();
    }

    Bool_t passesSelection = kTRUE;

    if (fEM < fEM_min) {
      passesSelection &= kFALSE;
    }

    if (passesSelection) {
      Electron *electron = new Electron(); // static_cast<Track*>(eflowtrack->Clone());

      // Correct the 4-vector for the mass
      TLorentzVector p4 = eflowtrack->P4();
      p4.SetPtEtaPhiM(eflowtrack->PT, eflowtrack->Eta, eflowtrack->Phi, _electron_mass);

      electron->PT  = p4.Pt();
      electron->Eta = p4.Eta();
      electron->Phi = p4.Phi();

      // electron->Mass = p4.M();
      electron->Charge      = eflowtrack->Charge;
      electron->EhadOverEem = (Eem > 0.0) ? Ehad / Eem : 999.0;

      electron->D0      = eflowtrack->D0;
      electron->DZ      = eflowtrack->DZ;
      electron->ErrorD0 = eflowtrack->ErrorD0;
      electron->ErrorDZ = eflowtrack->ErrorDZ;

      electron->Particle = eflowtrack->Particle;

      _outputList->Add(electron);
    }
  }


  (*DataStore)[_params["outputList"]] = _outputList;


  return true;
}
