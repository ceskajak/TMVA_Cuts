#include "../headers/TMVAApply.hxx"

using namespace std;

namespace
{
bool IsMethodEnabled(const TString &method)
{
    return activeMethodNames.find(method) != activeMethodNames.end();
}

void EnableBranch(TTree *tree, const char *branchName)
{
    tree->SetBranchStatus(branchName, 1);
}
} // namespace

void UpdateReaderMethodFlags()
{
    std::set<TString> available;
    for (const auto &method : vec_TMVAMethods)
    {
        const TString weightFile = Form("%s/TMVAOptimizeCuts_%s.weights.xml", weightPath.Data(), method.Data());
        if (CheckFileExists(weightFile))
        {
            available.insert(method);
        }
    }
    activeMethodNames = available;
}

/////////////////////////////////////////////////////////////

void AdjustVariableFlags()
{
    struct VarsetState
    {
        bool use2v;
        bool use5v;
        bool use6v;
        bool use16v;
    };

    static const std::vector<std::pair<TString, VarsetState>> lookup = {
        {"c16v", {true, true, true, true}},
        {"m16v", {true, true, true, true}},
        {"c6v", {true, true, true, false}},
        {"m6v", {true, true, true, false}},
        {"c5v", {true, true, false, false}},
        {"m5v", {true, true, false, false}},
        {"c2v", {true, false, false, false}},
        {"m2v", {true, false, false, false}},
    };

    k2v = false;
    k5v = false;
    k6v = false;
    k16v = false;

    for (const auto &entry : lookup)
    {
        if (weightPath.Contains(entry.first))
        {
            k2v = entry.second.use2v;
            k5v = entry.second.use5v;
            k6v = entry.second.use6v;
            k16v = entry.second.use16v;
            return;
        }
    }
}

/////////////////////////////////////////////////////////////

void ConnectBranches(TTree *tree)
{
    tree->SetBranchAddress("m", &var_m);
    tree->SetBranchAddress("charge", &var_charge);
    tree->SetBranchAddress("y", &var_y);
    tree->SetBranchAddress("nSigmaElectron1", &var_nSigmaElectron1);
    tree->SetBranchAddress("nSigmaElectron2", &var_nSigmaElectron2);
    tree->SetBranchAddress("dEdx1", &var_dEdx1);
    tree->SetBranchAddress("dEdx2", &var_dEdx2);
    tree->SetBranchAddress("eta1", &var_eta1);
    tree->SetBranchAddress("eta2", &var_eta2);
    tree->SetBranchAddress("phi1", &var_phi1);
    tree->SetBranchAddress("phi2", &var_phi2);
    tree->SetBranchAddress("invBeta1", &var_invBeta1);
    tree->SetBranchAddress("invBeta2", &var_invBeta2);
    tree->SetBranchAddress("E_TOW1", &var_E_TOW1);
    tree->SetBranchAddress("E_CLU1", &var_E_CLU1);
    tree->SetBranchAddress("E_TOW2", &var_E_TOW2);
    tree->SetBranchAddress("E_CLU2", &var_E_CLU2);
    tree->SetBranchAddress("p1", &var_p1);
    tree->SetBranchAddress("p2", &var_p2);
    tree->SetBranchAddress("pt1", &var_pt1);
    tree->SetBranchAddress("pt2", &var_pt2);
    tree->SetBranchAddress("adc1", &var_adc1);
    tree->SetBranchAddress("adc2", &var_adc2);
    tree->SetBranchAddress("bemcZDist1", &var_bemcZDist1);
    tree->SetBranchAddress("bemcZDist2", &var_bemcZDist2);
    tree->SetBranchAddress("bemcPhiDist1", &var_bemcPhiDist1);
    tree->SetBranchAddress("bemcPhiDist2", &var_bemcPhiDist2);
    tree->SetBranchAddress("VzTPC", &var_VzTPC);
    tree->SetBranchAddress("VzVPD", &var_VzVPD);
    tree->SetBranchAddress("nHitsFit1", &var_nHitsFit1);
    tree->SetBranchAddress("nHitsMax1", &var_nHitsMax1);
    tree->SetBranchAddress("nHitsFit2", &var_nHitsFit2);
    tree->SetBranchAddress("nHitsMax2", &var_nHitsMax2);
    tree->SetBranchAddress("DCA1", &var_DCA1);
    tree->SetBranchAddress("DCA2", &var_DCA2);
}

/////////////////////////////////////////////////////////////

void RegisterReaderVariables()
{
    if (k16v)
    {
        reader->AddVariable("dEdx1", &var_dEdx1);
        reader->AddVariable("dEdx2", &var_dEdx2);
        reader->AddVariable("eta1", &var_eta1);
        reader->AddVariable("eta2", &var_eta2);
        reader->AddVariable("phi1", &var_phi1);
        reader->AddVariable("phi2", &var_phi2);
        reader->AddVariable("E_TOW1", &var_E_TOW1);
        reader->AddVariable("E_CLU1", &var_E_CLU1);
        reader->AddVariable("E_TOW2", &var_E_TOW2);
        reader->AddVariable("E_CLU2", &var_E_CLU2);
    }
    if (k5v)
    {
        reader->AddVariable("min(E_TOW1/E_CLU1, E_TOW2/E_CLU2)", &var_min_ETOWoverECLU);
        reader->AddVariable("max(E_CLU1/p1, E_CLU2/p2)", &var_max_ECLUoverP);
        reader->AddVariable("min(E_CLU1/p1, E_CLU2/p2)", &var_min_ECLUoverP);
    }
    if (k2v)
    {
        reader->AddVariable("max(abs(bemcZDist1), abs(bemcZDist2))", &var_max_abs_bemcZDist);
        reader->AddVariable("max(abs(bemcPhiDist1), abs(bemcPhiDist2))", &var_max_abs_bemcPhiDist);
    }
    if (k6v)
    {
        reader->AddVariable("max(abs(DCA1), abs(DCA2))", &var_max_abs_DCA);
    }
}

/////////////////////////////////////////////////////////////

bool BookReader()
{
    reader = new TMVA::Reader("!Color:Silent");
    RegisterReaderVariables();
    UpdateReaderMethodFlags();

    cout << "Booking TMVA reader methods..." << endl;
    if (activeMethodNames.empty())
    {
        cerr << "ERROR: No TMVA reader methods booked!" << endl;
        cerr << "Quitting..." << endl;
        cerr << "(Hint: Turn on a switch in `../headers/TMVAApply.hxx`.)" << endl;
        delete reader;
        reader = nullptr;
        return 0;
    }

    auto BookReaderMethod = [&](const TString &methodName)
    {
        reader->BookMVA(methodName, Form("%s/TMVAOptimizeCuts_%s.weights.xml", weightPath.Data(), methodName.Data()));
    };

    if (IsMethodEnabled("Cuts"))
    {
        BookReaderMethod("Cuts");
        t_out->Branch("TMVACutsScore", &TMVACutsScore);
    }
    if (IsMethodEnabled("BDTG"))
    {
        BookReaderMethod("BDTG");
        t_out->Branch("TMVABDTGScore", &TMVABDTGScore, "TMVABDTGScore/F");
    }
    if (IsMethodEnabled("HMatrix"))
    {
        BookReaderMethod("HMatrix");
        t_out->Branch("TMVAHMatrixScore", &TMVAHMatrixScore, "TMVAHMatrixScore/F");
    }
    if (IsMethodEnabled("Fisher"))
    {
        BookReaderMethod("Fisher");
        t_out->Branch("TMVAFisherScore", &TMVAFisherScore, "TMVAFisherScore/F");
    }
    if (IsMethodEnabled("MLP"))
    {
        BookReaderMethod("MLP");
        t_out->Branch("TMVAMLPScore", &TMVAMLPScore, "TMVAMLPScore/F");
    }
    if (IsMethodEnabled("SVM"))
    {
        BookReaderMethod("SVM");
        t_out->Branch("TMVASVMScore", &TMVASVMScore, "TMVASVMScore/F");
    }

    return 1;
}

/////////////////////////////////////////////////////////////

bool TMVAApply(TString strVarset)
{
    using std::abs;
    using std::max;
    using std::min;

    const TString baseWeightPath = "dataset/weights";
    const TString requestedWeightPath = Form("%s/%s", baseWeightPath.Data(), strVarset.Data());
    bool useFallbackWeights = false;

    // Reset global path each run (do not accumulate suffixes between calls).
    weightPath = requestedWeightPath;

    if (gSystem->AccessPathName(requestedWeightPath))
        gSystem->mkdir(requestedWeightPath, true);

    bool hasRequestedWeights = false;
    for (const auto &method : vec_TMVAMethods)
    {
        const TString weightFile = Form("%s/TMVAOptimizeCuts_%s.weights.xml", requestedWeightPath.Data(), method.Data());
        if (CheckFileExists(weightFile))
        {
            hasRequestedWeights = true;
            break;
        }
    }
    if (!hasRequestedWeights)
    {
        weightPath = baseWeightPath;
        useFallbackWeights = true;
        cout << "No varset-specific weights found in " << requestedWeightPath
             << ", falling back to " << baseWeightPath << endl;
    }

    // Open input file
    t_main = GetBaseTree();
    if (!t_main)
    {
        cerr << "ERROR: Failed to get base tree." << endl;
        return false;
    }

    // All variable categories are subsets.
    // Base fallback weights are currently trained with 3 variables (2v + DCA),
    // so force a compatible reader configuration.
    if (useFallbackWeights)
    {
        k2v = true;
        k5v = false;
        k6v = true;
        k16v = false;
    }
    else
    {
        AdjustVariableFlags();
    }

    // Connect tree branches
    ConnectBranches(t_main);

    // Create output file
    f_out = TFile::Open(requestedWeightPath + "/tree_TMVA-processed.root", "RECREATE");
    if (!f_out || f_out->IsZombie())
    {
        cerr << "ERROR: Failed to create output file: " << requestedWeightPath << "/tree_TMVA-processed.root" << endl;
        TMVACleaner();
        return false;
    }

    // Ensure the output file is the current directory before cloning/writing
    f_out->cd();

    // Clone tree structure
    t_out = t_main->CloneTree(0);
    if (!t_out)
    {
        cerr << "ERROR: Failed to clone output tree." << endl;
        TMVACleaner();
        return false;
    }

    // Explicitly associate the cloned tree with the output file
    t_out->SetDirectory(f_out);

    // Read only branches required by the active varset.
    t_main->SetBranchStatus("*", 0);
    EnableBranch(t_main, "m");
    EnableBranch(t_main, "charge");

    if (k16v)
    {
        EnableBranch(t_main, "dEdx1");
        EnableBranch(t_main, "dEdx2");
        EnableBranch(t_main, "eta1");
        EnableBranch(t_main, "eta2");
        EnableBranch(t_main, "phi1");
        EnableBranch(t_main, "phi2");
        EnableBranch(t_main, "E_TOW1");
        EnableBranch(t_main, "E_CLU1");
        EnableBranch(t_main, "E_TOW2");
        EnableBranch(t_main, "E_CLU2");
    }

    if (k5v)
    {
        EnableBranch(t_main, "E_TOW1");
        EnableBranch(t_main, "E_CLU1");
        EnableBranch(t_main, "E_TOW2");
        EnableBranch(t_main, "E_CLU2");
        EnableBranch(t_main, "p1");
        EnableBranch(t_main, "p2");
    }

    if (k2v)
    {
        EnableBranch(t_main, "bemcZDist1");
        EnableBranch(t_main, "bemcZDist2");
        EnableBranch(t_main, "bemcPhiDist1");
        EnableBranch(t_main, "bemcPhiDist2");
    }

    if (k6v)
    {
        EnableBranch(t_main, "DCA1");
        EnableBranch(t_main, "DCA2");
    }

    t_main->SetCacheSize(10000000);
    t_main->AddBranchToCache("*", kTRUE);

    // Book TMVA reader methods
    if (!BookReader())
    {
        cerr << "ERROR: Failed to get S/B criteria!" << endl;
        cerr << "Quitting..." << endl;

        TMVACleaner();

        return false;
    }

    // Loop over events
    Long64_t nEntries = t_main->GetEntries();
    TMVACutsScore.reserve(11);

    for (Long64_t i = 0; i < nEntries; ++i)
    {
        TMVACutsScore.clear();

        // Load entry
        t_main->GetEntry(i);

        // Calculate derived variables
        var_min_ETOWoverECLU = min(var_E_TOW1 / var_E_CLU1, var_E_TOW2 / var_E_CLU2);
        var_max_ECLUoverP = max(var_E_CLU1 / var_p1, var_E_CLU2 / var_p2);
        var_min_ECLUoverP = min(var_E_CLU1 / var_p1, var_E_CLU2 / var_p2);
        var_max_abs_bemcZDist = max(abs(var_bemcZDist1), abs(var_bemcZDist2));
        var_max_abs_bemcPhiDist = max(abs(var_bemcPhiDist1), abs(var_bemcPhiDist2));
        var_max_abs_DCA = max(abs(var_DCA1), abs(var_DCA2));

        // Evaluate MLP methods
        // Cuts
        if (IsMethodEnabled("Cuts"))
            for (uint j = 0; j <= 10; j++)
                TMVACutsScore.push_back(reader->EvaluateMVA("Cuts", (float)j / 10.0));
        // BDTG
        if (IsMethodEnabled("BDTG"))
            TMVABDTGScore = reader->EvaluateMVA("BDTG");
        // HMatrix
        if (IsMethodEnabled("HMatrix"))
            TMVAHMatrixScore = reader->EvaluateMVA("HMatrix");
        // Fisher
        if (IsMethodEnabled("Fisher"))
            TMVAFisherScore = reader->EvaluateMVA("Fisher");
        // MLP
        if (IsMethodEnabled("MLP"))
            TMVAMLPScore = reader->EvaluateMVA("MLP");
        // SVM
        if (IsMethodEnabled("SVM"))
            TMVASVMScore = reader->EvaluateMVA("SVM");

        t_out->Fill();
    }

    // Write output
    if (!f_out || f_out->IsZombie())
    {
        cerr << "ERROR: Output file is not valid when attempting to write tree." << endl;
    }
    else
    {
        f_out->cd();
        t_out->SetDirectory(f_out);

        t_out->Write();
        f_out->Write();
        f_out->Flush();
    }

    TMVACleaner();

    return true;
}