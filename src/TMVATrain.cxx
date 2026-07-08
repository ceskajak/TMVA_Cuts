#include "../headers/TMVATrain.hxx"

bool GetSBCriteria()
{
    if (kMass && kCharge)
    {
        cerr << "ERROR: Multiple S/B criteria selected, undefined behaviour!" << endl;
        cerr << "Quitting..." << endl;
        cerr << "(Hint: Only one switch in `../headers/TMVATrain.hxx` can be active at once.)" << endl;
        f_in->Close();
        return 0;
    }
    else if (kMass)
    {
        str_sgCuts = Form("charge==0 && m > %f && m < %f", m_sig_lo, m_sig_hi);
        str_bgCuts = Form("charge==0 && ((m > %f && m < %f) || (m > %f && m < %f))", min_mass, m_sig_lo, m_sig_hi, max_mass);
    }
    else if (kCharge)
    {
        str_sgCuts = "charge==0";
        str_bgCuts = "charge!=0";
    }
    else
    {
        cerr << "ERROR: No S/B criteria selected, undefined behaviour!" << endl;
        cerr << "Quitting..." << endl;
        cerr << "(Hint: Turn on a switch in `../headers/TMVATrain.hxx`.)" << endl;
        f_in->Close();
        return 0;
    }

    t_sg = t_main->CopyTree(str_sgCuts);
    cout << "Signal tree created with selection: " << str_sgCuts << endl;
    cout << "Signal tree entries: " << t_sg->GetEntries() << endl;
    cout << endl;

    t_bg = t_main->CopyTree(str_bgCuts);
    cout << "Background tree created with selection: " << str_bgCuts << endl;
    cout << "Background tree entries: " << t_bg->GetEntries() << endl;
    cout << endl;

    if (!t_sg || !t_bg)
    {
        cerr << "ERROR: Failed to create TMVA S/B trees" << endl;
        if (t_sg)
            delete t_sg;
        if (t_bg)
            delete t_bg;

        f_in->Close();
        f_out->Close();

        delete f_in;
        delete f_out;

        return 0;
    }

    cout << "sg and bg trees created successfully!" << endl;
    return 1;
}

/////////////////////////////////////////////////////////////

bool BookFactory()
{
    cout << "Booking TMVA methods..." << endl;
    if (!kCuts && !kBDTG && !kHMatrix && !kFisher && !kMLP && !kSVM)
    {
        cerr << "ERROR: No TMVA methods booked!" << endl;
        cerr << "Quitting..." << endl;
        cerr << "(Hint: Turn on a switch in `../headers/TMVATrain.hxx`.)" << endl;
        if (t_sg)
            delete t_sg;
        if (t_bg)
            delete t_bg;
        f_in->Close();
        return 0;
    }

    if (kCuts)
        factory->BookMethod(dataloader,
            TMVA::Types::kCuts,
            "Cuts",
            "!H:"
            "!V:"
            "FitMethod=GA:"
            "EffSel:"
            "VarProp=FSmart"
        );

    if (kBDTG)
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

    if (kHMatrix)
        factory->BookMethod(dataloader,
            TMVA::Types::kHMatrix,
            "HMatrix",
            "!H:!V"
        );

    if (kFisher)
        factory->BookMethod(dataloader,
            TMVA::Types::kFisher,
            "Fisher",
            "!H:"
            "!V:"
            "Fisher:"
            "CreateMVAPdfs:"
            "PDFInterpolMVAPdf=Spline2:"
            "NbinsMVAPdf=60:"
            "NsmoothMVAPdf=10"
        );

    if (kMLP)
        factory->BookMethod(dataloader,
            TMVA::Types::kMLP,
            "MLP",
            "!H:"
            "!V:"
            "NeuronType=tanh:"
            "VarTransform=N:"
            "NCycles=1000:"
            "HiddenLayers=N+10:"
            "TestRate=5"
        );

    if (kSVM)
        factory->BookMethod(dataloader,
            TMVA::Types::kSVM,
            "SVM",
            "!H:"
            "!V:"
            "Gamma=0.25:"
            "Tol=0.001"
        );

    cout << "TMVA methods booked successfully!" << endl;
    return 1;
}

/////////////////////////////////////////////////////////////

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

    // Initialize TMVA tools (needed for standalone execution)
    TMVA::Tools::Instance();

    // Open the input file and get the tree
    t_main = GetBaseTree();

    // Ensure output file is open before creating CopyTree trees so
    // copied TTrees are written to disk instead of being memory-resident.
    f_out = TFile::Open("output/tmva/TMVAOutput.root", "RECREATE");

    // Fill signal and background trees based on the selected S/B criteria
    if (!GetSBCriteria())
    {
        cerr << "ERROR: Failed to get S/B criteria!" << endl;
        cerr << "Quitting..." << endl;

        TMVACleaner();

        return;
    }
    
    // Prepare TMVA DataLoader with the selected variables and trees
    cout << "Setting up TMVA DataLoader and Factory..." << endl;

    dataloader = new TMVA::DataLoader("dataset");
    for (auto &var : vec_inputVariables)
    {
        dataloader->AddVariable(var, 'F');
    }

    // Add signal and background trees to the dataloader with equal weights
    dataloader->AddSignalTree(t_sg, 1.0);
    dataloader->AddBackgroundTree(t_bg, 1.0);

    // Prepare training and testing trees
    dataloader->PrepareTrainingAndTestTree("",
                            "SplitMode=Random:"
                            "NormMode=NumEvents:"
                            "!V"
                            );


    // Prepare TMVA Factory
    factory = new TMVA::Factory("TMVAOptimizeCuts",
                                f_out,
                                "!V:"
                                "!Silent:"
                                "Color:"
                                "DrawProgressBar"
                                );

    // Book the TMVA methods based on the switches defined in the header
    if (!BookFactory())
    {
        cerr << "ERROR: Failed to book TMVA methods" << endl;
        cerr << "Quitting..." << endl;

        TMVACleaner();

        return;
    }

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

    /*
    cout << "Exitting..." << endl;
    return;
    */

    /*
    // Extract the optimized cuts from the Cuts method and evaluate the resulting cut candidate on the original dataset to get final metrics
    TMVA::IMethod * im = factory->GetMethod("dataset", "Cuts");
    TMVA::MethodCuts *cutMethod = TMVA::MethodCuts::DynamicCast(im);
    if (!cutMethod)
    {
        cerr << "ERROR: TMVA Cuts method not available" << endl;
        
        ~TMVATrain();

        return;
    }
    */

    // Clean-up
    TMVACleaner();
}