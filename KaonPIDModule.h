#ifndef KAONPIDMODULE_HH
#define KAONPIDMODULE_HH

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


class KaonPIDModule : public Module {

 public:

  KaonPIDModule(ExRootTreeReader* data, std::string name);

  ~KaonPIDModule();

  void initialize() override {};
  bool execute(std::map<std::string, std::any>* DataStore) override;
  void finalize() override {};

 private:

  TObjArray* _outputList = nullptr;
};

#endif
