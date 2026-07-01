#pragma once

#include "cuts.hxx"

// Class to generate cuts based on upsCand structure values
class CutGenerator
{
public:
    // Individual cut generators returning TString
    static TString GetRapidityCut(Float_t CUT_y_lo = -1.0, Float_t CUT_y_hi = 1.0)
    {
        return TString::Format("y > %f && y < %f", CUT_y_lo, CUT_y_hi);
    }

    static TString GetNSigmaECut(Float_t CUT_nSigmaE_lo = -1.2)
    {
        return TString::Format("nSigmaElectron1 >= %f && nSigmaElectron2 >= %f", CUT_nSigmaE_lo, CUT_nSigmaE_lo);
    }

    static TString GetBEMCValidCut()
    {
        return TString("E_TOW1 > 0 && E_CLU1 > 0 && E_TOW2 > 0 && E_CLU2 > 0");
    }

    static TString GetETowEcluCut(Float_t CUT_EtowEclu = 0.5)
    {
        return TString::Format("E_TOW1/E_CLU1 >= %f && E_TOW2/E_CLU2 >= %f", CUT_EtowEclu, CUT_EtowEclu);
    }

    static TString GetEOverPCut(Float_t CUT_Eoverp_lo = 0.55, Float_t CUT_Eoverp_hi = 1.45)
    {
        return TString::Format("E_CLU1/p1 > %f && E_CLU1/p1 < %f && E_CLU2/p2 > %f && E_CLU2/p2 < %f",
                               CUT_Eoverp_lo, CUT_Eoverp_hi, CUT_Eoverp_lo, CUT_Eoverp_hi);
    }

    static TString GetPtCut(Float_t CUT_pt_lo = 1.0)
    {
        return TString::Format("pt1 > %f && pt2 > %f", CUT_pt_lo, CUT_pt_lo);
    }

    static TString GetADCCut(Int_t CUT_adc_threshold = 25)
    {
        // ROOT formula parser does not support bitshift operations on floating-point variables.
        // Convert the shifted-threshold into the equivalent raw ADC threshold.
        return TString::Format("(adc1 >= %f || adc2 >= %f)", CUT_adc_threshold * 16.0, CUT_adc_threshold * 16.0);
    }

    static TString GetDZCut(Float_t CUT_DZ = 5.0)
    {
        return TString::Format("abs(bemcZDist1) < %f && abs(bemcZDist2) < %f", CUT_DZ, CUT_DZ);
    }

    static TString GetDphiCut(Float_t CUT_Dphi = 0.02)
    {
        return TString::Format("abs(bemcPhiDist1) < %f && abs(bemcPhiDist2) < %f", CUT_Dphi, CUT_Dphi);
    }

    static TString GetNHitsFitCut(Float_t CUT_nHitsFit = 20.0)
    {
        return TString::Format("nHitsFit1 >= %f && nHitsFit2 >= %f", CUT_nHitsFit, CUT_nHitsFit);
    }

    static TString GetNHitsRatioCut(Float_t CUT_nHitsRatio = 0.52)
    {
        // Avoid C-style cast syntax and ensure denominator is positive for RooFormula.
        return TString::Format("nHitsMax1 > 0 && nHitsFit1/nHitsMax1 > %f && nHitsMax2 > 0 && nHitsFit2/nHitsMax2 > %f",
                               CUT_nHitsRatio, CUT_nHitsRatio);
    }

    static TString GetDCACut(Float_t CUT_DCA = 1.0)
    {
        return TString::Format("abs(DCA1) < %f && abs(DCA2) < %f", CUT_DCA, CUT_DCA);
    }

    static TString GetVzDiffCut(Float_t CUT_VzDiff = 9999.0)
    {
        return TString::Format("abs(VzTPC - VzVPD) < %f", CUT_VzDiff);
    }

    // Function to append cuts together
    static TString AppendCuts(const TString &cut1, const TString &cut2)
    {
        if (cut1.Length() == 0)
            return cut2;
        if (cut2.Length() == 0)
            return cut1;
        TString combined = cut1;
        combined += " && ";
        combined += cut2;
        return combined;
    }

    // Variadic-like function to combine multiple cuts
    static TString CombineCuts(const TString &cut1, const TString &cut2 = "", const TString &cut3 = "",
                               const TString &cut4 = "", const TString &cut5 = "", const TString &cut6 = "",
                               const TString &cut7 = "", const TString &cut8 = "", const TString &cut9 = "",
                               const TString &cut10 = "", const TString &cut11 = "", const TString &cut12 = "",
                               const TString &cut13 = "", const TString &cut14 = "", const TString &cut15 = "")
    {
        TString combined = cut1;
        if (cut2.Length() > 0)
            combined = AppendCuts(combined, cut2);
        if (cut3.Length() > 0)
            combined = AppendCuts(combined, cut3);
        if (cut4.Length() > 0)
            combined = AppendCuts(combined, cut4);
        if (cut5.Length() > 0)
            combined = AppendCuts(combined, cut5);
        if (cut6.Length() > 0)
            combined = AppendCuts(combined, cut6);
        if (cut7.Length() > 0)
            combined = AppendCuts(combined, cut7);
        if (cut8.Length() > 0)
            combined = AppendCuts(combined, cut8);
        if (cut9.Length() > 0)
            combined = AppendCuts(combined, cut9);
        if (cut10.Length() > 0)
            combined = AppendCuts(combined, cut10);
        if (cut11.Length() > 0)
            combined = AppendCuts(combined, cut11);
        if (cut12.Length() > 0)
            combined = AppendCuts(combined, cut12);
        if (cut13.Length() > 0)
            combined = AppendCuts(combined, cut13);
        if (cut14.Length() > 0)
            combined = AppendCuts(combined, cut14);
        return combined;
    }

    // Combined cut - applies all cuts with default values
    static TString GetAllCuts()
    {
        return CombineCuts(
            GetRapidityCut(),
            GetNSigmaECut(),
            GetBEMCValidCut(),
            GetETowEcluCut(),
            GetEOverPCut(),
            GetPtCut(),
            GetADCCut(),
            GetDZCut(),
            GetDphiCut(),
            GetNHitsFitCut(),
            GetNHitsRatioCut(),
            GetDCACut(),
            GetVzDiffCut());
    }

    // Base cuts
    static TString GetBaseCuts()
    {
        return CombineCuts(
            GetBEMCValidCut(),
            GetVzDiffCut(9999.0),
            GetRapidityCut(-1, 1),
            GetNSigmaECut(nSigmaE_lo),
            GetBEMCValidCut(),
            GetETowEcluCut(EtowEclu),
            GetEOverPCut(0.55, Eoverp_hi),
            GetPtCut(d1_pT_lo),       // Assuming same pT cut for both electrons
            GetADCCut(adc_trig_BHT2), // Using adc_trig for ONE electron (partner cut is off)
            GetNHitsFitCut(nHitsFit),
            GetNHitsRatioCut(nHitsRatio),
            GetDCACut(3.0),
            GetDZCut(20),
            GetDphiCut(0.1));
    }
};