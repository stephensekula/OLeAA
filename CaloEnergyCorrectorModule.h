#ifndef REFINERMODULE_HH
#define REFINERMODULE_HH

/**
   This module allows for a list of particles to be input,
   their calorimeter towers identified, and the tower energies
   corrected into a new structure in the DataStore for later
   usage.
 **/


#include "classes/DelphesClasses.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <type_traits>

#include "Module.h"
#include "AnalysisFunctions.cc"


class CaloEnergyCorrectorModule : public Module {
public:

  CaloEnergyCorrectorModule(ExRootTreeReader *data,
                            std::string       name);

  ~CaloEnergyCorrectorModule();

  void initialize() override;
  bool execute(std::map<std::string, std::any> *DataStore) override;
  void finalize() override;

private:

  // Private data members

  std::map<std::string, std::string>_params;

  // Internal correction of calorimeter energy distribution based on Full
  // Simulation
  std::map<SortableObject *, Double_t> *_EMFractionMap = nullptr;
  TFile *_emfrac_file                                  = nullptr;

private:

  // Methods internal to the class
};


#endif // ifndef REFINERMODULE_HH
