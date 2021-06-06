#include "Selector.h"

template <class T> Selector<T>::Selector(std::string name)
{
  _name = name;
}

template <class T> Selector<T>::~Selector()
{
}

template <class T> bool Selector<T>::select(T* obj, std::pair<Double_t, Double_t> range)
{
  return true;
}

