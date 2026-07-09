CXX ?= g++

ROOT_CFLAGS := $(shell root-config --cflags)
ROOT_LIBS := $(shell root-config --libs)

CXXFLAGS ?= -O2 -std=c++17
CXXFLAGS += $(ROOT_CFLAGS)
LDFLAGS += $(ROOT_LIBS) -lTMVA -lRooFit -lRooFitCore

BIN_DIR := bin

COMMON_SOURCES := \
	src/variables.cxx \
	src/utils.cxx \
	src/cuts.cxx \
	src/TMVACommon.cxx

all: $(BIN_DIR)/ups_fitter $(BIN_DIR)/tmva_train $(BIN_DIR)/tmva_apply

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/ups_fitter: src/UpsFitter.cxx $(COMMON_SOURCES) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/tmva_train: src/TMVATrain.cxx src/TMVATrainMain.cxx $(COMMON_SOURCES) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/tmva_apply: src/TMVAApply.cxx src/TMVAApplyMain.cxx $(COMMON_SOURCES) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean
