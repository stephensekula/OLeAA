#include "ElectronPIDModule.h"

#include "TClonesArray.h"
#include "TRandom3.h"
#include "TDatabasePDG.h"
#include "Math/PdfFuncMathCore.h"

#include "AnalysisFunctions.cc"

#include <iostream>

ElectronPIDModule::ElectronPIDModule(ExRootTreeReader* data, std::string name)
  : Module(data, name)
{
  _outputList = new TObjArray();
  _electron_mass = TDatabasePDG().GetParticle(11)->Mass();
}

ElectronPIDModule::~ElectronPIDModule()
{

}


bool ElectronPIDModule::execute(std::map<std::string, std::any>* DataStore)
{
  auto data = getData();

  
  if (DataStore->find("ChargedElectron") != DataStore->end()) {
    std::stringstream message;
    message << "Cannot overwrite existing list ChargedElectron [" << getName() << "::ElectronPIDModule]" << std::endl;
    throw std::runtime_error(message.str());
  }    

  if (_outputList != nullptr && _outputList->GetEntries() > 0) {
    for (Int_t i = 0; i < _outputList->GetEntries(); i++) {
      delete _outputList->At(i);
    }
    _outputList->Clear();
  }


  if ((*DataStore).find("Tower") == (*DataStore).end()||
      (*DataStore).find("EFlowTrack") == (*DataStore).end()) {
    std::stringstream message;
    message << "Missing input lists requested from the DataStore! [" << getName() << "::ElectronPIDModule]" << std::endl;
    throw std::runtime_error(message.str());
    
  }

  auto CaloTower     = std::any_cast<TClonesArray*>((*DataStore)["Tower"]);
  auto EFlowTrack     = std::any_cast<TClonesArray*>((*DataStore)["EFlowTrack"]);


  // Loop over EFlowTracks. Store for each the EM and HAD energy in the associated
  // Calorimeter tower.

  for (Int_t e = 0; e < EFlowTrack->GetEntries(); e++) {
    auto eflowtrack = static_cast<Track*>(EFlowTrack->At(e));
    if (eflowtrack->PT == 0.0)
      continue;
    // if (TMath::Abs(eflowtrack->PID)!=11)
    //   continue;
    std::vector<Tower*> track_towers;

    for (Int_t t = 0 ; t < CaloTower->GetEntries(); t++) {
      auto calotower = static_cast<Tower*>(CaloTower->At(t));
      auto tower_particles = calotower->Particles;
      for (Int_t ref = 0; ref < tower_particles.GetEntries(); ref++) {
	TRef track_ref = eflowtrack->Particle;
	TRef calo_ref = tower_particles.At(ref);
	TObject* calo_obj = calo_ref.GetObject();
	if (track_ref.GetObject() == nullptr || calo_obj == nullptr)
	  continue;

	if (track_ref.GetObject() == calo_ref.GetObject()) {
	  track_towers.push_back(calotower);
	}
      }
    }
    

    Double_t Eem = 0.0;
    Double_t Ehad = 0.0;
    for (auto track_tower : track_towers) {
      //std::cout << "E(EM, HAD) = " << track_tower->Eem << ", " << track_tower->Ehad << std::endl;
      Eem += track_tower->Eem;
      Ehad += track_tower->Ehad;
    }

    Electron* electron = new Electron(); //static_cast<Track*>(eflowtrack->Clone());

    // Correct the 4-vector for the mass
    TLorentzVector p4 = eflowtrack->P4();
    p4.SetPtEtaPhiM(eflowtrack->PT, eflowtrack->Eta, eflowtrack->Phi, _electron_mass);

    electron->PT = p4.Pt();
    electron->Eta = p4.Eta();
    electron->Phi = p4.Phi();
    //electron->Mass = p4.M();
    electron->Charge = eflowtrack->Charge;
    electron->EhadOverEem = (Eem > 0.0) ? Ehad/Eem : 999.0;
    
    electron->D0 = eflowtrack->D0;
    electron->DZ = eflowtrack->DZ;
    electron->ErrorD0 = eflowtrack->ErrorD0;
    electron->ErrorDZ = eflowtrack->ErrorDZ;

    electron->Particle = eflowtrack->Particle;

    _outputList->AddLast(electron);
  }



  (*DataStore)["ChargedElectron"] = _outputList;


  return true;
}





