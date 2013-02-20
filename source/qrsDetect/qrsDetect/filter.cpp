#include "stdafx.h"
#include "filter.h"
#include <iostream>
#include <math.h>
#include <stdlib.h> 
#include <string>
#include "kiss_fft.h"
#include "kiss_fftr.h"

using namespace std;

//Bandpass Filter//
/************************************************************************
Reduces noise in ECG by attenuating noise from muscle, 60Hz interference, 
baseline wander, and T-wave interference. Passband to maximize energy is 
5-15Hz. Filter is created using a cascaded low pass and high pass filter
************************************************************************/

//Low Pass Filter//
/***********************************************************************
Code is based on Biomedical Digital Signal Processing by W.J. Tompkins

Transfer function of H(z)=((1-z^-6)^2)/((1-z^-1)^2)
Implemented as a difference equation of:
	y(nT) = 2y(nT – T) – y(nT – 2T) + x(nT) – 2x(nT – 6T) + x(nT – 12T)
Cutoff freq of 11Hz
Delay of 5 samples or 25ms
Gain of 36
************************************************************************/
int LowPassFilter(int data) {
	
	static int y1 = 0, y2 = 0, x[26], n = 12;
	int y0;

	x[n] = x[n + 13] = data;
	y0 = (y1 << 1) - y2 + x[n] - (x[n + 6] << 1) + x[n + 12];
	y2 = y1;
	y1 = y0;
	y0 >>= 5;
	if(--n < 0)
		n = 12;
	return(y0);
}

//High Pass Filter//
/***********************************************************************
Code is based on Biomedical Digital Signal Processing by W.J. Tompkins

Built by dividing output of low-pass filter by DC gain and subtracting 
from original signal.
Transfer function of H(z)=(z^-16)-(1/32)*(1-z^-32)/(1-z^-1)
Implemented as a difference equation of:
	p(nT) = x(nT – 16T) – (1/32)*[y(nT – T) + x(nT) – x(nT – 32T)])
	(in textbook)
	OR
	p(nT) = y(nT-T) - x(nT)/32 + x(nT-16T) - x(nT-17T) + x(nT-32T)/32
	(in paper)
Delay of 15.5 samples
Gain of 32
************************************************************************/
int HighPassFilter(int data) {
	
	static int y1 = 0, x[66], n = 32;
	int y0;
	
	x[n] = x[n + 33] = data;
	y0 = y1 + x[n] - x[n + 32];
	y1 = y0;
	
	if(--n < 0)
		n = 32;

	return(x[n + 16] - (y0 >> 5));
}

//Derivative//
/***********************************************************************
Code is based on Biomedical Digital Signal Processing by W.J. Tompkins

Signal goes through a5-point derivative to provide info about slope of QRS.
Transfer function of H(z)=0.1*(2+(z^–1)–(z^-3)=(2z^–4))
Implemented as a difference equation of:
	y(nT) = (2x(nT) + x(nT – T) – x(nT – 3T) – 2x(nT – 4T))/8
Delay of 10ms 
Gain of 0.1
************************************************************************/
int Derivative(int data) {

	int y, i;
	static int x_derv[4];
	
	/*y = 1/8 (2x( nT) + x( nT - T) - x( nT - 3T) - 2x( nT -
	4T))*/
	
	y = (data << 1) + x_derv[3] - x_derv[1] - ( x_derv[0] <<1);
	
	y >>= 3;
	for (i = 0; i < 3; i++)
		x_derv[i] = x_derv[i + 1];
	x_derv[3] = data;
	return(y);
}

//Squaring//
/***********************************************************************
Code is based on Biomedical Digital Signal Processing by W.J. Tompkins

Non-linear operation to make all data points positive and emphasize the 
higher frequencies of the signal, which are from QRS
Implemented as equation :
	y(nT) = [x(nT)]^2
************************************************************************/
int Squaring(int data) {

	int y;
	y = data * data;
	return(y);
}

//Moving Window Integral//
/***********************************************************************
Code is based on Biomedical Digital Signal Processing by W.J. Tompkins

Extracts features of QRS, in addition to R wave slope. 
Width of window should be approximately the same as widest QRS possible.
There is a delay between the original QRS and the corresponding waves
in the moving window integral signal (due to bandpass and d/dt).

Implemented as a difference equation of:
	y(nT) = (1/N)*[x(nT – (N – 1)T) + x(nT – (N – 2)T) +…+ x(nT)]
	where N is the number of samples in width of moving window
N is set to 150ms (But narrower window may be BETTER!)
************************************************************************/
int MovingWindowIntegral(int data) {

	const int windowWidth = (int)(150*SAMPLE_PER_MS);
	static int x[windowWidth], ptr = 0;
	static long sum = 0;
	long ly;
	int y;

	if(++ptr == windowWidth)
		ptr = 0;
	sum -= x[ptr];
	sum += data;
	x[ptr] = data;
	ly = sum >> 5;
	
	/*if(ly > 32400) //check for register overflow
		y = 32400;
	else
		y = (int) ly;*/
	y = (int) ly; 
	
	return(y);
}


int almostzero(double data) {
	if (floor(data)==0)
		return true;
	else
		return false;
}

//The following functions are from code created by Moussa Chehade on 12-03-10. 
//Copyright (c) 2012 UHN. All rights reserved.

//Piecewise-Continuous Cubic Interpolation//
/***********************************************************************
// FUNCTION: calcspline
// PURPOSE:
// Used to interpolate data of arbitrary spacing to a uniformly spaced
// data set, using Akima (piecewise-continuous cubic) interpolation
//
// PARAMETERS:
// name description
// ---------------------------------------------------------------------
// x_in, y_in x,y data pairs to be interpolated. x_in must be ordered chronologically
// n number of input data pairs
// x_interp output interpolated data, spaced evenly at 1/RRFREQ seconds
// interp_start time to start interpolated data series, in samples
// n_interp length of interpolated data series, in samples
//
// RETURN VALUE: <none>
//
// Created by Moussa Chehade on 12-03-10.
// Copyright (c) 2012 UHN. All rights reserved.
************************************************************************/
void calcspline(double * x_in, double * y_in, int n, float * x_interp, int interp_start, int n_interp){

	if (n>2) {
		int i, p;
		double *dx, *dy;
		double *x, *y;
		double *m, *t; //slopes
		double *C, *D; //coefficients
		double xd; //distance from data point

		x= (double *) malloc((n+4) * sizeof(double));
		y= (double *) malloc((n+4) * sizeof(double));

		// Move x[0] to x[2] to pad the data with x[0] and x[1] on the left
		memcpy(&x[2],&x_in[0], (size_t)(n*sizeof(double)));
		memcpy(&y[2],&y_in[0], (size_t)(n*sizeof(double)));
		n += 4;

		dx=(double *)malloc(n * sizeof(double));
		dy=(double *)malloc(n * sizeof(double));
		m=(double *)malloc(n * sizeof(double));
		t=(double *)malloc(n * sizeof(double));
		C=(double *)malloc(n * sizeof(double));
		D=(double *)malloc(n * sizeof(double));

		// a) Calculate the differences and the slopes m[i].
		//for all data points 3 to n-2
		for (i= 2; i < n-2; i++) {
			//calculate dy/dx for each input data point
			dx[i] = x[i+1] - x[i];
			dy[i] = y[i+1] - y[i];
			
			//if dx is zero for some strange reason, slope zero to avoid a divby-zero error
			if (almostzero(dx[i])) {
				m[i] = 0.0;
			}
			else {
			m[i] = dy[i]/dx[i];
			}
		}

		// b) interpolate the special case points: x1,x2,xn-1,xn */
		x[1] = x[2] + x[3] - x[4];
		dx[1] = x[2] - x[1];
		y[1] = dx[1]*(m[3] - 2*m[2]) + y[2];
		dy[1] = y[2] - y[1];
		if (almostzero(dx[1])) {
			m[1] = 0;
		}
		else {
			m[1] = dy[1]/dx[1];
		}

		x[0] = 2*x[2] - x[4];
		dx[0] = x[1] - x[0];
		y[0] = dx[0]*(m[2] - 2*m[1]) + y[1];
		dy[0] = y[1] - y[0];

		if (almostzero(dx[0])) {
			m[0] = 0;
		}
		else {
			m[0] = dy[0]/dx[0];
		}

		x[n-2] = x[n-3] + x[n-4] - x[n-5];
		y[n-2] = (2*m[n-4] - m[n-5])*(x[n-2] - x[n-3l]) + y[n-3];
		x[n-1] = 2*x[n-3] - x[n-5];
		y[n-1] = (2*m[n-3] - m[n-4])*(x[n-1] - x[n-2]) + y[n-2];

		// for the last 2 points
		for (i=n-3; i < n-1; i++) {
			dx[i]=x[i+1] - x[i];
			dy[i]=y[i+1] - y[i];
			m[i]=dy[i]/dx[i];
		}

		// the first x slopes and the last y ones are extrapolated: */
		// the first two are irrelevant
		t[0]=0.0;
		t[1]=0.0;
		t[n-1] = 0.0;
		t[n-2] = 0.0;

		// for all data points

		for (i=2; i < n-2; i++) {
			double num, den;
			
			num = fabs(m[i+1] - m[i])*m[i-1] + fabs(m[i-1] - m[i-2])*m[i];
			den = fabs(m[i+1] - m[i]) + fabs(m[i-1] - m[i-2]);
			
			if (!almostzero(den)) {
				t[i]=num/den;
			}
			else {
				t[i]=0.0;
			}
		}

		/* c) Create the polynomial coefficients */
		for (i=2; i < n-2; i++) {
			C[i] = (3*m[i] - 2*t[i] - t[i+1])/dx[i];
			D[i] = (t[i] + t[i+1] - 2*m[i])/(dx[i]*dx[i]);
		}
		C[0] = 0.0;
		C[1] = 0.0;
		C[n-2] = 0.0;
		C[n-1] = 0.0;
		D[0] = 0.0;
		D[1] = 0.0;
		D[n-2] = 0.0;
		D[n-1] = 0.0;

		/* 3rd step: output the coefficients for the subintervalls i=2..n-4 */
		/* calculate the intermediate values */
		p = 1; //counter for uninterpolated data

		for (i = interp_start; i < interp_start + n_interp; i++) {
			//advance the data forward so that uninterpolated data is ahead of interpolated
			while ((i*1.0/RRFREQ >= x[p]) && p < n) {
				p++;
			}
			//xd = x-distance from previous point
			xd = (i*1.0/RRFREQ - x[p-1]);
			if (xd < 0.1/RRFREQ) {
				x_interp[i-interp_start] = y[p-1];
			}
			else {
				x_interp[i-interp_start] = y[p-1] + (t[p-1] + (C[p-1] + D[p-1]
				*xd)*xd)*xd;
			}
		}

		free(dx); free(dy); free(m); free(t); free(C); free(D); free(x); free(y);
	}
}

//Compute Hamming Window//
/***********************************************************************
//Precomputes a Hamming window for data of length n
//wind: array of [double] to store the window
//n: size of array
//
// Created by Moussa Chehade on 12-03-10.
// Copyright (c) 2012 UHN. All rights reserved.
***********************************************************************/
void computeHamming(double * wind, int n) {
	double PI=3.14159265359;
	int i;
	for (i=0; i < n; i++) {
		wind[i] = 0.54 - 0.46*cos(2*PI*i/(double)(n-1));
	}
}

//Window Data//
/***********************************************************************
//Windows data by multiplying elementwise by a window (e.g. Hamming)
//note: size of data must match the size of the window or else no computation isdone
//
//data: data to be windows
//n_data: number of data points
//wind: window, precomupted
//n_wind: size of window, included as an explicit bug-check
//
// Created by Moussa Chehade on 12-03-10.
// Copyright (c) 2012 UHN. All rights reserved.
***********************************************************************/
void windowData(float * data, int n_data, double * wind, int n_wind) {
	int i;
	if (n_data != n_wind) {return;}
	for (i = 0; i < n_data; i++) {
		data[i] = data[i]*wind[i];
	}
}

//Compute Band Power with FFT//
/***********************************************************************
// FUNCTION: computeBandPwr
// PURPOSE:
// Computes (VLF),LF, and HF band powers from an array of R-R interval
// data using a Fourier transform
//
// PARAMETERS:
// name description
// ---------------------------------------------------------------------
// rr_data array of R-R interval values, previously windowed and
// evenly spaced at RRFREQ samples/second
// n length of rr_data
// VLF,LF,HF pointers to doubles where VLF,LF,HF band powers will be stored
//
// RETURN VALUE: <none>
//
// This code is a modified version of the computeBandPwr function created 
// by Moussa Chehade on 12-03-10. 
// Copyright (c) 2012 UHN. All rights reserved.

This code was modifed by Akib Uddin
Spectral Analysis
We need to compute the LF and HF. This is done by integrating the fourier transofrm data
The bands are defined as follows:
	VLF: 0 Hz to 0.04 Hz (normally it is 0.003 to 0.04 but we don't have the resolution).
	LF: 0.04 Hz to 0.15 Hz
	HF: 0.15 Hz to 0.4 Hz
The FFT function is from the Kiss-FFT library by Mark Borgerding under BSD License

Our sampling rate is 4Hz, and the nyquist frequency is 2Hz. The FFT will produce
an output of nfft/2+1 points, with nfft=512 (4Hz*128seconds) and 1 bin being the
DC value. So our output will be 256 bins and we have a resolution of 2/256 or 
0.0078125Hz (each bin contains the magnitude for 0.0078125Hz). 

To compute the band powers we need to integrate the magnitude of the complex number 
over the bins that fall under the specified band. The band power is the square of 
the integrated value. This is essentially a Periodogram of the signal.
***********************************************************************/

void computeBandPwr(float * rr_data, int n, double * VLF, double * LF, double *HF, string *powerspectrum) {
	
	kiss_fftr_cfg cfg = kiss_fftr_alloc(windowWidthSamples, 0, 0, 0);
			
	kiss_fft_scalar ffin[windowWidthSamples]; //same thing as rrFinalData
	kiss_fft_cpx ffout[windowWidthSamples];

	for (int c=0; c < windowWidthSamples; c++)
		ffin[c] = rr_data[c];

	kiss_fftr(cfg, rr_data, ffout);

	const int FFTLEN = n;
	double LF_MIN = 0.04;
	double HF_MIN = 0.15;
	double HF_MAX = 0.4;

	int VLF_i, LF_i, HF_i;

	//Nyquist freq (RRFREQ/2) appears at FFTLEN/2
	//Defines the boundaries of each band in bins 
	//VLF: Bins 1 to 5
	//LF: Bins 6 to 19
	//HF: Bins 20 to 51
	VLF_i = floor((LF_MIN / RRFREQ) * FFTLEN); //should be 5
	LF_i = floor((HF_MIN / RRFREQ) * FFTLEN); //should be 19
	HF_i = floor((HF_MAX / RRFREQ) * FFTLEN); //should be 51

	int vlf=0;
	int lf=0;
	int hf=0;
	int temp=0;
	int psd[windowWidthSamples/2];
	//Integrate the power spectrum to find VLF,LF,HF
	//We start at i=1 as i=0 is dc componenet
	for (int i=1; i <= HF_i; i++){
		temp = (sqrt(pow(ffout[i].r, 2) + pow(ffout[i].i, 2)));
		psd[i-1]=temp;

		if (i <=VLF_i)
			vlf += temp;
		else if (i <= LF_i)
			lf += temp;
		else 
			hf += temp;
	}

	*VLF = pow(vlf, 2);
	*LF = pow(lf, 2);
	*HF = pow(hf, 2);
	
	kiss_fftr_free(cfg);

	for (int i=0; i<(windowWidthSamples/2);i++)
		*powerspectrum = *powerspectrum + to_string(psd[i]) + ", ";
	/*for (i = 1; i < HF_i; i++) {
		if (i < VLF_i) {
			*VLF += 4.0*(pow(ffout[i].r,2)+pow(ffout[i].i,2))/FFTLEN/FFTLEN;
		}
		else if (i < LF_i) {
			*LF += 4.0*(pow(ffout[i].r,2)+pow(ffout[i].i,2))/FFTLEN/FFTLEN;
		}
		else {
			*HF += 4.0*(pow(ffout[i].r,2)+pow(ffout[i].i,2))/FFTLEN/FFTLEN;
		}
	}*/
}