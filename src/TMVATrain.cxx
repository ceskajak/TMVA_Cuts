#include "../headers/TMVATrain.hxx"

// Main function to perform TMVA optimization of cuts
void TMVATrain()
{
    cout << "Starting TMVA cut optimization..." << endl;
    cout << endl;

    // Ensure output directory exists
    if (!gSystem->AccessPathName("output"))
    {
        gSystem->mkdir("output/tmva", true);
    }
    else
    {
        gSystem->mkdir("output/tmva", true);
    }

    // Initialize TMVA tools (needed for standalone execution)
    TMVA::Tools::Instance();

    // Compile the fitter helper if needed.
    //gROOT->ProcessLine(".L UpsFitter.cxx+");

    // Open the input file and get the tree
    tree = GetBaseTree();

    // Ensure output file is open before creating CopyTree trees so
    // copied TTrees are written to disk instead of being memory-resident.
    TFile *tmvaOutput = TFile::Open("output/tmva/TMVAOutput.root", "RECREATE");

    // Prevent undefined behaviour
    if (useMassCuts && useChargeCuts)
    {
        cerr << "ERROR: Multiple S/B criteria selected, undefined behaviour!" << endl;
        cerr << "Quitting..." << endl;
        cerr << "(Hint: Only one switch in `../headers/TMVATrain.hxx` can be active at once.)" << endl;
        fin->Close();
        return;
    }
    // mass-based
    else if (useMassCuts)
    {
        signalSelection = Form("charge==0 && m > %f && m < %f", m_sig_lo, m_sig_hi);
        backgroundSelection = Form("charge==0 && ((m > %f && m < %f) || (m > %f && m < %f))", min_mass, m_sig_lo, m_sig_hi, max_mass);
    }

    // US/LS-based
    else if (useChargeCuts)
    {
        signalSelection = "charge==0";
        backgroundSelection = "charge!=0";
    }
    else
    {
        cerr << "ERROR: No S/B criteria selected, undefined behaviour!" << endl;
        cerr << "Quitting..." << endl;
        cerr << "(Hint: Turn on a switch in `../headers/TMVATrain.hxx`.)" << endl;
        fin->Close();
        return;
    }

    // Create TTrees for signal and background using CopyTree with the respective selections
    TTree * signalTree = tree->CopyTree(signalSelection);
    cout << "Signal tree created with selection: " << signalSelection << endl;
    cout << "Signal tree entries: " << signalTree->GetEntries() << endl;
    cout << endl;

    TTree * backgroundTree = tree->CopyTree(backgroundSelection);
    cout << "Background tree created with selection: " << backgroundSelection << endl;
    cout << "Background tree entries: " << backgroundTree->GetEntries() << endl;
    cout << endl;

    // Check if trees were created successfully
    if (!signalTree || !backgroundTree)
    {
        cerr << "ERROR: Failed to create TMVA S/B trees" << endl;
        if (signalTree)
            delete signalTree;
        if (backgroundTree)
            delete backgroundTree;
        fin->Close();
        return;
    }

    // Prepare TMVA DataLoader with the selected variables and trees
    cout << "Setting up TMVA DataLoader and Factory..." << endl;

    TMVA::DataLoader *dataloader = new TMVA::DataLoader("dataset");
    vector<TString> variables = GetTMVAInputVariables();
    for (auto &var : variables)
    {
        dataloader->AddVariable(var, 'F');
    }

    // Add signal and background trees to the dataloader with equal weights
    dataloader->AddSignalTree(signalTree, 1.0);
    dataloader->AddBackgroundTree(backgroundTree, 1.0);

    // Prepare training and testing trees (no additional cuts here since we already applied them in the CopyTree step)
    dataloader->PrepareTrainingAndTestTree("", "SplitMode=Random:NormMode=NumEvents:!V");


    // Prepare TMVA Factory
    TMVA::Factory *factory = new TMVA::Factory("TMVAOptimizeCuts", tmvaOutput, "!V:!Silent:Color:DrawProgressBar");

    // Book methods with options
    cout << "Booking TMVA methods..." << endl;
    if (!kTrainCuts && !kTrainBDTG && !kTrainHMatrix && !kTrainFisher && !kTrainMLP && !kTrainSVM)
    {
        cerr << "ERROR: No TMVA methods booked!" << endl;
        cerr << "Quitting..." << endl;
        cerr << "(Hint: Turn on a switch in `../headers/TMVATrain.hxx`.)" << endl;
        if (signalTree)
            delete signalTree;
        if (backgroundTree)
            delete backgroundTree;
        fin->Close();
        return;
    }
    // Cuts method options: use Genetic Algorithm for optimization, enable efficiency selection, and use "FSmart" variable selection for cuts
    if (kTrainCuts)
        factory->BookMethod(dataloader,
            TMVA::Types::kCuts,
            "Cuts",
            "!H:!V:FitMethod=GA:EffSel:VarProp=FSmart"
        );

    // BDTG method options: 800 trees, max depth of 3, gradient boosting with shrinkage, bagging with 50% sample fraction
    if (kTrainBDTG)
        factory->BookMethod(dataloader,
            TMVA::Types::kBDT,
            "BDTG",
            "!H:!V:"
            "NTrees=800:"
            "MaxDepth=3:"
            "BoostType=Grad:"
            "Shrinkage=0.05:"
            "UseBaggedBoost:"
            "BaggedSampleFraction=0.5"
        );

    // HMatrix method options: no additional options for now
    if (kTrainHMatrix)
        factory->BookMethod(dataloader,
            TMVA::Types::kHMatrix,
            "HMatrix",
            "!H:!V"
        );

    // Fisher method options: create PDFs for the MVA output using spline interpolation, with 60 bins and smoothing over 10 bins
    if (kTrainFisher)
        factory->BookMethod(dataloader,
            TMVA::Types::kFisher,
            "Fisher",
            "!H:!V:Fisher:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=60:NsmoothMVAPdf=10"
        );

    // MLP method options: use a multilayer perceptron with tanh activation, no variable transformation, 600 training cycles, and a hidden layer with N+5 neurons (where N is the number of input variables)
    if (kTrainMLP)
        factory->BookMethod(dataloader,
            TMVA::Types::kMLP,
            "MLP",
            //"!H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5"
            "!H:!V:NeuronType=tanh:VarTransform=N:NCycles=1000:HiddenLayers=N+10:TestRate=5"
        );

    // SVM method options: use a support vector machine with a radial basis function kernel, gamma parameter of 0.25, and a tolerance of 0.001 for convergence
    if (kTrainSVM)
        factory->BookMethod(dataloader,
            TMVA::Types::kSVM,
            "SVM",
        "!H:!V:Gamma=0.25:Tol=0.001"
        );

    cout << "TMVA Factory and DataLoader set up successfully!" << endl;

    // Train, test, and evaluate all methods; this will perform the optimization for the Cuts method and train the other methods for comparison
    cout << "Training..." << endl;
    factory->TrainAllMethods();
    cout << "Testing..." << endl;
    factory->TestAllMethods();
    cout << "Evaluating..." << endl;
    factory->EvaluateAllMethods();
    cout << "TMVA training/testing/evaluation complete!" << endl;
    cout << endl;

    cout << "Signal and background trees prepared with the following selections:" << endl;
    cout << " - Signal: " << signalSelection << endl;
    cout << " - Background: " << backgroundSelection << endl;
    cout << endl;

    cout << "Used variables for TMVA optimization: " << endl;
    for (const auto &var : variables)
    {
        cout << " - " << var << endl;
    }
    cout << endl;

    /*
    cout << "Exitting..." << endl;
    return;
    */

    // Extract the optimized cuts from the Cuts method and evaluate the resulting cut candidate on the original dataset to get final metrics
    TMVA::IMethod * im = factory->GetMethod("dataset", "Cuts");
    TMVA::MethodCuts *cutMethod = TMVA::MethodCuts::DynamicCast(im);
    if (!cutMethod)
    {
        cerr << "ERROR: TMVA Cuts method not available" << endl;
        delete factory;
        delete dataloader;
        delete signalTree;
        delete backgroundTree;
        fin->Close();
        return;
    }

    // Define a range of signal efficiency targets to evaluate multiple cut candidates along the efficiency curve,
    // and select the best one based on S/sqrt(S+B) and yield
    vector<double> efficiencies;
    for (int i = 1; i <= 9; ++i)
        efficiencies.push_back(i * 0.1);

    // Variables to track the best candidate and its metrics
    double bestYield = -1;
    double bestSig = -1;
    CutCandidate bestCandidate;
    FitResultMetrics bestMetrics = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    // Loop over the defined signal efficiency targets, get the corresponding cuts from TMVA, evaluate each candidate, and keep track of the best one
    for (double effS : efficiencies)
    {
        vector<double> cutMin(variables.size());
        vector<double> cutMax(variables.size());
        double effB = cutMethod->GetCuts(effS, cutMin, cutMax);
        CutCandidate cand{variables, cutMin, cutMax, effS};

        TString tmvaCutExpr = BuildCutExpression(cand);
        TString candidateCuts = tmvaCutExpr;

        cout << "Evaluating candidate with signal efficiency target: " << effS << endl;
        cout << "TMVA estimated background efficiency: " << effB << endl;
        cout << "TMVA cut expression: " << tmvaCutExpr << endl;

        TString label = Form("tmva_eff%.0f", effS * 100);
        FitResultMetrics metrics = EvaluateCandidate(tree, candidateCuts, label);

        cout << "Candidate effS=" << effS
             << ", yield=" << metrics.yield
             << ", S/sqrt(S+B)=" << metrics.S_over_sqrtSB << endl;

        if (metrics.S_over_sqrtSB > bestSig + 1e-6 ||
            (fabs(metrics.S_over_sqrtSB - bestSig) < 1e-6 && metrics.yield > bestYield))
        {
            bestYield = metrics.yield;
            bestSig = metrics.S_over_sqrtSB;
            bestCandidate = cand;
            bestMetrics = metrics;
        }
    }

    cout << "Cleaning up TMVA resources..." << endl;

    // Close and delete the output file first to ensure any TMVA-written
    // streams are finalized before destroying TMVA objects.
    if (tmvaOutput)
    {
        tmvaOutput->Close();
        delete tmvaOutput;
        tmvaOutput = nullptr;
    }

    // Now delete TMVA objects
    delete factory;
    delete dataloader;

    // Delete copied trees
    signalTree->Print();
    delete signalTree;
    delete backgroundTree;

    // Close the input file
    fin->Close();

    cout << "TMVATrain cleanup complete!" << endl;
}
