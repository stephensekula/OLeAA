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

// Other includes
#include "Module.h"
#include "AnalysisFunctions.cc"
#include "classes/DelphesClasses.h"
#include "JetTaggingTool.h"



class TreeWriterModule : public Module {
public:

  TreeWriterModule(ExRootTreeReader *data, std::string name);

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

  std::map<TString, Double_t> _global_vars;
  std::map<TString, std::vector<Double_t>> _candidate_vars;


 private:
  // Private methods
  Double_t kinVar(TString varName, TObject* obj) {
    if (obj->InheritsFrom("Jet")) {
      auto p = static_cast<Jet*>(obj);
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
      auto p = static_cast<Track*>(obj);
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
      auto p = static_cast<Electron*>(obj);
      if (varName.Contains("_PT")) {
	return p->P4().Pt();
      } else if (varName.Contains("_Eta")) {
	return p->P4().Eta();
      } else if (varName.Contains("_Phi")) {
	return p->P4().Phi();
      } else if (varName.Contains("_M")) {
	return p->P4().M();
      }
    } else if (obj->InheritsFrom("Muon")) {
      auto p = static_cast<Muon*>(obj);
      if (varName.Contains("_PT")) {
	return p->P4().Pt();
      } else if (varName.Contains("_Eta")) {
	return p->P4().Eta();
      } else if (varName.Contains("_Phi")) {
	return p->P4().Phi();
      } else if (varName.Contains("_M")) {
	return p->P4().M();
      }
    }
    return 0.0;
  }

  Double_t truthID(TObject* obj) {
    if (obj->InheritsFrom("Jet")) {
      auto p = static_cast<Jet*>(obj);
      return p->Flavor;
    } else if (obj->InheritsFrom("Track")) {
      auto p = static_cast<Track*>(obj);
      return p->PID;
    } else if (obj->InheritsFrom("Electron")) {
      auto p = static_cast<Electron*>(obj);
      return -11*p->Charge;
    } else if (obj->InheritsFrom("Muon")) {
      auto p = static_cast<Track*>(obj);
      return -13*p->Charge;
    }
    return 0.0;
  }

  Double_t jetTagging(TString varName, TObject* obj, std::map<std::string, std::any>* DataStore) {
    if (obj->InheritsFrom("Jet")) {
      auto p = static_cast<Jet*>(obj);
      
      JetTaggingTool* jet_tagger = jet_tagger->getInstance(getData());
      jet_tagger->execute(obj, DataStore);
      if (varName.Contains("sIP3DTagger")) {
	return jet_tagger->getJetTaggingInfo(obj).sIP3DTagged;
      }
      if (varName.Contains("t1_PT")) {
	return jet_tagger->getJetTaggingInfo(obj).t1_pt;
      }
      if (varName.Contains("t1_sIP3D")) {
	return jet_tagger->getJetTaggingInfo(obj).t1_sIP3D;
      }
      if (varName.Contains("t2_PT")) {
	return jet_tagger->getJetTaggingInfo(obj).t2_pt;
      }
      if (varName.Contains("t2_sIP3D")) {
	return jet_tagger->getJetTaggingInfo(obj).t2_sIP3D;
      }
      if (varName.Contains("t3_PT")) {
	return jet_tagger->getJetTaggingInfo(obj).t3_pt;
      }
      if (varName.Contains("t3_sIP3D")) {
	return jet_tagger->getJetTaggingInfo(obj).t3_sIP3D;
      }
      if (varName.Contains("t4_PT")) {
	return jet_tagger->getJetTaggingInfo(obj).t4_pt;
      }
      if (varName.Contains("t4_sIP3D")) {
	return jet_tagger->getJetTaggingInfo(obj).t4_sIP3D;
      }
      if (varName.Contains("k1_PT")) {
	return jet_tagger->getJetTaggingInfo(obj).k1_pt;
      }
      if (varName.Contains("k1_sIP3D")) {
	return jet_tagger->getJetTaggingInfo(obj).k1_sIP3D;
      }
      if (varName.Contains("k2_PT")) {
	return jet_tagger->getJetTaggingInfo(obj).k2_pt;
      }
      if (varName.Contains("k2_sIP3D")) {
	return jet_tagger->getJetTaggingInfo(obj).k2_sIP3D;
      }
    } 
    return 0.0;
  }

};

#endif // ifndef TREEWRITERMODULE_HH
