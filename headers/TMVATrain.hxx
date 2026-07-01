#pragma once

#include "UpsFitter.hxx"

/*
/////////////////////////////////////////////////////////////
GLOBAL VARIABLES
/////////////////////////////////////////////////////////////
*/

// Cuts for mass S/B discrimination
const double m_sig_lo = 8.8;
const double m_sig_hi = 11.1;

// S/B discrimination (on/off, only one can be active at once!)
bool useMassCuts = 1;
bool useChargeCuts = 0;

// TMVA Methods (on/off, at least one)
bool kTrainCuts = 1;
bool kTrainBDTG = 1;
bool kTrainHMatrix = 1;
bool kTrainFisher = 1;
bool kTrainMLP = 1;
bool kTrainSVM = 1;

vector<TString> TMVAInputVariables =
{
    "dEdx1",
    "dEdx2",
    "eta1",
    "eta2",
    "phi1",
    "phi2",
    //"invBeta1",
    //"invBeta2",
    "E_TOW1",
    "E_CLU1",
    "E_TOW2",
    "E_CLU2",
    "min(E_TOW1/E_CLU1, E_TOW2/E_CLU2)",
    "max(E_CLU1/p1, E_CLU2/p2)",
    "min(E_CLU1/p1, E_CLU2/p2)",
    "max(abs(bemcZDist1), abs(bemcZDist2))",
    "max(abs(bemcPhiDist1), abs(bemcPhiDist2))",
    "max(abs(DCA1), abs(DCA2))"
};

/*
/////////////////////////////////////////////////////////////
FUNCTIONS
/////////////////////////////////////////////////////////////
*/

void TMVATrain();

// Function to get the list of input variables for TMVA
inline vector<TString> GetTMVAInputVariables()
{
    return TMVAInputVariables;
};

// Function to evaluate a cut candidate by applying it to the dataset, performing fits, and extracting metrics
inline static FitResultMetrics EvaluateCandidate(TTree *tree, const TString &candidateCuts, const TString &label)
{
    // Apply candidate cuts to create US and LS datasets
    TString usCuts = candidateCuts + " && charge==0";
    TString lsCuts = candidateCuts + " && charge!=0";

    // Define RooRealVars for all relevant variables, including those needed for cuts and fitting
    RooRealVar m("m", "m", min_mass, max_mass, "GeV/c^{2}");
    RooRealVar charge("charge", "charge", -3, 3, "e");
    RooRealVar y("y", "rapidity", -2, 2);
    RooRealVar nSigmaElectron1("nSigmaElectron1", "nSigmaElectron1", -5, 5);
    RooRealVar nSigmaElectron2("nSigmaElectron2", "nSigmaElectron2", -5, 5);
    RooRealVar dEdx1("dEdx1", "dEdx1", 0, 1000);
    RooRealVar dEdx2("dEdx2", "dEdx2", 0, 1000);
    RooRealVar eta1("eta1", "eta1", -2, 2);
    RooRealVar eta2("eta2", "eta2", -2, 2);
    RooRealVar phi1("phi1", "phi1", -3.14, 3.14);
    RooRealVar phi2("phi2", "phi2", -3.14, 3.14);
    RooRealVar invBeta1("invBeta1", "invBeta1", -1, 3);
    RooRealVar invBeta2("invBeta2", "invBeta2", -1, 3);
    RooRealVar E_TOW1("E_TOW1", "E_TOW1", 0, 100, "GeV");
    RooRealVar E_CLU1("E_CLU1", "E_CLU1", 0, 100, "GeV");
    RooRealVar E_TOW2("E_TOW2", "E_TOW2", 0, 100, "GeV");
    RooRealVar E_CLU2("E_CLU2", "E_CLU2", 0, 100, "GeV");
    RooRealVar p1("p1", "p1", 0, 100, "GeV/c");
    RooRealVar p2("p2", "p2", 0, 100, "GeV/c");
    RooRealVar pt1("pt1", "pt1", 0, 100, "GeV/c");
    RooRealVar pt2("pt2", "pt2", 0, 100, "GeV/c");
    RooRealVar adc1("adc1", "adc1", 0, 1024);
    RooRealVar adc2("adc2", "adc2", 0, 1024);
    RooRealVar bemcZDist1("bemcZDist1", "bemcZDist1", -50, 50, "cm");
    RooRealVar bemcZDist2("bemcZDist2", "bemcZDist2", -50, 50, "cm");
    RooRealVar bemcPhiDist1("bemcPhiDist1", "bemcPhiDist1", -1, 1, "rad");
    RooRealVar bemcPhiDist2("bemcPhiDist2", "bemcPhiDist2", -1, 1, "rad");
    RooRealVar VzTPC("VzTPC", "VzTPC", -1000, 1000, "cm");
    RooRealVar VzVPD("VzVPD", "VzVPD", -1000, 1000, "cm");
    RooRealVar nHitsFit1("nHitsFit1", "nHitsFit1", 0, 100);
    RooRealVar nHitsMax1("nHitsMax1", "nHitsMax1", 0, 100);
    RooRealVar nHitsFit2("nHitsFit2", "nHitsFit2", 0, 100);
    RooRealVar nHitsMax2("nHitsMax2", "nHitsMax2", 0, 100);
    RooRealVar DCA1("DCA1", "DCA1", -10, 10, "cm");
    RooRealVar DCA2("DCA2", "DCA2", -10, 10, "cm");

    // Combine variables into a RooArgSet for dataset creation
    RooArgSet variables(
        charge, m, y, nSigmaElectron1, nSigmaElectron2,
        dEdx1, dEdx2, eta1, eta2, phi1, phi2,
        invBeta1, invBeta2,
        E_TOW1, E_CLU1, E_TOW2, E_CLU2, p1, p2, pt1, pt2,
        adc1, adc2, bemcZDist1, bemcZDist2, bemcPhiDist1, bemcPhiDist2,
        VzTPC, VzVPD, nHitsFit1, nHitsMax1, nHitsFit2, nHitsMax2,
        DCA1, DCA2);

    // Create separate TTrees for US and LS to avoid reading issues from the
    // original tree while TMVA/ROOT operates on it. CopyTree produces trees
    // we can keep in memory safely (we set directory 0 below).
    TTree *usTree = tree->CopyTree(usCuts);
    TTree *lsTree = tree->CopyTree(lsCuts);

    // Check if trees were created successfully
    if (!usTree || !lsTree)
    {
        if (usTree)
            delete usTree;
        if (lsTree)
            delete lsTree;
        FitResultMetrics empty = {0, 0, 0, 0, 0, 0, 0, 0, 0};
        return empty;
    }

    // Set directory to 0 to keep trees in memory and avoid issues with file closing
    usTree->SetDirectory(0);
    lsTree->SetDirectory(0);

    // Create RooDataSets from the TTrees with the respective cuts
    RooDataSet *ds_US = new RooDataSet("ds_US", "Us dataset", variables, Import(*usTree));
    RooDataSet *ds_LS = new RooDataSet("ds_LS", "Ls dataset", variables, Import(*lsTree));
    ds_US->SetName((label + "_US").Data());
    ds_LS->SetName((label + "_LS").Data());

    // The RooDataSet Import copies the entries, so we can delete the temporary trees
    delete usTree;
    delete lsTree;

    // Call the fitting function; use unique output names for plots/results
    FitResultMetrics metrics = fit_bin(ds_US, ds_LS, label, true);

    // Clean up datasets
    delete ds_US;
    delete ds_LS;
    return metrics;
};