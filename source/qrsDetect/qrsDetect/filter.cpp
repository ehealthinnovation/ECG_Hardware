#include "stdafx.h"
#include "filter.h"

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
