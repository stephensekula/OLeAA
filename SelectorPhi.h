#ifndef SELECTORPHI_HH
#define SELECTORPHI_HH

#include <iostream>
#include <map>
#include <string>
#include <any>

#include "Selector.h"

template <class T>
class SelectorPhi : public Selector<T> {

 public:

  SelectorPhi(std::string name);

  ~SelectorPhi();

  virtual bool select(T* obj, std::pair<Double_t, Double_t> range)
  {
    //std::cout << range.first << " <= " << obj->Phi << " <= " << range.second << "?" << std::endl;
    return (range.first <= obj->Phi && obj->Phi <= range.second);
  }

 private:
  


};

template class SelectorPhi<Jet>;
template class SelectorPhi<Track>;
template class SelectorPhi<Photon>;
template class SelectorPhi<Electron>;
template class SelectorPhi<Muon>;

#endif
