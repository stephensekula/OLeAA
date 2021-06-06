#ifndef SELECTORCHARGE_HH
#define SELECTORCHARGE_HH

#include <iostream>
#include <map>
#include <string>
#include <any>

#include "Selector.h"

template <class T>
class SelectorCharge : public Selector<T> {

 public:

  SelectorCharge(std::string name);

  ~SelectorCharge();

  bool select(T* obj, std::pair<Double_t, Double_t> range)
  {
    return true;
  }

  //
  // Class-dependent object charge computation
  //
  template<typename U = T>
  typename std::enable_if<std::is_same<U, Track>::value>::type
  select(U* obj, std::pair<Double_t, Double_t> range, Bool_t* selected)
  {
    (*selected) &= (range.first <= obj->Charge && obj->Charge <= range.second);
  }

  template<typename U = T>
  typename std::enable_if<std::is_same<U, Electron>::value>::type
  select(U* obj, std::pair<Double_t, Double_t> range, Bool_t* selected)
  {
    (*selected) &= (range.first <= obj->Charge && obj->Charge <= range.second);
  }

  template<typename U = T>
  typename std::enable_if<std::is_same<U, Muon>::value>::type
  select(U* obj, std::pair<Double_t, Double_t> range, Bool_t* selected)
  {
    (*selected) &= (range.first <= obj->Charge && obj->Charge <= range.second);
  }
  
  template<typename U = T>
  typename std::enable_if<std::is_same<U, Jet>::value>::type
  select(U* obj, std::pair<Double_t, Double_t> range, Bool_t* selected)
  {
    (*selected) &= (range.first <= obj->Charge && obj->Charge <= range.second);
  }
  
  template<typename U = T>
  typename std::enable_if<std::is_same<U, Photon>::value>::type
  select(U* obj, std::pair<Double_t, Double_t> range, Bool_t* selected)
  {
    (*selected) &= (range.first <= 0.0 && 0.0 <= range.second);
  }

 private:
  


};

template class SelectorCharge<Jet>;
template class SelectorCharge<Track>;
template class SelectorCharge<Photon>;
template class SelectorCharge<Electron>;
template class SelectorCharge<Muon>;

#endif
