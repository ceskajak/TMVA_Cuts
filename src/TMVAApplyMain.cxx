#include "../headers/TMVAApply.hxx"

int main(int argc, char **argv)
{
    SetProjectRootAsCwd();
    InstallRootWarningFilter();

    TString varset = "m2v";
    if (argc > 1)
        varset = argv[1];

    return TMVAApply(varset) ? 0 : 1;
}
