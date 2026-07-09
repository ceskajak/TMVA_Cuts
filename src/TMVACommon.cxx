#include "../headers/TMVACommon.hxx"

TFile *f_in = nullptr;
TFile *f_out = nullptr;

TTree *t_main = nullptr;
TTree *t_out = nullptr;

TTree *t_sg = nullptr;
TTree *t_bg = nullptr;

TString str_sgCuts;
TString str_bgCuts;

const double m_sig_lo = 8.8;
const double m_sig_hi = 11.1;

bool kMass = true;
bool kCharge = false;

std::vector<TString> vec_TMVAMethods = {
    "Cuts",
    "BDTG",
    "HMatrix",
    "Fisher",
    "MLP",
    "SVM"};

std::set<TString> activeMethodNames(vec_TMVAMethods.begin(), vec_TMVAMethods.end());

TMVA::Factory *factory = nullptr;
TMVA::DataLoader *dataloader = nullptr;
TMVA::Reader *reader = nullptr;

TString weightPath = "dataset/weights/";

bool k2v = false;
bool k5v = false;
bool k6v = false;
bool k16v = false;

std::vector<float> TMVACutsScore;
float TMVABDTGScore = 0.0f;
float TMVAHMatrixScore = 0.0f;
float TMVAFisherScore = 0.0f;
float TMVAMLPScore = 0.0f;
float TMVASVMScore = 0.0f;

std::vector<TString> vec_inputVariables = {
    "max(abs(bemcZDist1), abs(bemcZDist2))",
    "max(abs(bemcPhiDist1), abs(bemcPhiDist2))",
    "max(abs(DCA1), abs(DCA2))"};

void TMVACleaner()
{
    if (reader)
    {
        delete reader;
        reader = nullptr;
    }

    if (factory)
    {
        delete factory;
        factory = nullptr;
    }

    if (dataloader)
    {
        delete dataloader;
        dataloader = nullptr;
    }

    if (t_sg)
    {
        delete t_sg;
        t_sg = nullptr;
    }

    if (t_bg)
    {
        delete t_bg;
        t_bg = nullptr;
    }

    t_main = nullptr;

    if (t_out)
    {
        delete t_out;
        t_out = nullptr;
    }

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
}
