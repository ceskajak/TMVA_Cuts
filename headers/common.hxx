#pragma once

// Common C++, ROOT, RooFit
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "TSystem.h"
#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TROOT.h"
#include "TLatex.h"

#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooPlot.h"
#include "RooAddPdf.h"
#include "RooGaussian.h"
#include "RooExponential.h"
#include "RooHist.h"
#include "RooArgSet.h"
#include "RooFitResult.h"
#include "RooGenericPdf.h"
#include "RooCategory.h"
#include "RooFormulaVar.h"
#include "RooCBShape.h"
#include "RooSimultaneous.h"
#include "RooDataSet.h"
#include "RooFit.h"

#include "TMVA/Tools.h"
#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/MethodCuts.h"
#include "TMVA/Reader.h"

#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif  // __CINT__

// Namespaces
using namespace std;
using namespace RooFit;