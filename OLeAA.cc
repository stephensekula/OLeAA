#include <TROOT.h>
#include <TFile.h>
#include <TChain.h>
#include <TTree.h>
#include <TString.h>
#include <TObjString.h>
#include "TInterpreter.h"

#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <getopt.h>
#include <glob.h>
#include <vector>
#include <map>
#include <any>

#include "classes/DelphesClasses.h"
#include "external/ExRootAnalysis/ExRootTreeReader.h"
#include "external/ExRootAnalysis/ExRootConfReader.h"
#include "external/ExRootAnalysis/ExRootTask.h"
#include "external/ExRootAnalysis/ExRootTreeReader.h"

#include "ModuleHandler.h"
#include "TreeHandler.h"
#include "JetTaggingTool.h"

static std::string input_dir   = "";
static std::string output_file = "";
static std::string config_file = "";
static int nevents             = -1;

ModuleHandler  *ModuleHandler::instance  = 0;
TreeHandler    *TreeHandler::instance    = 0;
JetTaggingTool *JetTaggingTool::instance = 0;

// HELPER METHODS

void PrintHelp()
{
  std::cout <<
    "--input_dir=<i>:       Directory containing all the ROOT files you want to process\n"
    "--output_file=<o>:     Output ROOT file to store results\n"
    "--config_file=<s>:     The TCL-based configuration file.\n"
    "--nevents=<n>:         The total number of events to process, starting from the zeroth event in the input.\n"
    "--help:                Show this helpful message!\n";

  exit(1);
}

std::vector<std::string>fileVector(const std::string& pattern) {
  glob_t glob_result;

  glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
  std::vector<std::string> files;

  for (unsigned int i = 0; i < glob_result.gl_pathc; ++i) {
    files.push_back(std::string(glob_result.gl_pathv[i]));
  }
  globfree(&glob_result);
  return files;
}

// MAIN FUNCTION


int main(int argc, char *argv[])
{
  std::cout <<
    "===================== OLeAA =====================" << std::endl;


  // Handle complex TTree data storage types by defining them for ROOT
  gInterpreter->GenerateDictionary("std::vector<std::vector<float>>", "vector");


  if (argc <= 1) {
    PrintHelp();
  }

  const char *const short_opts = "i:o:c:n:h";
  const option long_opts[]     = {
    { "input_dir",   required_argument,     nullptr,           'i'                 },
    { "output_file", required_argument,     nullptr,           'o'                 },
    { "config_file", required_argument,     nullptr,           'c'                 },
    { "nevents",     optional_argument,     nullptr,           'n'                 },
    { "help",        no_argument,           nullptr,           'h'                 },
    { nullptr,       no_argument,           nullptr,                             0 }
  };

  while (true)
  {
    const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

    if (-1 == opt) break;

    switch (opt)
    {
      case 'i':
        input_dir = optarg;
        std::cout << "Input Directory: " << input_dir << std::endl;
        break;

      case 'o':
        output_file = optarg;
        std::cout << "Output File: " << output_file << std::endl;
        break;

      case 'c':
        config_file = optarg;
        std::cout << "Configuration file: " << config_file << std::endl;
        break;

      case 'n':
        std::cout << optarg << std::endl;
        nevents = std::stoi(optarg);
        std::cout << "Number of events to process: " << nevents << std::endl;
        break;


      case 'h': // -h or --help
      case '?': // Unrecognized option
        PrintHelp();
        break;
      default:
        PrintHelp();
        break;
    }
  }


  // Prepare the data input
  auto data = new TChain("Delphes");

  auto files = fileVector(input_dir);

  for (auto file : files)
  {
    data->Add(file.c_str());
  }

  ExRootTreeReader *treeReader = new ExRootTreeReader(data);

  int n_entries = data->GetEntries();

  std::cout
    << "The provided data set contains the following number of events: " << std::endl
    << n_entries
    << std::endl;


  // Setup the ModuleHandler
  ModuleHandler *module_handler = module_handler->getInstance(treeReader);


  // Read the connfiguration information from a TCL file
  ExRootConfReader *confReader = new ExRootConfReader();
  confReader->ReadFile(config_file.c_str());
  confReader->SetName("OLeAAConfReader");

  TString name;
  ExRootTask *task;
  const ExRootConfReader::ExRootTaskMap *modules = confReader->GetModules();
  ExRootConfReader::ExRootTaskMap::const_iterator itModules;

  ExRootConfParam param = confReader->GetParam("::ExecutionPath");
  Long_t i, size = param.GetSize();

  for (i = 0; i < size; ++i)
  {
    name      = param[i].GetString();
    itModules = modules->find(name);

    if (itModules != modules->end())
    {
      std::cout << "   Appending module " << itModules->second.Data() << std::endl;
      std::cout << "              named " << itModules->first.Data() << std::endl;
      module_handler->addModule(itModules->second.Data(), itModules->first.Data());

      Module *the_module = module_handler->getModule(itModules->first.Data());

      if (the_module == nullptr) {
        stringstream message;
        message << "module '" << itModules->first.Data();
        message << " of type " << itModules->second.Data();
        message << "' could not be retrieved from the ModuleHandler after its creation.";
        throw runtime_error(message.str());
      } else {
        the_module->setConfiguration(confReader);
      }

      // module_handler->getModule(itModules->first.Data())->setConfiguration(confReader);
    }
    else
    {
      stringstream message;
      message << "module '" << name;
      message << "' is specified in ExecutionPath but not configured.";
      throw runtime_error(message.str());
    }
  }


  // Load object pointers
  std::map<TString, TClonesArray *> branchPointer;
  branchPointer["Jet"]                = treeReader->UseBranch("Jet");
  branchPointer["Electron"]           = treeReader->UseBranch("Electron");
  branchPointer["EFlowPhoton"]        = treeReader->UseBranch("EFlowPhoton");
  branchPointer["EFlowNeutralHadron"] = treeReader->UseBranch("EFlowNeutralHadron");
  branchPointer["GenJet"]             = treeReader->UseBranch("GenJet");
  branchPointer["Particle"]           = treeReader->UseBranch("Particle");
  branchPointer["Track"]              = treeReader->UseBranch("Track");
  branchPointer["EFlowTrack"]         = treeReader->UseBranch("EFlowTrack");
  branchPointer["MissingET"]          = treeReader->UseBranch("MissingET");
  branchPointer["Tower"]              = treeReader->UseBranch("Tower");
  branchPointer["BeamSpot"]           = treeReader->UseBranch("BeamSpot");

  branchPointer["mRICHTrack"]      = treeReader->UseBranch("mRICHTrack");
  branchPointer["barrelDIRCTrack"] = treeReader->UseBranch("barrelDIRCTrack");
  branchPointer["dualRICHagTrack"] = treeReader->UseBranch("dualRICHagTrack");
  branchPointer["dualRICHcfTrack"] = treeReader->UseBranch("dualRICHcfTrack");


  // Setup the output storage
  TreeHandler *tree_handler = tree_handler->getInstance(output_file.c_str(), "tree");
  tree_handler->initialize();

  for (auto module : module_handler->getModules()) {
    module->initialize();
  }

  if (nevents < 0) {
    std::cout
      << "Processing all events in the sample..." << std::endl;
  } else {
    std::cout
      << "Processing " << nevents << " events in the sample..." << std::endl;
  }

  for (int i = 0; i < n_entries; ++i) {
    // event number printout
    if (i % 1000 == 0) {
      std::cout << "Processing Event " << i << std::endl;
    }

    if ((nevents >= 0) && (i >= nevents)) break;

    // read the data for i-th event
    // data->GetEntry(i);
    // Load selected branches with data from specified event
    treeReader->ReadEntry(i);

    std::map<std::string, std::any> DataStore;
    DataStore["Jet"]                = branchPointer["Jet"];
    DataStore["GenJet"]             = branchPointer["GenJet"];
    DataStore["Particle"]           = branchPointer["Particle"];
    DataStore["EFlowTrack"]         = branchPointer["EFlowTrack"];
    DataStore["Track"]              = branchPointer["Track"];
    DataStore["EFlowPhoton"]        = branchPointer["EFlowPhoton"];
    DataStore["Electron"]           = branchPointer["Electron"];
    DataStore["EFlowNeutralHadron"] = branchPointer["EFlowNeutralHadron"];
    DataStore["MissingET"]          = branchPointer["MissingET"];
    DataStore["Tower"]              = branchPointer["Tower"];
    DataStore["BeamSpot"]           = branchPointer["BeamSpot"];

    // PID system branches (lists of particles ID'd using PID systems)
    DataStore["mRICHTrack"]      = branchPointer["mRICHTrack"];
    DataStore["barrelDIRCTrack"] = branchPointer["barrelDIRCTrack"];
    DataStore["dualRICHagTrack"] = branchPointer["dualRICHagTrack"];
    DataStore["dualRICHcfTrack"] = branchPointer["dualRICHcfTrack"];

    for (auto module : module_handler->getModules()) {
      module->setJets(branchPointer["Jet"]);
      module->setGenJets(branchPointer["GenJet"]);
      module->setEFlowTracks(branchPointer["EFlowTrack"]);
      module->setTracks(branchPointer["Track"]);
      module->setGenParticles(branchPointer["Particle"]);
      module->setPhotons(branchPointer["EFlowPhoton"]);
      module->setElectrons(branchPointer["Electron"]);
      module->setNeutralHadrons(branchPointer["EFlowNeutralHadron"]);
      module->setMET(branchPointer["MissingET"]);

      bool result = module->execute(&DataStore);

      if (result == false) break;
    }

    tree_handler->execute();

    // Clean up the data store
    // SJS: the DataStore contains object it owns and objects it does not
    //      some of the latter are allocated using "new" and must be deleted.
    //      this code slowly leaks memory because of this. FIX!
    //      (problem: how to delete only objects we create in OLeAA?)

    for (auto datum : DataStore) {
      try {
        // TObjArray* store_obj = std::any_cast<TObjArray*>(datum.second);
        // for (Int_t i = 0; i < array->GetEntries(); i++) {
        //   array->At(i)->Delete();
        // }
        // store_obj->Delete();
      } catch (const std::bad_any_cast& e) {}
    }
  }

  for (auto module : module_handler->getModules()) {
    module->finalize();
  }
  tree_handler->finalize();


  std::cout <<
    "========================== FINIS =========================" << std::endl;

  exit(EXIT_SUCCESS);
}
