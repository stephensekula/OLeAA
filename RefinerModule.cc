#include "RefinerModule.h"

#include "TClonesArray.h"

#include "TPRegexp.h"
#include "TObjArray.h"
#include "TObjString.h"

#include "TreeHandler.h"

template <class T> RefinerModule<T>::RefinerModule(ExRootTreeReader* data, std::string name)
  : Module(data, name)
{
  _params = std::map<std::string, std::string>();
  _refinements = std::map<std::string, std::pair<Double_t,Double_t>>();
  _selectors = std::map<std::string, Selector<T>*>();
  _outputList = new TObjArray();
}

template <class T> RefinerModule<T>::~RefinerModule()
{

}

template <class T> void RefinerModule<T>::initialize()
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
  
  std::vector<std::string> optional;
  optional.push_back("selectors");

  for (auto o : optional) {

    ExRootConfParam p = getConfiguration()->GetParam(Form("%s::%s", getName().c_str(), o.c_str()));

    for (Int_t i = 0; i < p.GetSize(); i++) {
      TString selector = p[i].GetString();
      TObjArray* selector_parts = TPRegexp("^(.*) (.*):(.*)").MatchS(selector);
      if (selector_parts->GetEntries() != 4) {
	std::stringstream message;
	message << "Selector " << selector << " has incorrect syntax! [" << getName() << "::RefinerModule]" << std::endl;
	throw std::runtime_error(message.str());
      }
      std::string varName = (static_cast<TObjString*>(selector_parts->At(1)))->GetString().Data();
      Double_t minRange = std::stod((static_cast<TObjString*>(selector_parts->At(2)))->GetString().Data());
      Double_t maxRange = std::stod((static_cast<TObjString*>(selector_parts->At(3)))->GetString().Data());
      
      _refinements[varName] = std::pair(minRange, maxRange);

    }

  }
  

  _selectors["PT"] = new SelectorPT<T>("PT");
  _selectors["Eta"] = new SelectorEta<T>("Eta");
  _selectors["Phi"] = new SelectorPhi<T>("Phi");
  _selectors["Q"] = new SelectorCharge<T>("Q");
  

}

template <class T> void RefinerModule<T>::finalize()
{
  for (auto s : _selectors) {
    delete s.second;
  }
}
template <class T> bool RefinerModule<T>::execute(std::map<std::string, std::any>* DataStore)
{
  auto data = getData();

  TObjArray* inputList = nullptr;
  
  // search in the DataStore for this list
  if (DataStore->find(_params["inputList"]) != DataStore->end()) {
    try {
      inputList = std::any_cast<TObjArray* >((*DataStore)[_params["inputList"]]);
    } catch(const std::bad_any_cast& e)
    {
      try {
	inputList = std::any_cast<TClonesArray* >((*DataStore)[_params["inputList"]]);
      } catch (const std::bad_any_cast& e)
	{
	  std::stringstream message;
	  message << "Input list " << _params["inputList"] << " must be either TObjArray or TClonesArray! [" << getName() << "::RefinerModule]" << std::endl;
	  throw std::runtime_error(message.str());
	}
    }
  } else {
    std::stringstream message;
    message << "Unable to locate " << _params["inputList"] << " specified in module " << getName() << " of class RefinerModule!" << std::endl;
    throw std::runtime_error(message.str());
  }
  // Write a new list of refined candidates to the outputList
  if (DataStore->find(_params["outputList"]) != DataStore->end()) {
    // Do not allow overwrites of existing contents in the DataStore
    std::stringstream message;
    message << "An object named " << _params["outputList"] << " already exists in the DataStore! [" << getName() << "::RefinerModule]" << std::endl;
    throw std::runtime_error(message.str());
  }    

  if (_outputList != nullptr && _outputList->GetEntries() > 0) {
    for (Int_t i = 0; i < _outputList->GetEntries(); i++) {
      delete _outputList->At(i);
    }
    _outputList->Clear();
  }

  // Apply refinements to the contents of the inputList to generate the outputList
  TIter iterator(inputList);
  iterator.Reset();
  Candidate* candidate = nullptr;
  // while((candidate = static_cast<Candidate *>(iterator.Next()))) {
  for (int i = 0; i < inputList->GetEntries(); i++) {
    T* candidate  = static_cast<T*>(inputList->At(i));
    Bool_t keepCandidate = kTRUE;

    for (auto selection : _refinements) {
      std::string varName = selection.first;
      std::pair<Double_t, Double_t> varRange = selection.second;

      Double_t varValue = 0.0;

      if (varName == "PT") {
	if (_selectors.find(varName) != _selectors.end()) {
	  keepCandidate &= _selectors[varName]->select(candidate, varRange);
	}
      }
      else if (varName == "Eta") {
	if (_selectors.find(varName) != _selectors.end()) {
	  keepCandidate &= _selectors[varName]->select(candidate, varRange);
	}
      }
      else if (varName == "Phi") {
	if (_selectors.find(varName) != _selectors.end()) {
	  keepCandidate &= _selectors[varName]->select(candidate, varRange);
	}
      }
      else if (varName == "Q") {
	if (_selectors.find(varName) != _selectors.end()) {
	  auto selector = static_cast< SelectorCharge<T>* >(_selectors[varName]);
	  selector-> template select<T>(candidate, varRange, &keepCandidate);
	}
      }

    }
    if (keepCandidate) {
      Candidate* new_candidate = static_cast<Candidate*>(candidate->Clone());
      //new_candidate->Particle = candidate->Particle; // breaks for Photon objects
      _outputList->AddLast(new_candidate);
    }
  }

  // std::cout << "[" << getName() << "::RefinerModule]: candidate reduction is "<< inputList->GetEntries() << " => " << _outputList->GetEntries() << std::endl;

  (*DataStore)[_params["outputList"]] = _outputList;

  return true;
}

