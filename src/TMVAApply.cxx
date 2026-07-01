#include "../headers/TMVAApply.hxx"

void TMVAApply(TString strVarset = "m2v")
{
    // Set the dataset path for TMVA weights
    weightPath += strVarset;

    // Set the paths to the TMVA weight files based on the variable set
    SetWeightPaths();

    // Open input file
    fin = TFile::Open("data/main.root");
    tree = (TTree*)fin->Get("tree");

    // All variable categories are subsets
    AdjustVariableFlags();

    // Base cuts: BEMC validation and Vz difference (no DZ/Dphi restrictions here)
    cout << "Applying base cuts (same used in training)..." << endl;
    cout << endl;

    TString baseSelection = CutGenerator::GetBaseCuts();

    cout << "Base selection for TMVA evaluation: " << baseSelection << endl;
    cout << endl;

    TTree * cutTree = tree->CopyTree(baseSelection);
    cout << "Cut tree created with selection: " << baseSelection << endl;
    cout << "Cut tree entries: " << cutTree->GetEntries() << endl;
    cout << endl;

    // Connect tree branches
    cutTree->SetBranchAddress("m", &var_m);
    cutTree->SetBranchAddress("charge", &var_charge);
    cutTree->SetBranchAddress("y", &var_y);
    cutTree->SetBranchAddress("nSigmaElectron1", &var_nSigmaElectron1);
    cutTree->SetBranchAddress("nSigmaElectron2", &var_nSigmaElectron2);
    cutTree->SetBranchAddress("dEdx1", &var_dEdx1);
    cutTree->SetBranchAddress("dEdx2", &var_dEdx2);
    cutTree->SetBranchAddress("eta1", &var_eta1);
    cutTree->SetBranchAddress("eta2", &var_eta2);
    cutTree->SetBranchAddress("phi1", &var_phi1);
    cutTree->SetBranchAddress("phi2", &var_phi2);
    cutTree->SetBranchAddress("invBeta1", &var_invBeta1);
    cutTree->SetBranchAddress("invBeta2", &var_invBeta2);
    cutTree->SetBranchAddress("E_TOW1", &var_E_TOW1);
    cutTree->SetBranchAddress("E_CLU1", &var_E_CLU1);
    cutTree->SetBranchAddress("E_TOW2", &var_E_TOW2);
    cutTree->SetBranchAddress("E_CLU2", &var_E_CLU2);
    cutTree->SetBranchAddress("p1", &var_p1);
    cutTree->SetBranchAddress("p2", &var_p2);
    cutTree->SetBranchAddress("pt1", &var_pt1);
    cutTree->SetBranchAddress("pt2", &var_pt2);
    cutTree->SetBranchAddress("adc1", &var_adc1);
    cutTree->SetBranchAddress("adc2", &var_adc2);
    cutTree->SetBranchAddress("bemcZDist1", &var_bemcZDist1);
    cutTree->SetBranchAddress("bemcZDist2", &var_bemcZDist2);
    cutTree->SetBranchAddress("bemcPhiDist1", &var_bemcPhiDist1);
    cutTree->SetBranchAddress("bemcPhiDist2", &var_bemcPhiDist2);
    cutTree->SetBranchAddress("VzTPC", &var_VzTPC);
    cutTree->SetBranchAddress("VzVPD", &var_VzVPD);
    cutTree->SetBranchAddress("nHitsFit1", &var_nHitsFit1);
    cutTree->SetBranchAddress("nHitsMax1", &var_nHitsMax1);
    cutTree->SetBranchAddress("nHitsFit2", &var_nHitsFit2);
    cutTree->SetBranchAddress("nHitsMax2", &var_nHitsMax2);
    cutTree->SetBranchAddress("DCA1", &var_DCA1);
    cutTree->SetBranchAddress("DCA2", &var_DCA2);

    // Register variables in EXACTLY the same way as during training
    if (k16v)
    {
        reader.AddVariable("dEdx1", &var_dEdx1);
        reader.AddVariable("dEdx2", &var_dEdx2);
        reader.AddVariable("eta1", &var_eta1);
        reader.AddVariable("eta2", &var_eta2);
        reader.AddVariable("phi1", &var_phi1);
        reader.AddVariable("phi2", &var_phi2);
        //reader.AddVariable("invBeta1", &var_invBeta1);
        //reader.AddVariable("invBeta2", &var_invBeta2);
        reader.AddVariable("E_TOW1", &var_E_TOW1);
        reader.AddVariable("E_CLU1", &var_E_CLU1);
        reader.AddVariable("E_TOW2", &var_E_TOW2);
        reader.AddVariable("E_CLU2", &var_E_CLU2);
    }
    if (k5v)
    {
        reader.AddVariable("min(E_TOW1/E_CLU1, E_TOW2/E_CLU2)", &var_min_ETOWoverECLU);
        reader.AddVariable("max(E_CLU1/p1, E_CLU2/p2)", &var_max_ECLUoverP);
        reader.AddVariable("min(E_CLU1/p1, E_CLU2/p2)", &var_min_ECLUoverP);
    }
    if (k2v)
    {
        reader.AddVariable("max(abs(bemcZDist1), abs(bemcZDist2))", &var_max_abs_bemcZDist);
        reader.AddVariable("max(abs(bemcPhiDist1), abs(bemcPhiDist2))", &var_max_abs_bemcPhiDist);
    }
    if (k6v)
    {
        reader.AddVariable("max(abs(DCA1), abs(DCA2))", &var_max_abs_DCA);
    }

    

    BookIfValid("Cuts", strCutsWeights, kApplyCuts);
    BookIfValid("BDTG", strBDTGWeights, kApplyBDTG);
    BookIfValid("HMatrix", strHMatrixWeights, kApplyHMatrix);
    BookIfValid("Fisher", strFisherWeights, kApplyFisher);
    BookIfValid("MLP", strMLPWeights, kApplyMLP);
    BookIfValid("SVM", strSVMWeights, kApplySVM);
    
    // Create output file
    TFile * fout = TFile::Open(weightPath + "/tree_TMVA-processed.root", "RECREATE");

    // Clone tree structure
    TTree * outTree = cutTree->CloneTree(0);

    // New branch for MLP response
    if (kApplyCuts)
        outTree->Branch("TMVACutsScore", &TMVACutsScore, "TMVACutsScore/F");
    if (kApplyBDTG)
        outTree->Branch("TMVABDTGScore", &TMVABDTGScore, "TMVABDTGScore/F");
    if (kApplyHMatrix)
        outTree->Branch("TMVAHMatrixScore", &TMVAHMatrixScore, "TMVAHMatrixScore/F");
    if (kApplyFisher)
        outTree->Branch("TMVAFisherScore", &TMVAFisherScore, "TMVAFisherScore/F");
    if (kApplyMLP)
        outTree->Branch("TMVAMLPScore", &TMVAMLPScore, "TMVAMLPScore/F");
    if (kApplySVM)
        outTree->Branch("TMVASVMScore", &TMVASVMScore, "TMVASVMScore/F");

    // Loop over events
    Long64_t nEntries = cutTree->GetEntries();

    for (Long64_t i = 0; i < nEntries; ++i)
    {
        // Load entry
        cutTree->GetEntry(i);

        // Calculate derived variables
        var_min_ETOWoverECLU = min(var_E_TOW1 / var_E_CLU1, var_E_TOW2 / var_E_CLU2);
        var_max_ECLUoverP = max(var_E_CLU1 / var_p1, var_E_CLU2 / var_p2);
        var_min_ECLUoverP = min(var_E_CLU1 / var_p1, var_E_CLU2 / var_p2);
        var_max_abs_bemcZDist = max(abs(var_bemcZDist1), abs(var_bemcZDist2));
        var_max_abs_bemcPhiDist = max(abs(var_bemcPhiDist1), abs(var_bemcPhiDist2));
        var_max_abs_DCA = max(abs(var_DCA1), abs(var_DCA2));

        // Evaluate MLP methods
        // Cuts
        if (kApplyCuts)
            for (uint j = 0; j <= 10; j++)
            {
                TMVACutsScore.push_back(reader.EvaluateMVA("Cuts", (float)j/10.0));
            }
        // BDTG
        if (kApplyBDTG)
            TMVABDTGScore = reader.EvaluateMVA("BDTG");
        // HMatrix
        if (kApplyHMatrix)
            TMVAHMatrixScore = reader.EvaluateMVA("HMatrix");
        // Fisher
        if (kApplyFisher)
            TMVAFisherScore = reader.EvaluateMVA("Fisher");
        // MLP
        if (kApplyMLP)
            TMVAMLPScore = reader.EvaluateMVA("MLP");
        // SVM
        if (kApplySVM)
            TMVASVMScore = reader.EvaluateMVA("SVM");

        outTree->Fill();
    }

    // Write output
    outTree->Write();

    //TString TMVACuts = "MLPScore > 0.5"; // Example cut on MLP score for evaluation

    //FitResultMetrics metrics = EvaluateCandidate(outTree, TMVACuts, "response");

    fout->Close();
    fin->Close();

    return;
}