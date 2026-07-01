#pragma once

#include "TMVATrain.hxx"

/*
/////////////////////////////////////////////////////////////
GLOBAL VARIABLES
/////////////////////////////////////////////////////////////
*/
// Path to TMVA weights
TString weightPath = "dataset/weights/";

// Switches for variables
bool k2v = 0;
bool k5v = 0;
bool k6v = 0;
bool k16v = 0;

// TMVA Methods (on/off, at least one)
bool kApplyCuts = 1;
bool kApplyBDTG = 1;
bool kApplyHMatrix = 1;
bool kApplyFisher = 1;
bool kApplyMLP = 1;
bool kApplySVM = 1;

TString strCutsWeights;
TString strBDTGWeights;
TString strHMatrixWeights;
TString strFisherWeights;
TString strMLPWeights;
TString strSVMWeights;

// Input file and tree for TMVA application
TFile * fin;
TTree * tree;

// Create TMVA reader
TMVA::Reader reader("!Color:!Silent");

// Variables for TMVA output values
vector<float> TMVACutsScore;
float TMVABDTGScore;
float TMVAHMatrixScore;
float TMVAFisherScore;
float TMVAMLPScore;
float TMVASVMScore;

/*
/////////////////////////////////////////////////////////////
FUNCTIONS
/////////////////////////////////////////////////////////////
*/

void TMVAApply(TString strVarset); // Default variable set is "m16v" if not specified

inline void AdjustVariableFlags()
{
    if (weightPath.Contains("c2v") || weightPath.Contains("m2v"))
    {
        k2v = 1;
        k5v = 0;
        k6v = 0;
        k16v = 0;
    }
    else if (weightPath.Contains("c5v") || weightPath.Contains("m5v"))
    {
        k2v = 1;
        k5v = 1;
        k6v = 0;
        k16v = 0;
    }
    else if (weightPath.Contains("c6v") || weightPath.Contains("m6v"))
    {
        k2v = 1;
        k5v = 1;
        k6v = 1;
        k16v = 0;
    }
    else if (weightPath.Contains("c16v") || weightPath.Contains("m16v"))
    {
        k2v = 1;
        k5v = 1;
        k6v = 1;
        k16v = 1;
    }
}

inline void SetWeightPaths()
{
    strCutsWeights = weightPath + "/TMVAOptimizeCuts_Cuts.weights.xml";
    strBDTGWeights = weightPath + "/TMVAOptimizeCuts_BDTG.weights.xml";
    strHMatrixWeights = weightPath + "/TMVAOptimizeCuts_HMatrix.weights.xml";
    strFisherWeights = weightPath + "/TMVAOptimizeCuts_Fisher.weights.xml";
    strMLPWeights = weightPath + "/TMVAOptimizeCuts_MLP.weights.xml";
    strSVMWeights = weightPath + "/TMVAOptimizeCuts_SVM.weights.xml";
}

// Load trained network
inline void BookIfValid(const TString &methodName, const TString &weightFile, bool &kApplyMethod)
{
    if (kApplyMethod && CheckFileExists(weightFile))
    {
        reader.BookMVA(methodName, weightFile);
    }
    else
    {
        cerr << "WARNING: Weight file for method " << methodName << " not found: " << endl;
        kApplyMethod = false; // Disable the method if the weight file is missing
    }
};