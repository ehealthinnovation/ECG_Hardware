#include <deque>
#include "key.h"
#include <string>

/* The threshold coefficient is used in the calulation of the peak detection threshold. */
const double thc = 0.25;

const int SAMPLES_IN_125MS = (int)(125*SAMPLE_PER_MS);
const int SAMPLES_IN_200MS = (int)(200*SAMPLE_PER_MS);
const int SAMPLES_IN_225MS = (int)(225*SAMPLE_PER_MS);
const int SAMPLES_IN_120S = (int)(120*SAMPLERATE);

const int ecg_buffer_length = SAMPLES_IN_120S; //ECG Buffer is 2 minutes long, FIFO, add to front, remove from back

const bool thresholdBasedonMean = false;

int QRSDetect(const std::deque <double>& time, const std::deque <double>& voltage, std::string sampleNumber);

std::string retStr();