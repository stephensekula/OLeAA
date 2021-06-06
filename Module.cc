#include "Module.h"

Module::Module(ExRootTreeReader* data, std::string name)
{
  _data = data;
  _name = name;
}

Module::~Module()
{
}

void Module::initialize() 
{
}

bool Module::execute(std::map<std::string, std::any>* DataStore)
{
  return true;
}

void Module::finalize()
{
}


