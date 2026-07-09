#pragma once

#include "TMVACommon.hxx"

/*
/////////////////////////////////////////////////////////////
FUNCTION DECLARATIONS
/////////////////////////////////////////////////////////////
*/

bool TMVAApply(TString strVarset);
void UpdateReaderMethodFlags();
void AdjustVariableFlags();
void ConnectBranches(TTree *t_main);
void RegisterReaderVariables();
bool BookReader();