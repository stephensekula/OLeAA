#ifndef SELECTOR_HH
#define SELECTOR_HH

#include <map>
#include <string>
#include <any>

#include "classes/DelphesClasses.h"
#include "TTree.h"

template <class T>
class Selector {

 public:

  Selector(std::string name);

  ~Selector();

  virtual bool select(T* obj, std::pair<Double_t, Double_t> range);

  std::string getName() {
    return _name;
  }


 private:

  std::string _name = "";



};

template class Selector<Jet>;
template class Selector<Track>;
template class Selector<Photon>;
template class Selector<Electron>;
template class Selector<Muon>;

#endif
