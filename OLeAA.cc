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

static std::string input_dir = "";
static std::string output_file = "";
static std::string config_file = "";
static int nevents = -1;

ModuleHandler *ModuleHandler::instance = 0;
TreeHandler *TreeHandler::instance = 0;


// HELPER METHODS

void PrintHelp()
{
  std::cout <<
    "--input_dir <i>:       Directory containing all the ROOT files you want to process\n"
    "--output_file <o>:     Output ROOT file to store results\n"
    "--module_sequence <s>: A string comma-separated list of modules to load; order is preserved in execution.\n"
    "--nevents <n>:         The total number of events to process, starting from the zeroth event in the input.\n"
    "--help:                Show this helpful message!\n";
  exit(1);
}

std::vector<std::string> fileVector(const std::string& pattern){
  glob_t glob_result;
  glob(pattern.c_str(),GLOB_TILDE,NULL,&glob_result);
  std::vector<std::string> files;
  for(unsigned int i=0;i<glob_result.gl_pathc;++i){
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
  gInterpreter->GenerateDictionary("std::vector<std::vector<float>>","vector");

	    

  if (argc <= 1) {
    PrintHelp();
  }

  const char* const short_opts = "i:o:c:n:h";
  const option long_opts[] = {
    {"input_dir", required_argument, nullptr, 'i'},
    {"output_file", required_argument, nullptr, 'o'},
    {"config_file", required_argument, nullptr, 'c'},
    {"nevents", optional_argument, nullptr, 'n'},
    {"help", no_argument, nullptr, 'h'},
    {nullptr, no_argument, nullptr, 0}
  };

  while (true)
    {
      const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);
      
      if (-1 == opt)
	break;
      
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

  for(i = 0; i < size; ++i)
    {
      name = param[i].GetString();
      itModules = modules->find(name);
      if(itModules != modules->end())
	{
	  std::cout << "   Appending module " << itModules->second.Data() << std::endl;
	  module_handler->addModule(itModules->second.Data());
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
  TClonesArray *branchJet = treeReader->UseBranch("Jet");
  TClonesArray *branchElectron = treeReader->UseBranch("Electron");
  TClonesArray *branchPhoton = treeReader->UseBranch("EFlowPhoton");
  TClonesArray *branchNeutralHadron = treeReader->UseBranch("EFlowNeutralHadron");
  TClonesArray *branchGenJet = treeReader->UseBranch("GenJet");
  TClonesArray *branchGenParticle = treeReader->UseBranch("Particle");
  TClonesArray *branchRawTrack = treeReader->UseBranch("Track");
  TClonesArray *branchEFlowTrack = treeReader->UseBranch("EFlowTrack");
  TClonesArray *branchMET = treeReader->UseBranch("MissingET");


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
      << "Processing "<< nevents << " events in the sample..." << std::endl;
  }

  for(int i=0; i < n_entries; ++i) {
    // event number printout
    if(i%1000==0) {
      std::cout << "Processing Event " << i << std::endl;
    }

    if (nevents >= 0 && i >= nevents)
      break;

    // read the data for i-th event
    // data->GetEntry(i);
    // Load selected branches with data from specified event
    treeReader->ReadEntry(i);

    std::map<std::string, std::any> DataStore;


    for (auto module : module_handler->getModules()) {
      module->setJets(branchJet);
      module->setGenJets(branchGenJet);
      module->setEFlowTracks(branchEFlowTrack);
      module->setTracks(branchRawTrack);
      module->setGenParticles(branchGenParticle);
      module->setPhotons(branchPhoton);
      module->setElectrons(branchElectron);
      module->setNeutralHadrons(branchNeutralHadron);
      module->setMET(branchMET);

      bool result = module->execute(&DataStore);
      if (result == false) 
	break;
    }

    tree_handler->execute();
    

    // if (DataStore.find("CharmJets") != DataStore.end()) {
    //   std::vector<Jet*> charm_jets = std::any_cast<std::vector<Jet*>>(DataStore["CharmJets"]);
    // }

  }

  for (auto module : module_handler->getModules()) {
    module->finalize();
  }
  tree_handler->finalize();


  std::cout <<
    "========================== FINIS =========================" << std::endl;

  exit(EXIT_SUCCESS);  
}
