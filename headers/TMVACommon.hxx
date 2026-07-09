#pragma once

#include "common.hxx"
#include "variables.hxx"
#include "utils.hxx"

/*
/////////////////////////////////////////////////////////////
SHARED TMVA STATE
/////////////////////////////////////////////////////////////
*/

// File and TTree
extern TFile *f_in;
extern TFile *f_out;

extern TTree *t_main;
extern TTree *t_out;

extern TTree *t_sg;
extern TTree *t_bg;

// Signal and background selection for TMVA
extern TString str_sgCuts;
extern TString str_bgCuts;

// Cuts for mass S/B discrimination
extern const double m_sig_lo;
extern const double m_sig_hi;

// S/B discrimination (on/off, only one can be active at once)
extern bool kMass;
extern bool kCharge;

// TMVA methods
extern std::vector<TString> vec_TMVAMethods;
extern std::set<TString> activeMethodNames;

extern TMVA::Factory *factory;
extern TMVA::DataLoader *dataloader;
extern TMVA::Reader *reader;

// Path to TMVA weights
extern TString weightPath;

// Switches for variable sets
extern bool k2v;
extern bool k5v;
extern bool k6v;
extern bool k16v;

// Variables for TMVA output values
extern std::vector<float> TMVACutsScore;
extern float TMVABDTGScore;
extern float TMVAHMatrixScore;
extern float TMVAFisherScore;
extern float TMVAMLPScore;
extern float TMVASVMScore;

extern std::vector<TString> vec_inputVariables;

/*
/////////////////////////////////////////////////////////////
SHARED TMVA DECLARATIONS
/////////////////////////////////////////////////////////////
*/

bool TMVAApply(TString strVarset = "m2v");
void UpdateReaderMethodFlags();
void AdjustVariableFlags();
void ConnectBranches(TTree *t_main);
void RegisterReaderVariables();
bool BookReader();

bool TMVATrain();
void TMVACleaner();
bool GetSBCriteria();
bool BookFactory();
