#pragma once

#include "utils.hxx"
#include "CutGenerator.hxx"
#include "variables.hxx"

/*
/////////////////////////////////////////////////////////////
INPUT FILES
/////////////////////////////////////////////////////////////
*/

extern TString infile_BHT1;
extern TString infile_BHT2;

// Optional override: when non-empty, GetRooDataSet() reads this file/tree
// (e.g. a TMVAApply "tree_TMVA-processed.root") and applies fitCutExpr instead
// of the default "data/cands+base.root" input.
extern TString fitInputFile;
extern TString fitCutExpr;

/*
/////////////////////////////////////////////////////////////
NAME HANDLERS
/////////////////////////////////////////////////////////////
*/

extern TString tag;
extern TString date;

/*
/////////////////////////////////////////////////////////////
RUN PARAMETERS
/////////////////////////////////////////////////////////////
*/

extern const Double_t bin_width;
extern const Int_t sigma;
extern Bool_t useLinear;

/*
/////////////////////////////////////////////////////////////
GLOBAL VARIABLES
/////////////////////////////////////////////////////////////
*/

extern TFile *f_data;
extern RooDataSet *rds_data;

extern std::map<TString, std::vector<std::vector<Double_t>>> map_par;
extern std::map<TString, std::vector<std::vector<Double_t>>> map_frac;

extern std::vector<std::vector<Double_t>> par;
extern std::vector<std::vector<Double_t>> frac;

/*
/////////////////////////////////////////////////////////////
FUNCTIONS
/////////////////////////////////////////////////////////////
*/

bool UpsFitter();
FitResultMetrics fit_bin(RooDataSet *ds_US, RooDataSet *ds_LS, TString outname, Bool_t saveResults = true);
RooDataSet *GetRooDataSet(const TString &infile = "", const TString &cutExpr = "");
