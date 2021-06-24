#ifndef REFINERMODULE_HH
#define REFINERMODULE_HH

/**
   This module allows for a list of particles to be input,
   refined, and written out to a new list for usage by another
   module. All configuration and customization is handled 
   by the TCL configuration file loaded into OLeAA.
 **/


#include "classes/DelphesClasses.h"

#include <iostream>
#include <iomanip>  
#include <fstream>
#include <type_traits>

#include "Module.h"
#include "AnalysisFunctions.cc"

// Object and other selectors
#include "SelectorPT.h"
#include "SelectorEta.h"
#include "SelectorPhi.h"
#include "SelectorCharge.h"

template <class T>
class RefinerModule : public Module {

public:
  
  RefinerModule(ExRootTreeReader* data, std::string name);
  
  ~RefinerModule();
  
  void initialize() override;
  bool execute(std::map<std::string, std::any>* DataStore) override;
  void finalize() override;

  // parameter-setting methods
  void setParam(std::string param, std::string value) {
    _params[param] = value;
  };

  std::string getParam(std::string param) {
    if (_params.find(param) != _params.end()) {
      return _params[param];
    } else {
      std::stringstream message;
      message << "Requested parameter " << param << " has not been specified or set in module " << getName() << std::endl;
      throw std::runtime_error(message.str());
    }
  };

 private:
  // Private data members

  std::map<std::string,std::string> _params;
  std::map<std::string, std::pair<Double_t,Double_t>> _refinements;

  std::map<std::string, Selector<T>*> _selectors; 

  TObjArray* _outputList = nullptr;
  
 private:
  // Methods internal to the class

  //
  // Class-dependent object cloning
  //
  template<typename U = T>
  typename std::enable_if<std::is_same<U, Track>::value>::type
  Clone(U* obj, U* clone)
  {
    clone = static_cast<U*>(obj->Clone());
    clone->Particle = obj->Particle;
  }

  template<typename U = T>
  typename std::enable_if<std::is_same<U, Jet>::value>::type
  Clone(U* obj, U* clone)
  {
    clone = static_cast<U*>(obj->Clone());
  }

  template<typename U = T>
  typename std::enable_if<std::is_same<U, Photon>::value>::type
  Clone(U* obj, U* clone)
  {
    clone = static_cast<U*>(obj->Clone());
  }


};

template class RefinerModule<Jet>;
typedef RefinerModule<Jet> JetRefinerModule; 

template class RefinerModule<Track>;
typedef RefinerModule<Track> TrackRefinerModule; 

template class RefinerModule<Photon>;
typedef RefinerModule<Photon> NeutralRefinerModule;

template class RefinerModule<Electron>;
typedef RefinerModule<Electron> ElectronRefinerModule;

template class RefinerModule<Muon>;
typedef RefinerModule<Muon> MuonRefinerModule;



#endif
