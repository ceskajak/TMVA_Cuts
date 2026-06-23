#pragma once

/*
/////////////////////////////////////////////////////////////
CUTS
/////////////////////////////////////////////////////////////
*/

/*
y				- lower and upper bound for quarkonium rapidity

nSigmaElectron 	- lower bound for nSigmaElectron

E_TOW/E_CLU 	- lower bound for E_TOW/E_CLU

E_CLU/p 		- lower and upper bound for E_CLU/p

d1_pT, d2_pT 	- lower bound for electron pT

adc_trig		- DSM-ADC for trigger electron
					(value set depending on dataset)
adc_par 		- lower bound for partner electron DSM-ADC
					(off due to greater than comparison)

DZ				- upper bound for the distance of the center of gravity
					of the cluster and the track projection
					in the Z direction
					(same for both electrons)
Dphi			- upper bound for the distance of the center of gravity
					of the cluster and the track projection
					in the Phi direction
					(same for both electrons)

// Systematic variations

nHitsFit		- nHitsFit for electron
nHitsRatio		- nHitsFit/nHitsMax for electron
DCA				- DCA for electron
VzDiff			- maximum difference between VzTPC and VzVPD
*/

const float y_lo = -1;			// lower bound for quarkonium rapidity
const float y_hi = 1;			// upper bound for quarkonium rapidity

const float nSigmaE_lo = -1.2;	// lower bound for nSigmaElectron (same for both electrons)

const float EtowEclu = 0.5;	    // lower bound for E_TOW/E_CLU (same for both electrons)

const float Eoverp_lo = 0.7;	// lower bound for E_CLU/p (same for both electrons)
const float Eoverp_hi = 1.45;   // upper bound for E_CLU/p (same for both electrons)

const float d1_pT_lo = 1;	    // lower bound for electron1 pT
const float d2_pT_lo = 1;	    // lower bound for electron2 pT

int adc_trig;				// DSM-ADC for trigger electron (value set depending on dataset)
const int adc_trig_BHT1 = 18;	// DSM-ADC for trigger electron (BHT1 dataset)
const int adc_trig_BHT2 = 25;	// DSM-ADC for trigger electron (BHT2 dataset)
const int adc_par = -1;	        // DSM-ADC for partner electron. off due to greater than

const float DZ = 5;			    // ZDist
const float Dphi = 0.02;	    // PhiDist

// Systematic variations
const float nHitsFit = 20;		// nHitsFit for electron

const float nHitsRatio = 0.52;	// nHitsFit/nHitsMax for electron

const float DCA = 1;			// DCA for electron

const float VzDiff = 9999;		// maximum difference between VzTPC and VzVPD