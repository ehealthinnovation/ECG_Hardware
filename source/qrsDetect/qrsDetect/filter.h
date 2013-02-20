#include <string>

#define SAMPLERATE	360	/* Sample rate in Hz. */
#define SAMPLE_PER_MS ((double) SAMPLERATE / (double) 1000)

/* Window lengths for power spectrum analysis of HRV*/
const double RRFREQ = 4.0;
const double windowWidthTime = 128.0;
//const int windowWidthSamples = (int) windowWidthTime * RRFREQ;
const int windowWidthSamples = 512;

int LowPassFilter(int data);

int HighPassFilter(int data);

int Derivative(int data);

int Squaring(int data);

int MovingWindowIntegral(int data);

int almostzero(double data);

void calcspline(double * x_in, double * y_in, int n, float * x_interp, int interp_start, int n_interp);

void computeHamming(double * wind, int n);

void windowData(float * data, int n_data, double * wind, int n_wind);

void computeBandPwr(float * rr_data, int n, double * VLF, double * LF, double *HF, std::string *powerspectrum);