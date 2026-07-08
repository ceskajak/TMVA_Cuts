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
static TFile * f_in = nullptr;
static TFile * f_out = nullptr;

static TTree * t_main = nullptr;
static TTree * t_out = nullptr;

static TTree * t_sg = nullptr;
static TTree * t_bg = nullptr;

// Signal and background selection for TMVA
static TString str_sgCuts;
static TString str_bgCuts;

// Cuts for mass S/B discrimination
static const double m_sig_lo = 8.8;
static const double m_sig_hi = 11.1;

// S/B discrimination (on/off, only one can be active at once!)
static bool kMass = 1;
static bool kCharge = 0;

// TMVA Methods (on/off, at least one)
static bool kCuts = 1;
static bool kBDTG = 1;
static bool kHMatrix = 1;
static bool kFisher = 1;
static bool kMLP = 1;
static bool kSVM = 1;

static vector<TString> vec_TMVAMethods =
{
    "Cuts",
    "BDTG",
    "HMatrix",
    "Fisher",
    "MLP",
    "SVM"
};

static TMVA::Factory * factory = nullptr;
static TMVA::DataLoader * dataloader = nullptr;
static TMVA::Reader * reader = nullptr;

// Path to TMVA weights
static TString weightPath = "dataset/weights/";

// Switches for variables
static bool k2v = 0;
static bool k5v = 0;
static bool k6v = 0;
static bool k16v = 0;

static TString strCutsWeights;
static TString strBDTGWeights;
static TString strHMatrixWeights;
static TString strFisherWeights;
static TString strMLPWeights;
static TString strSVMWeights;

// Variables for TMVA output values
static vector<float> TMVACutsScore;
static float TMVABDTGScore = 0.0f;
static float TMVAHMatrixScore = 0.0f;
static float TMVAFisherScore = 0.0f;
static float TMVAMLPScore = 0.0f;
static float TMVASVMScore = 0.0f;

static vector<TString> vec_inputVariables =
{
    //"dEdx1",
    //"dEdx2",
    //"eta1",
    //"eta2",
    //"phi1",
    //"phi2",
    //"E_TOW1",
    //"E_CLU1",
    //"E_TOW2",
    //"E_CLU2",
    //"min(E_TOW1/E_CLU1, E_TOW2/E_CLU2)",
    //"max(E_CLU1/p1, E_CLU2/p2)",
    //"min(E_CLU1/p1, E_CLU2/p2)",
    "max(abs(bemcZDist1), abs(bemcZDist2))",
    "max(abs(bemcPhiDist1), abs(bemcPhiDist2))",
    "max(abs(DCA1), abs(DCA2))"
};

/*
/////////////////////////////////////////////////////////////
SHARED TMVA DECLARATIONS
/////////////////////////////////////////////////////////////
*/

void TMVAApply(TString strVarset);
void UpdateReaderMethodFlags();
void AdjustVariableFlags();
void ConnectBranches(TTree *t_main);
void RegisterReaderVariables();
bool BookReader();

void TMVATrain();
void TMVACleaner();
bool GetSBCriteria();
bool BookFactory();

inline void TMVACleaner()
{
    if (reader)
    {
        delete reader;
        reader = nullptr;
    }

    if (factory)
    {
        delete factory;
        factory = nullptr;
    }

    if (dataloader)
    {
        delete dataloader;
        dataloader = nullptr;
    }

    if (t_sg)
    {
        delete t_sg;
        t_sg = nullptr;
    }
    if (t_bg)
    {
        delete t_bg;
        t_bg = nullptr;
    }
    if (t_main)
    {
        delete t_main;
        t_main = nullptr;
    }
    if (t_out)
    {
        delete t_out;
        t_out = nullptr;
    }

    if (f_in)
    {
        f_in->Close();
        delete f_in;
        f_in = nullptr;
    }
    if (f_out)
    {
        f_out->Close();
        delete f_out;
        f_out = nullptr;
    }
}
