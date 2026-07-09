#include "../headers/TMVATrain.hxx"

using namespace std;

namespace
{
bool IsMethodEnabled(const TString &method)
{
    return activeMethodNames.find(method) != activeMethodNames.end();
}
} // namespace

bool GetSBCriteria()
{
    if (!t_main)
    {
        cerr << "ERROR: Input tree is null. Cannot build S/B criteria." << endl;
        return 0;
    }

    if (kMass && kCharge)
    {
        cerr << "ERROR: Multiple S/B criteria selected, undefined behaviour!" << endl;
        cerr << "Quitting..." << endl;
        cerr << "(Hint: Only one switch in `../headers/TMVATrain.hxx` can be active at once.)" << endl;
        if (f_in)
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
        if (f_in)
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

    if (!t_sg || !t_bg || t_sg->GetEntries() == 0 || t_bg->GetEntries() == 0)
    {
        cerr << "ERROR: Failed to create non-empty TMVA S/B trees" << endl;
        if (t_sg)
            delete t_sg;
        if (t_bg)
            delete t_bg;

        t_sg = nullptr;
        t_bg = nullptr;

        if (f_in)
        {
            f_in->Close();
            delete f_in;
            f_in = nullptr;
        }
        if (f_out)
        {
            f_out->Close();
            delete f_out;
            f_out = nullptr;
        }

        return 0;
    }

    cout << "sg and bg trees created successfully!" << endl;
    return 1;
}

/////////////////////////////////////////////////////////////

bool BookFactory()
{
    cout << "Booking TMVA methods..." << endl;
    if (activeMethodNames.empty())
    {
        cerr << "ERROR: No TMVA methods booked!" << endl;
        cerr << "Quitting..." << endl;
        cerr << "(Hint: Turn on a switch in `../headers/TMVATrain.hxx`.)" << endl;
        if (t_sg)
            delete t_sg;
        if (t_bg)
            delete t_bg;
        if (f_in)
            f_in->Close();
        return 0;
    }

    if (IsMethodEnabled("Cuts"))
        factory->BookMethod(dataloader,
            TMVA::Types::kCuts,
            "Cuts",
            "!H:"
            "!V:"
            "FitMethod=GA:"
            "EffSel:"
            "VarProp=FSmart"
        );

    if (IsMethodEnabled("BDTG"))
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

    if (IsMethodEnabled("HMatrix"))
        factory->BookMethod(dataloader,
            TMVA::Types::kHMatrix,
            "HMatrix",
            "!H:!V"
        );

    if (IsMethodEnabled("Fisher"))
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

    if (IsMethodEnabled("MLP"))
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

    if (IsMethodEnabled("SVM"))
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
bool TMVATrain()
{
    cout << "Starting TMVA cut optimization..." << endl;
    cout << endl;

    // Ensure output directory exists
    if (gSystem->AccessPathName("output/tmva"))
    {
        gSystem->mkdir("output/tmva", true);
    }

    activeMethodNames = std::set<TString>(vec_TMVAMethods.begin(), vec_TMVAMethods.end());

    // Initialize TMVA tools (needed for standalone execution)
    TMVA::Tools::Instance();

    // Open the input file and get the tree
    t_main = GetBaseTree();
    if (!t_main)
    {
        cerr << "ERROR: Failed to get input tree." << endl;
        TMVACleaner();
        return false;
    }

    // Ensure output file is open before creating CopyTree trees so
    // copied TTrees are written to disk instead of being memory-resident.
    f_out = TFile::Open("output/tmva/TMVAOutput.root", "RECREATE");

    // Fill signal and background trees based on the selected S/B criteria
    if (!GetSBCriteria())
    {
        cerr << "ERROR: Failed to get S/B criteria!" << endl;
        cerr << "Quitting..." << endl;

        TMVACleaner();

        return false;
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
                                "Silent:"
                                "Color:"
                                "DrawProgressBar"
                                );

    // Book the TMVA methods based on the switches defined in the header
    if (!BookFactory())
    {
        cerr << "ERROR: Failed to book TMVA methods" << endl;
        cerr << "Quitting..." << endl;

        TMVACleaner();

        return false;
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

    // Clean-up
    TMVACleaner();
    return true;
}