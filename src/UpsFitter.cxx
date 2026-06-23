#include "../headers/UpsFitter.hxx"

int main()
{
	UpsFitter();
	return 0;
}

/////////////////////////////////////////////////////////////
void UpsFitter()
{
	// Create a unique outname using tag + date + timestamp
	TString outname = Form("%s_%s_%ld", tag.Data(), date.Data(), (long)time(NULL));

	d_main = PreprocessData();

	// Use DZ/Dphi defaults from CutGenerator
	TString cuts = CutGenerator::CombineCuts(
		CutGenerator::GetDZCut(),
		CutGenerator::GetDphiCut()
	);

	RooDataSet * ds_US = (RooDataSet *)d_main->reduce(RooArgSet(m), cuts+"&&charge==0");
	ds_US->SetName((outname + "_ds_US").Data());
	ds_US->Print();

	RooDataSet * ds_LS = (RooDataSet *)d_main->reduce(RooArgSet(m), cuts+"&&charge!=0");
	ds_LS->SetName((outname + "_ds_LS").Data());
	ds_LS->Print();

	// This calls the fitting function; use unique output names for plots/results
	TString fitOutName = outname + "_fit";
	fit_bin(ds_US, ds_LS, fitOutName);

	// Clean up
	delete ds_US;
	delete ds_LS;
	cout << "Fit for " << outname << " completed!" << endl;
}

/////////////////////////////////////////////////////////////