#include "stdafx.h"
#include "filter.h"
#include "detect.h"
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
#include "kiss_fft.h"
#include "kiss_fftr.h"

using namespace std;
using namespace std::tr1;

/* Scaling Factors are used to convert input ECG data to microVolts. 
MIT data is stored as milliVolts.*/
const double scalingFactorMIT= 1000.0;

/* Match window is used to determine the accuracy of the peak detection for MIT data. 
Any peaks within matchWindow of the correct value is considered "detected". */
const double matchWindow = 0.050; //in seconds

//Add to circular buffer
void addToBuffer(deque <double>& timeDQ, deque <double>& voltageDQ, double seconds, double volt){

	if (timeDQ.size() > ecg_buffer_length)
		timeDQ.pop_back();
	
	if (voltageDQ.size() > ecg_buffer_length)
		voltageDQ.pop_back();

	timeDQ.push_front(seconds);
	voltageDQ.push_front(volt);
}

int _tmain(int argc, _TCHAR* argv[])
{		


	cout << "Starting QRS Detection Program" << endl << endl; 

	string ECGDataFile; //Input File
	string processedDataFile; //Output File
	string sampleNumber; //File Number
	

	cout << "Please enter sample number: ";
	cin >> sampleNumber;
	if (sampleNumber == "none")
		sampleNumber="";
		
	ECGDataFile = "C:\\Users\\Akib\\Desktop\\sampleData\\mitdb\\samples"+sampleNumber+".csv";
	processedDataFile = "C:\\Users\\Akib\\Desktop\\sampleData\\outputRT"+sampleNumber+".csv";

	
	//Input File Parser//
	/**********************************************************************************
	Parse input data from CSV file into vector. 

	File format is as follows:
	'Elapsed Time'	'MLII Voltage'			Line 1
	'seconds'	'mV OR V'					Line 2
	0	-0.145								Line 3 onwards

	We must ignore first two lines of data as they are not data. 

	The MLII Voltage for MIT-BIH comes in milliVolts.
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
				ecgValue=(int) (temp*scalingFactorMIT); //Scaling and integerizing the leadII milliVolts to microVolts
			}	
		}

		ECGData.push_back(make_pair(secondValue, ecgValue)); //Add to end of vector array
	}
	inFile.close();

	//QRS Detection Processing//
	/************************************************************************
	************************************************************************/
	
	cout << "QRS Processing" << endl;

	//deque<pair<double, double> > ECG_Buffer_2min;
	deque <double> ECG_Buffer_Time;
	deque <double> ECG_Buffer_Voltage;

	int dataSize = ECGData.size();

	int count = 0;


	ofstream outFile;
	outFile.open(processedDataFile);
	if (!outFile){
		cout << "ERROR: The output file does not exist" << endl;
		exit (EXIT_FAILURE);
	}

	//Loop through each sample in the record

	outFile << "Time, Voltage, LP, HP, DDT, SQ, INT, Thres, #LastPeak, #2ndLastPeak, #LastQRS, returnFlag" << endl;

	int counter=0;

	for(int n=0; n < dataSize; n++){ ///replace with a live feedin (wihtout the first big data pull)

		//add to circular buffer
		addToBuffer(ECG_Buffer_Time, ECG_Buffer_Voltage, ECGData.at(n).first, ECGData.at(n).second);
		//cout << ECG_Buffer_Time.size() << endl;
	
		int x=0;
		x = QRSDetect(ECG_Buffer_Time, ECG_Buffer_Voltage, sampleNumber);
		string temp = retStr();
		outFile << temp << ", " << x << endl;

		if (x != 0){
			count++;
			//outFile << ECG_Buffer_Time.at(x) << ", " << ECG_Buffer_Voltage.at(x) << endl;
			RPeakData.push_back(make_pair(ECG_Buffer_Time.at(x), ECG_Buffer_Voltage.at(x)));
		}

		if(counter==10000){
			cout << 100.0*n/dataSize << endl; // progress bar
			counter=0;
		}
		counter++;

	}

	cout << "QRS Found: " << count << endl;








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
	for(int i=0; i < RPeakData.size(); i++){ //search through foundPeaks list
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

	cout << "Performance Summary" << endl;
	cout << "Number of Samples: " << ECGData.size() << endl;
	cout << "Sampling Rate: " << SAMPLERATE << " Hz" << endl;
	cout << "Sampling Time: " << (ECGData.at(dataSize-1).first - ECGData.at(0).first) << " s" << endl;
	cout << "Number of Detected Peaks: " << RPeakData.size() << endl;
	//cout << "Average RR Interval: " << avgRRInterval << " s" << endl;

	double sensitivity = 100.0*((truePositiveCount)/(truePositiveCount+falseNegativeCount));
	double positivePredictivity = 100.0*((truePositiveCount)/(truePositiveCount+falsePositiveCount));

	cout << "Match Window: " << matchWindow <<  " s" <<endl;
	cout << "True Positives (TP): " << truePositiveCount << endl;
	cout << "False Negatives (FP): "<< falseNegativeCount << endl;
	cout << "False Positives (FP): " << falsePositiveCount << endl;
	cout << "Sensitivity: " << sensitivity << " %" << endl; 
	cout << "Positive Predictivity: " << positivePredictivity << " %" << endl; 

	outFile << endl;
	outFile << "Performance Summary" << endl;
	outFile << "Number of Samples:," << ECGData.size() << endl;
	outFile << "Sampling Rate:," << SAMPLERATE << endl;
	outFile << "Sampling Time:," << (ECGData.at(dataSize-1).first - ECGData.at(0).first) << endl;
	outFile << "Number of Detected Peaks:," << RPeakData.size() << endl;
	//outFile << "Average RR Interval: " << avgRRInterval << endl;
	outFile << "Match Window:," << matchWindow <<  endl;
	outFile << "True Positives (TP):," << truePositiveCount << endl;
	outFile << "False Negatives (FP):,"<< falseNegativeCount << endl;
	outFile << "False Positives (FP):," << falsePositiveCount << endl;
	outFile << "Sensitivity:," << sensitivity << endl; 
	outFile << "Positive Predictivity:," << positivePredictivity <<endl; 

	//Output all detected peaks and times
	outFile << endl << endl;
	outFile << "Time (s), MLII (mV), " << endl;
	for (int n=0; n < RPeakData.size(); n++){
		outFile << RPeakData.at(n).first << ", " << RPeakData.at(n).second << endl;
	}

	//Output TP, FP and FN
	outFile << endl;
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




	outFile.close();

	return 0;
}

