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

	rds_data = GetRooDataSet();

	RooDataSet * rds_US = (RooDataSet *)rds_data->reduce(RooArgSet(m), "charge==0");
	rds_US->SetName((outname + "_rds_US").Data());
	rds_US->Print();

	RooDataSet * rds_LS = (RooDataSet *)rds_data->reduce(RooArgSet(m), "charge!=0");
	rds_LS->SetName((outname + "_rds_LS").Data());
	rds_LS->Print();

	// This calls the fitting function; use unique output names for plots/results
	TString fitOutName = outname + "_fit";
	
	fit_bin(rds_US, rds_LS, fitOutName);

	// Clean up
	delete rds_US;
	delete rds_LS;
	cout << "Fit for " << outname << " completed!" << endl;
}

/////////////////////////////////////////////////////////////