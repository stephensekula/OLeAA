CXX = g++
CXXFLAGS = -std=c++17 -Wl,-rpath-link=$(shell pythia8-config  --libdir) $(shell root-config --cflags --ldflags --libs) -lEG -lTMVA
CFILES   = $(wildcard *.cc)
INCLUDE  = -I$(DELPHES_PATH) -I$(DELPHES_PATH)/external/ 
LIBS     = -L$(DELPHES_PATH) -lDelphes

.PHONY: build check-env


build: check-env OLeAA.exe

OLeAA.exe: *.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LIBS) -I. -o $@ $(CFILES) 

debug: CXXFLAGS := -O0 -g3 -fno-inline $(CXXFLAGS) 
debug: build


clean:
	rm -f OLeAA.exe


check-env:
ifndef DELPHES_PATH
	$(error DELPHES_PATH is undefined)
endif
