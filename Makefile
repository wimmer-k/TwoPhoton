.EXPORT_ALL_VARIABLES:

.PHONY: clean all docs

BIN_DIR = $(HOME)/bin
LIB_DIR = $(HOME)/lib
COMMON_DIR = $(HOME)/programs/common/

ROOTCFLAGS  := $(shell root-config --cflags)
ROOTLIBS    := $(shell root-config --libs)
ROOTGLIBS   := $(shell root-config --glibs)
ROOTINC     := -I$(shell root-config --incdir)

CPP         = g++
CFLAGS	    = -Wall -Wno-long-long -g -O3 $(ROOTCFLAGS) -fPIC -D_FILE_OFFSET_BITS=64 -MMD

INCLUDES    = -I./inc -I$(COMMON_DIR)
BASELIBS    = -lm $(ROOTLIBS) $(ROOTGLIBS) -L$(LIB_DIR) 
LIBS  	    =  $(BASELIBS) -lCommandLineInterface -lTwoPhoton

LFLAGS	    = -g -fPIC -shared
CFLAGS 	    += -Wl,--no-as-needed
LFLAGS 	    += -Wl,--no-as-needed 
CFLAGS 	    += -Wno-unused-variable -Wno-unused-but-set-variable -Wno-write-strings
LIB_O_FILES = build/Germanium.o build/GermaniumDictionary.o


USING_ROOT_6 = $(shell expr $(shell root-config --version | cut -f1 -d.) \>= 6)
ifeq ($(USING_ROOT_6),1)
	EXTRAS =  GermaniumDictionary_rdict.pcm 
endif

all: $(LIB_DIR)/libTwoPhoton.so $(EXTRAS) BuildEvents ViewTrace


BuildEvents: BuildEvents.cc $(LIB_DIR)/libTwoPhoton.so
	@echo "Compiling $@"
	@$(CPP) $(CFLAGS) $(INCLUDES) $< $(LIBS) $(O_FILES) -o $(BIN_DIR)/$@ 

ViewTrace: ViewTrace.cc $(LIB_DIR)/libTwoPhoton.so
	@echo "Compiling $@"
	@$(CPP) $(CFLAGS) $(INCLUDES) $< $(LIBS) $(O_FILES) -o $(BIN_DIR)/$@ 

$(LIB_DIR)/libTwoPhoton.so: $(LIB_O_FILES)
	@echo "Making $@"
	@$(CPP) $(LFLAGS) -o $@ $^ -lc

build/%.o: src/%.cc inc/%.hh
	@echo "Compiling $@"
	@mkdir -p $(dir $@)
	@$(CPP) $(CFLAGS) $(INCLUDES) -c $< -o $@ 

build/%Dictionary.o: build/%Dictionary.cc
	@echo "Compiling $@"
	@mkdir -p $(dir $@)
	@$(CPP) $(CFLAGS) $(INCLUDES) -fPIC -c $< -o $@

build/%Dictionary.cc: inc/%.hh inc/%LinkDef.h
	@echo "Building $@"
	@mkdir -p $(dir $@)
	@rootcint -f $@ -c $(INCLUDES) $(SWITCH) $(notdir $^)

build/%Dictionary_rdict.pcm: build/%Dictionary.cc
	@echo "Confirming $@"
	@touch $@

%Dictionary_rdict.pcm: build/%Dictionary_rdict.pcm
	@echo "Placing $@"
	@cp build/$@ $(LIB_DIR)

clean:
	@echo "Cleaning up"
	@rm -rf build doc
	@rm -f inc/*~ src/*~ *~
	@rm -f scripts/*~  scripts/*_C.*
