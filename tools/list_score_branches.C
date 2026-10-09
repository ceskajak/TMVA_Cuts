// Prints the names of all TMVA score branches (TMVA*Score) present in the
// "tree" TTree of a TMVAApply-produced file, one per line.
void list_score_branches(const char *file)
{
    std::unique_ptr<TFile> f(TFile::Open(file, "READ"));
    if (!f || f->IsZombie())
    {
        std::cerr << "ERROR: could not open " << file << std::endl;
        return;
    }

    TTree *t = static_cast<TTree *>(f->Get("tree"));
    if (!t)
    {
        std::cerr << "ERROR: could not find tree in " << file << std::endl;
        return;
    }

    TObjArray *branches = t->GetListOfBranches();
    for (int i = 0; i < branches->GetEntries(); ++i)
    {
        TString name = branches->At(i)->GetName();
        if (name.BeginsWith("TMVA") && name.EndsWith("Score"))
            std::cout << name << std::endl;
    }
}
