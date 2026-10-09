#include "../headers/UpsFitter.hxx"

using namespace RooFit;

TString infile_BHT1 = "../data/BHT1.masssp.root";
TString infile_BHT2 = "../data/BHT2_251128.default.cands.root";

TString fitInputFile = "";
TString fitCutExpr = "";

TString tag = "default";
TString date = init_date_string();

const Double_t bin_width = 0.2;
const Int_t sigma = 1;
Bool_t useLinear = true;

TFile *f_data = nullptr;
RooDataSet *rds_data = nullptr;

std::map<TString, std::vector<std::vector<Double_t>>> map_par;
std::map<TString, std::vector<std::vector<Double_t>>> map_frac;

std::vector<std::vector<Double_t>> par = {
    {-0.232, -10.0, 10.0},
    {-0.21, -10.0, 10.0},
    {9.31, 9.2, 9.4},
    {9.9, 9.8, 10.1},
    {10.27, 10.2, 10.35},
    {0.302521, 0.300231, 0.304811},
    {0.323332, 0.321315, 0.325349},
    {0.337254, 0.335369, 0.339139},
    {1.639359, 1.610738, 2.183158},
    {1.649815, 1.622159, 1.677471},
    {1.653388, 1.630794, 1.675982},
    {1.714986, 1.644381, 1.785591},
    {1.678214, 1.615692, 1.740736},
    {1.657812, 1.606731, 1.708893}};

std::vector<std::vector<Double_t>> frac = {
    {0.74, 0.0, 1.0},
    {0.63, 0.0, 1.0},
    {0.7126, 0.71, 0.715},
    {0.6828, 0.68, 0.685}};

int main(int argc, char **argv)
{
    SetProjectRootAsCwd();
    InstallRootWarningFilter();

    // Usage: ups_fitter [inputFile] [cutExpr] [tag]
    // inputFile: a TMVAApply-produced tree (e.g. dataset/weights/m2v/tree_TMVA-processed.root).
    //            Leave empty ("") to keep the default data/cands+base.root input.
    // cutExpr:   a TTreeFormula-style cut, e.g. "TMVACutsScore[5]>0.5" or "TMVABDTGScore>0.2".
    // tag:       overrides the run tag used for output file/plot naming.
    if (argc > 1)
        fitInputFile = argv[1];
    if (argc > 2)
        fitCutExpr = argv[2];
    if (argc > 3)
        tag = argv[3];

    return UpsFitter() ? 0 : 1;
}

bool UpsFitter()
{
    gSystem->mkdir("output", true);
    gSystem->mkdir("output/corr", true);
    gSystem->mkdir("output/fig", true);
    gSystem->mkdir("output/results", true);
    gSystem->mkdir("output/rootfiles", true);

    const TString outname = Form("%s_%s_%ld", tag.Data(), date.Data(), (long)time(NULL));

    rds_data = GetRooDataSet(fitInputFile, fitCutExpr);
    if (!rds_data)
    {
        std::cerr << "ERROR: Failed to construct RooDataSet." << std::endl;
        return false;
    }

    RooDataSet *rds_US = static_cast<RooDataSet *>(rds_data->reduce(RooArgSet(m), "charge==0"));
    rds_US->SetName((outname + "_rds_US").Data());
    rds_US->Print();

    RooDataSet *rds_LS = static_cast<RooDataSet *>(rds_data->reduce(RooArgSet(m), "charge!=0"));
    rds_LS->SetName((outname + "_rds_LS").Data());
    rds_LS->Print();

    // bgLS_frac is computed as LS/US entries in fit_bin(); an empty US sample (common at
    // extreme TMVA cut-points) would divide by zero and send the whole fit to NaN.
    if (rds_US->sumEntries() <= 0)
    {
        std::cerr << "ERROR: Unlike-sign dataset for " << outname
                   << " is empty after cuts; skipping fit." << std::endl;
        delete rds_US;
        delete rds_LS;
        delete rds_data;
        rds_data = nullptr;
        return false;
    }

    const TString fitOutName = outname + "_fit";
    fit_bin(rds_US, rds_LS, fitOutName);

    delete rds_US;
    delete rds_LS;
    delete rds_data;
    rds_data = nullptr;

    std::cout << "Fit for " << outname << " completed!" << std::endl;
    return true;
}

FitResultMetrics fit_bin(RooDataSet *ds_US, RooDataSet *ds_LS, TString outname, Bool_t saveResults)
{
    RooRealVar massVar("m", "m", min_mass, max_mass, "GeV/c^{2}");
    massVar.setRange("full", min_mass, max_mass);

    const double US_entries = ds_US->sumEntries();
    const double LS_entries = ds_LS->sumEntries();

    FitResultMetrics metrics{};

    RooCategory sample("sample", "sample");
    sample.defineType("US");
    sample.defineType("LS");

    std::unique_ptr<RooDataSet> ds_comb(new RooDataSet(
        "ds_comb", "Combined dataset", RooArgSet(massVar), Index(sample), Import("US", *ds_US), Import("LS", *ds_LS)));

    RooRealVar bgLS("bgLS", "LS", par[0][0], par[0][1], par[0][2]);
    RooRealVar bgUS("bgUS", "US", par[1][0], par[1][1], par[1][2]);
    RooRealVar sig1_mu("sig1_mu", "#mu", par[2][0], par[2][1], par[2][2]);
    RooRealVar sig2_mu("sig2_mu", "#mu", par[3][0], par[3][1], par[3][2]);
    RooRealVar sig3_mu("sig3_mu", "#mu", par[4][0], par[4][1], par[4][2]);
    RooRealVar sig1_sigma("sig1_sigma", "#sigma", par[5][0], par[5][1], par[5][2]);
    RooRealVar sig2_sigma("sig2_sigma", "#sigma", par[6][0], par[6][1], par[6][2]);
    RooRealVar sig3_sigma("sig3_sigma", "#sigma", par[7][0], par[7][1], par[7][2]);
    RooRealVar sig1_alpha("sig1_alpha", "#alpha", par[8][0], par[8][1], par[8][2]);
    RooRealVar sig2_alpha("sig2_alpha", "#alpha", par[9][0], par[9][1], par[9][2]);
    RooRealVar sig3_alpha("sig3_alpha", "#alpha", par[10][0], par[10][1], par[10][2]);
    RooRealVar sig1_n("sig1_n", "#n", par[11][0], par[11][1], par[11][2]);
    RooRealVar sig2_n("sig2_n", "#n", par[12][0], par[12][1], par[12][2]);
    RooRealVar sig3_n("sig3_n", "#n", par[13][0], par[13][1], par[13][2]);

    RooRealVar bgLS_frac("bgLS_frac", "bgLS_frac", frac[0][0], frac[0][1], frac[0][2]);
    RooRealVar bgUS_frac("bgUS_frac", "bgUS_frac", frac[1][0], frac[1][1], frac[1][2]);
    RooRealVar sig1_frac("sig1_frac", "sig1_frac", frac[2][0], frac[2][1], frac[2][2]);
    RooRealVar sig2_frac("sig2_frac", "sig2_frac", frac[3][0], frac[3][1], frac[3][2]);

    // Seed from the raw LS/US count ratio but let the fit determine it, rather than
    // hard-fixing the assumption that LS/US count ratio equals the background fraction.
    bgLS_frac.setVal(ds_LS->sumEntries() / ds_US->sumEntries());

    RooFormulaVar sig2_mu_bound("sig2_mu", "sig2_mu", "@0 + 0.560180", RooArgList(sig1_mu));
    RooFormulaVar sig3_mu_bound("sig3_mu", "sig3_mu", "@0 + 0.890502", RooArgList(sig1_mu));

    RooExponential expLS("expLS", "LS exp background", massVar, bgLS);
    RooExponential expUS("expUS", "US Exp background", massVar, bgUS);
    RooCBShape gauss1("gauss1", "Gaussian signal 1", massVar, sig1_mu, sig1_sigma, sig1_alpha, sig1_n);
    RooCBShape gauss2("gauss2", "Gaussian signal 2", massVar, sig2_mu_bound, sig2_sigma, sig2_alpha, sig2_n);
    RooCBShape gauss3("gauss3", "Gaussian signal 3", massVar, sig3_mu_bound, sig3_sigma, sig3_alpha, sig3_n);

    RooAddPdf total_sim("total_sim", "US total", RooArgList(expLS, expUS, gauss1, gauss2, gauss3), RooArgList(bgLS_frac, bgUS_frac, sig1_frac, sig2_frac), kTRUE);

    RooSimultaneous simPdf("simPdf", "simPdf", sample);
    simPdf.addPdf(expLS, "LS");
    simPdf.addPdf(total_sim, "US");

    std::unique_ptr<RooFitResult> result(simPdf.fitTo(*ds_comb,
                                                      Extended(true),
                                                      Minimizer("Minuit2", "migrad"),
                                                      MaxCalls(100000),
                                                      Strategy(2),
                                                      EvalErrorWall(1),
                                                      RecoverFromUndefinedRegions(1000),
                                                      Save()));

    if (gSystem->Getenv("UPSFITTER_DEBUG_COVQ"))
    {
        result->Print("v");
        std::cout << "covQual: " << result->covQual() << " (3=full accurate, <3=degraded)" << std::endl;

        const RooArgList &fpars = result->floatParsFinal();
        const TMatrixDSym &corr = result->correlationMatrix();
        std::cout << "-- largest |correlation| pairs --" << std::endl;
        std::vector<std::tuple<double, TString, TString>> pairs;
        for (int ii = 0; ii < fpars.getSize(); ++ii)
            for (int jj = ii + 1; jj < fpars.getSize(); ++jj)
                pairs.emplace_back(std::abs(corr(ii, jj)), fpars.at(ii)->GetName(), fpars.at(jj)->GetName());
        std::sort(pairs.rbegin(), pairs.rend());
        for (int k = 0; k < std::min<int>(10, pairs.size()); ++k)
            std::cout << "  " << std::get<1>(pairs[k]) << " x " << std::get<2>(pairs[k]) << " = " << std::get<0>(pairs[k]) << std::endl;
    }

    RooGenericPdf yield1("yield1", "yield from signal fit", "(1.0-bgLS_frac)*(1.0-bgUS_frac)", RooArgSet(bgLS_frac, bgUS_frac));
    RooGenericPdf yield2("yield2", "yield from signal 2 fit", "(1.0-bgLS_frac)*(1.0-bgUS_frac)*(1.0-sig1_frac)", RooArgSet(bgLS_frac, bgUS_frac, sig1_frac));
    RooGenericPdf yield3("yield3", "yield from signal 3 fit", "(1.0-bgLS_frac)*(1.0-bgUS_frac)*(1.0-sig1_frac)*(1.0-sig2_frac)", RooArgSet(bgLS_frac, bgUS_frac, sig1_frac, sig2_frac));

    // --- Alternative background model: 2nd-degree polynomial instead of exponential ---
    // Signal shape parameters are independent (suffixed "_p") since this is a fully separate
    // fit of the same dataset, sharing only the initial seed values from par[]/frac[].
    RooRealVar bgLS_c1("bgLS_c1", "bgLS_c1", -0.1, -10.0, 10.0);
    RooRealVar bgLS_c2("bgLS_c2", "bgLS_c2", 0.01, -10.0, 10.0);
    RooRealVar bgUS_c1("bgUS_c1", "bgUS_c1", -0.1, -10.0, 10.0);
    RooRealVar bgUS_c2("bgUS_c2", "bgUS_c2", 0.01, -10.0, 10.0);

    // lowestOrder=1: shape is 1 + c1*x + c2*x^2 (the implicit constant term is redundant
    // with the RooAddPdf fraction and would make the fit degenerate if left floating).
    RooPolynomial polyLS("polyLS", "LS quadratic background", massVar, RooArgList(bgLS_c1, bgLS_c2), 1);
    RooPolynomial polyUS("polyUS", "US quadratic background", massVar, RooArgList(bgUS_c1, bgUS_c2), 1);

    RooRealVar sig1_mu_p("sig1_mu_p", "#mu", par[2][0], par[2][1], par[2][2]);
    RooRealVar sig2_mu_p("sig2_mu_p", "#mu", par[3][0], par[3][1], par[3][2]);
    RooRealVar sig3_mu_p("sig3_mu_p", "#mu", par[4][0], par[4][1], par[4][2]);
    RooRealVar sig1_sigma_p("sig1_sigma_p", "#sigma", par[5][0], par[5][1], par[5][2]);
    RooRealVar sig2_sigma_p("sig2_sigma_p", "#sigma", par[6][0], par[6][1], par[6][2]);
    RooRealVar sig3_sigma_p("sig3_sigma_p", "#sigma", par[7][0], par[7][1], par[7][2]);
    RooRealVar sig1_alpha_p("sig1_alpha_p", "#alpha", par[8][0], par[8][1], par[8][2]);
    RooRealVar sig2_alpha_p("sig2_alpha_p", "#alpha", par[9][0], par[9][1], par[9][2]);
    RooRealVar sig3_alpha_p("sig3_alpha_p", "#alpha", par[10][0], par[10][1], par[10][2]);
    RooRealVar sig1_n_p("sig1_n_p", "#n", par[11][0], par[11][1], par[11][2]);
    RooRealVar sig2_n_p("sig2_n_p", "#n", par[12][0], par[12][1], par[12][2]);
    RooRealVar sig3_n_p("sig3_n_p", "#n", par[13][0], par[13][1], par[13][2]);

    RooRealVar bgLS_frac_p("bgLS_frac_p", "bgLS_frac_p", frac[0][0], frac[0][1], frac[0][2]);
    RooRealVar bgUS_frac_p("bgUS_frac_p", "bgUS_frac_p", frac[1][0], frac[1][1], frac[1][2]);
    RooRealVar sig1_frac_p("sig1_frac_p", "sig1_frac_p", frac[2][0], frac[2][1], frac[2][2]);
    RooRealVar sig2_frac_p("sig2_frac_p", "sig2_frac_p", frac[3][0], frac[3][1], frac[3][2]);
    bgLS_frac_p.setVal(ds_LS->sumEntries() / ds_US->sumEntries());

    RooFormulaVar sig2_mu_p_bound("sig2_mu_p", "sig2_mu_p", "@0 + 0.560180", RooArgList(sig1_mu_p));
    RooFormulaVar sig3_mu_p_bound("sig3_mu_p", "sig3_mu_p", "@0 + 0.890502", RooArgList(sig1_mu_p));

    RooCBShape gauss1_p("gauss1_p", "Gaussian signal 1 (poly bg)", massVar, sig1_mu_p, sig1_sigma_p, sig1_alpha_p, sig1_n_p);
    RooCBShape gauss2_p("gauss2_p", "Gaussian signal 2 (poly bg)", massVar, sig2_mu_p_bound, sig2_sigma_p, sig2_alpha_p, sig2_n_p);
    RooCBShape gauss3_p("gauss3_p", "Gaussian signal 3 (poly bg)", massVar, sig3_mu_p_bound, sig3_sigma_p, sig3_alpha_p, sig3_n_p);

    RooAddPdf total_sim_poly("total_sim_poly", "US total (poly bg)", RooArgList(polyLS, polyUS, gauss1_p, gauss2_p, gauss3_p), RooArgList(bgLS_frac_p, bgUS_frac_p, sig1_frac_p, sig2_frac_p), kTRUE);

    RooSimultaneous simPdf_poly("simPdf_poly", "simPdf_poly", sample);
    simPdf_poly.addPdf(polyLS, "LS");
    simPdf_poly.addPdf(total_sim_poly, "US");

    std::unique_ptr<RooFitResult> result_poly(simPdf_poly.fitTo(*ds_comb,
                                                                Extended(true),
                                                                Minimizer("Minuit2", "migrad"),
                                                                MaxCalls(100000),
                                                                Strategy(2),
                                                                EvalErrorWall(1),
                                                                RecoverFromUndefinedRegions(1000),
                                                                Save()));

    RooGenericPdf yield1_poly("yield1_poly", "yield from poly-bg fit", "(1.0-bgLS_frac_p)*(1.0-bgUS_frac_p)", RooArgSet(bgLS_frac_p, bgUS_frac_p));

    Double_t chi2ndf_US = -1;
    Double_t chi2ndf_LS = -1;
    Double_t chi2ndf_US_poly = -1;
    Double_t chi2ndf_LS_poly = -1;

    if (saveResults)
    {
        std::unique_ptr<TCanvas> cnv(new TCanvas("cnv", "cnv", 1600, 1200));

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

        bgLS_c1.setRange(-10, 10);
        bgLS_c2.setRange(-10, 10);
        bgUS_c1.setRange(-10, 10);
        bgUS_c2.setRange(-10, 10);
        sig1_mu_p.setRange(1, 100);
        sig2_mu_p.setRange(1, 100);
        sig3_mu_p.setRange(1, 100);
        sig1_sigma_p.setRange(0, 100);
        sig2_sigma_p.setRange(0, 100);
        sig3_sigma_p.setRange(0, 100);
        sig1_n_p.setRange(-10, 10);
        sig2_n_p.setRange(-10, 10);
        sig3_n_p.setRange(-10, 10);
        sig1_alpha_p.setRange(0, 10);
        sig2_alpha_p.setRange(0, 10);
        sig3_alpha_p.setRange(0, 10);
        bgUS_frac_p.setRange(0, 1);
        sig1_frac_p.setRange(0, 1);
        sig2_frac_p.setRange(0, 1);

        cnv->cd();
        RooPlot *frame = massVar.frame(((max_mass - min_mass) / bin_width));
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

        ds_LS->plotOn(frame, MarkerColor(kRed), LineColor(kRed), Name("data_LS"));
        ds_US->plotOn(frame, Name("data_US"));
        // Only the VisualizeError band is drawn filled - it's the one region on this plot that
        // actually represents an uncertainty. Every component below is outline-only: stacking
        // several semi-transparent FillStyle(3001) fills on top of each other optically merges
        // into a single uniform gray/yellow wash instead of distinct colored regions.
        total_sim.plotOn(frame, VisualizeError(*result, sigma, useLinear), DrawOption("F"), FillStyle(3001));
        total_sim.plotOn(frame);
        total_sim.plotOn(frame, Components(expLS), LineColor(kRed), LineStyle(kDashed));
        total_sim.plotOn(frame, Components(expUS), LineColor(kGray + 2), LineStyle(kDashed));
        total_sim.plotOn(frame, Components(gauss1), LineColor(kMagenta), LineStyle(kSolid));
        total_sim.plotOn(frame, Components(gauss2), LineColor(kMagenta + 2), LineStyle(kSolid));
        total_sim.plotOn(frame, Components(gauss3), LineColor(kMagenta + 4), LineStyle(kSolid));
        total_sim.plotOn(frame, Components(RooArgSet(gauss1, gauss2, gauss3)), LineColor(kCyan + 2), LineStyle(kSolid));
        total_sim.plotOn(frame, Components(RooArgSet(expLS, expUS)), LineColor(kOrange + 7), LineStyle(kSolid));
        total_sim.plotOn(frame);

        const Int_t nparam_US = total_sim.getParameters(*ds_US)->selectByAttrib("Constant", kFALSE)->getSize();
        chi2ndf_US = frame->chiSquare(nparam_US);

        const Int_t nparam_LS = expLS.getParameters(*ds_LS)->selectByAttrib("Constant", kFALSE)->getSize();
        chi2ndf_LS = frame->chiSquare(nparam_LS);

        // Overlay the polynomial-background model: outline only (no fill), to keep it
        // visually distinct from the exponential model's filled components above.
        total_sim_poly.plotOn(frame, LineColor(kGreen + 2), LineStyle(kDashDotted), Name("curve_total_poly"));
        total_sim_poly.plotOn(frame, Components(RooArgSet(polyLS, polyUS)), LineColor(kGreen + 2), LineStyle(kDotted));
        // Without an explicit Normalization, plotOn scales a bare pdf to whichever dataset
        // was drawn last in the frame (data_US), not the actual LS entry count.
        polyLS.plotOn(frame, LineColor(kGreen + 2), LineStyle(kDashDotted), Name("curve_polyLS"),
                      Normalization(LS_entries, RooAbsReal::NumEvent));

        const Int_t nparam_US_poly = total_sim_poly.getParameters(*ds_US)->selectByAttrib("Constant", kFALSE)->getSize();
        chi2ndf_US_poly = frame->chiSquare("curve_total_poly", "data_US", nparam_US_poly);

        const Int_t nparam_LS_poly = polyLS.getParameters(*ds_LS)->selectByAttrib("Constant", kFALSE)->getSize();
        chi2ndf_LS_poly = frame->chiSquare("curve_polyLS", "data_LS", nparam_LS_poly);

        // Leave headroom above the peak so the text block below doesn't overlap the curves.
        frame->SetMaximum(frame->GetMaximum() * 1.55);
        frame->Draw("F");

        gStyle->SetOptStat(0);
        std::unique_ptr<TH2> hcorr(result->correlationHist());

        const Double_t yield_poly = yield1_poly.getVal() * US_entries;
        const Double_t yieldErr_poly = yield1_poly.getPropagatedError(*result_poly) * US_entries;

        std::ofstream out("output/results/" + outname + ".txt");
        out << "US entries: " << US_entries << ", LS entries: " << LS_entries << std::endl;
        out << "-- exponential background --" << std::endl;
        out << "#chi^{2}/N^{US}_{dof} = " << chi2ndf_US << ", #chi^{2}/N^{LS}_{dof} = " << chi2ndf_LS << std::endl;
        out << "frac: " << yield1.getVal() << ", yield: " << (yield1.getVal() * US_entries) << std::endl;
        out << "error frac: " << yield1.getPropagatedError(*result) << ", error: " << (yield1.getPropagatedError(*result) * US_entries) << std::endl;
        out << "-- 2nd-degree polynomial background --" << std::endl;
        out << "#chi^{2}/N^{US}_{dof} = " << chi2ndf_US_poly << ", #chi^{2}/N^{LS}_{dof} = " << chi2ndf_LS_poly << std::endl;
        out << "frac: " << yield1_poly.getVal() << ", yield: " << yield_poly << std::endl;
        out << "error frac: " << yield1_poly.getPropagatedError(*result_poly) << ", error: " << yieldErr_poly << std::endl;

        const Double_t yield_exp = yield1.getVal() * US_entries;
        const Double_t yieldErr_exp = yield1.getPropagatedError(*result) * US_entries;

        // The plot's y-max was stretched 1.55x above the actual peak (see SetMaximum above)
        // specifically to keep this whole block clear of the curves - it lives in that
        // reserved headroom (roughly NDC y >= 0.67), not layered on top of the fit.
        TLatex latex1;
        latex1.SetTextSize(0.030);
        latex1.SetNDC();
        latex1.SetTextColor(kBlue + 2);
        latex1.DrawLatex(0.50, 0.93, "Exponential background:");
        latex1.DrawLatex(0.50, 0.895, Form("#chi^{2}/N_{dof}: US = %.2f, LS = %.2f", chi2ndf_US, chi2ndf_LS));
        latex1.DrawLatex(0.50, 0.86, Form("Y(1S+2S+3S) = %.0f #pm %.0f", yield_exp, yieldErr_exp));
        latex1.DrawLatex(0.50, 0.825, Form("S/#sqrt{S+B} = %.2f", yield_exp / sqrt(US_entries)));
        latex1.SetTextColor(kGreen + 2);
        latex1.DrawLatex(0.50, 0.77, "2nd-degree polynomial background:");
        latex1.DrawLatex(0.50, 0.735, Form("#chi^{2}/N_{dof}: US = %.2f, LS = %.2f", chi2ndf_US_poly, chi2ndf_LS_poly));
        latex1.DrawLatex(0.50, 0.70, Form("Y(1S+2S+3S) = %.0f #pm %.0f", yield_poly, yieldErr_poly));
        latex1.DrawLatex(0.50, 0.665, Form("S/#sqrt{S+B} = %.2f", yield_poly / sqrt(US_entries)));

        cnv->SaveAs("output/fig/" + outname + ".png");

        if (hcorr)
        {
            gStyle->SetPaintTextFormat("4.2f");
            hcorr->Draw("colz text");
            gPad->SetRightMargin(0.15);
        }

        delete frame;
    }

    metrics.S = yield1.getVal() * US_entries;
    metrics.B = (1.0 - yield1.getVal()) * US_entries;
    metrics.yield = metrics.S;
    metrics.yieldErr = yield1.getPropagatedError(*result) * US_entries;
    metrics.S_over_sqrtSB = metrics.S / sqrt(metrics.S + metrics.B);
    metrics.US_entries = US_entries;
    metrics.LS_entries = LS_entries;
    metrics.chi2ndf_US = chi2ndf_US;
    metrics.chi2ndf_LS = chi2ndf_LS;

    metrics.yield_poly = yield1_poly.getVal() * US_entries;
    metrics.yieldErr_poly = yield1_poly.getPropagatedError(*result_poly) * US_entries;
    metrics.chi2ndf_US_poly = chi2ndf_US_poly;
    metrics.chi2ndf_LS_poly = chi2ndf_LS_poly;

    return metrics;
}

RooDataSet *GetRooDataSet(const TString &infile, const TString &cutExpr)
{
    const bool useOverride = !infile.IsNull();
    const TString path = useOverride ? infile : TString("data/cands+base.root");

    std::unique_ptr<TFile> file(TFile::Open(path, "READ"));
    TTree *tree = nullptr;

    if (!file || file->IsZombie())
    {
        if (useOverride)
        {
            std::cerr << "ERROR: Could not open input file: " << path << std::endl;
            return nullptr;
        }
        tree = GetBaseTree();
    }
    else
    {
        tree = static_cast<TTree *>(file->Get("tree"));
    }

    if (!tree)
    {
        std::cerr << "ERROR: Could not find input tree!" << std::endl;
        return nullptr;
    }

    // RooDataSet's Cut() option evaluates against the imported RooArgSet variables only,
    // so it can't see tree-only branches like TMVACutsScore. Pre-filter with TTree::CopyTree
    // (backed by a proper TTreeFormula, so vector indexing like "TMVACutsScore[5]" works)
    // and import the filtered tree instead.
    std::unique_ptr<TTree> filteredTree;
    if (!cutExpr.IsNull())
    {
        // CopyTree() attaches its output to the current gDirectory, which at this point is
        // the read-only source file - basket writes into a read-only TFile fail once enough
        // entries pass to trigger a flush. cd() into gROOT first so the copy is genuinely
        // memory-resident (writable) instead of implicitly tied to a read-only file.
        TDirectory::TContext ctx(gROOT);
        filteredTree.reset(tree->CopyTree(cutExpr));
        if (!filteredTree)
        {
            std::cerr << "ERROR: Failed to apply cut expression: " << cutExpr << std::endl;
            return nullptr;
        }
        tree = filteredTree.get();
    }

    return new RooDataSet("rds_cands+base", "Dataset with cuts", RooArgSet(m, charge), Import(*tree));
}
