#ifndef SELECTORETA_HH
#define SELECTORETA_HH

#include <iostream>
#include <map>
#include <string>
#include <any>

#include "Selector.h"

template <class T>
class SelectorEta : public Selector<T> {

 public:

  SelectorEta(std::string name);

  ~SelectorEta();

  virtual bool select(T* obj, std::pair<Double_t, Double_t> range)
  {
    //std::cout << range.first << " <= " << obj->Eta << " <= " << range.second << "?" << std::endl;
    return (range.first <= obj->Eta && obj->Eta <= range.second);
  }

 private:
  


};

template class SelectorEta<Jet>;
template class SelectorEta<Track>;
template class SelectorEta<Photon>;
template class SelectorEta<Electron>;
template class SelectorEta<Muon>;

#endif
