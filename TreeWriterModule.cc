#include "TreeWriterModule.h"


TreeWriterModule::TreeWriterModule(ExRootTreeReader *data, std::string name)
  : Module(data, name)
{
  _global_vars    = std::map<TString, Double_t>();
  _candidate_vars = std::map<TString, std::vector<Double_t> >();


  _mpi = TDatabasePDG().GetParticle(211)->Mass();
  _mK  = TDatabasePDG().GetParticle(321)->Mass();
  _me  = TDatabasePDG().GetParticle(11)->Mass();
  _mmu = TDatabasePDG().GetParticle(13)->Mass();
  _mp  = TDatabasePDG().GetParticle(2212)->Mass();


  // Correcting ECAL/HCAL energy distribution using Full Simulation
  _cache_emfrac = std::map<SortableObject *, Double_t>();
  _emfrac_file  = TFile::Open("share/EMRatioPDFs.root");
}

TreeWriterModule::~TreeWriterModule()
{}

void TreeWriterModule::initialize()
{
  // Tree handler initialization
  TreeHandler *tree_handler = tree_handler->getInstance();

  if (tree_handler->getTree() != nullptr) {
    ExRootConfParam p = getConfiguration()->GetParam(Form("%s::branches", getName().c_str()));

    for (Int_t i = 0; i < p.GetSize(); i = i + 3) {
      Int_t   block_type_index = i;
      Int_t   list_index       = i + 1;
      Int_t   variables_index  = i + 2;
      TString blockName        = p[block_type_index].GetString();
      TString listName         = p[list_index].GetString();
      TString varNames         = p[variables_index].GetString();

      // Enforce some syntax
      if (blockName.Contains("_")) {
        std::stringstream message;
        message << "Naming syntax error: do not use underscores in block names! " << blockName.Data() << " violates this rule. [" << getName() << "::TreeWriterModule]" << std::endl;
        throw std::runtime_error(message.str());
      }

      TObjArray *varList = varNames.Tokenize(" ");

      for (Int_t v = 0; v < varList->GetEntries(); v++) {
        TString varName = static_cast<TObjString *>(varList->At(v))->GetString();

        if (listName == "") {
          // no list name ... treat these like global variables.

          if (varName == "MET") {
            _global_vars[blockName + "_MET_ET"]  = Double_t(0.0);
            _global_vars[blockName + "_MET_Phi"] = Double_t(0.0);
          } else if (varName == "DIS") {
            _global_vars[blockName + "_BJx"]  = Double_t(0.0);
            _global_vars[blockName + "_BJy"]  = Double_t(0.0);
            _global_vars[blockName + "_BJQ2"] = Double_t(0.0);
            _global_vars[blockName + "_JBx"]  = Double_t(0.0);
            _global_vars[blockName + "_JBQ2"] = Double_t(0.0);
          }
        } else {
          // A list provided means we compute for each list item
          TString prefix = blockName + "_" + listName;

          if (varName == "Kinematics") {
            _candidate_vars[prefix + "_KIN_PT"]  = std::vector<Double_t>();
            _candidate_vars[prefix + "_KIN_Eta"] = std::vector<Double_t>();
            _candidate_vars[prefix + "_KIN_Phi"] = std::vector<Double_t>();
            _candidate_vars[prefix + "_KIN_M"]   = std::vector<Double_t>();
          } else if (varName == "Calorimeter") {
            _candidate_vars[prefix + "_CALO_Eem"]  = std::vector<Double_t>();
            _candidate_vars[prefix + "_CALO_Ehad"] = std::vector<Double_t>();
          } else if (varName == "Truth") {
            _candidate_vars[prefix + "_TRU_ID"] = std::vector<Double_t>();
          } else if (varName == "JetTagging") {
            _candidate_vars[prefix + "_TAG_jet_charge_05"]   = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_sIP3DTagger"]     = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_kTagger"]         = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_CharmIPXDTagger"] = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t1_PT"]           = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t1_d0"]           = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t1_d0err"]        = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t1_z0"]           = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t1_z0err"]        = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t1_sIP3D"]        = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t1_IP3D"]         = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t1_IP2D"]         = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t2_PT"]           = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t2_d0"]           = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t2_d0err"]        = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t2_z0"]           = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t2_z0err"]        = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t2_sIP3D"]        = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t2_IP3D"]         = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t2_IP2D"]         = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t3_PT"]           = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t3_d0"]           = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t3_d0err"]        = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t3_z0"]           = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t3_z0err"]        = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t3_sIP3D"]        = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t3_IP3D"]         = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t3_IP2D"]         = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t4_PT"]           = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t4_d0"]           = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t4_d0err"]        = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t4_z0"]           = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t4_z0err"]        = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t4_sIP3D"]        = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t4_IP3D"]         = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_t4_IP2D"]         = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_k1_PT"]           = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_k1_q"]            = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_k1_sIP3D"]        = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_k1_IP3D"]         = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_k1_IP2D"]         = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_k2_PT"]           = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_k2_q"]            = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_k2_sIP3D"]        = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_k2_IP3D"]         = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_k2_IP2D"]         = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_e1_PT"]           = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_e1_sIP3D"]        = std::vector<Double_t>();

            // _candidate_vars[prefix + "_TAG_e1_EhadOverEM"] =
            // std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_e2_PT"]    = std::vector<Double_t>();
            _candidate_vars[prefix + "_TAG_e2_sIP3D"] = std::vector<Double_t>();

            // _candidate_vars[prefix + "_TAG_e2_EhadOverEM"] =
            // std::vector<Double_t>();
          }
        }
      }

      if (varList != nullptr) varList->Delete();
    }


    std::map<TString, Double_t>::iterator itg = _global_vars.begin();

    while (itg != _global_vars.end()) {
      tree_handler->getTree()->Branch(itg->first, &(itg->second), Form("%s/D", itg->first.Data()));
      itg++;
    }


    std::map<TString, std::vector<Double_t> >::iterator itc = _candidate_vars.begin();

    while (itc != _candidate_vars.end()) {
      tree_handler->getTree()->Branch(itc->first, "std::vector<Double_t>", &(itc->second));
      itc++;
    }


    tree_handler->getTree()->Print();
  }
}

void TreeWriterModule::finalize()
{}

bool TreeWriterModule::execute(std::map<std::string, std::any> *DataStore)
{
  // Clear any caches
  _cache_emfrac.clear();

  // Proceed to process the event

  auto data = getData();

  // Compute global DIS variables
  auto dis_variables = DISVariables(getGenParticles());
  auto jb_variables  = DISJacquetBlondel(getEFlowTracks(), getElectrons(), getPhotons(), getNeutralHadrons());


  // Get the MET object
  MissingET *MET = nullptr;

  for (int imet = 0; imet < getMET()->GetEntries(); imet++) {
    MET = static_cast<MissingET *>(getMET()->At(imet));
  }

  std::map<TString, Double_t>::iterator itg = _global_vars.begin();

  while (itg != _global_vars.end()) {
    if (itg->first.Contains("MET_ET")) {
      itg->second = MET->MET;
    } else if (itg->first.Contains("MET_Phi")) {
      itg->second = MET->Phi;
    } else if (itg->first.Contains("BJx")) {
      itg->second = dis_variables["x"];
    } else if (itg->first.Contains("BJy")) {
      itg->second = dis_variables["y"];
    } else if (itg->first.Contains("BJQ2")) {
      itg->second = dis_variables["Q2"];
    } else if (itg->first.Contains("JBx")) {
      itg->second = jb_variables["x_JB"];
    } else if (itg->first.Contains("JBQ2")) {
      itg->second = jb_variables["Q2_JB"];
    }

    itg++;
  }


  std::map<TString, std::vector<Double_t> >::iterator itc = _candidate_vars.begin();

  while (itc != _candidate_vars.end()) {
    // clear out any old data
    itc->second.clear();

    TString key          = itc->first;
    TObjArray *key_parts = key.Tokenize("_");

    TString listName = static_cast<TObjString *>(key_parts->At(1))->GetString();

    if (key_parts) delete key_parts;

    // Load the list from the DataStore
    TObjArray *candidateList = nullptr;

    if (DataStore->find(listName.Data()) != DataStore->end()) {
      try {
        candidateList = std::any_cast<TObjArray *>((*DataStore)[listName.Data()]);
      } catch (const std::bad_any_cast& e)
      {
        try {
          candidateList = std::any_cast<TClonesArray *>((*DataStore)[listName.Data()]);
        } catch (const std::bad_any_cast& e)
        {
          std::stringstream message;
          message << "Input list " << listName.Data()
                  << " must be either TObjArray or TClonesArray! [" << getName() << "::TreeWriterModule]" << std::endl;
          throw std::runtime_error(message.str());
        }
      }
    } else {
      std::stringstream message;
      message << "List " << listName.Data()
              << " does not exist in the DataStore! [" << getName() << "::TreeWriterModule]" << std::endl;
      throw std::runtime_error(message.str());
    }

    if (candidateList != nullptr) {
      for (Int_t c = 0; c < candidateList->GetEntries(); c++) {
        auto candidate = candidateList->At(c);

        if (itc->first.Contains("_KIN_")) {
          itc->second.push_back(kinVar(itc->first, candidate));
        } else if (itc->first.Contains("_CALO_")) {
          itc->second.push_back(caloVar(itc->first, candidate, DataStore));
        } else if (itc->first.Contains("TRU_ID")) {
          itc->second.push_back(truthID(candidate));
        } else if (itc->first.Contains("_TAG_")) {
          itc->second.push_back(jetTagging(itc->first, candidate, DataStore));
        }
      }
    }


    itc++;
  }


  return true;
}
