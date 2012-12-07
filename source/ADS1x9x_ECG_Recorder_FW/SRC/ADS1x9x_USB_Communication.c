/****************************************************************************************************************************************************
*	ADS1x9x_USB_Communication.c  - Provides interface to Labview																					*
* 		Functions: 																																	*
* 				1.Accquire_ECG_Samples();																																	*
*				2.Stream_ECG_data_packets(); 						 							                                                  								*							*
*												                                                  													*
*****************************************************************************************************************************************************/


#include "..\Common\device.h"
#include "..\Common\types.h"          // Basic Type declarations
#include "..\USB_CDC_API\UsbCdc.h"
#include "ADS1x9x_USB_Communication.h"
#include "ADS1x9x_main.h"
#include "ADS1x9x.h"
#include "USB_constructs.h"
#include "ADS1x9x_Nand_Flash.h"
#include "ADS1x9x_ECG_Processing.h"

/**************************************************************************************************************************************************
*	        Prototypes									                                                  										  *
**************************************************************************************************************************************************/
void Accquire_ECG_Samples(void);
/**************************************************************************************************************************************************
*	        Global Variables										                                  											  *
**************************************************************************************************************************************************/
unsigned short NumPackets,ReqSamples;
unsigned char NumFrames;
unsigned char NumFrames1;

extern unsigned char ECGTxPacket[64],ECGTxCount,ECGTxPacketRdy , SPI_Rx_buf[];
extern unsigned char ECG_Data_rdy;
extern unsigned char ADS1x9xRegVal[];
extern struct ADS1x9x_state ECG_Recoder_state;
extern unsigned short timeCtr;
extern unsigned int packetCounter , AcqpacketCounter;
extern struct NANDAddress  Acquire_NANDAddress;
extern long ADS1x9x_ECG_Data_buf[6];
extern short ECGRawData[4],ECGFilteredData[4] ;
extern unsigned short QRS_Heart_Rate;
extern unsigned short Respiration_Rate ;
extern unsigned char LeadStatus;

unsigned short BlockNum, t1;

unsigned char *ECGPacketAcqPrt, *ECGPacketAcqHoldPrt;

/**************************************************************************************************************************************************
*	        Global function										                                  											  *
**************************************************************************************************************************************************/
extern unsigned char ECGRecorder_data_Buf[512], Recorder_head,Recorder_tail;
//unsigned char ECGRecorder_ACQdata_Buf[64];
unsigned char Store_data_rdy;
/* =============================================================================================
 * Function : Accquire_ECG_Samples
 *
 * 	Parameters:
 * 		 : None.
 * 
 * 	Description:
 *         This function acquire requested number of samples and send packet by packet to PC application. It uses
 * 			1. ECGRecorder_data_Buf[]:Produced by ISR and consumed by main
 * 			2. Recorder_head		:Used by ISR
 * 			3. Recorder_tail		: Used by main
 * 			4. 4KSPS and below sampling rate data foworded to USB port packet by packet of 8 samples.
 * 			5. 8KSPS case adc data is stored first in memory and after storing all samples then the sored samples are forworded to USB port.
 *			
 * 	Return code:
 * 		NONE
 * 
 * ============================================================================================== */
void Accquire_ECG_Samples(void)
{
	unsigned char *ptr;
   	//unsigned short cPointer;
   
   if ( Recorder_head != Recorder_tail)
   {
		if ((ADS1x9xRegVal[1] & 0x07) == 6)
		{
				while ( Recorder_tail != Recorder_head)
				{
					Recorder_tail++;							// Increment tail

					if ( (Recorder_tail % 8)  == 0)				// Reset tail after 32 samples
					{
		  			P2OUT |= BIT7;								//Debug 			
						NumPackets -= 8;						// Reduce number of samples to be captured by 8
					/* After every 8 samples store  ECG data to memory */
						Store_Acquired_ECG_Samples_to_Flash(&ECGRecorder_data_Buf[(Recorder_tail-8)<<3] , NumPackets);
						if ( Recorder_tail  == 32)				// Reset tail after 32 samples
							Recorder_tail = 0;
			  		P2OUT &= ~BIT7;								// Debug 			
					}
					if ( NumPackets == 0)
					{
						/* Terminate Acquired data after specified sample count*/
						Disable_ADS1x9x_DRDY_Interrupt();		// Disable DRDY interrupt
						Stop_Read_Data_Continuous();			// SDATAC command
	
			   	   		ECG_Recoder_state.state = IDLE_STATE;	// Switch to Idle state

						P5OUT |= BIT0;                          // Set P5.0 LED1 Change the LED staus to IDLE
						P5OUT &= ~BIT1;                         // Clear P5.1 LED2 Change the LED staus to IDLE
	
						__delay_cycles(500);
						
						/* Set SPI clock to 1 MHz */
						
			   	   		UCB0CTL1 |= UCSWRST;               		// Enable SW reset
						UCB0CTL0 |= UCMSB+UCMST+UCSYNC;			//[b0]   1 -  Synchronous mode 
																//[b2-1] 00-  3-pin SPI
																//[b3]   1 -  Master mode
																//[b4]   0 - 8-bit data
																//[b5]   1 - MSB first
																//[b6]   0 - Clock polarity low.
																//[b7]   1 - Clock phase - Data is captured on the first UCLK edge and changed on the following edge.
					
						UCB0CTL1 |= UCSSEL__ACLK;              	// ACLK
						UCB0BR0 = 24;                           // 1 MHz
						UCB0BR1 = 0;                            // Hi-prescaler
						UCB0CTL1 &= ~UCSWRST;              		// Clear SW reset, resume operation
						__delay_cycles(500);
						timeCtr =199;	   	
	
						/* Construct 7 byte packet */
						ECGTxPacket[0] = START_DATA_HEADER;	//Start of packet
						ECGTxPacket[1] = ACQUIRE_DATA_COMMAND;	// Acquire header
						ECGTxPacket[2] = (unsigned char)((ReqSamples >> 8) & 0xFF);
						ECGTxPacket[3] = (unsigned char)(ReqSamples & 0xFF);
						ECGTxPacket[4] = END_DATA_HEADER;
						ECGTxPacket[5] = END_DATA_HEADER;		//endof packet
						ECGTxPacket[6] = '\n';
						
						cdc_sendDataInBackground((BYTE*)&ECGTxPacket,7,0,0);          // Send the response over USB
											
						Recorder_head = 0;					// Reset Head
						Recorder_tail =0;					// Reset tail
						Acquire_NANDAddress.usBlockNum = 5;	//Set Memory block 
						Acquire_NANDAddress.ucPageNum = 0;	// Set Memory page
						Acquire_NANDAddress.usColNum = 0;	// Set Memory colum
						AcqpacketCounter = 0;				// Initialize pointer
						/* Send stored data from memory*/
						Send_Acquired_ECG_Samples_to_USB(ReqSamples);
					}	//if ( NumPackets == 0)
				}	//while ( Recorder_tail != Recorder_head)
		}	// if ((ADS1x9xRegVal[1] & 0x07) == 6)
		else
		{
		   	ECGPacketAcqPrt = &ECGTxPacket[NumFrames * ECG_DATA_PACKET_LENGTH + 4]; //Packet pointer
			ptr = &ECGRecorder_data_Buf[(Recorder_tail << 3)+ 2];	//pointer for data read from circular buffer

		  	*ECGPacketAcqPrt++ = *ptr++;			//CH0 D15-D8
		  	*ECGPacketAcqPrt++ = *ptr++;			//CH0 D15-D8
		  	*ECGPacketAcqPrt++ = *ptr++;			//CH0 D7-D0
		  	*ECGPacketAcqPrt++ = *ptr++;			//CH1 D23-D16
		  	*ECGPacketAcqPrt++ = *ptr++;			//CH1 D15-D8
		  	*ECGPacketAcqPrt++ = *ptr++;			//CH1 D7-D0

			Recorder_tail++;						// Increment tail
			if ( Recorder_tail == 32)				// Reset tail after 32 samples
				Recorder_tail = 0;					
			
			NumFrames++;							// increment packet counter
			if (NumFrames == 8)
			{
				NumFrames = 0;						// Reset packet counter

			/* After every 8 samples send  ECG data to USB port */
				
				*ECGPacketAcqPrt++ = END_DATA_HEADER;			// End-of-packet
				//*ECGPacketAcqPrt++ = '\n';					//  
				
				ECGTxPacket[0]= START_DATA_HEADER;				//Start-of-packet.
				ECGTxPacket[1]= ACQUIRE_DATA_PACKET;			// Packet header
				
				ECGTxPacket[2]= ECGRecorder_data_Buf[Recorder_tail<<3];			//Status
				ECGTxPacket[3]= ECGRecorder_data_Buf[(Recorder_tail <<3) + 1];	//Status

				NumPackets -= 8;					// Reduce numer of samples by 8
				
				/* Send packet of 8 smples in packet of length ECG_ACQUIRE_PACKET_LENGTH */		
				cdc_sendDataInBackground((BYTE*)&ECGTxPacket,ECG_ACQUIRE_PACKET_LENGTH,0,0);          // Send the response over USB
				
				if ( NumPackets == 0)
				{
				/* Terminate Acquired data after specified sample count*/

					Disable_ADS1x9x_DRDY_Interrupt();		// Disable DRDY interrupt
					Stop_Read_Data_Continuous();			// SDATAC command
					__delay_cycles(500);

					/* Set SPI Clock to 1 MHz */
		   	   		UCB0CTL1 |= UCSWRST;               		// Enable SW reset
					UCB0CTL0 |= UCMSB+UCMST+UCSYNC;			//[b0]   1 -  Synchronous mode 
															//[b2-1] 00-  3-pin SPI
															//[b3]   1 -  Master mode
															//[b4]   0 - 8-bit data
															//[b5]   1 - MSB first
															//[b6]   0 - Clock polarity low.
															//[b7]   1 - Clock phase - Data is captured on the first UCLK edge and changed on the following edge.
				
					UCB0CTL1 |= UCSSEL__ACLK;              	// ACLK
					UCB0BR0 = 24;                           // 1 MHz
					UCB0BR1 = 0;                            //
					UCB0CTL1 &= ~UCSWRST;              		// Clear SW reset, resume operation
					__delay_cycles(500);
					timeCtr =199;	   	
		   	   		ECG_Recoder_state.state = IDLE_STATE;	// Switch to Idle state
				}	//if ( NumPackets == 0)
			}	//if (NumFrames == 8)
		}	// else of if ((ADS1x9xRegVal[1] & 0x07) == 6)
   }	// if ( Recorder_head != Recorder_tail)
   
} // void Accquire_ECG_Samples(void)

/*******************************************************************************************************
 * Function : Stream_ECG_data_packets()
 * 
 * 	Input 	: ECG_Data_rdy : ADC sets this flag after reading ADC readings.
 * 			: ADS1x9x_ECG_Data_buf[]: Satatus,ECG&RESP dada
 * 			: sampleCNT : Static variable to track number of samples in a packet
 * 
 * Constants: PACK_NUM_SAMPLES Number of samples in a packet
 * 	
 * 	Output	:
 * 			: ECGTxPacket[] : Tx array.
 * 			: ECGTxPacketRdy : Flag to indicate datapacket is ready
 * 			: ECGTxCount : Holds number of bytes need to send
 * *****************************************************************************************************/

void Stream_ECG_data_packets(void)
{
unsigned char StreamCount = 0, ucLoopCnt;
static unsigned char sampleCNT = 0;
#define PACK_NUM_SAMPLES 14
   if (  ECG_Data_rdy ==1)
   {
	   ECG_Data_rdy = 0;
	   ADS1x9x_Filtered_ECG();
	   if ( sampleCNT == 0)
	   {
	   	   StreamCount = 0;
		   ECGTxPacket[StreamCount++]= START_DATA_HEADER;				// Packet start Header
		   ECGTxPacket[StreamCount++]= DATA_STREAMING_PACKET;			// Live ECG Streaming Header
		   LeadStatus = 0x00;
		   ADS1x9x_ECG_Data_buf[0] = ADS1x9x_ECG_Data_buf[0] & 0x0F8000;
		   ADS1x9x_ECG_Data_buf[0] = ADS1x9x_ECG_Data_buf[0] >> 15;
		   LeadStatus = (unsigned char ) ADS1x9x_ECG_Data_buf[0];
	
		   // Set the Current Heart rate//
		   ECGTxPacket[StreamCount++] = QRS_Heart_Rate;					// Heart Rate
		   
		   // Set the current Leadoff status//
		   ECGTxPacket[StreamCount++] = Respiration_Rate;				// Respiration Rate
		   ECGTxPacket[StreamCount++] = LeadStatus ;					// Lead Status
	   }
	   if ( sampleCNT > PACK_NUM_SAMPLES) sampleCNT =0;
	   StreamCount = sampleCNT << 2;									// Get Packet pointer
	   StreamCount+=5;													// Offset of 5 bytes header
	   for ( ucLoopCnt =0 ; ucLoopCnt < 2; ucLoopCnt++)
	   {
		   ECGTxPacket[StreamCount++]= (ECGFilteredData[ucLoopCnt] & 0x00FF);			// High Byte B15-B8
		   ECGTxPacket[StreamCount++]= (ECGFilteredData[ucLoopCnt] & 0xFF00) >> 8 ;		// Low byte B7-B0
	   }	   
	   sampleCNT++;
	   if ( sampleCNT == PACK_NUM_SAMPLES)
	   {
		   sampleCNT = 0;
		   ECGTxPacket[StreamCount++]= END_DATA_HEADER;	// Packet end header
		   ECGTxPacket[StreamCount++]= '\n';
		   ECGTxPacketRdy = TRUE;						// Set packet ready flag after every 14th sample.
		   ECGTxCount = StreamCount;					// Define number of bytes to send as 54.
	   }
	}
}


