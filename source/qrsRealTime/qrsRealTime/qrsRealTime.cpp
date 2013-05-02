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

	outFile << "Time, Voltage, LP, HP, DDT, SQ, INT" << endl;

	for(int n=0; n < dataSize; n++){ ///replace with a live feedin (wihtout the first big data pull)

		//add to circular buffer
		addToBuffer(ECG_Buffer_Time, ECG_Buffer_Voltage, ECGData.at(n).first, ECGData.at(n).second);
		//cout << ECG_Buffer_Time.size() << endl;
	
		int x=0;
		x = QRSDetect(ECG_Buffer_Time, ECG_Buffer_Voltage);
		string temp = retStr();
		outFile << temp;

		if (x != 0){
			count++;
			//outFile << ECG_Buffer_Time.at(x) << ", " << ECG_Buffer_Voltage.at(x) << endl;
			RPeakData.push_back(make_pair(ECG_Buffer_Time.at(x), ECG_Buffer_Voltage.at(x)));
		}

	}

	cout << "QRS Found: " << count << endl;

	outFile << endl <<"Number of Detected Peaks, " << count << endl;
	outFile << "Time (s), MLII (mV), " << endl;
	for (int n=0; n < RPeakData.size(); n++){
		outFile << RPeakData.at(n).first << ", " << RPeakData.at(n).second << endl;
	}



	


	outFile.close();

	return 0;
}

