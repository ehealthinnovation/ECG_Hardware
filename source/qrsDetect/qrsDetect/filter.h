#define SAMPLERATE	853	/* Sample rate in Hz. */
#define SAMPLE_PER_MS ((double) SAMPLERATE / (double) 1000)
 
int LowPassFilter(int data);

int HighPassFilter(int data);

int Derivative(int data);

int Squaring(int data);

int MovingWindowIntegral(int data);

//twoPoleRecursive(int data);

