#include "../headers/utils.hxx"
#include "TKey.h"
#include "TError.h"

#include <cstring>

namespace
{
std::unique_ptr<TFile> gBaseTreeFile;
ErrorHandlerFunc_t gPreviousErrorHandler = nullptr;

TTree *ResolveTree(TFile *file)
{
    if (!file)
        return nullptr;

    if (TTree *tree = static_cast<TTree *>(file->Get("tree")))
        return tree;

    if (TTree *tree = static_cast<TTree *>(file->Get("UpsTree")))
        return tree;

    TIter next(file->GetListOfKeys());
    while (TObject *obj = next())
    {
        TKey *key = dynamic_cast<TKey *>(obj);
        if (!key)
            continue;

        TObject *candidate = key->ReadObj();
        TTree *tree = dynamic_cast<TTree *>(candidate);
        if (tree)
            return tree;
        delete candidate;
    }

    return nullptr;
}

void FilteredRootErrorHandler(Int_t level, Bool_t abort, const char *location, const char *msg)
{
    if (level == kWarning && msg && std::strstr(msg, "no dictionary for class StUpsCandidate is available"))
        return;

    if (gPreviousErrorHandler)
    {
        gPreviousErrorHandler(level, abort, location, msg);
        return;
    }

    DefaultErrorHandler(level, abort, location, msg);
}
}

TString init_date_string()
{
    const time_t timestamp = time(NULL);
    const struct tm datetime = *localtime(&timestamp);

    char output[8];
    strftime(output, sizeof(output), "%y%m%d", &datetime);
    return TString(output);
}

TTree *GetBaseTree()
{
    const TString basePath = "data/cands+base.root";
    const TString mainPath = "data/main.root";

    if (gSystem->AccessPathName(basePath))
    {
        std::cout << "Preprocessed dataset not found. Creating..." << std::endl;

        std::unique_ptr<TFile> inputFile(TFile::Open(mainPath, "READ"));
        if (!inputFile || inputFile->IsZombie())
        {
            std::cerr << "ERROR: Failed to open input file: " << mainPath << std::endl;
            return nullptr;
        }

        TTree *inputTree = ResolveTree(inputFile.get());
        if (!inputTree)
        {
            std::cerr << "ERROR: Could not find any TTree in input file." << std::endl;
            return nullptr;
        }

        std::unique_ptr<TFile> outputFile(TFile::Open(basePath, "RECREATE"));
        if (!outputFile || outputFile->IsZombie())
        {
            std::cerr << "ERROR: Failed to create preprocessed file: " << basePath << std::endl;
            return nullptr;
        }

        TTree *processedTree = inputTree->CopyTree(CutGenerator::GetBaseCuts());
        if (!processedTree)
        {
            std::cerr << "ERROR: Failed to build preprocessed tree." << std::endl;
            return nullptr;
        }

        outputFile->cd();
        processedTree->Write("tree", TObject::kOverwrite);
        outputFile->Write();
    }
    else
    {
        std::cout << "Preprocessed dataset found. Loading..." << std::endl;
    }

    gBaseTreeFile.reset(TFile::Open(basePath, "READ"));
    if (!gBaseTreeFile || gBaseTreeFile->IsZombie())
    {
        std::cerr << "ERROR: Failed to open preprocessed file: " << basePath << std::endl;
        gBaseTreeFile.reset();
        return nullptr;
    }

    TTree *baseTree = ResolveTree(gBaseTreeFile.get());
    if (!baseTree)
    {
        std::cerr << "ERROR: Could not find tree in preprocessed file." << std::endl;
        return nullptr;
    }

    return baseTree;
}

TString BuildCutExpression(const CutCandidate &cand)
{
    TString expr;
    for (size_t iv = 0; iv < cand.varNames.size(); ++iv)
    {
        const TString &var = cand.varNames[iv];
        const double lo = cand.cutMin[iv];
        const double hi = cand.cutMax[iv];

        if (lo > -1e8)
        {
            const TString term = Form("%s > %.8g", var.Data(), lo);
            if (expr.Length())
                expr += " && ";
            expr += term;
        }
        if (hi < 1e8)
        {
            const TString term = Form("%s < %.8g", var.Data(), hi);
            if (expr.Length())
                expr += " && ";
            expr += term;
        }
    }
    return expr;
}

bool CheckFileExists(const TString &filename)
{
    return gSystem->AccessPathName(filename) == 0;
}

void InstallRootWarningFilter()
{
    if (!gPreviousErrorHandler)
    {
        gPreviousErrorHandler = GetErrorHandler();
        SetErrorHandler(FilteredRootErrorHandler);
    }
}
