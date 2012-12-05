/*******************************************************************************************************
 *
 * 
 *  
 * 
 * 
 * *****************************************************************************************************/
#include "..\Common\device.h"
#include "..\Common\types.h"          // Basic Type declarations
#include "..\USB_Common\descriptors.h"
#include "..\USB_Common\usb.h"        // USB- specific functions
#include "..\Common\hal_UCS.h"
#include "..\Common\hal_pmm.h"
#ifdef _CDC_
    #include "..\USB_CDC_API\UsbCdc.h"
#endif
#ifdef _HID_
    #include "..\USB_HID_API\UsbHid.h"
#endif

#include <intrinsics.h>
#include <string.h>
#include "USB_constructs.h"
#include "ADS1x9x.h"
#include "ADS1x9x_main.h"
#include "ADS1x9x_USB_Communication.h"
#include "ADS1x9x_Version.h"
#include "ADS1x9x_ECG_Processing.h"
#include "ADS1x9x_Nand_Flash.h"


#define SYSUNIV_BUSIFG	SYSUNIV_SYSBUSIV

// Function declarations

VOID Init_StartUp(VOID);
VOID Init_TimerA1(VOID);
BYTE retInString(char* string);

extern unsigned char ADS1x9xRegVal[16];
extern unsigned char SPI_Rx_buf[];
extern unsigned char ECG_Data_rdy;
extern long ADS1x9x_ECG_Data_buf[6];
extern unsigned short QRS_Heart_Rate;
extern short ECGRawData[4],ECGFilteredData[4] ;
extern unsigned short NumPackets,ReqSamples;
extern unsigned char NumFrames;
extern unsigned char Filter_Option;
extern unsigned char ECGRecorder_data_Buf[512], Recorder_head,Recorder_tail; 
//unsigned short dataCnt = 0;
//unsigned short Resp_Rr_val;
//unsigned long Page_num;
extern unsigned short Respiration_Rate ;
extern struct NANDAddress Read_Recorder_NANDAddress;
extern struct NANDAddress  Recorder_NANDAddress;
extern struct NANDAddress  Acquire_NANDAddress;
extern unsigned char *ECGPacketAcqPrt, *ECGPacketAcqHoldPrt;
extern unsigned char Dwn_NandReadBuf[256], Dwn_head, Dwn_tail;

extern unsigned int packetCounter , AcqpacketCounter;
extern unsigned short BlockNum;
extern unsigned char Store_data_rdy;

unsigned char KeyPressed = 0;
unsigned char keyCount = 0;

unsigned char Req_Dwnld_occured;

unsigned char LeadStatus = 0x0F;
// Global flags set by events
volatile BYTE bCDCDataReceived_event = FALSE;// Indicates data has been received without an open rcv operation
                 
#define MAX_STR_LENGTH 64
//char wholeString[MAX_STR_LENGTH] = "";     // The entire input string from the last 'return'
unsigned int SlowToggle_Period = 20000-1;
unsigned int FastToggle_Period = 2000-1;
unsigned short Two_5millisec_Period = 60000;

unsigned int EcgPtr =0;
unsigned char regval, Live_Streaming_flag = FALSE;
extern void XT1_Stop(void);

unsigned char ECGTxPacket[64],ECGTxCount,ECGTxPacketRdy ;
unsigned char ECGRxPacket[64],ECGRxCount, dumy ;

struct ADS1x9x_state ECG_Recoder_state;
extern unsigned short Respiration_Rate;
unsigned short timeCtr =0;

union ECG_REC_Data_Packet {
	unsigned char ucECG_data_rec[32];
	short sECG_data_rec[16];
};
unsigned char ECG_Proc_data_cnt = 0;

union ECG_REC_Data_Packet ECG_REC_Proc_Data_Packet;

void Init_KBD(void)
{
	P1DIR &= ~(BIT6|BIT7);				// Set Bit As input
	P1SEL &= ~(BIT6|BIT7);				// Set Bit As input
}

/*******************************************************************************************************
 * 
 * 
 * 
 * *****************************************************************************************************/
void Decode_Recieved_Command(void)
{
	if (ECG_Recoder_state.state == IDLE_STATE)
	{
			switch(ECG_Recoder_state.command)
			{
				unsigned short strcpy_i;
		       	case WRITE_REG_COMMAND: 	// Write Reg
				{

		              if ( (ECGRxPacket[2] < 12)) 
		              {
		            	  TA1CTL &= ~MC_1;                          // Turn off Timer                     
						  P5OUT &= ~BIT0;                           // Clear P5.0 LED1
						  P5OUT |= BIT1;                            // Set P5.1 LED2

						  ADS1x9x_Disable_Start();

						  Disable_ADS1x9x_DRDY_Interrupt();
						  Stop_Read_Data_Continuous();				// SDATAC command
						  
		                  ADS1x9x_Reg_Write (ECGRxPacket[2], ECGRxPacket[3]);
		                  ADS1x9xRegVal[ECGRxPacket[2]] = ECGRxPacket[3];
						  
//						  __delay_cycles(30);
//						  Start_Read_Data_Continuous();			//RDATAC command
//						  __delay_cycles(30);
//						  Enable_ADS1x9x_DRDY_Interrupt();

						  ECG_Data_rdy = 0;
						  P5OUT &= ~BIT0;                           // Clear P5.0 LED1
						  P5OUT &= ~BIT1;                           // Clear P5.1 LED2
	            	   	  TA1CTL |= MC_1;                           // Turn ON Timer                     
						  Set_Device_out_bytes();
		              }
		              else
		              {
		              	ECGRxPacket[2]  =0;
		              	ECGRxPacket[3]  =0;
		              }

					  for (strcpy_i = 0; strcpy_i < 7; strcpy_i++)
					  {		                      
		              	ECGTxPacket[strcpy_i]=ECGRxPacket[strcpy_i];  // Prepare the outgoing string
					  }
		              
					   ECGTxCount = 7;
					   ECGTxPacketRdy = TRUE;
					
		  		}
		  		break;
		       	case READ_REG_COMMAND: 	// Read Reg
				{
		
		              if ( (ECGRxPacket[2] < 12)) 
		              {
	            	   	  TA1CTL &= ~MC_1;                          // Turn off Timer                     
						  P5OUT |= BIT1;                            // Set P5.1 LED2
						  P5OUT &= ~BIT0;                           // Clear P5.0 LED1

						  ADS1x9x_Disable_Start();					// Disable Start

					  	  Disable_ADS1x9x_DRDY_Interrupt();
					  	  Stop_Read_Data_Continuous();				// SDATAC command

		                  ECGRxPacket[3] = ADS1x9x_Reg_Read (ECGRxPacket[2]);
		                  ADS1x9xRegVal[ECGRxPacket[2]] = ECGRxPacket[3];
						  __delay_cycles(300);
					      ECG_Data_rdy = 0;
					  	  ECG_Recoder_state.state = IDLE_STATE;
		                  P5OUT &= ~BIT0;                           // Clear P5.0 LED1
					  	  P5OUT &= ~BIT1;                           // Clear P5.1 LED2
		                  TA1CTL |= MC_1;                           // Turn ON Timer                     
		                  
		              }
		              else
		              {
		              	ECGRxPacket[2]  =0;
		              	ECGRxPacket[3]  =0;
		              }
					  for (strcpy_i = 0; strcpy_i < 7; strcpy_i++)
					  {		                      
		              	ECGTxPacket[strcpy_i]=ECGRxPacket[strcpy_i];                                      // Prepare the outgoing string
					  }
		              
					   ECGTxCount = 7;
					   ECGTxPacketRdy = TRUE;
		  		}
		  		break;
		       	case DATA_STREAMING_COMMAND: 	// Data streaming
				{
					
					  TA1CTL &= ~MC_1;                          // Turn off Timer                     
					  ADS1x9x_Disable_Start();					// Disable START (SET START to high)

					  Clear_ADS1x9x_Chip_Enable(); 				// CS=HIgh
				      P5OUT &= ~BIT0;                           // clear P5.0 LED1
				      P5OUT &= ~BIT1;                           // clear P5.1 LED2
					  __delay_cycles(300);
					  Set_ADS1x9x_Chip_Enable();				// CS=Low

					  Start_Read_Data_Continuous();				//RDATAC command
					  __delay_cycles(30);
					  Enable_ADS1x9x_DRDY_Interrupt();			// Enable DRDY interrupt

					  ADS1x9x_Enable_Start();					// Enable START (SET START to high)

		  			  ECG_Recoder_state.state =DATA_STREAMING_STATE;	// Set to Live Streaming state
					  Live_Streaming_flag = TRUE;				// Set Live Streaming Flag
					  ECG_Data_rdy = 0;
					  TA1CTL |= MC_1;                          	// Turn ON Timer                     
					  
		  		}
		  		break;
		       	case ACQUIRE_DATA_COMMAND: 	// Acquire Data 
				{
					short i_acq;
					if ( (ECGRxPacket[2] | ECGRxPacket[3]) && ((ADS1x9xRegVal[1] & 0x80) != 0x80) )
					{
						TA1CTL &= ~MC_1;                        // Turn off Timer                     
					    P5OUT &= ~BIT0;                         // clear P5.0 LED1
					    P5OUT |= BIT1;                          // Set P5.1 LED2
	
						ADS1x9x_Disable_Start();				// Disable START (SET START to high)
	
						Set_ADS1x9x_Chip_Enable();				// CS = 0
						__delay_cycles(300);
						Clear_ADS1x9x_Chip_Enable();			// CS = 1
	
				   	   	UCB0CTL1 |= UCSWRST;               		// Enable SW reset
						UCB0CTL0 |= UCMSB+UCMST+UCSYNC;			//[b0]   1 -  Synchronous mode 
																//[b2-1] 00-  3-pin SPI
																//[b3]   1 -  Master mode
																//[b4]   0 - 8-bit data
																//[b5]   1 - MSB first
																//[b6]   0 - Clock polarity low.
																//[b7]   1 - Clock phase - Data is captured on the first UCLK edge and changed on the following edge.
					
						UCB0CTL1 |= UCSSEL__ACLK;               // ACLK
						UCB0BR0 = 2;                            // 12 MHz
						UCB0BR1 = 0;                            //
						UCB0CTL1 &= ~UCSWRST;              		// Clear SW reset, resume operation
						
						NumPackets = ECGRxPacket[2];			// Parse the 16 bit sample count
						NumPackets = NumPackets << 8;
						NumPackets |= ECGRxPacket[3];
						ReqSamples = NumPackets;
						NumFrames = 0;
					    ECGTxCount = 7;
					    ECGTxPacketRdy = TRUE;
						BlockNum =5;
						//if ((ADS1x9xRegVal[1] & 0x07) == 6)
						if ((ADS1x9xRegVal[1] & 0x07) == 6)
						{					
							NAND_Init();
							NAND_Reset();
							__delay_cycles(300);  				    
							__delay_cycles(3000);  				    
							for ( i_acq= 1; i_acq < 4096; i_acq++)
							{
								NAND_EraseBlock(i_acq);			// Erase blank  
							}
	
						    ECGTxPacketRdy = FALSE;
						    Store_data_rdy =0;
						}
						Recorder_head = 0;
						Recorder_tail =0;
						Acquire_NANDAddress.usBlockNum = BlockNum;
						Acquire_NANDAddress.ucPageNum = 0;
						Acquire_NANDAddress.usColNum = 0;
						AcqpacketCounter = 0;
						
						Set_ADS1x9x_Chip_Enable();				// CS =0
						__delay_cycles(300);  				    
						Start_Read_Data_Continuous();			//RDATAC command
						__delay_cycles(300);
		  				ECG_Recoder_state.state = ACQUIRE_DATA_STATE;	// state = ACQUIRE_DATA_STATE
						ECG_Data_rdy = 0;
						Enable_ADS1x9x_DRDY_Interrupt();		// Enable DRDY interrupt
						ADS1x9x_Enable_Start();					// Enable START (SET START to high)
	
					  	for (strcpy_i = 0; strcpy_i < 7; strcpy_i++)
					  	{		                      
		              		ECGTxPacket[strcpy_i]=ECGRxPacket[strcpy_i];  // Prepare the outgoing string
					  	}
						TA1CTL |= MC_1;                          // Turn ON Timer                     
		              
					}
					else
					{
						ECGRxPacket[2] = 0; 
						ECGRxPacket[3] = 0; 
						
					  	for (strcpy_i = 0; strcpy_i < 7; strcpy_i++)
					  	{		                      
		              		ECGTxPacket[strcpy_i]=ECGRxPacket[strcpy_i];  // Prepare the outgoing string
					  	}
					   ECGTxCount = 7;
					   ECGTxPacketRdy = TRUE;
						
					}
		  		}
		  		break;
		  		
		       	case DATA_DOWNLOAD_COMMAND: 	// RAW DATA DUMP 
				{
					Req_Dwnld_occured = TRUE;
					
				}
		  		break;
		
		       	case START_RECORDING_COMMAND: 	// Processed Data Dump 
				{
				}
		  		break;
		  		
		       	case FIRMWARE_UPGRADE_COMMAND: 	// FIRMWARE UPGRADE 
				{
					USB_disable();   							// Disable CDC connection						
		            TA1CTL &= ~MC_1;                            // Turn off Timer                     
				    Disable_ADS1x9x_DRDY_Interrupt();			// Disable interrupt 
					__delay_cycles(200);						// Delay
					((void (*)())0x1000)();						// Go to BSL
		  		}
		
		  		break;
		       	case FIRMWARE_VERSION_REQ: 	// firmware Version request 
				{
				   for (strcpy_i = 0; strcpy_i < 7; strcpy_i++)
				   {		                      
				   		ECGTxPacket[strcpy_i]=ECGRxPacket[strcpy_i];         
				   												// Prepare the outgoing string
				   }
				  
				   ECGTxPacket[2] = ADS1x9x_Major_Number;		// Firmware Major number
				   ECGTxPacket[3] = ADS1x9x_Minor_Number;		// Firmware Minor number
				   
				   ECGTxCount = 7;								// number of bytes to send 
				   ECGTxPacketRdy = TRUE;
				}
		  		break;
		
		       	case STATUS_INFO_REQ: 	// Status Request 
				{
		
				}
		  		break;
		       	case FILTER_SELECT_COMMAND: 	// Filter Select request 
				{
		              if ( (ECGRxPacket[2] < 4) && (ECGRxPacket[2] != 1) ) 
		              {
		            	  TA1CTL &= ~MC_1;                          // Turn off Timer                     
						  P5OUT &= ~BIT0;                           // Clear P5.0 LED1
						  P5OUT |= BIT1;                            // Set P5.1 LED2
						  Filter_Option =  ECGRxPacket[3];			// Filter option from user
						  P5OUT &= ~BIT0;                           // Clear P5.0 LED1
						  P5OUT &= ~BIT1;                           // Clear P5.1 LED2
	            	   	  TA1CTL |= MC_1;                           // Turn ON Timer                     
		
		              }
		              else
		              {
		              	ECGRxPacket[2]  =0;
		              	ECGRxPacket[3]  =0;
		              }

					  for (strcpy_i = 0; strcpy_i < 7; strcpy_i++)
					  {		                      
		              	ECGTxPacket[strcpy_i]=ECGRxPacket[strcpy_i];  // Prepare the outgoing string
					  }
		              
					   ECGTxCount = 7;
					   ECGTxPacketRdy = TRUE;
					
		
				}
		  		break;
		       	case ERASE_MEMORY_COMMAND: 	// MEMORY ERASE Command 
				{
					Erase_NAND_Flash();
				}
		
				default:
				
				break;					
			}
	}
	else
	{
		
			switch(ECG_Recoder_state.command)
			{

		       	case DATA_STREAMING_COMMAND: 
				{
					
					Disable_ADS1x9x_DRDY_Interrupt();		// Disable DRDY interrupt
					Stop_Read_Data_Continuous();			// SDATAC command
	  				ECG_Recoder_state.state = IDLE_STATE;	// Switch to Idle state
					Clear_ADS1x9x_Chip_Enable();			// Disable Chip select
					ECG_Data_rdy = 0;
					Live_Streaming_flag = FALSE;			// Disable Live streaming flag
				}
				break;
				default:
				
				break;
			}
	}
	ECG_Recoder_state.command = 0;
}

/*----------------------------------------------------------------------------+
| Main Routine                                                                |
+----------------------------------------------------------------------------*/
VOID main(VOID)
{
// 	unsigned char ADS1x9xegs[27];
	volatile unsigned short i, j;
//unsigned char Flash_data_buf[2048];
 	
    WDTCTL = WDTPW + WDTHOLD;	    // Stop watchdog timer
    Init_StartUp();                 //initialize device
    Init_TimerA1();
    XT1_Stop();

	ADS1x9x_PowerOn_Init();    
	Filter_Option = 3;						// Default filter option is 40Hz LowPass
   	Start_Read_Data_Continuous();			//RDATAC command
	for ( i =0; i < 10000; i++);
	for ( i =0; i < 10000; i++);
	for ( i =0; i < 10000; i++);

    ADS1x9x_Disable_Start();
	ADS1x9x_Enable_Start();

	for ( i =0; i < 10000; i++);
	for ( i =0; i < 10000; i++);
	for ( i =0; i < 10000; i++);

	NAND_Init();
	NAND_Reset();
	Get_Raw_ECG_Samples_Data_pointer();	
	/* Read ID Test */

	USB_init();
	
	Init_KBD();
	
	
    // Enable various USB event handling routines
    USB_setEnabledEvents(kUSB_VbusOnEvent+kUSB_VbusOffEvent+kUSB_receiveCompletedEvent
                          +kUSB_dataReceivedEvent+kUSB_UsbSuspendEvent+kUSB_UsbResumeEvent+kUSB_UsbResetEvent);
    
    // See if we're already attached physically to USB, and if so, connect to it
    // Normally applications don't invoke the event handlers, but this is an exception.  
    if (USB_connectionInfo() & kUSB_vbusPresent)
      USB_handleVbusOnEvent();

	  P5DIR |= BIT0+BIT1;                      // P5.0 and P5.1 set as output

	  P5OUT |= BIT0;                           // Set P5.0 LED1
	  P5OUT |= BIT1;                           // Set P5.1 LED2
	//dataCnt = 0;
	Live_Streaming_flag = 0;

	ECG_Recoder_state.state = IDLE_STATE;

	ECG_Recoder_state.command = 0;

	Set_Device_out_bytes();
// Ste Timer 5 m.sec
	TA1CTL &= ~MC_1;                             // Turn off Timer                     
	TA1CCR0 = Two_5millisec_Period;              // Set Timer Period for fast LED toggle
	TA1CTL |= MC_1;
	BlockNum = 1;
    while(1)
    {
    	
    	if ( KeyPressed == 1)
    	{
    		KeyPressed = 0;
    		if (keyCount == 1)
    		{
    			if ((ECG_Recoder_state.state == IDLE_STATE))
    			{
					
					    ADS1x9x_Disable_Start();
						ADS1x9x_Enable_Start();

						ADS1x9x_Disable_Start();					// Disable START (SET START to high)
	
						Set_ADS1x9x_Chip_Enable();					// CS = 0
						__delay_cycles(300);
						Clear_ADS1x9x_Chip_Enable();				// CS = 1
						
						__delay_cycles(30000);
						__delay_cycles(30000);
						
						Set_ADS1x9x_Chip_Enable();				// CS =0
						__delay_cycles(300);  				    
						Start_Read_Data_Continuous();			//RDATAC command
						__delay_cycles(300);
						Enable_ADS1x9x_DRDY_Interrupt();		// Enable DRDY interrupt
						ADS1x9x_Enable_Start();				// Enable START (SET START to high)

    				
    				ECG_Recoder_state.state = ECG_RECORDING_STATE;
    			}
    			else if ((ECG_Recoder_state.state == ECG_RECORDING_STATE))
    			{
    				ECG_Recoder_state.state = IDLE_STATE;
    				Disable_ADS1x9x_DRDY_Interrupt();		// Disable DRDY interrupt
					Stop_Read_Data_Continuous();			// SDATAC command
    				
    			}
    			keyCount = 0;
    			
    		}
    	}
		if (Req_Dwnld_occured == TRUE)
		{
				/* Construct 7 byte packet */
				ECGTxPacket[0] = START_DATA_HEADER;	//Start of packet
				ECGTxPacket[1] = DATA_DOWNLOAD_COMMAND;	// Download header
				ECGTxPacket[2] = 0;
				ECGTxPacket[3] = 0;
				ECGTxPacket[4] = END_DATA_HEADER;
				ECGTxPacket[5] = END_DATA_HEADER;		//endof packet
				ECGTxPacket[6] = '\n';
	
			if ( ECG_Recoder_state.state == ECG_RECORDING_STATE)
			{
				if (Recorder_NANDAddress.usColNum == 0)
				{
					
					ECG_Recoder_state.state = ECG_DOWNLOAD_STATE;
					Read_Recorder_NANDAddress.usBlockNum = RECORD_START_BLOCK;
					Read_Recorder_NANDAddress.ucPageNum = 0;
					Read_Recorder_NANDAddress.usColNum = 0;
					Req_Dwnld_occured = FALSE;
					Dwn_head = 0; Dwn_tail =0;
					cdc_sendDataInBackground((BYTE*)ECGTxPacket,7,0,0);          // Send the response over USB					
				}
			}
			else
			{
				ECG_Recoder_state.state = ECG_DOWNLOAD_STATE;
				Read_Recorder_NANDAddress.usBlockNum = RECORD_START_BLOCK;
				Read_Recorder_NANDAddress.ucPageNum = 0;
				Read_Recorder_NANDAddress.usColNum = 0;
				Req_Dwnld_occured = FALSE;
				Dwn_head = 0; Dwn_tail =0;
				cdc_sendDataInBackground((BYTE*)ECGTxPacket,7,0,0);          // Send the response over USB
			}
		}
        switch(ECG_Recoder_state.state)
        {
           case IDLE_STATE:
           {
				if ( ECG_Recoder_state.command != 0)
				{
//					Decode_Recieved_Command();
					ECG_Recoder_state.command = 0;
				}
           }	
           break;
           case DATA_STREAMING_STATE:
           {
         		Stream_ECG_data_packets();
           }
           break;

           case ACQUIRE_DATA_STATE:

	           Accquire_ECG_Samples();

                break;
           case ECG_DOWNLOAD_STATE:

	            Send_Recorded_ECG_Samples_to_USB();
	            
                break;

           case ECG_RECORDING_STATE:
           {

	           if (Recorder_head !=Recorder_tail)
	           {
	           	 	//unsigned char *ptr, ECG_Proc_data_cnt_temp;
					Recorder_tail++;							// Increment tail
					if ( (Recorder_tail % 4)  == 0)				// Reset tail after 32 samples
					{
		  				P2OUT |= BIT7;							//Debug 			

					/* After every 8 samples store  ECG data to memory */
	           			Store_Processed_ECG_Samples_to_Flash(&ECGRecorder_data_Buf[(Recorder_tail-4)<<3]);
						if ( Recorder_tail  == 32)				// Reset tail after 32 samples
							Recorder_tail = 0;

			  			P2OUT &= ~BIT7;							// Debug 			
					}
	           }
           }
           break;
                
           
           default:
                break;	
        }
        	
        switch(USB_connectionState())
        {
           case ST_USB_DISCONNECTED:
                 //__bis_SR_register(LPM3_bits + GIE); 	             // Enter LPM3 w/ interrupts enabled

                _NOP();                                              // For Debugger

                break;
                
           case ST_USB_CONNECTED_NO_ENUM:

                break;
                
           case ST_ENUM_ACTIVE:

                //__bis_SR_register(LPM0_bits + GIE); 	             // Enter LPM0 (can't do LPM3 when active)
                _NOP();                                              // For Debugger
				if(ECGTxPacketRdy == 1)
				{				
					ECGTxPacketRdy =0;
				 	cdc_sendDataInBackground((BYTE*)ECGTxPacket,ECGTxCount,0,0);          // Send the response over USB
				}
			
                // Exit LPM on USB receive and perform a receive operation 
                if(bCDCDataReceived_event)                              // Some data is in the buffer; begin receiving a command              
                {
                  unsigned char readBytes = 0;
                  // Add bytes in USB buffer to theCommand
                  readBytes = cdc_receiveDataInBuffer((BYTE*)ECGRxPacket,MAX_STR_LENGTH,0);                     // Get the next piece of the string
                  if ( readBytes > 0 && ECGRxPacket[0] == START_DATA_HEADER && ECGRxPacket[readBytes -2]== END_DATA_HEADER)
                  {
					ECG_Recoder_state.command = ECGRxPacket[1];
					Decode_Recieved_Command();
                  }
                  bCDCDataReceived_event = FALSE;
                }
					
                break;
                
           case ST_ENUM_SUSPENDED:

                //__bis_SR_register(LPM3_bits + GIE); 	        // Enter LPM3 w/ interrupts
                _NOP();
                
                break;
                
           case ST_ENUM_IN_PROGRESS:
                break;
           
           case ST_NOENUM_SUSPENDED:
                //__bis_SR_register(LPM3_bits + GIE);  
                _NOP();
                break;
                
           case ST_ERROR:
                _NOP();
                break;
                
           default:;
        }	//switch(USB_connectionState())
    
    }  // while(1) 
} //main()

/*----------------------------------------------------------------------------+
| System Initialization Routines                                              |
+----------------------------------------------------------------------------*/

// Initializes the clocks.  Starts the DCO at USB_MCLK_FREQ (the CPU freq set with the Desc 
// Tool), using the REFO as the FLL reference.  Configures the high-freq crystal, but 
// doesn't start it yet.  Takes some special actions for F563x/663x.  
VOID Init_Clock(VOID)
{
    if (USB_PLL_XT == 2)
    {
        // Enable XT2 pins
        #if defined (__MSP430F552x) || defined (__MSP430F550x)
          P5SEL |= 0x0C;                      
        #elif defined (__MSP430F563x_F663x)
          P7SEL |= 0x0C;
        #endif
          
        // Use the REFO oscillator as the FLL reference, and also for ACLK
        UCSCTL3 = (UCSCTL3 & ~(SELREF_7)) | (SELREF__REFOCLK);
        UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | (SELA__XT2CLK);
        // Start the FLL, which will drive MCLK (not the crystal)
        Init_FLL(USB_MCLK_FREQ/1000, USB_MCLK_FREQ/32768);  
    }
    else
    {
        // Enable XT1 pins
        #if defined (__MSP430F552x) || defined (__MSP430F550x)
          P5SEL |= 0x10;                    
        #endif 
        
        // Use the REFO oscillator as the FLL reference, and also for ACLK
        UCSCTL3 = SELREF__REFOCLK;             
        UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | (SELA__REFOCLK); 
        
        // Start the FLL, which will drive MCLK (not the crystal)
        Init_FLL(USB_MCLK_FREQ/1000, USB_MCLK_FREQ/32768); // set FLL (DCOCLK)
    }
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

VOID Init_StartUp(VOID)
{
    __disable_interrupt();               // Disable global interrupts
    
    SetVCore(3);                         // USB core requires the VCore set to 1.8 volt, independ of CPU clock frequency
    Init_Clock();

    __enable_interrupt();                // enable global interrupts
}


#pragma vector = UNMI_VECTOR
__interrupt VOID UNMI_ISR(VOID)
{
    switch (__even_in_range(SYSUNIV, SYSUNIV_BUSIFG))
    {
    case SYSUNIV_NONE:
      __no_operation();
      break;
    case SYSUNIV_NMIIFG:
      __no_operation();
      break;
    case SYSUNIV_OFIFG:
      UCSCTL7 &= ~(DCOFFG+XT1LFOFFG+XT1HFOFFG+XT2OFFG); // Clear OSC flaut Flags fault flags
      SFRIFG1 &= ~OFIFG;                                // Clear OFIFG fault flag
      break;
    case SYSUNIV_ACCVIFG:
      __no_operation();
      break;
    case SYSUNIV_BUSIFG:
      // If bus error occured - the cleaning of flag and re-initializing of USB is required.
      SYSBERRIV = 0;            			// clear bus error flag
      USB_disable();            			// Disable
    }
}

// TimerA Init 
VOID Init_TimerA1(VOID)
{
  TA1CCTL0 = CCIE;                          // CCR0 interrupt enabled
  TA1CTL = TASSEL_1 + TACLR;                // ACLK, clear TAR
}

// This function returns true if there's an 0x0D character in the string; and if so, 
// it trims the 0x0D and anything that had followed it.  
BYTE retInString(char* string)
{
  BYTE retPos=0,i,len;
  char tempStr[MAX_STR_LENGTH] = "";
  
  strncpy(tempStr,string,strlen(string));     // Make a copy of the string
  len = strlen(tempStr);    
  while((tempStr[retPos] != 0x0A) && (tempStr[retPos] != 0x0D) && (retPos++ < len)); // Find 0x0D; if not found, retPos ends up at len
  
  if((retPos<len) && (tempStr[retPos] == 0x0D))                          // If 0x0D was actually found...
  {
    for(i=0;i<MAX_STR_LENGTH;i++)             // Empty the buffer
      string[i] = 0x00;
    strncpy(string,tempStr,retPos);           // ...trim the input string to just before 0x0D
    return TRUE;                              // ...and tell the calling function that we did so
  }
  
  else if((retPos<len) && (tempStr[retPos] == 0x0A))                                // If 0x0D was actually found...
  {
    for(i=0;i<MAX_STR_LENGTH;i++)             // Empty the buffer
      string[i] = 0x00;
    strncpy(string,tempStr,retPos);           // ...trim the input string to just before 0x0D
    return TRUE;                              // ...and tell the calling function that we did so
  }
  else if (tempStr[retPos] == 0x0D)
    {
    for(i=0;i<MAX_STR_LENGTH;i++)             // Empty the buffer
      string[i] = 0x00;
    strncpy(string,tempStr,retPos);           // ...trim the input string to just before 0x0D
    return TRUE;                              // ...and tell the calling function that we did so
  }
    
  else if (retPos<len)
     {
    for(i=0;i<MAX_STR_LENGTH;i++)             // Empty the buffer
      string[i] = 0x00;
    strncpy(string,tempStr,retPos);           // ...trim the input string to just before 0x0D
    return TRUE;                              // ...and tell the calling function that we did so
  }
 
  return FALSE;                               // Otherwise, it wasn't found
}


// Timer1 A0 interrupt service routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
	static unsigned char kbCount = 0;
	timeCtr++;
	//P2OUT ^= BIT0;                           // Clear P2.0 debug
	
		if (((P1IN & BIT7) == 0x00))
		{
			kbCount++;
			//key press detected
			if ( kbCount > 200 )
			{
				kbCount -=1;
				keyCount = 1;
			}
		} 
		else 
		{
			if ( kbCount > 150)
			{
				kbCount =0;
				KeyPressed = 1;
			}
		}	
        if (ECG_Recoder_state.state  == IDLE_STATE)
        {
           if (timeCtr == 1)
           {
           	
				  P5OUT &= ~BIT1;                           // Clear P5.1 LED2
				  P5OUT |= BIT0;                            // Set P5.0 LED1
           }
           else if (timeCtr == 200)
           {
				  P5OUT &= ~BIT0;                           // Clear P5.0 LED1
				  P5OUT &= ~BIT1;                           // Clear P5.1 LED2
           	
           } 
           if (timeCtr > 600) timeCtr =0;
        }
        else
        {           
           if (timeCtr == 1)
           {
           	
				  P5OUT |= BIT1;                           	// Set P5.1 LED2
				  P5OUT &= ~BIT0;                           // Clear P5.0 LED1
				  timeCtr = 0;
           }
           else if (timeCtr == 150)
           {
				  P5OUT &= ~BIT0;                           // Clear P5.0 LED1
				  P5OUT &= ~BIT1;                           // Clear P5.1 LED2
           } 
           if (timeCtr > 600) timeCtr =0;
        }
            //__bis_SR_register(LPM0_bits + GIE); 	             // Enter LPM0 (can't do LPM3 when active)
            //_NOP();                                              // For Debugger

}

/*----------------------------------------------------------------------------+
| End of source file                                                          |
+----------------------------------------------------------------------------*/
/*------------------------ Nothing Below This Line --------------------------*/
