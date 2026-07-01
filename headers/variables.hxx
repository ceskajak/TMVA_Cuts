#pragma once

#include "common.hxx"

/*
/////////////////////////////////////////////////////////////
GLOBAL VARIABLES
/////////////////////////////////////////////////////////////
*/

// Mass limits
const Double_t min_mass = 7.0;	// x-axis lower limit for fitting
const Double_t max_mass = 13.0;	// x-axis upper limit for fitting

// Variables for fitting
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

// Variables used by the TMVA models
// (must match the variables used during training)
float var_m;
short var_charge;
float var_y;
float var_nSigmaElectron1;
float var_nSigmaElectron2;
float var_dEdx1;
float var_dEdx2;
float var_eta1;
float var_eta2;
float var_phi1;
float var_phi2;
float var_invBeta1;
float var_invBeta2;
float var_E_TOW1;
float var_E_CLU1;
float var_E_TOW2;
float var_E_CLU2;
float var_p1;
float var_p2;
float var_pt1;
float var_pt2;
int var_adc1;
int var_adc2;
float var_bemcZDist1;
float var_bemcZDist2;
float var_bemcPhiDist1;
float var_bemcPhiDist2;
float var_VzTPC;
float var_VzVPD;
int var_nHitsFit1;
int var_nHitsMax1;
int var_nHitsFit2;
int var_nHitsMax2;
float var_DCA1;
float var_DCA2;

float var_min_ETOWoverECLU;
float var_max_ECLUoverP;
float var_min_ECLUoverP;
float var_max_abs_bemcZDist;
float var_max_abs_bemcPhiDist;
float var_max_abs_DCA;