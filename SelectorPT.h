#ifndef SELECTORPT_HH
#define SELECTORPT_HH

#include <iostream>
#include <map>
#include <string>
#include <any>

#include "Selector.h"

template <class T>
class SelectorPT : public Selector<T> {

 public:

  SelectorPT(std::string name);

  ~SelectorPT();

  virtual bool select(T* obj, std::pair<Double_t, Double_t> range)
  {
    //std::cout << range.first << " <= " << obj->PT << " <= " << range.second << "?" << std::endl;
    return (range.first <= obj->PT && obj->PT <= range.second);
  }

 private:
  


};

template class SelectorPT<Jet>;
template class SelectorPT<Track>;
template class SelectorPT<Photon>;
template class SelectorPT<Electron>;
template class SelectorPT<Muon>;
#endif
