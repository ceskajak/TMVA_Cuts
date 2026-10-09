#pragma once

#include "variables.hxx"
#include "CutGenerator.hxx"

/*
/////////////////////////////////////////////////////////////
DATA STRUCTURES
/////////////////////////////////////////////////////////////
*/
struct FitResultMetrics {
    Double_t S;
    Double_t B;
    Double_t S_over_sqrtSB;
    Double_t yield;
    Double_t yieldErr;
    Double_t US_entries;
    Double_t LS_entries;
    Double_t chi2ndf_US;
    Double_t chi2ndf_LS;

    // Alternative background model (2nd-degree polynomial instead of exponential),
    // fit and drawn alongside the exponential-background result above.
    Double_t yield_poly;
    Double_t yieldErr_poly;
    Double_t chi2ndf_US_poly;
    Double_t chi2ndf_LS_poly;
};

// Structure to hold cut variables, values and signal efficiency
struct CutCandidate
{
    std::vector<TString> varNames;
    std::vector<double> cutMin;
    std::vector<double> cutMax;
    double effS;
};

/*
/////////////////////////////////////////////////////////////
FUNCTIONS
/////////////////////////////////////////////////////////////
*/
// Current date (YYMMDD) for output purposes
TString init_date_string();

/////////////////////////////////////////////////////////////

TTree *GetBaseTree();

/////////////////////////////////////////////////////////////

// Function to build a cut expression from a CutCandidate
TString BuildCutExpression(const CutCandidate &cand);

/////////////////////////////////////////////////////////////

bool CheckFileExists(const TString &filename);

// Ensure relative paths are resolved from repository root even when executable is launched from bin/.
bool SetProjectRootAsCwd();

// Suppress known non-actionable ROOT warning noise while preserving all other diagnostics.
void InstallRootWarningFilter();

/////////////////////////////////////////////////////////////