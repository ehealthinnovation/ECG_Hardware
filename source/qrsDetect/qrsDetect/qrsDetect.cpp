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

//The preprocessor statements SINGLE AND MULTIPLE are used to tell the program to 
//look for one input file or  predefined multiple input files.
//Never use BOTH!!! Always use ONE!!!
//#define MULTIPLE
#define SINGLE

//Only 1 of MITCOMPARE AND TIDATA can be true at any time
const bool MITCOMPARE = 0;
const bool TIDATA=1;

using namespace std;
using namespace std::tr1;

const double scalingFactor= 1000.0;
const double baselineShift=0.0;
const double thc = 0.3;
const double matchWindow = 0.010; //in seconds

const int SAMPLES_IN_100MS = (int)(100*SAMPLE_PER_MS);
const int SAMPLES_IN_125MS = (int)(125*SAMPLE_PER_MS);
const int SAMPLES_IN_200MS = (int)(200*SAMPLE_PER_MS);
const int SAMPLES_IN_225MS = (int)(225*SAMPLE_PER_MS);
const int SAMPLES_IN_360MS = (int)(360*SAMPLE_PER_MS);
const int SAMPLES_IN_1000MS = (int)(1000*SAMPLE_PER_MS);



int _tmain(int argc, _TCHAR* argv[])
{		
	if (TIDATA==MITCOMPARE){
		cout << "ERROR: You cannot analyze MIT and TI data at the same time" << endl;
		exit (EXIT_FAILURE);
	}

#ifdef MULTIPLE
	//Performance Summary Details
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

	//for testing
	string mitSamples[46] = {"100", "101", "103", "105", "106", "107", "108", "109", "111", "112", 
		"113", "114", "115", "116", "117", "118", "119", "121", "122", "123", "124", "200", "201", "202", "203", 
		"205", "207", "208", "209", "210", "212", "213", "214", "215", "217", "219", "220", "221", 
		"222", "223", "228", "230", "231", "232", "233", "234"};
	//string mitSamples[2]= {"900", "908"}; //These are just smaller versions of 100 and 108

	////
	for (int c=0; c < 46; c++){
#endif

	cout << "Starting QRS Detection Program" << endl; 
	//Input Declarations
	string ECGDataFile;
	string processedDataFile;
	
	string sampleNumber;
	
	if (MITCOMPARE){

		cout << endl;
		cout << "Please enter sample number: ";

#ifdef SINGLE
		cin >> sampleNumber;
#endif

#ifdef MULTIPLE
		sampleNumber=mitSamples[c];
		cout << sampleNumber << endl;
#endif

		if (sampleNumber=="none")
			sampleNumber="";
		
		ECGDataFile = "C:\\Users\\Akib\\Desktop\\sampleData\\mitdb\\samples"+sampleNumber+".csv"; //INPUT FILE
		processedDataFile = "C:\\Users\\Akib\\Desktop\\sampleData\\output"+sampleNumber+".csv"; //OUTPUT FILE
	} else if (TIDATA==1){

		cout << endl;
		cout << "Please enter sample number: ";
		cin >> sampleNumber;

		if (sampleNumber=="none")
			sampleNumber="";
		
		ECGDataFile = "C:\\Users\\Akib\\Desktop\\sampleData\\tidata\\beatSim"+sampleNumber+".csv"; //INPUT FILE
		processedDataFile = "C:\\Users\\Akib\\Desktop\\sampleData\\output"+sampleNumber+".csv"; //OUTPUT FILE
	}
	else{
		ECGDataFile = "C:\\Users\\Akib\\Desktop\\sampleData\\samples.csv"; //INPUT FILE
		processedDataFile = "C:\\Users\\Akib\\Desktop\\sampleData\\output.csv"; //OUTPUT FILE
	}

	vector<pair<double, double> > ECGData;
	vector<pair<double, double> > RPeakData;

	int dataSize;

	//////////
	//PARSER//
	//////////
	//Parse Data from CSV file into vector
	//We must ignore first two lines of data as that is string
	//Data is formatted as Elapsed Time (seconds) and MLII (mV or V)
	//MIT-BIH data comes in mV, while TI data comes in V,
	//The voltage will be scaled by scalingFactor=1000 so that we can work in integers and not in doubles, 
	//The program takes MLII in microVolts (uV)

	ifstream inFile;
	inFile.open(ECGDataFile);
	if (!inFile){
		cout << "ERROR: The input signal file does not exist" << endl;
		exit (EXIT_FAILURE);
	}

	string line;
	string header1;
	string header2;
	
	cout << "Parsing Input Data" << endl; 
	
	getline(inFile, header1);//Skip first 2 lines, which are just column headings
	getline(inFile, header2);

	while(getline(inFile, line))
	{
		stringstream lineStream(line);
		string cell;
		double value1;
		double value2;
		int value3;
		int x=0;

		while(getline(lineStream, cell, ','))
		{
			if (x==0){ //First Value
				value1=atof(cell.c_str()); 
				x++;
			}
			else { //Second Value
				value2=atof(cell.c_str()); 

				if (MITCOMPARE)
					value3=(int) (value2*scalingFactor); //Scaling and integerizing the leadII milliVolts to microVolts
				else
					value3=(int) (value2*scalingFactor*scalingFactor)-baselineShift; // Scaling and intergerizing the leadII volts to microVolts
			}	
		}

		ECGData.push_back(make_pair(value1, value3)); //Add to end of vector array
	}
	inFile.close();

	dataSize=ECGData.size();

	//////////////
	//Processing//
	//////////////
	
	cout << "QRS Processing" << endl;

	//Preprocessing Buffers
	vector <int> lowPassOutput;
	vector <int> highPassOutput;
	vector <int> derivativeOutput;
	vector <int> squaringOutput; 
	vector <int> integralOutput;
	vector <double> peaks;
	vector <double> thresholdLevel;
	vector <double>  RRInterval;

	//FIFO Queue to store last 8 noise and QRS peaks
	deque <int> last8Noise; 
	deque <int> last8QRS;
	
	//initialize to the typical threshold value
	if(MITCOMPARE){
		for(int x=0;x<8;x++)
			last8Noise.push_back(500);
		for(int x=0;x<8;x++)
			last8QRS.push_back(11000);
	}
	else
	{
		for(int x=0;x<8;x++)
			last8Noise.push_back(1000);
		for(int x=0;x<8;x++)
			last8QRS.push_back(7700);
	}
	
	int peakValue=0;
	int peakSample=0;
	int maxValue=0;
	int maxSample=0;
	int prevValue = 0;
	int halfPeakSample=0;

	double detectionThreshold=0;
	int RCount=0;
	int previousIntPeakSample=0;
	int prevRPeakLocation=-1;
	int max=0;

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

		peaks.push_back(0); //initialize to 0

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
		
		//Correcting Threshold:
		//The threshold is initialized to 4000, however this is too high for some of the MIT-BIH records
		//So we then correct threshold by setting the "QRS" mean to the max integral value in the 
		//first 1 second with the noise mean being set to 0.
		if (currentValue>max){
			max=currentValue;
		}
		if (currentSample == SAMPLES_IN_1000MS){
			cout << max << endl;
			for(int x=0;x<8;x++)
				last8Noise.at(x)=0;
			for(int x=0;x<8;x++)
				last8QRS.at(x)=max;
		}
		
		//Peak Detection
		if ((currentValue > maxValue) && (currentValue>prevValue)){
			//new potential peak
			maxValue = currentValue;
			maxSample = currentSample;
		}
		else if (currentValue <= maxValue >> 1){
			//we found the middle of falling slope

			double QRSMean=0; //mean of last 8 QRS peans
			double noiseMean=0; //mean of last 8 noise peaks
			double temp=0;
			double count=0;
		
			//Calculate QRS mean
			for(int i=0;i<last8QRS.size();i++){
				temp=temp+last8QRS.at(i);
				count++;
			}
			QRSMean=temp/count;

			//Calculate Noise mean
			temp=0;
			count=0;
			for(int i=0;i<last8Noise.size();i++){
				temp=temp+last8Noise.at(i);
				count++;
			}
			noiseMean=temp/count;
		
			//set Threshold
			detectionThreshold = noiseMean+thc*(QRSMean-noiseMean);

			//Check if Peak is higher than the thershold
			if (maxValue > detectionThreshold){
				//peaks.at(currentSample)=1000;
				//now we need to look at an interval from 225ms to 125ms prior to half way point
				//and find highest peak in bandpassed signal (hpSample)
				//This is setting the fiducial point
				int beginInt = currentSample-SAMPLES_IN_225MS;
				int endInt=currentSample-SAMPLES_IN_125MS;
				if (beginInt<0)	
					beginInt=0;
				if (endInt<0)
					endInt=0;

				int maxBPSignal=0;
				int maxBPLocation=0;

				//cout << "SI225: " << SAMPLES_IN_225MS << "; " << "SI125: " << SAMPLES_IN_125MS << endl;
				//cout << "Current: " << currentSample << "; " << "BI: " << beginInt << "; " << "EI: " << endInt << endl;

				for (int i=beginInt; i<=endInt;  i++){
					if (highPassOutput.at(i)>maxBPSignal){
						maxBPSignal=highPassOutput.at(i); //finding R-peak
						maxBPLocation=i;
					}
					//peaks.at(i)=i;
				}

				//The low pass filter has a delay of 5 samples (see paper)
				//The high pass filter has a delay of 15.5 samples (see paper)
				//This is compensated here, as the R-peak is found 20.5 samples prior to the max bandpass signal point
				//peaks.at(maxBPLocation)=300;
				int rPeakSample = maxBPLocation-20;
				double rPeak=0;

				//Check if Peak occurs more than 200ms from previous peak (refractory period blanking)
				//If not, it is probably noise peaks
				if (rPeakSample<0){
					rPeakSample=0;
					rPeak=0;
				}else if (abs(rPeakSample-prevRPeakLocation) >= SAMPLES_IN_200MS){ //200ms refractory blanking

					rPeak = ECGData.at(rPeakSample).second;
					RCount++;

					RPeakData.push_back(make_pair(ECGData.at(rPeakSample).first, rPeak));
					
					if(MITCOMPARE){
						peaks.at(rPeakSample)=1;
					} else{
						peaks.at(rPeakSample)=2000;
						//peaks.at(currentSample)=2500;
						//peaks.at(maxSample)=2500;
					}

					//Determine the RR intervals between each peak
					//The time from the start of record to first peak is not counted as an RR interval
					if (prevRPeakLocation != -1){
						double rri = ECGData.at(rPeakSample).first - ECGData.at(prevRPeakLocation).first;
						//cout << ECGData.at(rPeakSample).first << ", " << ECGData.at(rPeakSample).second << ", " << rri << endl;
						RRInterval.push_back(rri);
					}

					prevRPeakLocation=rPeakSample;
					previousIntPeakSample=maxSample; //this is now the new previous peak location
					//cout << "Found peak at " << rPeakSample << " of " << rPeak << endl;
					
					if(last8QRS.size() > 8)
						last8QRS.pop_front();
					last8QRS.push_back(maxValue); //add QRS to buffer of 8 most recent peaks
				} else{
					if(last8Noise.size() > 8)
						last8Noise.front();
					last8Noise.push_back(maxValue); //add noise to buffer of 8 most recent peaks
				}
			} 
			else{
				if(last8Noise.size() > 8)
						last8Noise.front();
				last8Noise.push_back(maxValue); //add noise to buffer of 8 most recent peaks
			}

			maxValue=0;
			maxSample=0;
		}
		
		prevValue=currentValue;
		thresholdLevel.push_back(detectionThreshold);

	}
	
	//cout << "Number of R-peaks Detected: " << RCount << endl;
	if (RCount != RPeakData.size())
		cout << "ERROR: R-peak counts do not match" << endl;
	
	//Calculate the mean threshold value
	double x=0;
	double y=0;
	for(int i=0;i<thresholdLevel.size();i++){
		x=x+thresholdLevel.at(i);
		y++;
	}
	double meanThreshold = x/y;

	//Calculate average R peak in mV
	double avgRPeak=0;
	for (int i=0; i<RPeakData.size(); i++)
		avgRPeak=avgRPeak+RPeakData.at(i).second;
	avgRPeak = avgRPeak/RPeakData.size();
	
	if (MITCOMPARE)
		avgRPeak = avgRPeak/scalingFactor;
		else
			avgRPeak = avgRPeak/(scalingFactor*scalingFactor);

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
	
	**********************************************************************************/



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
		//Load the annotation files to determine which samples are Peaks
		string PeakAnnotationFile;
		PeakAnnotationFile = "C:\\Users\\Akib\\Desktop\\sampleData\\mitdb\\annotations"+sampleNumber+".csv"; //INPUT Annotation FILE

		vector<pair<double, string> > PeakAnnotation; //stores the sample time and type for each beat
		vector <int> binList; //used in calculating Se and Sp // If 1, then the PeakAnnotation sample at the value was found in RPeakData

		ifstream inFileAnno;
		inFileAnno.open(PeakAnnotationFile);
		if (!inFileAnno){
			cout << "ERROR: The input annotation file does not exist" << endl;
			exit (EXIT_FAILURE);
		}

		string line2;
		string annoHeader1;
		double peakTime=0;
		string peakType;

		//Not all samples in annotation file are beat annotations. We must filter out the others. 
		//Annotation types:
		//http://www.physionet.org/physiobank/annotations.shtml#aux
		//For beats, it can be of the following types:
		//N, L, R, B, A, a, J, S, V, r, F, e, j, n, E, /, f, Q, ?
		string typeList[19] = {"N", "L", "R", "B", "A", "a", "J", "S", "V", "r", "F", "e", "j", "n", "E", "/", "f", "Q", "?"};

		getline(inFileAnno, annoHeader1);//Skip first line, which are just column headings

		while(getline(inFileAnno, line2))
		{
			stringstream lineStream(line2);
			string cell2;
			int h=0;

			while(getline(lineStream, cell2, ',')){
				if (h==0){ // first value
					peakTime = atof(cell2.c_str()); 
					h++;
				}
				else{ // second value
						peakType = cell2.c_str();
				}
			}
			
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


		/*ofstream outFile2("C:\\Users\\Akib\\Desktop\\sampleData\\test.csv");

		outFile2 << "Time" << ", " << "Type" << endl;

		int yy=PeakAnnotation.size();
		for(int u=0; u < yy; u++){
			outFile2 << PeakAnnotation.at(u).first << ", " << PeakAnnotation.at(u).second << endl;
		}

		outFile2.close();*/
	}


	///////////////
	//DATA OUTPUT//
	///////////////
	//Write data into output file
	//Parse Data from CSV file into vector
	//We must first put in two headers
	//Data format is <time><LeadII voltage>: 'hh:mm:ss.mmm','mV'

	//Note, the voltage is divided by 1000 to rescale it to mV from microVolts
	//The scaling doesn't apply to the Squaring function, as we scaled already!

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

	outFile << "Performance Summary" << endl;
	outFile << "Number of Samples" << ", " << dataSize << endl;
	outFile << "Sampling Rate" << ", " << SAMPLERATE << endl;
	outFile << "Sampling Time" << ", " << samplingTime << endl;
	outFile << "Number of Detected Peaks" << ", " << RCount << endl;
	outFile << "Average R-Peak" << ", " << avgRPeak << endl;
	outFile << "Average RR Interval" << ", " << avgRRInterval << endl;
	outFile << "Mean Threshold" << ", " << meanThreshold << endl;

#ifdef MULTIPLE
	outputDataSize.push_back(dataSize);
	outputSampleRate.push_back(SAMPLERATE);
	outputSamplingTime.push_back(samplingTime);
	outputRCount.push_back(RCount);
	outputAvgRPeak.push_back(avgRPeak);
	outputAvgRRInterval.push_back(avgRRInterval);
	outputMeanThreshold.push_back(meanThreshold);
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
			v = (ECGData.at(y).second)/scalingFactor;
		else
			v = (ECGData.at(y).second)/(scalingFactor*scalingFactor);

		outFile << ECGData.at(y).first << ", " << v << ", " 
				<< ECGData.at(y).second << ", " << lowPassOutput.at(y) 
				<< ", " << highPassOutput.at(y) << ", " << derivativeOutput.at(y) 
				<< ", " << squaringOutput.at(y) << ", " << integralOutput.at(y) 
				<< ", " << peaks.at(y) << ", " << thresholdLevel.at(y) << endl;
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
		outFile3 << "Record Number" << ", " << "Number of Samples" << ", " << "Sampling Rate" << ", " << "Sampling Time" << ", " << "Number of Detected Peaks" << 
			", " << "Average R-Peak" << ", " << "Average RR Interval" << ", " << "Mean Threshold" << ", " << "Match Window" << ", " << 
			"True Positives (TP)" << ", " << "False Negatives (FP)" << ", " << "False Positives (FP)" << ", " << "Sensitivity" << ", " 
			<< "Positive Predictivity" << endl;

		int numInput=outputDataSize.size();

		for(int u=0; u < numInput; u++){
			outFile3 << mitSamples[u] << ", " << outputDataSize.at(u) << ", " << outputSampleRate.at(u) << ", " << outputSamplingTime.at(u) << ", " 
				<< outputRCount.at(u) << ", " << outputAvgRPeak.at(u) << ", " << outputAvgRRInterval.at(u) << ", " 
				<< outputMeanThreshold.at(u) << ", " << outputMatchWindow.at(u) << ", " << outputTruePositiveCount.at(u) << ", " 
				<<  outputFalseNegativeCount.at(u) << ", " << outputFalsePositiveCount.at(u) << ", " << outputSensitivity.at(u) << ", " 
				<< outputPositivePredictivity.at(u) << endl;
		}
		outFile3.close();
	}
#endif

	cout << endl << "Terminating Program" << endl << endl;

	return 0;
}
