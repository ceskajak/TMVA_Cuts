#include "../headers/TMVATrain.hxx"

int main()
{
    SetProjectRootAsCwd();
    InstallRootWarningFilter();
    return TMVATrain() ? 0 : 1;
}
