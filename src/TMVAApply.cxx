#include "../headers/TMVAApply.hxx"

void TMVAApply()
{
    // Open input file
    TFile *fin = TFile::Open("data/main.root");
    TTree *tree = (TTree*)fin->Get("tree");

    // Base cuts: BEMC validation and Vz difference (no DZ/Dphi restrictions here)
    cout << "Applying base cuts (same used in training)..." << endl;
    cout << endl;

    TString baseSelection = CutGenerator::GetBEMCValidCut();
    baseSelection = CutGenerator::CombineCuts(baseSelection, CutGenerator::GetVzDiffCut(9999.0));

    // Additional cuts to reduce the dataset for TMVA optimization; these are the "minimum cuts" that define the starting point for optimization
    TString minCuts = CutGenerator::CombineCuts(
        CutGenerator::GetRapidityCut(-2, 2),
        CutGenerator::GetNSigmaECut(nSigmaE_lo),
        CutGenerator::GetBEMCValidCut(),
        CutGenerator::GetETowEcluCut(EtowEclu),
        CutGenerator::GetEOverPCut(0.55, Eoverp_hi),
        CutGenerator::GetPtCut(d1_pT_lo),       // Assuming same pT cut for both electrons
        CutGenerator::GetADCCut(adc_trig_BHT2), // Using adc_trig for ONE electron (partner cut is off)
        CutGenerator::GetNHitsFitCut(nHitsFit),
        CutGenerator::GetNHitsRatioCut(nHitsRatio),
        CutGenerator::GetDCACut(3.0),
        CutGenerator::GetDZCut(20),
        CutGenerator::GetDphiCut(0.1));

    // Combine base cuts with additional cuts to form the full selection for TMVA optimization
    baseSelection = CutGenerator::CombineCuts(baseSelection, minCuts);

    cout << "Base selection for TMVA evaluation: " << baseSelection << endl;
    cout << endl;

    TTree * cutTree = tree->CopyTree(baseSelection);
    cout << "Cut tree created with selection: " << baseSelection << endl;
    cout << "Cut tree entries: " << cutTree->GetEntries() << endl;
    cout << endl;

    // Variables used by the MLP
    float m;
    short charge;
    float y;
    float nSigmaElectron1;
    float nSigmaElectron2;
    float dEdx1;
    float dEdx2;
    float eta1;
    float eta2;
    float phi1;
    float phi2;
    float invBeta1;
    float invBeta2;
    float E_TOW1;
    float E_CLU1;
    float E_TOW2;
    float E_CLU2;
    float p1;
    float p2;
    float pt1;
    float pt2;
    int adc1;
    int adc2;
    float bemcZDist1;
    float bemcZDist2;
    float bemcPhiDist1;
    float bemcPhiDist2;
    float VzTPC;
    float VzVPD;
    int nHitsFit1;
    int nHitsMax1;
    int nHitsFit2;
    int nHitsMax2;
    float DCA1;
    float DCA2;

    float min_ETOWoverECLU;
    float max_ECLUoverP;
    float min_ECLUoverP;
    float max_abs_bemcZDist;
    float max_abs_bemcPhiDist;
    float max_abs_DCA;

    // Connect tree branches
    cutTree->SetBranchAddress("m", &m);
    cutTree->SetBranchAddress("charge", &charge);
    cutTree->SetBranchAddress("y", &y);
    cutTree->SetBranchAddress("nSigmaElectron1", &nSigmaElectron1);
    cutTree->SetBranchAddress("nSigmaElectron2", &nSigmaElectron2);
    cutTree->SetBranchAddress("dEdx1", &dEdx1);
    cutTree->SetBranchAddress("dEdx2", &dEdx2);
    cutTree->SetBranchAddress("eta1", &eta1);
    cutTree->SetBranchAddress("eta2", &eta2);
    cutTree->SetBranchAddress("phi1", &phi1);
    cutTree->SetBranchAddress("phi2", &phi2);
    cutTree->SetBranchAddress("invBeta1", &invBeta1);
    cutTree->SetBranchAddress("invBeta2", &invBeta2);
    cutTree->SetBranchAddress("E_TOW1", &E_TOW1);
    cutTree->SetBranchAddress("E_CLU1", &E_CLU1);
    cutTree->SetBranchAddress("E_TOW2", &E_TOW2);
    cutTree->SetBranchAddress("E_CLU2", &E_CLU2);
    cutTree->SetBranchAddress("p1", &p1);
    cutTree->SetBranchAddress("p2", &p2);
    cutTree->SetBranchAddress("pt1", &pt1);
    cutTree->SetBranchAddress("pt2", &pt2);
    cutTree->SetBranchAddress("adc1", &adc1);
    cutTree->SetBranchAddress("adc2", &adc2);
    cutTree->SetBranchAddress("bemcZDist1", &bemcZDist1);
    cutTree->SetBranchAddress("bemcZDist2", &bemcZDist2);
    cutTree->SetBranchAddress("bemcPhiDist1", &bemcPhiDist1);
    cutTree->SetBranchAddress("bemcPhiDist2", &bemcPhiDist2);
    cutTree->SetBranchAddress("VzTPC", &VzTPC);
    cutTree->SetBranchAddress("VzVPD", &VzVPD);
    cutTree->SetBranchAddress("nHitsFit1", &nHitsFit1);
    cutTree->SetBranchAddress("nHitsMax1", &nHitsMax1);
    cutTree->SetBranchAddress("nHitsFit2", &nHitsFit2);
    cutTree->SetBranchAddress("nHitsMax2", &nHitsMax2);
    cutTree->SetBranchAddress("DCA1", &DCA1);
    cutTree->SetBranchAddress("DCA2", &DCA2);

    // Create TMVA reader
    TMVA::Reader reader("!Color:!Silent");

    // Register variables in EXACTLY the same way as during training
    reader.AddVariable("dEdx1", &dEdx1);
    reader.AddVariable("dEdx2", &dEdx2);
    reader.AddVariable("eta1", &eta1);
    reader.AddVariable("eta2", &eta2);
    reader.AddVariable("phi1", &phi1);
    reader.AddVariable("phi2", &phi2);
    //reader.AddVariable("invBeta1", &invBeta1);
    //reader.AddVariable("invBeta2", &invBeta2);
    reader.AddVariable("E_TOW1", &E_TOW1);
    reader.AddVariable("E_CLU1", &E_CLU1);
    reader.AddVariable("E_TOW2", &E_TOW2);
    reader.AddVariable("E_CLU2", &E_CLU2);
    reader.AddVariable("min(E_TOW1/E_CLU1, E_TOW2/E_CLU2)", &min_ETOWoverECLU);
    reader.AddVariable("max(E_CLU1/p1, E_CLU2/p2)", &max_ECLUoverP);
    reader.AddVariable("min(E_CLU1/p1, E_CLU2/p2)", &min_ECLUoverP);
    reader.AddVariable("max(abs(bemcZDist1), abs(bemcZDist2))", &max_abs_bemcZDist);
    reader.AddVariable("max(abs(bemcPhiDist1), abs(bemcPhiDist2))", &max_abs_bemcPhiDist);
    reader.AddVariable("max(abs(DCA1), abs(DCA2))", &max_abs_DCA);

    // Load trained network
    reader.BookMVA(
        "MLP",
        "dataset/weights/TMVAOptimizeCuts_MLP.weights.xml"
    );

    // Create output file
    TFile *fout = TFile::Open("output/tree_TMVA-processed.root", "RECREATE");

    // Clone tree structure
    TTree *outTree = cutTree->CloneTree(0);

    // New branch for MLP response
    float mlpScore;
    outTree->Branch("MLPScore", &mlpScore, "MLPScore/F");

    // Loop over events
    Long64_t nEntries = cutTree->GetEntries();

    for (Long64_t i = 0; i < nEntries; ++i)
    {
        cutTree->GetEntry(i);

        // Evaluate MLP
        mlpScore = reader.EvaluateMVA("MLP");

        outTree->Fill();
    }

    // Write output
    outTree->Write();

    TString TMVACuts = "MLPScore > 0.1"; // Example cut on MLP score for evaluation

    FitResultMetrics metrics = EvaluateCandidate(outTree, TMVACuts, "response");

    fout->Close();
    fin->Close();

    return;
}