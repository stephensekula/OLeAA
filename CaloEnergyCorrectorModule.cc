#include "CaloEnergyCorrectorModule.h"

#include "TClonesArray.h"

#include "TPRegexp.h"
#include "TObjArray.h"
#include "TObjString.h"

#include "TreeHandler.h"

CaloEnergyCorrectorModule::CaloEnergyCorrectorModule(ExRootTreeReader *data, std::string name)
  : Module(data, name)
{
  _params        = std::map<std::string, std::string>();
  _EMFractionMap = new std::map<SortableObject *, Double_t>();
  _emfrac_file   = TFile::Open("share/EMRatioPDFs.root");
}

CaloEnergyCorrectorModule::~CaloEnergyCorrectorModule()
{}

void CaloEnergyCorrectorModule::initialize()
{
  // Verify required parameters are specified
  std::vector<std::string> required;

  required.push_back("inputTrackList");
  required.push_back("inputTowerList");
  required.push_back("outputEMFractionMap");

  for (auto r : required) {
    ExRootConfParam p = getConfiguration()->GetParam(Form("%s::%s", getName().c_str(), r.c_str()));

    if (p.GetSize() > 0) {
      _params[r] = p.GetString();
    }

    if (_params.find(r) == _params.end()) {
      std::stringstream message;
      message << "Required parameter " << r << " not specified in module " << getName() << " of class CaloEnergyCorrectorModule!" << std::endl;
      throw std::runtime_error(message.str());
    } else {
      std::cout << getName() << "::" << r << ": value set to " << _params[r] << std::endl;
    }
  }
}

void CaloEnergyCorrectorModule::finalize()
{}

bool CaloEnergyCorrectorModule::execute(std::map<std::string, std::any> *DataStore)
{
  auto data = getData();

  TObjArray *inputTrackList = nullptr;
  TObjArray *inputTowerList = nullptr;

  // search in the DataStore for this list
  if (DataStore->find(_params["inputTrackList"]) != DataStore->end()) {
    try {
      inputTrackList = std::any_cast<TObjArray *>((*DataStore)[_params["inputTrackList"]]);
    } catch (const std::bad_any_cast& e)
    {
      try {
        inputTrackList = std::any_cast<TClonesArray *>((*DataStore)[_params["inputTrackList"]]);
      } catch (const std::bad_any_cast& e)
      {
        std::stringstream message;
        message << "Input list " << _params["inputTrackList"] << " must be either TObjArray or TClonesArray! [" << getName() << "::CaloEnergyCorrectorModule]" << std::endl;
        throw std::runtime_error(message.str());
      }
    }
  } else {
    std::stringstream message;
    message << "Unable to locate " << _params["inputTrackList"] << " specified in module " << getName() << " of class CaloEnergyCorrectorModule!" << std::endl;
    throw std::runtime_error(message.str());
  }

  // search in the DataStore for this list
  if (DataStore->find(_params["inputTowerList"]) != DataStore->end()) {
    try {
      inputTowerList = std::any_cast<TObjArray *>((*DataStore)[_params["inputTowerList"]]);
    } catch (const std::bad_any_cast& e)
    {
      try {
        inputTowerList = std::any_cast<TClonesArray *>((*DataStore)[_params["inputTowerList"]]);
      } catch (const std::bad_any_cast& e)
      {
        std::stringstream message;
        message << "Input list " << _params["inputTowerList"] << " must be either TObjArray or TClonesArray! [" << getName() << "::CaloEnergyCorrectorModule]" << std::endl;
        throw std::runtime_error(message.str());
      }
    }
  } else {
    std::stringstream message;
    message << "Unable to locate " << _params["inputTowerList"] << " specified in module " << getName() << " of class CaloEnergyCorrectorModule!" << std::endl;
    throw std::runtime_error(message.str());
  }

  // Write a new list of refined candidates to the outputTowerList
  if (DataStore->find(_params["outputEMFractionMap"]) != DataStore->end()) {
    // Do not allow overwrites of existing contents in the DataStore
    std::stringstream message;
    message << "An object named " << _params["outputEMFractionMap"] << " already exists in the DataStore! [" << getName() << "::CaloEnergyCorrectorModule]" << std::endl;
    throw std::runtime_error(message.str());
  }


  // Apply refinements to the contents of the inputTrackList to generate the
  // outputTowerList
  TIter iterator(inputTrackList);
  iterator.Reset();
  Candidate *candidate = nullptr;

  for (int i = 0; i < inputList->GetEntries(); i++) {
    Track *p = static_cast<Track *>(inputList->At(i));

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
    _EMFractionMap[p] = emfrac;
  }

  (*DataStore)[_params["outputEMFractionMap"]] = _EMFractionMap;

  return true;
}
