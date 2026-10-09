// Prints 11 decile cut values (0.0 .. 1.0, one per line) for a scalar score
// branch in the "tree" TTree of a TMVAApply-produced file. Used to turn a
// method's raw score range into evenly-populated cut-points, since methods
// like BDTG/Fisher/MLP/SVM/HMatrix don't share a common [0,1] score scale.
void score_deciles(const char *file, const char *branch)
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

    const Long64_t n = t->Draw(branch, "", "goff");
    if (n <= 0)
    {
        std::cerr << "ERROR: no entries drawn for branch " << branch << std::endl;
        return;
    }

    std::vector<double> vals(t->GetV1(), t->GetV1() + n);
    std::sort(vals.begin(), vals.end());

    for (int i = 0; i <= 10; ++i)
    {
        const double prob = i / 10.0;
        Long64_t idx = static_cast<Long64_t>(prob * (n - 1));
        printf("%.6f\n", vals[idx]);
    }
}
