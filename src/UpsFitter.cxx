#include "../headers/UpsFitter.hxx"

using namespace RooFit;

TString infile_BHT1 = "../data/BHT1.masssp.root";
TString infile_BHT2 = "../data/BHT2_251128.default.cands.root";

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

int main()
{
    SetProjectRootAsCwd();
    InstallRootWarningFilter();
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

    rds_data = GetRooDataSet();
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

    bgLS_frac.setVal(ds_LS->sumEntries() / ds_US->sumEntries());
    bgLS_frac.setConstant(true);

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

    RooGenericPdf yield1("yield1", "yield from signal fit", "(1.0-bgLS_frac)*(1.0-bgUS_frac)", RooArgSet(bgLS_frac, bgUS_frac));
    RooGenericPdf yield2("yield2", "yield from signal 2 fit", "(1.0-bgLS_frac)*(1.0-bgUS_frac)*(1.0-sig1_frac)", RooArgSet(bgLS_frac, bgUS_frac, sig1_frac));
    RooGenericPdf yield3("yield3", "yield from signal 3 fit", "(1.0-bgLS_frac)*(1.0-bgUS_frac)*(1.0-sig1_frac)*(1.0-sig2_frac)", RooArgSet(bgLS_frac, bgUS_frac, sig1_frac, sig2_frac));

    Double_t chi2ndf_US = -1;
    Double_t chi2ndf_LS = -1;

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

        ds_LS->plotOn(frame, MarkerColor(kRed), LineColor(kRed));
        ds_US->plotOn(frame);
        total_sim.plotOn(frame, VisualizeError(*result, sigma, useLinear), DrawOption("F"), FillStyle(3001));
        total_sim.plotOn(frame);
        total_sim.plotOn(frame, Components(expLS), LineColor(kRed), LineStyle(kDashed), DrawOption("F"), FillColor(kRed), FillStyle(3001));
        total_sim.plotOn(frame, Components(expLS), LineColor(kRed), LineStyle(kDashed));
        total_sim.plotOn(frame, Components(expUS), LineColor(kGray), LineStyle(kDashed), DrawOption("F"), FillColor(kGray), FillStyle(3001));
        total_sim.plotOn(frame, Components(expUS), LineColor(kGray), LineStyle(kDashed));
        total_sim.plotOn(frame, Components(gauss1), LineColor(kMagenta), LineStyle(kSolid), DrawOption("F"), FillColor(kMagenta), FillStyle(3001));
        total_sim.plotOn(frame, Components(gauss1), LineColor(kMagenta), LineStyle(kSolid));
        total_sim.plotOn(frame, Components(gauss2), LineColor(kMagenta + 2), LineStyle(kSolid), DrawOption("F"), FillColor(kMagenta + 2), FillStyle(3001));
        total_sim.plotOn(frame, Components(gauss2), LineColor(kMagenta + 2), LineStyle(kSolid));
        total_sim.plotOn(frame, Components(gauss3), LineColor(kMagenta + 4), LineStyle(kSolid), DrawOption("F"), FillColor(kMagenta + 4), FillStyle(3001));
        total_sim.plotOn(frame, Components(gauss3), LineColor(kMagenta + 4), LineStyle(kSolid));
        total_sim.plotOn(frame, Components(RooArgSet(gauss1, gauss2, gauss3)), LineColor(kCyan), LineStyle(kSolid), DrawOption("F"), FillColor(kTeal), FillStyle(3001));
        total_sim.plotOn(frame, Components(RooArgSet(gauss1, gauss2, gauss3)), LineColor(kCyan), LineStyle(kSolid));
        total_sim.plotOn(frame, Components(RooArgSet(expLS, expUS)), LineColor(kOrange), LineStyle(kSolid), DrawOption("F"), FillColor(kOrange), FillStyle(3001));
        total_sim.plotOn(frame, Components(RooArgSet(expLS, expUS)), LineColor(kOrange), LineStyle(kSolid));
        total_sim.plotOn(frame);
        frame->Draw("F");

        const Int_t nparam_US = total_sim.getParameters(*ds_US)->selectByAttrib("Constant", kFALSE)->getSize();
        chi2ndf_US = frame->chiSquare(nparam_US);

        const Int_t nparam_LS = expLS.getParameters(*ds_LS)->selectByAttrib("Constant", kFALSE)->getSize();
        chi2ndf_LS = frame->chiSquare(nparam_LS);

        gStyle->SetOptStat(0);
        std::unique_ptr<TH2> hcorr(result->correlationHist());

        std::ofstream out("output/results/" + outname + ".txt");
        out << "US entries: " << US_entries << ", LS entries: " << LS_entries << std::endl;
        out << "#chi^{2}/N^{US}_{dof} = " << chi2ndf_US << ", #chi^{2}/N^{LS}_{dof} = " << chi2ndf_LS << std::endl;
        out << "frac: " << yield1.getVal() << ", yield: " << (yield1.getVal() * US_entries) << std::endl;
        out << "error frac: " << yield1.getPropagatedError(*result) << ", error: " << (yield1.getPropagatedError(*result) * US_entries) << std::endl;

        TLatex latex1;
        latex1.SetTextSize(0.045);
        latex1.SetNDC();
        latex1.DrawLatex(0.55, 0.85, Form("#frac{#chi^{2}}{N^{US}_{dof}} = %.2f, #frac{#chi^{2}}{N^{LS}_{dof}} = %.2f", chi2ndf_US, chi2ndf_LS));
        latex1.DrawLatex(0.55, 0.75, Form("#Upsilon(1S+2S+3S) = %.0f #pm %.0f", (yield1.getVal() * US_entries), (yield1.getPropagatedError(*result) * US_entries)));
        latex1.DrawLatex(0.55, 0.65, Form("S/#sqrt{S+B} = %.2f", (yield1.getVal() * US_entries) / sqrt(US_entries)));

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

    return metrics;
}

RooDataSet *GetRooDataSet()
{
    std::unique_ptr<TFile> file(TFile::Open("data/cands+base.root", "READ"));
    TTree *tree = nullptr;

    if (!file || file->IsZombie())
    {
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

    return new RooDataSet("rds_cands+base", "Dataset with cuts", RooArgSet(m, charge), Import(*tree));
}
