#pragma once

#include "utils.hxx"
#include "CutGenerator.hxx"
#include "variables.hxx"

/*
/////////////////////////////////////////////////////////////
INPUT FILES
/////////////////////////////////////////////////////////////
*/

// File locations
TString infile_BHT1 = "../data/BHT1.masssp.root";
TString infile_BHT2 = "../data/BHT2_251128.default.cands.root";

/*
/////////////////////////////////////////////////////////////
NAME HANDLERS
/////////////////////////////////////////////////////////////
*/

TString tag = "default";	// tag for output files

TString date = init_date_string();

/*
/////////////////////////////////////////////////////////////
RUN PARAMETERS
/////////////////////////////////////////////////////////////
*/
const Double_t bin_width = 0.2;	// Bin width for drawing

const Int_t sigma = 1;	// sigma for error drawing

Bool_t useLinear = true;	// Use linear error visualisation (faster, less accurate for strong correlations)

/*
/////////////////////////////////////////////////////////////
GLOBAL VARIABLES
/////////////////////////////////////////////////////////////
*/

// Data file
TFile * f_data;

// Dataset after applying cuts
RooDataSet * rds_data;

// Global maps to store parameters for different bins/triggers
map<TString, vector<vector<Double_t>>> map_par;
map<TString, vector<vector<Double_t>>> map_frac;

vector<vector<Double_t>> par =
{
	{-0.232, -10.0, 10.0},				// LS bg exponent
	{-0.21, -10.0, 10.0},				// US bg exponent
	{9.31, 9.2, 9.4},					// sig1 mean
	{9.9, 9.8, 10.1},					// sig2 mean
	{10.27, 10.2, 10.35},				// sig3 mean
	{0.302521, 0.300231, 0.304811},		// sig1 width
	{0.323332, 0.321315, 0.325349},		// sig2 width
	{0.337254, 0.335369, 0.339139},		// sig3 width
	{1.639359, 1.610738, 2.183158},		// sig1 alpha
	{1.649815, 1.622159, 1.677471},		// sig2 alpha
	{1.653388, 1.630794, 1.675982},		// sig3 alpha
	{1.714986, 1.644381, 1.785591},		// sig1 n
	{1.678214, 1.615692, 1.740736},		// sig2 n
	{1.657812, 1.606731, 1.708893}		// sig3 n
};

vector<vector<Double_t>> frac =
{
	
	{0.74, 0.0, 1.0},		// LS bg frac
    {0.63, 0.0, 1.0},		// US bg frac
    {0.7126, 0.71, 0.715},	// sig1_frac
    {0.6828, 0.68, 0.685}	// sig2_frac
	/*
	2 sigma
	{0.7126, 0.7074, 0.7178},	// sig1_frac
	{0.6828, 0.6782, 0.6874}	// sig2_frac
	*/
};

/*
/////////////////////////////////////////////////////////////
FUNCTIONS
/////////////////////////////////////////////////////////////
*/

void UpsFitter();
/////////////////////////////////////////////////////////////
FitResultMetrics fit_bin(RooDataSet * ds_US, RooDataSet * ds_LS, TString outname, Bool_t saveResults = true)
{
	// Define variable + ranges
	RooRealVar m("m", "m", min_mass, max_mass, "GeV/c^{2}");
	m.setRange("full", min_mass, max_mass);

	// Save number of entries
	double US_entries = ds_US->sumEntries();
	double LS_entries = ds_LS->sumEntries();

	FitResultMetrics metrics;

	// Define a sample for simultaneous fitting
	RooCategory sample("sample", "sample");
	sample.defineType("US");
	sample.defineType("LS");

	// Define a combined dataset 
	RooDataSet * ds_comb = new RooDataSet("ds_comb", "Combined dataset", RooArgList(m), Index(sample), Import("US", *ds_US), Import("LS", *ds_LS));

	// Load the parameters from the array (saved in the header)
	RooRealVar bgLS("bgLS","LS", par[0][0], par[0][1], par[0][2]);
	RooRealVar bgUS("bgUS","US", par[1][0], par[1][1], par[1][2]);
	RooRealVar sig1_mu("sig1_mu","#mu", par[2][0], par[2][1], par[2][2]);
	RooRealVar sig2_mu("sig2_mu","#mu", par[3][0], par[3][1], par[3][2]);
	RooRealVar sig3_mu("sig3_mu","#mu", par[4][0], par[4][1], par[4][2]);
	RooRealVar sig1_sigma("sig1_sigma","#sigma", par[5][0], par[5][1], par[5][2]);
	RooRealVar sig2_sigma("sig2_sigma","#sigma", par[6][0], par[6][1], par[6][2]);
	RooRealVar sig3_sigma("sig3_sigma","#sigma", par[7][0], par[7][1], par[7][2]);
	RooRealVar sig1_alpha("sig1_alpha","#alpha", par[8][0], par[8][1], par[8][2]);
	RooRealVar sig2_alpha("sig2_alpha","#alpha", par[9][0], par[9][1], par[9][2]);
	RooRealVar sig3_alpha("sig3_alpha","#alpha", par[10][0], par[10][1], par[10][2]);
	RooRealVar sig1_n("sig1_n","#n", par[11][0], par[11][1], par[11][2]);
	RooRealVar sig2_n("sig2_n","#n", par[12][0], par[12][1], par[12][2]);
	RooRealVar sig3_n("sig3_n","#n", par[13][0], par[13][1], par[13][2]);

	// used for 2D CB
	/*
	RooRealVar sig1_alphaL("sig1_alphaL","#alpha_{L}", par[10][0], par[10][1], par[10][2]);
	RooRealVar sig2_alphaL("sig2_alphaL","#alpha_{L}", par[11][0], par[11][1], par[11][2]);
	RooRealVar sig3_alphaL("sig3_alphaL","#alpha_{L}", par[12][0], par[12][1], par[12][2]);
	RooRealVar sig1_alphaR("sig1_alphaR","#alpha_{R}", par[13][0], par[13][1], par[13][2]);
	RooRealVar sig2_alphaR("sig2_alphaR","#alpha_{R}", par[14][0], par[14][1], par[14][2]);
	RooRealVar sig3_alphaR("sig3_alphaR","#alpha_{R}", par[15][0], par[15][1], par[15][2]);
	RooRealVar sig1_nL("sig1_nL","#n_{L}", par[16][0], par[16][1], par[16][2]);
	RooRealVar sig2_nL("sig2_nL","#n_{L}", par[17][0], par[17][1], par[17][2]);
	RooRealVar sig3_nL("sig3_nL","#n_{L}", par[18][0], par[18][1], par[18][2]);
	RooRealVar sig1_nR("sig1_nR","#n_{R}", par[19][0], par[19][1], par[19][2]);
	RooRealVar sig2_nR("sig2_nR","#n_{R}", par[20][0], par[20][1], par[20][2]);
	RooRealVar sig3_nR("sig3_nR","#n_{R}", par[21][0], par[21][1], par[21][2]);
	*/

	// Load recursive fractions for the components
	RooRealVar bgLS_frac("bgLS_frac","bgLS_frac", frac[0][0], frac[0][1], frac[0][2]);
	RooRealVar bgUS_frac("bgUS_frac","bgUS_frac", frac[1][0], frac[1][1], frac[1][2]);
	RooRealVar sig1_frac("sig1_frac","sig1_frac", frac[2][0], frac[2][1], frac[2][2]);
	RooRealVar sig2_frac("sig2_frac","sig2_frac", frac[3][0], frac[3][1], frac[3][2]);

	// Ensure the combinatorial background describes the LS data
	bgLS_frac.setVal(ds_LS->sumEntries()/ds_US->sumEntries());
	bgLS_frac.setConstant(true);

	// Mass delta from embedding:
	RooFormulaVar sig2_mu_bound("sig2_mu", "sig2_mu", "@0 + 0.560180", RooArgList(sig1_mu));
	RooFormulaVar sig3_mu_bound("sig3_mu", "sig3_mu", "@0 + 0.890502", RooArgList(sig1_mu));

	// Create PDFs
	RooExponential *expLS = new RooExponential("expLS","LS exp background", m, bgLS);
	RooExponential *expUS = new RooExponential("expUS","US Exp background", m, bgUS);
	RooCBShape *gauss1 = new RooCBShape("gauss1", "Gaussian signal 1", m, sig1_mu, sig1_sigma, sig1_alpha, sig1_n);
	RooCBShape *gauss2 = new RooCBShape("gauss2", "Gaussian signal 2", m, sig2_mu_bound, sig2_sigma, sig2_alpha, sig2_n);
	RooCBShape *gauss3 = new RooCBShape("gauss3", "Gaussian signal 3", m, sig3_mu_bound, sig3_sigma, sig3_alpha, sig3_n);

	/*
	RooCrystalBall *gauss1 = new RooCrystalBall("gauss1", "Gaussian signal 1", m, sig1_mu, sig1_sigma, sig1_alphaL, sig1_alphaR, sig1_nL, sig1_nR);
	RooCrystalBall *gauss2 = new RooCrystalBall("gauss2", "Gaussian signal 2", m, sig2_mu, sig2_sigma, sig2_alphaL, sig2_alphaR, sig2_nL, sig2_nR);
	RooCrystalBall *gauss3 = new RooCrystalBall("gauss3", "Gaussian signal 3", m, sig3_mu, sig3_sigma, sig3_alphaL, sig3_alphaR, sig3_nL, sig3_nR);
	*/

	// Add PDFs together with recursive fractions (forces all ratios to be in [0,1])
	RooAddPdf *total_sim = new RooAddPdf("total_sim", "US total", RooArgList(*expLS, *expUS, *gauss1, *gauss2, *gauss3), RooArgList(bgLS_frac, bgUS_frac, sig1_frac, sig2_frac), kTRUE);

	// Simultaneous model
	RooSimultaneous * simPdf = new RooSimultaneous("simPdf", "simPdf", sample);

	// Add PDFs to the simultaneous model
	simPdf->addPdf(*expLS, "LS");
	simPdf->addPdf(*total_sim, "US");

	// Fit the model to the combined dataset
	RooFitResult *result = simPdf->fitTo(*ds_comb,
										Extended(true),
										//Minimizer("Minuit2", "scan"),
										Minimizer("Minuit2", "migrad"),
										MaxCalls(100000),
										Strategy(2),
										EvalErrorWall(1),
										RecoverFromUndefinedRegions(1000),
										Save());

	// Compute yields
	RooGenericPdf yield1("yield1", "yield from signal fit", "(1.0-bgLS_frac)*(1.0-bgUS_frac)", RooArgSet(bgLS_frac, bgUS_frac));
	RooGenericPdf yield2("yield2", "yield from signal 2 fit", "(1.0-bgLS_frac)*(1.0-bgUS_frac)*(1.0-sig1_frac)", RooArgSet(bgLS_frac, bgUS_frac, sig1_frac));
	RooGenericPdf yield3("yield3", "yield from signal 3 fit", "(1.0-bgLS_frac)*(1.0-bgUS_frac)*(1.0-sig1_frac)*(1.0-sig2_frac)", RooArgSet(bgLS_frac, bgUS_frac, sig1_frac, sig2_frac));
	cout << "US entries: " << US_entries << ", LS entries: " << LS_entries << endl << endl;
	cout << "yield frac: " << yield1.getVal() << ", yield: " << (yield1.getVal()*US_entries) << endl;
	cout << "error frac: " << yield1.getPropagatedError(*result) << ", error: " << (yield1.getPropagatedError(*result)*US_entries) << endl << endl;
	cout << "yield frac: " << yield1.getVal() << ", yield: " << ((yield1.getVal()-yield2.getVal()-yield3.getVal())*US_entries) << endl;
	cout << "yield2 frac: " << yield2.getVal() << ", yield2: " << ((yield2.getVal()-yield3.getVal()) * US_entries) << endl;
	cout << "yield3 frac: " << yield3.getVal() << ", yield3: " << (yield3.getVal() * US_entries) << endl;

	Double_t chi2ndf_US = -1;
	Double_t chi2ndf_LS = -1;

	if (saveResults)
	{
		// Create a canvas for plotting
		TCanvas *cnv = new TCanvas("cnv", "cnv", 1600, 1200);
		cnv->SetRightMargin(1.2);

		// Change variable ranges to accommodate variations
		bgLS.setRange(-10, 10);
		bgUS.setRange(-10, 10);
		sig1_mu.setRange(1, 100);
		sig2_mu.setRange(1, 100);
		sig3_mu.setRange(1, 100);
		sig1_sigma.setRange(0, 100);
		sig2_sigma.setRange(0, 100);
		sig3_sigma.setRange(0, 100);
		sig1_n.setRange(-10, 10);
		sig2_n.setRange(-10, 10);
		sig3_n.setRange(-10, 10);
		sig1_alpha.setRange(0, 10);
		sig2_alpha.setRange(0, 10);
		sig3_alpha.setRange(0, 10);

		bgUS_frac.setRange(0, 1);
		sig1_frac.setRange(0, 1);
		sig2_frac.setRange(0, 1);

		// Plot simulated data
		cnv->cd();
		RooPlot* frame = m.frame(((max_mass - min_mass)/bin_width));
		frame->SetTitle("");
		frame->GetYaxis()->SetLabelSize(0.04);
		frame->GetYaxis()->SetTitleSize(0.05);
		frame->GetYaxis()->SetTitleOffset(0.9);
		frame->GetXaxis()->SetLabelSize(0.04);
		frame->GetXaxis()->SetTitleSize(0.05);
		frame->GetXaxis()->SetTitleOffset(0.85);
		frame->GetXaxis()->SetTitle("m_{ee} (GeV/c^{2})");
		gPad->SetLeftMargin(0.1);
		gPad->SetRightMargin(0.02);
		gPad->SetBottomMargin(0.15);
		gPad->SetTopMargin(0.05);
		ds_LS->plotOn(frame, MarkerColor(kRed), LineColor(kRed));
		ds_US->plotOn(frame);
		total_sim->plotOn(frame, VisualizeError(*result, sigma, useLinear), DrawOption("F"), FillStyle(3001));
		total_sim->plotOn(frame);
		total_sim->plotOn(frame, Components(*expLS), LineColor(kRed), LineStyle(kDashed), VisualizeError(*result, sigma, useLinear), DrawOption("F"), FillColor(kRed), FillStyle(3001));
		total_sim->plotOn(frame, Components(*expLS), LineColor(kRed), LineStyle(kDashed));
		total_sim->plotOn(frame, Components(*expUS), LineColor(kGray), LineStyle(kDashed), VisualizeError(*result, sigma, useLinear), DrawOption("F"), FillColor(kGray), FillStyle(3001));
		total_sim->plotOn(frame, Components(*expUS), LineColor(kGray), LineStyle(kDashed));
		total_sim->plotOn(frame, Components(*gauss1), LineColor(kMagenta), LineStyle(kSolid), VisualizeError(*result, sigma, useLinear), DrawOption("F"), FillColor(kMagenta), FillStyle(3001));
		total_sim->plotOn(frame, Components(*gauss1), LineColor(kMagenta), LineStyle(kSolid));
		total_sim->plotOn(frame, Components(*gauss2), LineColor(kMagenta+2), LineStyle(kSolid), VisualizeError(*result, sigma, useLinear), DrawOption("F"), FillColor(kMagenta+2), FillStyle(3001));
		total_sim->plotOn(frame, Components(*gauss2), LineColor(kMagenta+2), LineStyle(kSolid));
		total_sim->plotOn(frame, Components(*gauss3), LineColor(kMagenta+4), LineStyle(kSolid), VisualizeError(*result, sigma, useLinear), DrawOption("F"), FillColor(kMagenta+4), FillStyle(3001));
		total_sim->plotOn(frame, Components(*gauss3), LineColor(kMagenta+4), LineStyle(kSolid));
		total_sim->plotOn(frame, Components(RooArgSet(*gauss1, *gauss2, *gauss3)), LineColor(kCyan), LineStyle(kSolid), VisualizeError(*result, sigma, useLinear), DrawOption("F"), FillColor(kTeal), FillStyle(3001));
		total_sim->plotOn(frame, Components(RooArgSet(*gauss1, *gauss2, *gauss3)), LineColor(kCyan), LineStyle(kSolid));
		total_sim->plotOn(frame, Components(RooArgSet(*expLS, *expUS)), LineColor(kOrange), LineStyle(kSolid), VisualizeError(*result, sigma, useLinear), DrawOption("F"), FillColor(kOrange), FillStyle(3001));
		total_sim->plotOn(frame, Components(RooArgSet(*expLS, *expUS)), LineColor(kOrange), LineStyle(kSolid));
		total_sim->plotOn(frame);
		frame->Draw("F");
		//--------------------------------------------

		// Calculate chi2 after extracting the number of parameters - used to calculate Ndof
		Int_t nparam_US = total_sim->getParameters(*ds_US)->selectByAttrib("Constant", kFALSE)->getSize();
		cout << "nparam_US = " << nparam_US << endl;
		chi2ndf_US = frame->chiSquare(nparam_US);

		Int_t nparam_LS = expLS->getParameters(*ds_LS)->selectByAttrib("Constant", kFALSE)->getSize();
		cout << "nparam_LS = " << nparam_LS << endl;
		chi2ndf_LS = frame->chiSquare(nparam_LS);

		/*
		// pulls
		TCanvas *cnv_pull = new TCanvas("cnv_pull", "cnv_pull");
		cnv_pull->cd();

		RooHist* hpull;
		hpull = frame->pullHist("ds_US", "total_Norm[mass]");

		hpull->Draw();
		*/

		gStyle->SetOptStat(0) ;
	    TH2* hcorr = result->correlationHist();

		result->correlationMatrix().Print();
		result->covarianceMatrix().Print();

		//RooGenericPdf yield1("yield1", "yield from signal fit", "(1.0-bgLS_frac)*(1.0-bgUS_frac)", RooArgSet(bgLS_frac, bgUS_frac));

		// Save results in a text file
		std::ofstream out;
		out.open("output/results/" + outname + ".txt");
		out << "US entries: " << US_entries << ", LS entries: " << LS_entries << endl;
		out << "#chi^{2}/N^{US}_{dof} = " << chi2ndf_US << ", #chi^{2}/N^{LS}_{dof} = " << chi2ndf_LS << endl;
		out << "frac: " << yield1.getVal() << ", yield: " << (yield1.getVal()*US_entries) << endl;
		out << "error frac: " << yield1.getPropagatedError(*result) << ", error: " << (yield1.getPropagatedError(*result)*US_entries) << endl;
		out << "yield frac: " << yield1.getVal() << ", yield: " << ((yield1.getVal()-yield2.getVal()-yield3.getVal())*US_entries) << endl;
		out << "yield2 frac: " << yield2.getVal() << ", yield2: " << ((yield2.getVal()-yield3.getVal()) * US_entries) << endl;
		out << "yield3 frac: " << yield3.getVal() << ", yield3: " << (yield3.getVal() * US_entries) << endl;
		out << "bgLS: " << bgLS.getVal() << " #pm " << bgLS.getError() << endl;
		out << "bgUS: " << bgUS.getVal() << " #pm " << bgUS.getError() << endl;
		out << "sig1 mu: " << sig1_mu.getVal() << " #pm " << sig1_mu.getError() << endl;
		out << "sig2 mu: " << sig2_mu.getVal() << " #pm " << sig2_mu.getError() << endl;
		out << "sig3 mu: " << sig3_mu.getVal() << " #pm " << sig3_mu.getError() << endl;
		out << "sig1 sigma: " << sig1_sigma.getVal() << " #pm " << sig1_sigma.getError() << endl;
		out << "sig2 sigma: " << sig2_sigma.getVal() << " #pm " << sig2_sigma.getError() << endl;
		out << "sig3 sigma: " << sig3_sigma.getVal() << " #pm " << sig3_sigma.getError() << endl;
		out << "sig1 alpha: " << sig1_alpha.getVal() << " #pm " << sig1_alpha.getError() << endl;
		out << "sig2 alpha: " << sig2_alpha.getVal() << " #pm " << sig2_alpha.getError() << endl;
		out << "sig3 alpha: " << sig3_alpha.getVal() << " #pm " << sig3_alpha.getError() << endl;
		out << "sig1 n: " << sig1_n.getVal() << " #pm " << sig1_n.getError() << endl;
		out << "sig2 n: " << sig2_n.getVal() << " #pm " << sig2_n.getError() << endl;
		out << "sig3 n: " << sig3_n.getVal() << " #pm " << sig3_n.getError() << endl;
		out << "bgLS_frac: " << bgLS_frac.getVal() << " #pm " << bgLS_frac.getError() << endl;
		out << "bgUS_frac: " << bgUS_frac.getVal() << " #pm " << bgUS_frac.getError() << endl;
		out << "sig1_frac: " << sig1_frac.getVal() << " #pm " << sig1_frac.getError() << endl;
		out << "sig2_frac: " << sig2_frac.getVal() << " #pm " << sig2_frac.getError() << endl;
		out << "S = " << (yield1.getVal()*US_entries) << ", B = " << (1 - yield1.getVal())*US_entries  << endl;
		out << "S/sqrt(S+B) = " << (yield1.getVal()*US_entries)/ sqrt(US_entries) << endl;
		out.close();

		TLatex *latex1 = new TLatex();
		latex1->SetTextSize(0.045);
		latex1->SetNDC();
		latex1->DrawLatex(0.55, 0.85, Form("#frac{#chi^{2}}{N^{US}_{dof}} = %.2f, #frac{#chi^{2}}{N^{LS}_{dof}} = %.2f", chi2ndf_US, chi2ndf_LS));
		latex1->DrawLatex(0.55, 0.75, Form("#Upsilon(1S+2S+3S) = %.0f #pm %.0f", (yield1.getVal()*US_entries), (yield1.getPropagatedError(*result)*US_entries)));
		latex1->DrawLatex(0.55, 0.65, Form("S/#sqrt{S+B} = %.2f", (yield1.getVal()*US_entries)/ sqrt(US_entries)));

		cout << "S = " << (yield1.getVal()*US_entries) << ", B = " << (1 - yield1.getVal())*US_entries  << endl;
		cout << "S/sqrt(S+B) = " << (yield1.getVal()*US_entries)/ sqrt(US_entries) << endl;

		/*
		TLatex *latex3 = new TLatex();
		latex3->SetTextSize(0.05);
		latex3->SetNDC();
		latex3->DrawLatex(0.55, 0.6, "1 < #LT TofMult #GT < 2");
		*/

		// TODO rewrite as with pT bins!!!
		cnv->cd()->SaveAs("output/fig/" + outname + ".png");
		//cnv->cd()->SaveAs("output/rootfiles/" + outname + ".root");
		
		gStyle->SetPaintTextFormat("4.2f");
		hcorr->Draw("colz text");
		gPad->SetRightMargin(0.15);
		//cnv->cd()->SaveAs("output/corr/" + outname + "_corr.png");

		// Clean up
		delete latex1;
		delete cnv;
		delete frame;
	}
	else
	{
		cout << "Fit results: S = " << (yield1.getVal()*US_entries) << ", B = " << (1 - yield1.getVal())*US_entries << ", S/sqrt(S+B) = " << ((yield1.getVal()*US_entries) / sqrt(US_entries)) << endl;
	}

	metrics.S = yield1.getVal()*US_entries;
	metrics.B = (1.0 - yield1.getVal())*US_entries;
	metrics.yield = metrics.S;
	metrics.yieldErr = yield1.getPropagatedError(*result)*US_entries;
	metrics.S_over_sqrtSB = metrics.S / sqrt(metrics.S + metrics.B);
	metrics.US_entries = US_entries;
	metrics.LS_entries = LS_entries;
	metrics.chi2ndf_US = chi2ndf_US;
	metrics.chi2ndf_LS = chi2ndf_LS;

	delete result;
	return metrics;
}
/////////////////////////////////////////////////////////////

inline static RooDataSet * GetRooDataSet()
{
    TFile * f_tmp = TFile::Open("data/cands+base.root", "READ");
	TTree * t_tmp = nullptr;
    RooDataSet * rds_tmp = nullptr;

    if (!f_tmp || f_tmp->IsZombie())
        t_tmp = GetBaseTree();
	else
		t_tmp = (TTree *)f_tmp->Get("tree");

	if (!t_tmp)
	{
		cerr << "ERROR: Could not find tree in file!" << endl;
		return nullptr;
	}

	/*
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
	*/

	rds_tmp = new RooDataSet("rds_cands+base", "Dataset with cuts",
		RooArgSet(charge, m, y, nSigmaElectron1, nSigmaElectron2,
		dEdx1, dEdx2, eta1, eta2, phi1, phi2, invBeta1, invBeta2,
		E_TOW1, E_CLU1, E_TOW2, E_CLU2, p1, p2, pt1, pt2, adc1, adc2,
		bemcZDist1, bemcZDist2, bemcPhiDist1, bemcPhiDist2, VzTPC, VzVPD,
		nHitsFit1, nHitsMax1, nHitsFit2, nHitsMax2, DCA1, DCA2),
		Import(*t_tmp));
	rds_tmp = (RooDataSet *)rds_tmp->reduce(RooArgSet(m, charge, bemcZDist1, bemcZDist2, bemcPhiDist1, bemcPhiDist2));


    f_tmp->Close();
    return rds_tmp;
}