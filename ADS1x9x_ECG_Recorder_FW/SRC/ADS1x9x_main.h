#ifndef ADS1x9x_MAIN_H_
#define ADS1x9x_MAIN_H_
struct ADS1x9x_state{
	unsigned char state;
	unsigned char SamplingRate;
	unsigned char command;
};

typedef enum stECG_RECORDER_STATE {
	
	IDLE_STATE =0,
	DATA_STREAMING_STATE,
	ACQUIRE_DATA_STATE,
	ECG_DOWNLOAD_STATE,
	ECG_RECORDING_STATE
}ECG_RECORDER_STATE;

#endif /*ADS1x9x_MAIN_H_*/
