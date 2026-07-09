#include "../headers/TMVATrain.hxx"

int main()
{
    InstallRootWarningFilter();
    return TMVATrain() ? 0 : 1;
}
