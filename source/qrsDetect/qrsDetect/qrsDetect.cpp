#include "stdafx.h"
#include "filter.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <deque>
#include <algorithm>
#include <iterator>
#include <cmath>

using namespace std;
using namespace std::tr1;

/* Preprocesser statements SINGLE and MULTIPLE are used to define whether the program 
looks for one input or muliple predefined input files. Always use one. Never both! */
//#define MULTIPLE
#define SINGLE

/*These boolean variables define whether the program is going to analyze
MIT data or TI data. Only 1 of MITCOMPARE AND TIDATA can be true at any time. */
const bool MITCOMPARE = 1;
const bool TIDATA=0;

/* This boolean variable is used to determine which Threshold Calcuation Method to use.
If true, then the threshold will be calculated using the means of the last 8 QRS and 
noise peaks, as described in the Hamilton/Tompkins paper. If false, then the threshold 
will be calculated using running estimates of noise and signal, as described in the
Biomedical Signal Processing textbook. */
const bool thresholdBasedonMean= 0;

/* Scaling Factors are used to convert input ECG data to microVolts. 
MIT data is stored as milliVolts. TI data is stored as Volts.
The baseline shift is used sometimes for TI data due to a wandering baseline. */
const double scalingFactorMIT= 1000.0;
const double scalingFactorTI = 1000000.0;
const double baselineShift=0.0;

/* The threshold coefficient is used in the calulation of the peak detection threshold. */
const double thc = 0.25;

/* Match window is used to determine the accuracy of the peak detection for MIT data. 
Any peaks within matchWindow of the correct value is considered "detected". */
const double matchWindow = 0.050; //in seconds

/* These are used to determine the number of samples for a period of time. */
const int SAMPLES_IN_100MS = (int)(100*SAMPLE_PER_MS);
const int SAMPLES_IN_125MS = (int)(125*SAMPLE_PER_MS);
const int SAMPLES_IN_200MS = (int)(200*SAMPLE_PER_MS);
const int SAMPLES_IN_225MS = (int)(225*SAMPLE_PER_MS);
const int SAMPLES_IN_360MS = (int)(360*SAMPLE_PER_MS);
const int SAMPLES_IN_1000MS = (int)(1000*SAMPLE_PER_MS);


int _tmain(int argc, _TCHAR* argv[])
{		
	if (TIDATA == MITCOMPARE){
		cout << "ERROR: You cannot analyze MIT and TI data at the same time" << endl;
		exit (EXIT_FAILURE);
	}

#ifdef MULTIPLE
	//Performance Summary Buffers
	vector <int> outputDataSize;
	vector <int> outputSampleRate;
	vector <double> outputSamplingTime;
	vector <int> outputRCount;
	vector <double> outputAvgRPeak;
	vector <double> outputAvgRRInterval;
	vector <double> outputMeanThreshold;
	vector <double> outputMatchWindow;
	vector <double> outputTruePositiveCount;
	vector <double> outputFalseNegativeCount;
	vector <double> outputFalsePositiveCount;
	vector <double> outputSensitivity;
	vector <double> outputPositivePredictivity; 
	vector <double> outputMeanNNInterval;
	vector <double> outputSDNN;
	vector <double> outputSDANN;
	vector <double> outputMeanHR;

	//Predefined input filenames for MIT-BIH data evaluation
	string mitSamples[46] = {"100", "101", "103", "105", "106", "107", "108", "109", "111", "112", 
		"113", "114", "115", "116", "117", "118", "119", "121", "122", "123", "124", "200", "201", "202", "203", 
		"205", "207", "208", "209", "210", "212", "213", "214", "215", "217", "219", "220", "221", 
		"222", "223", "228", "230", "231", "232", "233", "234"};
	
	//string mitSamples[2]= {"900", "908"}; //These are just smaller versions of 100 and 108

	//Loop to cycle through multiple input files
	for (int c=0; c < 46; c++){
#endif

	cout << "Starting QRS Detection Program" << endl << endl; 

	string ECGDataFile; //Input File
	string processedDataFile; //Output File
	string sampleNumber; //File Number
	
	if (MITCOMPARE){
		cout << "Please enter sample number: ";

#ifdef SINGLE
		cin >> sampleNumber;
#endif

#ifdef MULTIPLE
		sampleNumber=mitSamples[c];
		cout << sampleNumber << endl;
#endif

		if (sampleNumber == "none")
			sampleNumber="";
		
		ECGDataFile = "C:\\Users\\Akib\\Desktop\\sampleData\\mitdb\\samples"+sampleNumber+".csv";
		processedDataFile = "C:\\Users\\Akib\\Desktop\\sampleData\\output"+sampleNumber+".csv";
	} 
	else if (TIDATA){
		cout << endl << "Please enter sample number: ";
		cin >> sampleNumber;

		if (sampleNumber == "none")
			sampleNumber="";
		
		ECGDataFile = "C:\\Users\\Akib\\Desktop\\sampleData\\tidata\\beatSim"+sampleNumber+".csv";
		processedDataFile = "C:\\Users\\Akib\\Desktop\\sampleData\\output"+sampleNumber+".csv";
	}
	else{
		ECGDataFile = "C:\\Users\\Akib\\Desktop\\sampleData\\samples.csv";
		processedDataFile = "C:\\Users\\Akib\\Desktop\\sampleData\\output.csv";
	}
	
	//Input File Parser//
	/**********************************************************************************
	Parse input data from CSV file into vector. 

	File format is as follows:
	'Elapsed Time'	'MLII Voltage'			Line 1
	'seconds'	'mV OR V'			Line 2
	0	-0.145						Line 3 onwards

	We must ignore first two lines of data as they are not data. 

	The MLII Voltage for MIT-BIH comes in milliVolts, while the data from TI comes in volts.
	The scaling factors are used to convert the data from double to integers and to convert
	units to microVolts.

	ECGData takes in the input as (Elapsed Time, MLII Voltage). 
	Elapsed Time is in seconds, while MLII Voltage is in microVolts ((uV).
	**********************************************************************************/

	vector<pair<double, double> > ECGData;
	vector<pair<double, double> > RPeakData;

	ifstream inFile;
	inFile.open(ECGDataFile);
	if (!inFile){
		cout << "ERROR: The input signal file does not exist" << endl;
		exit (EXIT_FAILURE);
	}
	
	cout << "Parsing Input Data" << endl; 
	
	string line;
	string header1;
	string header2;
	
	getline(inFile, header1);//Skip first 2 lines, which are just column headings
	getline(inFile, header2);

	while(getline(inFile, line))
	{
		stringstream lineStream(line);
		string cell;
		double secondValue;
		double temp;
		int ecgValue;
		int x=0;

		while(getline(lineStream, cell, ','))
		{
			if (x==0){ //Get Time value
				secondValue=atof(cell.c_str()); 
				x++;
			}
			else { //Get Voltage value
				temp=atof(cell.c_str()); 

				if (MITCOMPARE)
					ecgValue=(int) (temp*scalingFactorMIT); //Scaling and integerizing the leadII milliVolts to microVolts
				else
					ecgValue=(int) (temp*scalingFactorTI)-baselineShift; // Scaling and intergerizing the leadII volts to microVolts
			}	
		}

		ECGData.push_back(make_pair(secondValue, ecgValue)); //Add to end of vector array
	}
	inFile.close();


	//QRS Detection Processing//
	/************************************************************************
	This section analyzes the ECG data in real-time to detect beats.

	This is done using the a modification of the Hamilton-Tompkins Algorithms,
	as described in the paper, "Quantitative Investigation of QRS Detection 
	Rules Using the MIT/BIH Arrhythmia Database, by Patrick S. Hamilton and
	Willis J. Tompkins. 

	The algorithm works as follows:
		ECG -> Low-pass Filter -> High-pass Filter -> Differentiator
			-> Squaring Function -> Moving Window Integrator 
			-> Decision Rules and Thresholding

	Decision Rules and Thresholding include:
		Peak Threshold
		Searchback Threshold
		200MS Refractory Blanking

	************************************************************************/
	
	cout << "QRS Processing" << endl;

	//Data Buffers
	vector <int> lowPassOutput;
	vector <int> highPassOutput;
	vector <int> derivativeOutput;
	vector <int> squaringOutput; 
	vector <int> integralOutput;
	vector <double> peaks;
	vector <double> thresholdLevel;
	vector <double>  RRInterval;
	vector <double> heartRate;
	vector <double> RRI_Buffer;
	vector <double> MeanRRI_SDANN;

	//Threshold Values for Textbook Method
	double SPKI=0;
	double NPKI=0;

	//FIFO Queue to store last 8 noise and QRS peaks for Threshold Paper Method
	deque <int> last8Noise; 
	deque <int> last8QRS;
	double QRSMean; //mean of last 8 QRS peaks
	double noiseMean; //mean of last 8 noise peaks

	//FIFO Queue to store last 8 RR intervals
	deque <double> rrAvg1;
	deque <double> rrAvg2;

	double avgrr = 0.85; // RRi for Heart Rate of 70
	for(int x=0;x<8;x++)
		rrAvg1.push_back(avgrr);
	for(int x=0;x<8;x++)
		rrAvg2.push_back(avgrr);

	//Calculating the Searchback Limits
	double rrAverage2=avgrr;
	double rrLowLimit=0.92*rrAverage2;
	double rrHighLimit=1.16*rrAverage2;
	double rrMissedLimit=1.66*rrAverage2;
	
	//Threshold Initialization
	double detectionThreshold=0;
	double thres=0;
	double noi=0;
	double sig=0;

	if(MITCOMPARE){
		if ((sampleNumber == "108") || (sampleNumber == "121") || (sampleNumber == "207") || (sampleNumber == "222")) //These records tend to have a lower threshold
			thres=1000;
		else
			thres=4000;

		noi=1000;
		sig=(thres+(noi*(thc-1)))/thc;
		
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
	else{
		thres=1000;
		noi=500;
		sig=(thres+(noi*(thc-1)))/thc;

		if (thresholdBasedonMean){
			for(int x=0;x<8;x++)
				last8Noise.push_back(noi);
			for(int x=0;x<8;x++)
				last8QRS.push_back(sig);
		}
		else {
			SPKI=sig;
			NPKI=noi;
		}
	}
	
	//Temporary Variables for Algorithm Function
	int peakValue=0;
	int peakSample=0;
	int maxValue=0;
	int maxSample=0;
	int prevValue = 0;
	int halfPeakSample=0;
	int previousIntPeakSample=0;
	int prevRPeakLocation=-1;
	int RCount=0;
	
	//Temporary Variables for HRV Analysis
	double hr=0;
	double time=0;
	
	int dataSize = ECGData.size();

	//Loop through each sample in the record
	for(int n=0; n < dataSize; n++){

		int currentSample=n;

		//Low Pass Filter//
		int inputSample = (int) ECGData.at(n).second;
		int lpSample=LowPassFilter(inputSample);
		lowPassOutput.push_back(lpSample);

		//High Pass Filter//
		int hpSample = HighPassFilter(lpSample);
		highPassOutput.push_back(hpSample);

		//Derivative//
		int ddtSample = Derivative(hpSample);
		derivativeOutput.push_back(ddtSample);

		//Squaring// 
		int sqSample = Squaring(ddtSample);
		squaringOutput.push_back(sqSample);

		//Moving Integral//
		int currentValue = MovingWindowIntegral(sqSample);
		integralOutput.push_back(currentValue);

		//Threshold and Decision Rules//
		/**********************************************************************************
		This algorithm finds the R-peak by analyzing the time averaged signal. The R peak 
		occurs ideally at the middle of the rising slope of the time averaged signal.
		Peak detection will establish a valid peak at the middle of the falling slope of 
		time averaged signal. Thus, the R-peak occurs ideally a windows width back in time 
		from point of peak detection.

		To be consistant once peak is detected, we look back in an interval of 225 to 125ms
		and find the largest bandpassed signal peak. This is where the R-peak is actually.
		Since bandpass filter has a delay (5+15.5 samples), we must compensate for this. 

		A threshold is used to determine whether the detected peak is noise or QRS signal.
		This is done by keeping track of the last 8 QRS and noise peaks and using their
		means to calculate a threshold value that is just above the noise level. This is 
		done with the following formula:
			Threshold = Noise Level + Coefficient * (QRS Level - Noise Level)
		The coefficient is currently set to 0.333 with the variable "thc".
		**********************************************************************************/
	
		peaks.push_back(0); //initialize to 0

		//Peak Detection
		if ((currentValue > maxValue) && (currentValue>prevValue)){
			maxValue = currentValue; 
			maxSample = currentSample;
		}
		else if (currentValue <= maxValue >> 1){
			//As described in the paper, the middle of the falling slope indicates that a peak
			//was detected at maxSample. 
		
			if (thresholdBasedonMean){
				QRSMean=0;
				noiseMean=0;

				for(int i=0;i<last8QRS.size();i++) 	//Calculate QRS mean
					QRSMean+=last8QRS.at(i);
				QRSMean=QRSMean/last8QRS.size();

				for(int i=0;i<last8Noise.size();i++) //Calculate Noise mean
					noiseMean+=last8Noise.at(i);
				noiseMean=noiseMean/last8Noise.size();
			}else {
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
				This peak is then considered a QRS complex*/

			if ((maxValue > detectionThreshold) || 
				((abs(maxSample-previousIntPeakSample) >= rrMissedLimit*SAMPLE_PER_MS) && (maxValue > (detectionThreshold/2.0)))){

				bool searchback =0;
				if ((abs(maxSample-previousIntPeakSample) >= rrMissedLimit*SAMPLE_PER_MS) && (maxValue > (detectionThreshold/2.0)))
					searchback=1;

				/*We need to analyze the bandpassed signal in an interval 225ms to 125ms prior to the middle 
				of the falling slope of the integrated signal. Within this interval, we need to find the 
				highest peak and mark this as the fiducial point OR R-Peak of the QRS complex.*/ 
				int beginInt = currentSample-SAMPLES_IN_225MS;
				int endInt=currentSample-SAMPLES_IN_125MS;
				if (beginInt<0)	
					beginInt=0;
				if (endInt<0)
					endInt=0;

				int maxBPSignal=0;
				int maxBPLocation=0;

				for (int i=beginInt; i<=endInt;  i++){
					if (highPassOutput.at(i)>maxBPSignal){
						maxBPSignal=highPassOutput.at(i); //Identifiy highest point
						maxBPLocation=i;
					}
				}

				/*Filter Compensation
				The low pass filters has a delay of 5 samples, while the high-pass filter has a delay
				of 15.5 sampels (see Textbook). We compensate by setting the R-peak 20.5 samples prior 
				to the max bandpass signal point. As we have integer samples we round down to 20.*/
				int rPeakSample = maxBPLocation-20;

				/*200MS Refractory Blanking
				After a R-peak occurs, another cannot occur for 200MS. If any peaks are detected
				within 200MS, it is most likely noise, and is ignored.*/
				double rPeak=0;
				
				if (rPeakSample<0){
					rPeakSample=0;
					rPeak=0;
				}else if (abs(rPeakSample-prevRPeakLocation) >= SAMPLES_IN_200MS){ //200ms Refractory Blanking
					rPeak = ECGData.at(rPeakSample).second;
					RPeakData.push_back(make_pair(ECGData.at(rPeakSample).first, rPeak));
					RCount++;

					if(MITCOMPARE){
						peaks.at(rPeakSample)=1;
					} else{
						peaks.at(rPeakSample)=2000;
					}

					//Determine the R-R interval
					//Note: The time from the start of record to first peak is not counted as an RR interval
					if (prevRPeakLocation != -1){
						double rri = ECGData.at(rPeakSample).first - ECGData.at(prevRPeakLocation).first;
						RRInterval.push_back(rri);
						RRI_Buffer.push_back(rri); //Place it in the 5 minute buffer, used for SDANN calculation

						hr=60.0/rri; //Calculate Heart Rate

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

					prevRPeakLocation=rPeakSample; //Previous R-peak sample location
					previousIntPeakSample=maxSample; //Previous Integrated Peak location, corresponding to prevRPeakLocation
					
					if (thresholdBasedonMean){
						if(last8QRS.size() > 8)
							last8QRS.pop_front();
						last8QRS.push_back(maxValue); //Signal is QRS
					} else{
						if (searchback)
							SPKI= 0.25*maxValue + 0.75*SPKI; //Signal estimate is updated for QRS found with searchback
						SPKI=0.125*maxValue + 0.875*SPKI; //Signal estimate is updated for normal QRS
					}
				} else{
					if (thresholdBasedonMean){
						if(last8Noise.size() > 8)
							last8Noise.front();
						last8Noise.push_back(maxValue); //Signal is noise
					}else
						NPKI=0.125*maxValue + 0.875*NPKI; //Noise estimate is updated
				}
			} 
			else{
				if (thresholdBasedonMean){
					if(last8Noise.size() > 8)
						last8Noise.front();
					last8Noise.push_back(maxValue); //Signal is noise
				}else
					NPKI=0.125*maxValue + 0.875*NPKI; //Noise estimate is updated
			}

			maxValue=0;
			maxSample=0;
		}
		
		prevValue=currentValue;

		thresholdLevel.push_back(detectionThreshold);
		heartRate.push_back(hr);

		//Every 5 minute calculate the average RR interval 
		//This is for SDANN calculations
		if (abs(ECGData.at(currentSample).first - time)>=(5*60)){
			cout << ECGData.at(currentSample).first << endl;
			double meanRROver5Min=0;
			for(int i=0; i < RRI_Buffer.size(); i++)
				meanRROver5Min += RRI_Buffer.at(i);
			meanRROver5Min = meanRROver5Min/RRI_Buffer.size();

			MeanRRI_SDANN.push_back(meanRROver5Min);

			RRI_Buffer.clear();
			time = ECGData.at(currentSample).first;
		}

	}
	
	//Clear out what is remaining in RRI_Buffer, if record ended before 5 min passed
	if (!RRI_Buffer.empty()){
		double meanRROver5Min=0;
		for(int i=0; i < RRI_Buffer.size(); i++)
			meanRROver5Min += RRI_Buffer.at(i);
		meanRROver5Min = meanRROver5Min/RRI_Buffer.size();

		MeanRRI_SDANN.push_back(meanRROver5Min);
		RRI_Buffer.clear();
	}

	if (RCount != RPeakData.size())
		cout << "ERROR: R-peak counts do not match" << endl;
	
	//Calculate the mean threshold value
	double meanThreshold=0;
	for(int i=0;i<thresholdLevel.size();i++){
		meanThreshold += thresholdLevel.at(i);
	}
	meanThreshold = meanThreshold/thresholdLevel.size();

	//Calculate average R peak value and convert to mV
	double avgRPeak=0;
	for (int i=0; i<RPeakData.size(); i++)
		avgRPeak=avgRPeak+RPeakData.at(i).second;
	avgRPeak = avgRPeak/RPeakData.size();
	
	if (MITCOMPARE)
		avgRPeak = avgRPeak/scalingFactorMIT;
	else
		avgRPeak = avgRPeak/scalingFactorTI;

	//Calculate average RR Intervals
	double avgRRInterval=0;
	for (int i=0; i<RRInterval.size(); i++)
		avgRRInterval=avgRRInterval+RRInterval.at(i);
	avgRRInterval = avgRRInterval/RRInterval.size();

	//Calculate the total sampling time
	double samplingTime = ECGData.at(dataSize-1).first - ECGData.at(0).first;

	//////////////////////////
	//HEART RATE VARIABILITY//
	//////////////////////////
	/**********************************************************************************
	LF, HF, LF/HF, SDNN/SDANN, mean HR, root mean square

	Time Domain Methods
	meanNNInterval - Mean NN Intervals
	SDNN - Standard Deviation of NN Intervals
	SDANN- Standard deviation of the average NN intervals calculated over short periods, usually 5 minutes,
	meanHR - Mean Heart Rate
	RMSSD - square root of the mean squared differences of successive NN intervals ? (is this root mean square)
			- sqrt of the mean of the sum of squares of differences between adjacent noraml RR-intervals

	Frequency Domain Methods
	LF
	HF

	The measurement of VLF, LF, and HF power components is usually made in
absolute values of power (milliseconds squared). LF and HF may also be measured
in normalized units, [15,24] which represent the relative value of each power
component in proportion to the total power minus the VLF component. The Figure 3
component in proportion to the total power minus the VLF component. The
representation of LF and HF in normalized units emphasizes the controlled and
balanced behavior of the two branches of the autonomic nervous system.
M oreover, the normalization tends to minimize the effect of the changes in total
power on the values of LF and HF components (Figure 3). Nevertheless, normalized
units should always be quoted with absolute values of the LF and HF power in
order to describe completely the distribution of power in spectral components.

	**********************************************************************************/

	//Determine Mean N-N Interval
	double meanNNInterval=0;
	for(int i=0; i < RRInterval.size(); i++)
		meanNNInterval += RRInterval.at(i);
	meanNNInterval = meanNNInterval/RRInterval.size();

	//Determine Standard Deviation of N-N Intervals (SDNN)
	double diffSq=0;
	for(int i=0; i < RRInterval.size(); i++)
		diffSq += pow((RRInterval.at(i)-meanNNInterval), 2);
	double SDNN = sqrt(diffSq/RRInterval.size());

	//Determine Mean HR
	double meanHR;
	meanHR = 60.0/meanNNInterval;

	//Determine Standard Deviation of Sequential Five Minute N-N Interval Averages/Means (SDANN)
	double meanRRIAverage=0;
	for(int i=0; i < MeanRRI_SDANN.size(); i++)
		meanRRIAverage += MeanRRI_SDANN.at(i);
	meanRRIAverage = meanRRIAverage/MeanRRI_SDANN.size();

	diffSq=0;
	for(int i=0; i < MeanRRI_SDANN.size(); i++)
		diffSq += pow((MeanRRI_SDANN.at(i)-meanRRIAverage), 2);
	double SDANN = sqrt(diffSq/MeanRRI_SDANN.size());

	cout << "Mean NN Interval: " << meanNNInterval << endl;
	cout << "SDNN: " << SDNN << endl;
	cout << "SDANN: " << SDANN << endl;
	cout << "Mean HR: " << meanHR << endl;

	///////////////////
	//DATA EVALUATION//
	///////////////////
	/**********************************************************************************
	Evaluation of QRS detection is done using two parameters:

	Sensitivity							Se=TP/(TP+FN)
	Positive Predictivity				PP=TP/(TP+FP) 
	where	TP = # of true positves (true peak detected)
			FP = # of false positives (false peak detected)
			FN = # of false negatives (true peak not detected)

	Sensitivity is the % of true results that test detected as positive
	Positive Predictivity is the % of positive results  that are true positives

	Samples are considered positive if it is detected within 10ms of the annotated point. 
	**********************************************************************************/
	double truePositiveCount=0;
	double falsePositiveCount=0;
	double falseNegativeCount=0;
	vector <double> tp;
	vector <double> fp;
	vector <double> fn;

	if (MITCOMPARE){

		cout << "Data Validation with MIT-BIH database" << endl;
		
		string PeakAnnotationFile;
		PeakAnnotationFile = "C:\\Users\\Akib\\Desktop\\sampleData\\mitdb\\annotations"+sampleNumber+".csv"; //INPUT Annotation FILE

		vector<pair<double, string> > PeakAnnotation; //stores the sample time and type for each beat
		vector <int> binList; //Used in calculating Se and Sp. If 1, then the PeakAnnotation sample at the value was found in RPeakData

		ifstream inFileAnno;
		inFileAnno.open(PeakAnnotationFile);
		if (!inFileAnno){
			cout << "ERROR: The input annotation file does not exist" << endl;
			exit (EXIT_FAILURE);
		}

		string line2;
		string annoHeader1;
		string peakType;
		double peakTime=0;

		getline(inFileAnno, annoHeader1);//Skip first line, which are just column headings

		while(getline(inFileAnno, line2))
		{
			stringstream lineStream(line2);
			string cell2;
			int h=0;

			while(getline(lineStream, cell2, ',')){
				if (h==0){ // Peak Time
					peakTime = atof(cell2.c_str()); 
					h++;
				}
				else{ // Annotation
					peakType = cell2.c_str();
				}
			}
			
			/*The MIT-BIH database annotations are for more than beat. We only need to compare to 
			annotations that are considered beats. We must filter out the others 
			Annotation types:
			http://www.physionet.org/physiobank/annotations.shtml#aux
			For beats, it can be of the following types:
				N, L, R, B, A, a, J, S, V, r, F, e, j, n, E, /, f, Q, ? 
			*/
			string typeList[19] = {"N", "L", "R", "B", "A", "a", "J", "S", "V", "r", "F", "e", "j", "n", "E", "/", "f", "Q", "?"};

			//Check to see if type is acceptable and matches one in the list
			if (std::end(typeList) != std::find(std::begin(typeList), std::end(typeList), cell2)) {
				PeakAnnotation.push_back(make_pair(peakTime, peakType));
				binList.push_back(0);
			}
		}
		inFileAnno.close();

		int annoSize = PeakAnnotation.size();
		int flag=0;

		//We will compare the time values in RPeakData and PeakAnnotation
		for(int i=0; i < RCount; i++){ //search through foundPeaks list
			flag=0;
			for(int j=0; ((j < annoSize) && (flag==0)) ; j++){ //search through knownPeaks list
				if(abs(RPeakData.at(i).first - PeakAnnotation.at(j).first) < matchWindow){ // Peak in foundPeaks list is in knownPeaks list = true positive
					truePositiveCount++;
					binList.at(j) = 1; // If 1, then the PeakAnnotation sample at the value was found in RPeakData
					tp.push_back(RPeakData.at(i).first);
					flag=1;
				}
			}
			if (flag==0){ // Peak in foundPeaks list is not in knownPeaks list = false positive
				falsePositiveCount++;
				fp.push_back(RPeakData.at(i).first);
			}
		}

		//Now we look through annotation list to find the peaks that weren't found. 
		for(int k=0; k < annoSize; k++){
			if (binList.at(k)==0){
				falseNegativeCount++; //Peak in knownPeaks list was not found in foundPeaks = false negative
				fn.push_back(PeakAnnotation.at(k).first);
			}
		}
	}


	///////////////
	//DATA OUTPUT//
	///////////////
	/**********************************************************************************
	Write data into output file
	Parse Data from CSV file into vector
	We must first put in two headers
	Data format is <time><LeadII voltage>: 'hh:mm:ss.mmm','mV'

	Note, the voltage is rescaled into the appropriate unit
	**********************************************************************************/
	ofstream outFile;
	outFile.open(processedDataFile);
	if (!outFile){
		cout << "ERROR: The output file does not exist" << endl;
		exit (EXIT_FAILURE);
	}


	cout << "Creating Output Data Files" << endl << endl;;
	
	cout << "Performance Summary" << endl;
	cout << "Number of Samples: " << dataSize << endl;
	cout << "Sampling Rate: " << SAMPLERATE << " Hz" << endl;
	cout << "Sampling Time: " << samplingTime << " s" << endl;
	cout << "Number of Detected Peaks: " << RCount << endl;
	cout << "Average R-Peak: " << avgRPeak << " mV" << endl;
	cout << "Average RR Interval: " << avgRRInterval << " s" << endl;
	cout << "Mean Threshold: " << meanThreshold << endl;
	cout << "Mean NN Interval: " << meanNNInterval << endl;
	cout << "SDNN: " << SDNN << endl;
	cout << "SDANN: " << SDANN << endl;
	cout << "Mean HR: " << meanHR << endl;

	outFile << "Performance Summary" << endl;
	outFile << "Number of Samples" << ", " << dataSize << endl;
	outFile << "Sampling Rate" << ", " << SAMPLERATE << endl;
	outFile << "Sampling Time" << ", " << samplingTime << endl;
	outFile << "Number of Detected Peaks" << ", " << RCount << endl;
	outFile << "Average R-Peak" << ", " << avgRPeak << endl;
	outFile << "Average RR Interval" << ", " << avgRRInterval << endl;
	outFile << "Mean Threshold" << ", " << meanThreshold << endl;
	outFile << "Mean NN Interval" << ", " << meanNNInterval << endl;
	outFile << "SDNN" << ", " << SDNN << endl;
	outFile << "SDANN" << ", " << SDANN << endl;
	outFile << "Mean HR" << ", " << meanHR << endl;

#ifdef MULTIPLE
	outputDataSize.push_back(dataSize);
	outputSampleRate.push_back(SAMPLERATE);
	outputSamplingTime.push_back(samplingTime);
	outputRCount.push_back(RCount);
	outputAvgRPeak.push_back(avgRPeak);
	outputAvgRRInterval.push_back(avgRRInterval);
	outputMeanThreshold.push_back(meanThreshold);
	outputMeanNNInterval.push_back(meanNNInterval);
	outputSDNN.push_back(SDNN);
	outputSDANN.push_back(SDANN);
	outputMeanHR.push_back(meanHR); 
#endif

	if (MITCOMPARE){
		//do sensitivity and Positive Predictivity calculations
		outFile << "Match Window" << ", " << matchWindow << endl;
		outFile << "True Positives (TP)" << ", " << truePositiveCount << endl;
		outFile << "False Negatives (FP)" << ", " << falseNegativeCount << endl;
		outFile << "False Positives (FP)" << ", " << falsePositiveCount << endl;

		double sensitivity = 100.0*((truePositiveCount)/(truePositiveCount+falseNegativeCount));
		double positivePredictivity = 100.0*((truePositiveCount)/(truePositiveCount+falsePositiveCount));

		outFile << "Sensitivity Se(%)" << ", " << sensitivity << endl;
		outFile << "Positive Predictivity PP(%)" << ", " << positivePredictivity << endl;

		cout << "Match Window: " << matchWindow <<  " s" <<endl;
		cout << "True Positives (TP): " << truePositiveCount << endl;
		cout << "False Negatives (FP): "<< falseNegativeCount << endl;
		cout << "False Positives (FP): " << falsePositiveCount << endl;
		cout << "Sensitivity: " << sensitivity << " %" << endl; 
		cout << "Positive Predictivity: " << positivePredictivity << " %" << endl; 

#ifdef MULTIPLE
		outputMatchWindow.push_back(matchWindow);
		outputTruePositiveCount.push_back(truePositiveCount);
		outputFalseNegativeCount.push_back(falseNegativeCount);
		outputFalsePositiveCount.push_back(falsePositiveCount);
		outputSensitivity.push_back(sensitivity);
		outputPositivePredictivity.push_back(positivePredictivity);
#endif
	}

	outFile << endl << endl;

	outFile << header1 << endl;
	outFile << header2 << ", " << "uV" << ", " << "uV LP" << ", " << "uV HP" << ", " << "uV d/Dt" 
			<< ", " << "uV ^2" << ", " << "uV int" << ", " << "Peaks"  << ", " << "Threshold"
			<< endl;

	for(int y=0; y < dataSize; y++){
		
		double v=0;
		if (MITCOMPARE)
			v = (ECGData.at(y).second)/scalingFactorMIT;
		else
			v = (ECGData.at(y).second)/(scalingFactorTI);

		outFile << ECGData.at(y).first << ", " << v << ", " 
				<< ECGData.at(y).second << ", " << lowPassOutput.at(y) 
				<< ", " << highPassOutput.at(y) << ", " << derivativeOutput.at(y) 
				<< ", " << squaringOutput.at(y) << ", " << integralOutput.at(y) 
				<< ", " << peaks.at(y) << ", " << thresholdLevel.at(y) 
				<< endl;
	}

	outFile << endl << endl;

	if (MITCOMPARE){
		outFile << "True Positives" << endl;
		for(int u=0; u < tp.size(); u++){
			outFile << tp.at(u) << endl;
		}

		outFile << endl << "False Positives" << endl;
		for(int u=0; u < fp.size(); u++){
			outFile << fp.at(u) << endl;
		}

		outFile << endl << "False Negatives" << endl;
		for(int u=0; u < fn.size(); u++){
			outFile << fn.at(u) << endl;
		}
	}
	
	outFile << endl << endl;
	outFile << "Detected R Peaks" << endl;
	outFile << "Time (s)" << ", "<< "MLII (mV)" << endl;
	for(int u=0; u < RPeakData.size(); u++){
		outFile << RPeakData.at(u).first << ", " << RPeakData.at(u).second << endl;
	}

	outFile << endl << endl;
	outFile << "R-R Interval Times" << endl;
	for(int u=0; u < RRInterval.size(); u++){
		outFile << RRInterval.at(u) << endl;
	}
	
	outFile.close();
	
#ifdef MULTIPLE
	}// for the FORLOOP
#endif

#ifdef MULTIPLE
	if (MITCOMPARE){
		cout << "Preparing Performance Summary File" << endl;

		string summaryFile = "C:\\Users\\Akib\\Desktop\\sampleData\\outputSummary.csv"; //OUTPUT FILE
		ofstream outFile3;
		outFile3.open(summaryFile);
		if (!outFile3){
			cout << "ERROR: The output summary file does not exist" << endl;
			exit (EXIT_FAILURE);
		}

		outFile3 << "Performance Summary" << endl;
		outFile3 << "Record Number" << ", " << "Number of Samples" << ", " << "Sampling Rate" << ", " << "Sampling Time" << ", " << "Number of Detected Peaks"  
			<< ", " << "Average R-Peak" << ", " << "Average RR Interval" << ", " << "Mean Threshold" << ", "
			<< "Mean NN Interval" << ", " << "SDNN" << ", " << "SDANN" << ", "<< "Mean Heart Rate" << ", "
			<< "Match Window" << ", " <<  "True Positives (TP)" << ", " << "False Negatives (FP)" << ", " << "False Positives (FP)" << ", " 
			<< "Sensitivity" << ", " << "Positive Predictivity" << endl;

		int numInput=outputDataSize.size();
				
		vector <double> outputMeanNNInterval;
	vector <double> outputSDNN;
	vector <double> outputSDANN;
	vector <double> outputMeanHR;

		for(int u=0; u < numInput; u++){
			outFile3 << mitSamples[u] << ", " << outputDataSize.at(u) << ", " << outputSampleRate.at(u) << ", " << outputSamplingTime.at(u) << ", " 
				<< outputRCount.at(u) << ", " << outputAvgRPeak.at(u) << ", " << outputAvgRRInterval.at(u) << ", " 
				<< outputMeanThreshold.at(u) << ", " << outputMeanNNInterval.at(u) << ", " << outputSDNN.at(u) << ", " << outputSDANN.at(u) 
				<< ", " << outputMeanHR.at(u) << ", " << outputMatchWindow.at(u) << ", " << outputTruePositiveCount.at(u) << ", " 
				<<  outputFalseNegativeCount.at(u) << ", " << outputFalsePositiveCount.at(u) << ", " << outputSensitivity.at(u) << ", " 
				<< outputPositivePredictivity.at(u) << endl;
		}
		outFile3.close();
	}
#endif

	cout << endl << "Terminating Program" << endl << endl;

	return 0;
}
