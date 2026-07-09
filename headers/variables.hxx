#pragma once

#include "common.hxx"

/*
/////////////////////////////////////////////////////////////
GLOBAL VARIABLES
/////////////////////////////////////////////////////////////
*/

// Mass limits
extern const Double_t min_mass;
extern const Double_t max_mass;

// Variables for fitting
extern RooRealVar m;
extern RooRealVar charge;
extern RooRealVar y;
extern RooRealVar nSigmaElectron1;
extern RooRealVar nSigmaElectron2;
extern RooRealVar dEdx1;
extern RooRealVar dEdx2;
extern RooRealVar eta1;
extern RooRealVar eta2;
extern RooRealVar phi1;
extern RooRealVar phi2;
extern RooRealVar invBeta1;
extern RooRealVar invBeta2;
extern RooRealVar E_TOW1;
extern RooRealVar E_CLU1;
extern RooRealVar E_TOW2;
extern RooRealVar E_CLU2;
extern RooRealVar p1;
extern RooRealVar p2;
extern RooRealVar pt1;
extern RooRealVar pt2;
extern RooRealVar adc1;
extern RooRealVar adc2;
extern RooRealVar bemcZDist1;
extern RooRealVar bemcZDist2;
extern RooRealVar bemcPhiDist1;
extern RooRealVar bemcPhiDist2;
extern RooRealVar VzTPC;
extern RooRealVar VzVPD;
extern RooRealVar nHitsFit1;
extern RooRealVar nHitsMax1;
extern RooRealVar nHitsFit2;
extern RooRealVar nHitsMax2;
extern RooRealVar DCA1;
extern RooRealVar DCA2;

// Variables used by the TMVA models
extern float var_m;
extern short var_charge;
extern float var_y;
extern float var_nSigmaElectron1;
extern float var_nSigmaElectron2;
extern float var_dEdx1;
extern float var_dEdx2;
extern float var_eta1;
extern float var_eta2;
extern float var_phi1;
extern float var_phi2;
extern float var_invBeta1;
extern float var_invBeta2;
extern float var_E_TOW1;
extern float var_E_CLU1;
extern float var_E_TOW2;
extern float var_E_CLU2;
extern float var_p1;
extern float var_p2;
extern float var_pt1;
extern float var_pt2;
extern int var_adc1;
extern int var_adc2;
extern float var_bemcZDist1;
extern float var_bemcZDist2;
extern float var_bemcPhiDist1;
extern float var_bemcPhiDist2;
extern float var_VzTPC;
extern float var_VzVPD;
extern int var_nHitsFit1;
extern int var_nHitsMax1;
extern int var_nHitsFit2;
extern int var_nHitsMax2;
extern float var_DCA1;
extern float var_DCA2;

extern float var_min_ETOWoverECLU;
extern float var_max_ECLUoverP;
extern float var_min_ECLUoverP;
extern float var_max_abs_bemcZDist;
extern float var_max_abs_bemcPhiDist;
extern float var_max_abs_DCA;
