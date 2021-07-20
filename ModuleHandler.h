#ifndef MODULEHANDLER_HH
#define MODULEHANDLER_HH

#include <iostream>
#include <vector>

#include "TTree.h"
#include "external/ExRootAnalysis/ExRootTreeReader.h"

#include "Module.h"
#include "KaonPIDModule.h"
#include "ElectronPIDModule.h"
#include "RefinerModule.h"
#include "TreeWriterModule.h"
#include "CaloEnergyCorrectorModule.h"

using namespace std;

class ModuleHandler {
  static ModuleHandler *instance;
  std::vector<Module *>module_sequence;

  // Private constructor so that no objects can be created.
  ModuleHandler(ExRootTreeReader *data) {
    module_sequence = std::vector<Module *>();
    _data           = data;
  }

public:

  static ModuleHandler* getInstance(ExRootTreeReader *data) {
    if (!instance) instance = new ModuleHandler(data);
    return instance;
  }

  std::vector<Module *>getModules() {
    return this->module_sequence;
  }

  void addModule(std::string mod_class) {
    addModule(mod_class, "");
  }

  void addModule(std::string mod_class, std::string mod_name) {
    Module *module = nullptr;

    if (mod_class == "KaonPIDModule") {
      module = new KaonPIDModule(_data, mod_name);
    }
    else if (mod_class == "ElectronPIDModule") {
      module = new ElectronPIDModule(_data, mod_name);
    }
    else if (mod_class == "JetRefinerModule") {
      module = new JetRefinerModule(_data, mod_name);
    }
    else if (mod_class == "TrackRefinerModule") {
      module = new TrackRefinerModule(_data, mod_name);
    }
    else if (mod_class == "ElectronRefinerModule") {
      module = new ElectronRefinerModule(_data, mod_name);
    }
    else if (mod_class == "MuonRefinerModule") {
      module = new MuonRefinerModule(_data, mod_name);
    }
    else if (mod_class == "NeutralRefinerModule") {
      module = new NeutralRefinerModule(_data, mod_name);
    }
    else if (mod_class == "TreeWriterModule") {
      module = new TreeWriterModule(_data, mod_name);
    }
    else if (mod_class == "CaloEnergyCorrectorModule") {
      module = new CaloEnergyCorrectorModule(_data, mod_name);
    } else {
      std::cout << "ModuleHandler(): The requested module class, " << mod_class << ", is unknown to the ModuleHandler!" << std::endl;
      assert(1 == 1);
    }

    if (module != nullptr) this->module_sequence.push_back(module);
  }

  Module* getModule(std::string mod_name)
  {
    for (auto m : this->module_sequence) {
      if (m->getName() == mod_name) {
        return m;
      }
    }
    return nullptr;
  }

private:

  ExRootTreeReader *_data = nullptr;
};

#endif // ifndef MODULEHANDLER_HH
