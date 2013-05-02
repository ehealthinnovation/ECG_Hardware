#include "detect.h"
#include "filter.h"
#include "key.h"
#include <iostream>
#include <math.h>
#include <stdlib.h> 
#include <string>
#include <deque>
#include <string>

//have a initialize function

//have a actual detect function

//have threshold calculation function

using namespace std;

static string abc;

string retStr(){
	return abc;
}
int QRSDetect(const deque <double>& time, const deque <double>& voltage){

	int returnFlag = 0; 

	static int maxValue = 0;
	static int prevValue = 0;
	static int numberOfSamplesSinceLastPeak = 0; //number of func calls since last detection, indicating the shift in buffers
	static int numberOfSamplesSinceLastQRS = 0;
	static int numberOfSamplesSinceSecondLastPeak=0;
	static int thresholdBasedonMean = 0;
	static int detectionThreshold = 0;
	static int RCount=0;

	//FIFO Queues to store last 8 QRS and Noise values for threshold calculations
	static deque <int> last8QRS;
	static deque <int> last8Noise;
	static double SPKI=0;
	static double NPKI=0;
	if(last8QRS.size() == 0){
		int thres = 4000;
		int noi=1000;
		int sig=(thres+(noi*(thc-1)))/thc;

		if (thresholdBasedonMean){
			for(int x=0;x<8;x++)
				last8Noise.push_back(noi);
			for(int x=0;x<8;x++)
				last8QRS.push_back(sig);
			}
		else{
			SPKI=sig;
			NPKI=noi;
		}
	}
	
	//FIFO Queue to store last 8 RR intervals
	static deque <double> rrAvg1;
	static deque <double> rrAvg2;
	double avgrr = 0.85; // RRi for Heart Rate of 70
	if (rrAvg1.size() == 0){
		for(int x=0;x<8;x++)
			rrAvg1.push_back(avgrr);
		for(int x=0;x<8;x++)
			rrAvg2.push_back(avgrr);
	}
	//Calculating the Searchback Limits
	static double rrAverage2=avgrr;
	static double rrLowLimit=0.92*avgrr;
	static double rrHighLimit=1.16*avgrr;
	static double rrMissedLimit=1.66*avgrr;

	//Highpass filter output
	static deque <int> highPassOutput; //this should mimic buffer, going from most recent to oldest
	if (highPassOutput.size() > ecg_buffer_length)
		highPassOutput.pop_back();
	
	int inputSample = (int) voltage.front();

	//Low Pass Filter//
	int lpSample=LowPassFilter(inputSample);

	//High Pass Filter//
	int hpSample = HighPassFilter(lpSample);
	highPassOutput.push_front(hpSample);

	//Derivative//
	int ddtSample = Derivative(hpSample);

	//Squaring// 
	int sqSample = Squaring(ddtSample);

	//Moving Integral//
	int currentValue = MovingWindowIntegral(sqSample);

	abc = "";
	abc = to_string(time.front()) + ", " + to_string(voltage.front()) + ", " + to_string(lpSample) + ", " + to_string(hpSample)
		 + ", " + to_string(ddtSample) + ", " + to_string(sqSample) + ", " + to_string(currentValue) + "\n";

	//Threshold and Decision Rules//
	/**********************************************************************************
	**********************************************************************************/
	//Peak Detection
	if ((currentValue > maxValue) && (currentValue>prevValue)){
		maxValue = currentValue; 
		numberOfSamplesSinceSecondLastPeak = numberOfSamplesSinceLastPeak;
		numberOfSamplesSinceLastPeak = 0;
	}
	else if (currentValue <= maxValue >> 1){
		//As described in the paper, the middle of the falling slope indicates that a peak
		//was detected at numberOfSamplesSinceLastPeak. 

		int QRSMean, noiseMean;
		if (thresholdBasedonMean){
			QRSMean=0;
			noiseMean=0;
			for(int i=0;i<last8QRS.size();i++) 	//Calculate QRS mean
				QRSMean+=last8QRS.at(i);
			QRSMean=QRSMean/last8QRS.size();

			for(int i=0;i<last8Noise.size();i++) //Calculate Noise mean
				noiseMean+=last8Noise.at(i);
			noiseMean=noiseMean/last8Noise.size();
		}
		else{
			noiseMean=NPKI;
			QRSMean=SPKI;
		}
		//Determine Threshold based on Past Noise and QRS Means
		detectionThreshold = noiseMean + thc*(QRSMean-noiseMean);


		/*Threshold Verification for Peak+ Searchback
			Two thresholds are used.
			1. In the normal situation the max value must be greater than detectedThreshold.
			
			2. However if the last rPeak occured more than rrMissedLimit seconds ago, then we need 
				to SEARCHBACK and find a peak using a lower threshold (detectedThreshold/2.0). 
				This peak is then considered a QRS complex.
				We do this by comparing the time from the current max peak to previous max peak and
				compare that to rrMissedLimit.*/


		if ((maxValue > detectionThreshold) || 
			((abs(numberOfSamplesSinceLastPeak-numberOfSamplesSinceSecondLastPeak) >= rrMissedLimit*SAMPLE_PER_MS) && (maxValue > (detectionThreshold/2.0)))){
			
			bool searchback = 0;
			if ((abs(numberOfSamplesSinceLastPeak-numberOfSamplesSinceSecondLastPeak) >= rrMissedLimit*SAMPLE_PER_MS) && (maxValue > (detectionThreshold/2.0)))
				searchback=1; // meets searchback condition

			/*We need to analyze the bandpassed signal in an interval 225ms to 125ms prior to the middle 
			of the falling slope of the integrated signal. Within this interval, we need to find the 
			highest peak and mark this as the fiducial point OR R-Peak of the QRS complex.*/ 
			int beginInt = SAMPLES_IN_125MS;
			int endInt= SAMPLES_IN_225MS;

			if (beginInt >= highPassOutput.size())	
				beginInt=highPassOutput.size()-1;
			if (endInt >= highPassOutput.size())
				endInt=highPassOutput.size()-1;

			int maxBPSignal=0;
			int maxBPLocation=0;

			for (int i=beginInt; i<=endInt;  i++){
				if (highPassOutput.at(i)>maxBPSignal){
					maxBPSignal=highPassOutput.at(i); //Identify highest point
					maxBPLocation=i;
				}
			}

			/*Filter Compensation
			The low pass filters has a delay of 5 samples, while the high-pass filter has a delay
			of 15.5 sampels (see Textbook). We compensate by setting the R-peak 20.5 samples prior 
			to the max bandpass signal point. As we have integer samples we round down to 20.*/
			int rPeakSample = maxBPLocation+20;

			/*200MS Refractory Blanking
			After a R-peak occurs, another cannot occur for 200MS. If any peaks are detected
			within 200MS, it is most likely noise, and is ignored.*/
			double rPeak=0;
			if (rPeakSample >= voltage.size()){
				rPeakSample=0;
				rPeak=0;
			}
			else if (abs(rPeakSample-numberOfSamplesSinceLastQRS) >= SAMPLES_IN_200MS){ //200ms Refractory Blanking
				rPeak = voltage.at(rPeakSample);
				RCount++;
				
				//Determine the R-R interval
				//Note: The time from the start of record to first peak is not counted as an RR interval
				if (RCount > 1){
					double rri = abs(time.at(rPeakSample) - time.at(numberOfSamplesSinceLastQRS));
					
					//Adaptive Threshold Parameter Update
					//8 most recent beats are added to rrAvg1
					if(rrAvg1.size() > 8)
						rrAvg1.pop_front();
					rrAvg1.push_back(rri);

					//If RRi falls between the limits it is added to rrAvg2
					if ((rri>=rrLowLimit) && (rri<=rrHighLimit)){
						if(rrAvg2.size() > 8)
							rrAvg2.pop_front();
						rrAvg2.push_back(rri);

						rrAverage2=0;
						for(int i=0; i<8;i++)
							rrAverage2+=rrAvg2.at(i);
						rrLowLimit=0.92*0.125*rrAverage2;
						rrHighLimit=1.16*0.125*rrAverage2;
						rrMissedLimit=1.66*rrAverage2;
					}
				}
				
				//Set return value to the index of rPeak
				numberOfSamplesSinceLastQRS=rPeakSample;
				returnFlag = rPeakSample;
					
				if (thresholdBasedonMean){
					if(last8QRS.size() > 8)
						last8QRS.pop_front();
					last8QRS.push_back(maxValue); //Signal is QRS
				} 
				else{
					if (searchback)
						SPKI= 0.25*maxValue + 0.75*SPKI; //Signal estimate is updated for QRS found with searchback
					SPKI=0.125*maxValue + 0.875*SPKI; //Signal estimate is updated for normal QRS
				}
			} 
			else{ // Peak does not meet requirements of QRS and is a noise
				if (thresholdBasedonMean){
					if(last8Noise.size() > 8)
						last8Noise.front();
					last8Noise.push_back(maxValue); //Signal is noise
				}
				else
					NPKI=0.125*maxValue + 0.875*NPKI; //Noise estimate is updated
			}
		}
		else{ // Peak does not meet requirements of QRS and is a noise
			if (thresholdBasedonMean){
				if(last8Noise.size() > 8)
					last8Noise.front();
				last8Noise.push_back(maxValue); //Signal is noise
			}else
				NPKI=0.125*maxValue + 0.875*NPKI; //Noise estimate is updated
		}

		maxValue=0;
		
	}

	prevValue = currentValue;
	numberOfSamplesSinceLastQRS++;
	numberOfSamplesSinceSecondLastPeak++;
	numberOfSamplesSinceLastPeak++;
		
	if (returnFlag != 0)
		return returnFlag; //return location of rPeak in Buffer
	else
		return 0;
}