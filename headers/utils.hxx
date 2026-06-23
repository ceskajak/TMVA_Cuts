#pragma once

#include "common.hxx"

/*
/////////////////////////////////////////////////////////////
DATA STRUCTURES
/////////////////////////////////////////////////////////////
*/
struct FitResultMetrics {
    Double_t S;
    Double_t B;
    Double_t S_over_sqrtSB;
    Double_t yield;
    Double_t yieldErr;
    Double_t US_entries;
    Double_t LS_entries;
    Double_t chi2ndf_US;
    Double_t chi2ndf_LS;
};

// Structure to hold cut variables, values and signal efficiency
struct CutCandidate
{
    vector<TString> varNames;
    vector<double> cutMin;
    vector<double> cutMax;
    double effS;
};

/*
/////////////////////////////////////////////////////////////
FUNCTIONS
/////////////////////////////////////////////////////////////
*/
// Curent date (YYMMDD) for output purposes
static TString init_date_string()
{
    time_t timestamp = time(NULL);
    struct tm datetime = *localtime(&timestamp);

    char output[6];
    strftime(output, 8, "%y%m%d", & datetime);
    return TString(output);
}

// Function to build a cut expression from a CutCandidate
inline TString BuildCutExpression(const CutCandidate &cand)
{
    // Build a cut expression like "var1 > cutMin1 && var1 < cutMax1 && var2 > cutMin2 && var2 < cutMax2 ..."
    TString expr;
    for (size_t iv = 0; iv < cand.varNames.size(); ++iv)
    {
        TString var = cand.varNames[iv];
        double lo = cand.cutMin[iv];
        double hi = cand.cutMax[iv];
        if (lo > -1e8)
        {
            TString term = Form("%s > %.8g", var.Data(), lo);
            if (expr.Length())
                expr += " && ";
            expr += term;
        }
        if (hi < 1e8)
        {
            TString term = Form("%s < %.8g", var.Data(), hi);
            if (expr.Length())
                expr += " && ";
            expr += term;
        }
    }
    return expr;
}