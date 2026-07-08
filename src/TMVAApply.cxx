#include "../headers/TMVAApply.hxx"

void UpdateReaderMethodFlags()
{
    for (size_t iter = 0; iter < vec_TMVAMethods.size(); ++iter)
    {
        if (!CheckFileExists(Form("%s/TMVAOptimizeCuts_%s.weights.xml", weightPath.Data(), vec_TMVAMethods[iter].Data())))
        {
            if (vec_TMVAMethods[iter] == "Cuts")
                kCuts = 0;
            else if (vec_TMVAMethods[iter] == "BDTG")
                kBDTG = 0;
            else if (vec_TMVAMethods[iter] == "HMatrix")
                kHMatrix = 0;
            else if (vec_TMVAMethods[iter] == "Fisher")
                kFisher = 0;
            else if (vec_TMVAMethods[iter] == "MLP")
                kMLP = 0;
            else if (vec_TMVAMethods[iter] == "SVM")
                kSVM = 0;
        }
    }
}

/////////////////////////////////////////////////////////////

void AdjustVariableFlags()
{
    if (weightPath.Contains("c2v") || weightPath.Contains("m2v"))
    {
        k2v = 1;
        k5v = 0;
        k6v = 0;
        k16v = 0;
    }
    else if (weightPath.Contains("c5v") || weightPath.Contains("m5v"))
    {
        k2v = 1;
        k5v = 1;
        k6v = 0;
        k16v = 0;
    }
    else if (weightPath.Contains("c6v") || weightPath.Contains("m6v"))
    {
        k2v = 1;
        k5v = 1;
        k6v = 1;
        k16v = 0;
    }
    else if (weightPath.Contains("c16v") || weightPath.Contains("m16v"))
    {
        k2v = 1;
        k5v = 1;
        k6v = 1;
        k16v = 1;
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
    reader = new TMVA::Reader("!Color:!Silent");
    RegisterReaderVariables();
    UpdateReaderMethodFlags();

    cout << "Booking TMVA reader methods..." << endl;
    if (!kCuts && !kBDTG && !kHMatrix && !kFisher && !kMLP && !kSVM)
    {
        cerr << "ERROR: No TMVA reader methods booked!" << endl;
        cerr << "Quitting..." << endl;
        cerr << "(Hint: Turn on a switch in `../headers/TMVAApply.hxx`.)" << endl;
        if (t_sg)
            delete t_sg;
        if (t_bg)
            delete t_bg;
        f_in->Close();
        return 0;
    }

    auto BookReaderMethod = [&](const TString &methodName)
    {
        reader->BookMVA(methodName, Form("%s/TMVAOptimizeCuts_%s.weights.xml", weightPath.Data(), methodName.Data()));
    };

    if (kCuts)
    {
        BookReaderMethod("Cuts");
        t_out->Branch("TMVACutsScore", &TMVACutsScore, "TMVACutsScore/F");
    }
    if (kBDTG)
    {
        BookReaderMethod("BDTG");
        t_out->Branch("TMVABDTGScore", &TMVABDTGScore, "TMVABDTGScore/F");
    }
    if (kHMatrix)
    {
        BookReaderMethod("HMatrix");
        t_out->Branch("TMVAHMatrixScore", &TMVAHMatrixScore, "TMVAHMatrixScore/F");
    }
    if (kFisher)
    {
        BookReaderMethod("Fisher");
        t_out->Branch("TMVAFisherScore", &TMVAFisherScore, "TMVAFisherScore/F");
    }
    if (kMLP)
    {
        BookReaderMethod("MLP");
        t_out->Branch("TMVAMLPScore", &TMVAMLPScore, "TMVAMLPScore/F");
    }
    if (kSVM)
    {
        BookReaderMethod("SVM");
        t_out->Branch("TMVASVMScore", &TMVASVMScore, "TMVASVMScore/F");
    }

    return 1;
}

/////////////////////////////////////////////////////////////

void TMVAApply(TString strVarset = "m2v")
{
    // Set the dataset path for TMVA weights
    weightPath += strVarset;

    // Open input file
    t_main = GetBaseTree();

    // All variable categories are subsets
    AdjustVariableFlags();

    // Connect tree branches
    ConnectBranches(t_main);

    // Create output file
    f_out = TFile::Open(weightPath + "/tree_TMVA-processed.root", "RECREATE");
    if (!f_out || f_out->IsZombie())
    {
        cerr << "ERROR: Failed to create output file: " << weightPath << "/tree_TMVA-processed.root" << endl;
        TMVACleaner();
        return;
    }

    // Ensure the output file is the current directory before cloning/writing
    f_out->cd();

    // Clone tree structure
    t_out = t_main->CloneTree(0);
    if (!t_out)
    {
        cerr << "ERROR: Failed to clone output tree." << endl;
        TMVACleaner();
        return;
    }

    // Explicitly associate the cloned tree with the output file
    t_out->SetDirectory(f_out);

    // Book TMVA reader methods
    if (!BookReader())
    {
        cerr << "ERROR: Failed to get S/B criteria!" << endl;
        cerr << "Quitting..." << endl;

        TMVACleaner();

        return;
    }

    // Loop over events
    Long64_t nEntries = t_main->GetEntries();

    for (Long64_t i = 0; i < nEntries; ++i)
    {
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
        if (kCuts)
            for (uint j = 0; j <= 10; j++)
                TMVACutsScore.push_back(reader->EvaluateMVA("Cuts", (float)j / 10.0));
        // BDTG
        if (kBDTG)
            TMVABDTGScore = reader->EvaluateMVA("BDTG");
        // HMatrix
        if (kHMatrix)
            TMVAHMatrixScore = reader->EvaluateMVA("HMatrix");
        // Fisher
        if (kFisher)
            TMVAFisherScore = reader->EvaluateMVA("Fisher");
        // MLP
        if (kMLP)
            TMVAMLPScore = reader->EvaluateMVA("MLP");
        // SVM
        if (kSVM)
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

    return;
}