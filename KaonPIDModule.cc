#include "KaonPIDModule.h"

#include "TClonesArray.h"
#include "TRandom3.h"
#include "Math/PdfFuncMathCore.h"

#include "AnalysisFunctions.cc"

#include <iostream>

KaonPIDModule::KaonPIDModule(ExRootTreeReader* data, std::string name)
  : Module(data, name)
{
  _outputList = new TObjArray();
  _kaon_mass = TDatabasePDG().GetParticle(321)->Mass();
}

KaonPIDModule::~KaonPIDModule()
{

}


bool KaonPIDModule::execute(std::map<std::string, std::any>* DataStore)
{
  auto data = getData();


  if ((*DataStore).find("mRICHTrack") == (*DataStore).end() ||
      (*DataStore).find("barrelDIRCTrack") == (*DataStore).end() ||
      (*DataStore).find("dualRICHagTrack") == (*DataStore).end() ||
      (*DataStore).find("dualRICHcfTrack") == (*DataStore).end() ||
      (*DataStore).find("Track") == (*DataStore).end()) {
    std::stringstream message;
    message << "Missing track lists from DataStore! [" << getName() << "::KaonPIDModule]" << std::endl;
    throw std::runtime_error(message.str());
    
  }

  auto mRICHTrack     = std::any_cast<TClonesArray*>((*DataStore)["mRICHTrack"]);
  auto barrelDIRCTrack = std::any_cast<TClonesArray*>((*DataStore)["barrelDIRCTrack"]);
  auto dualRICHagTrack = std::any_cast<TClonesArray*>((*DataStore)["dualRICHagTrack"]);
  auto dualRICHcfTrack = std::any_cast<TClonesArray*>((*DataStore)["dualRICHcfTrack"]);
  auto RawTrack        = std::any_cast<TClonesArray*>((*DataStore)["Track"]);


  if (_outputList != nullptr && _outputList->GetEntries() > 0) {
    for (Int_t i = 0; i < _outputList->GetEntries(); i++) {
      delete _outputList->At(i);
    }
    _outputList->Clear();
  }

  if (mRICHTrack != nullptr) {
    for (int itrk = 0; itrk < mRICHTrack->GetEntries(); itrk++) {
      Track *track = (Track *)mRICHTrack->At(itrk);

      if ((track->Eta  < -3.5) || (-1.0 < track->Eta)) continue;
      
      Int_t reco_pid = track->PID;

      if (TMath::Abs(reco_pid) == 321) {
	_outputList->AddLast(newKaon(track));
      }
    }
  }

  if (barrelDIRCTrack != nullptr) {
    for (int itrk = 0; itrk < barrelDIRCTrack->GetEntries(); itrk++) {
      Track *track = (Track *)barrelDIRCTrack->At(itrk);
      
      if ((track->Eta  < -1.0) || (1.0 < track->Eta)) continue;
      
      Int_t reco_pid = track->PID;

      if (TMath::Abs(reco_pid) == 321) {
	_outputList->AddLast(newKaon(track));
      }
    }
  }


  // Handle tracks in the forward direction (dualRICH)
  for (Int_t t = 0; t < RawTrack->GetEntries(); t++) {
    auto track = static_cast<Track*>(RawTrack->At(t));
    if (1.0 <= track->Eta && track->Eta <= 3.5) {
      Int_t final_pid = 0;
      Double_t p_track = track->P4().Vect().Mag();
      Double_t ag_p_threshold = 12.0;

      if (p_track < ag_p_threshold) {
	// region of sensitivity for Aerogel
	if (dualRICHagTrack != nullptr) {
	  for (int itrk = 0; itrk < dualRICHagTrack->GetEntries(); itrk++) {
	    Track *track_ag = (Track *)dualRICHagTrack->At(itrk);
	    if (track_ag->Particle.GetObject() == track->Particle.GetObject()) {
	      final_pid = track_ag->PID;
	      break;
	    }
	  }
	}
      } else {
	if (dualRICHcfTrack != nullptr) {
	  for (int itrk = 0; itrk < dualRICHcfTrack->GetEntries(); itrk++) {
	    Track *track_cf = (Track *)dualRICHcfTrack->At(itrk);
	    if (track_cf->Particle.GetObject() == track->Particle.GetObject()) {
	      final_pid = track_cf->PID;
	      break;
	    }
	  }
	}
      }
      
      Track drich_track = *track;
      drich_track.PID = final_pid;
      
      if (TMath::Abs(final_pid) == 321) 
	_outputList->AddLast(newKaon(&drich_track));

    }


  }




  (*DataStore)["ChargedKaon"] = _outputList;


  return true;
}


Track* KaonPIDModule::newKaon(Track* track)
{
  if (track != nullptr) {
      Track* charged_kaon = static_cast<Track*>(track->Clone());
      auto p4 = charged_kaon->P4();
      p4.SetPtEtaPhiM(charged_kaon->PT, charged_kaon->Eta, charged_kaon->Phi, _kaon_mass);
      charged_kaon->PT = p4.Pt();
      charged_kaon->Eta = p4.Eta();
      charged_kaon->Phi = p4.Phi();
      charged_kaon->Mass = p4.M();

      charged_kaon->Particle = track->Particle;

      return charged_kaon;
    }

  return nullptr;
}





