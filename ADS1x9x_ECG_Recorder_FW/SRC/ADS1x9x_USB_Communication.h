#ifndef ADS1x9x_USB_COMMUNICATION_H_
#define ADS1x9x_USB_COMMUNICATION_H_


#define START_DATA_HEADER			0x02
#define WRITE_REG_COMMAND			0x91
#define READ_REG_COMMAND			0x92
#define DATA_STREAMING_COMMAND		0x93
#define DATA_STREAMING_PACKET		0x93
#define ACQUIRE_DATA_COMMAND		0x94
#define ACQUIRE_DATA_PACKET 		0x94
#define PROC_DATA_DOWNLOAD_COMMAND	0x95
#define DATA_DOWNLOAD_COMMAND		0x96
#define FIRMWARE_UPGRADE_COMMAND	0x97
#define START_RECORDING_COMMAND		0x98
#define FIRMWARE_VERSION_REQ		0x99
#define STATUS_INFO_REQ 			0x9A
#define FILTER_SELECT_COMMAND		0x9B
#define ERASE_MEMORY_COMMAND		0x9C
#define RESTART_COMMAND				0x9D
#define END_DATA_HEADER				0x03

void Accquire_ECG_Samples(void);
void Stream_ECG_data_packets(void);

#define ECG_DATA_PACKET_LENGTH 6 // 3 Bytes (24 bits) * 1 Ch status + 2 Ch data = 3 * 3 = 9

#define ECG_ACQUIRE_PACKET_LENGTH 54		

#endif /*ADS1x9x_USB_COMMUNICATION_H_*/
