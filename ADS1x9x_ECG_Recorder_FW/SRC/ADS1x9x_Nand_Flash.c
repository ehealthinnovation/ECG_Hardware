/*******************************************************************************
*
*    File Name:  NAND_IO.c
*     Revision:  2.0
*  Part Number:  MT29F
*
*  Description:  Micron NAND I/O Driver
*
*
* *****************************************************************************/

#include "..\Common\device.h"
#include "..\Common\types.h"          // Basic Type declarations
#include "ADS1x9x.h"
#include "ADS1x9x_Nand_Flash.h"
#include "ADS1x9x_Nand_LLD.h"
#include "ADS1x9x_Version.h"
#include "..\USB_CDC_API\UsbCdc.h"
#include "ADS1x9x_main.h"
#include "ADS1x9x_USB_Communication.h"
#include "USB_constructs.h"
#include <string.h>

#define PAGE_SIZE				2112
#define PACKETS_PER_PAGE		66
#define ACQ_PACKETS_PER_PAGE	33
#define PAGES_PER_BLOCK			64
#define MAX_BLOCKS				4096	
#define RECORD_START_PAGE 		64

 
#define MAX_PAGES PAGES_PER_BLOCK*MAX_BLOCKS
/*
 * 
 * -------------------------------------------------------------
 * |Cycle   |I/O7 | I/O6 | I/O5 | I/O4 |I/O3 |I/O2 |I/O1 |I/O0 |
 * |--------|-----|------|------|------|-----|-----|-----|-----|
 * | First  | CA7 | CA6  | CA5  | CA4  | CA3 | CA2 | CA1 | CA0 | // CA[0-11] - cols[0-2111]
 * | Second | LOW | LOW  | LOW  | LOW  | CA11| CA10| CA9 | CA8 |
 * | Third  | BA7 | BA6  | PA5  | PA4  | PA3 | PA2 | PA1 | PA0 | //PA[0-5] - Page Number's 0-63
 * | Fourth | BA15| BA14 | BA13 | BA12 | BA11| BA10| BA9 | BA8 |
 * | Fifth  | LOW | LOW  | LOW  | LOW  | LOW | LOW | LOW | BA16| // BA - Number of Blocks
 * |-----------------------------------------------------------|
*/

struct NANDAddress  Recorder_NANDAddress;
struct NANDAddress  Read_Recorder_NANDAddress;
struct NANDAddress  Acquire_NANDAddress;

//struct ECGPage ECGPageBuf1_data = {0};
unsigned char HeadrPacket[32] = {"ADS1x9x_ECG_Recorder\n           "};
	
unsigned char ECGRecorder_data_Buf[512], Recorder_head,Recorder_tail;
unsigned int packetCounter = 0, AcqpacketCounter = 0;
extern unsigned char ECG_Proc_data_cnt;
//extern unsigned char bankFlag;
//extern unsigned char ECG_recorderReadyFlag;
extern struct ADS1x9x_state ECG_Recoder_state;
extern unsigned char ECGTxPacket[64],ECGTxCount,ECGTxPacketRdy ;
extern unsigned char ADS1x9xRegVal[];
unsigned char Dwn_NandReadBuf[256], Dwn_head, Dwn_tail;
	
/*-------------------------------------------------------------------------------------------*/
short NAND_Init()
{
	Flash_MT298G08AAAWP_init();
	packetCounter = 0;
	
	Recorder_head = Recorder_tail =0;
	ECG_Proc_data_cnt = 0;
	
	return NAND_IO_RC_PASS;
} /* NAND_Init */


/* ************************************************************************************************
 * Function : NAND_Reset
 * Parameters:	None
 * 				
 * Description:
 * 				This function resets the NAND device using the 0xFF command.
 * 
 * Return code:
 * 				NAND_IO_RC_PASS =		0 : The function completes operation successfully.
 * 				NAND_IO_RC_FAIL =		1 : The function does not complete operation successfully.
 * 				NAND_IO_RC_TIMEOUT =	2 : The function times out before operation completes.
 * 
 * ************************************************************************************************/
short NAND_Reset(void)
{
	short rc = 0;

//	Clear_CE(); 		// Enable Chip Select
	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_CE;

/* Issue  command */
	WRITE_NAND_CLE(NAND_CMD_RESET);

//	Set_CE(); 			// Disable Chip Select
	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_CE;

	return rc;
} /* NAND_Reset */


/* ================================================================================================
Function : NAND_ReadID

	Parameters:
		a_ReadID	Buffer for Read ID bytes
	Description:
        The ReadID function is used to read 4 bytes of identifier code which is  
		programmed into the NAND device at the factory.  

	Return code:
		None

================================================================================================ */
short NAND_ReadID(unsigned char *ucReadID)
{
//	int i;

//	Clear_CE(); 		// Enable Chip Select
	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_CE;

/* Issue  command */
	WRITE_NAND_CLE(NAND_CMD_READ_ID);		

/* Issue address */
	WRITE_NAND_ALE(0x00);	
	
//	for (i=0;i<4;i++)							/* Read 4 bytes from the device */
//		READ_NAND_BYTE(ucReadID[i]);	
	
	READ_NAND_ARRAY(ucReadID, 4);

//	Set_CE(); 			// Disable Chip Select
	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_CE;


	return 0;

} /* NAND_ReadID */

/* ================================================================================================
Function : NAND_ReadStatus

	Parameters:
		None
	Description:
        This function Reads the status register of the NAND device by issuing a 0x70 command.	

	Return code:
		NAND_IO_RC_PASS		=0 : The function completes operation successfully.
		NAND_IO_RC_FAIL		=1 : The function does not complete operation successfully.
		NAND_IO_RC_TIMEOUT	=2 : The function times out before operation completes.

================================================================================================ */
short NAND_ReadStatus(void)
{
	int nReadStatusCount;
	unsigned char ucStatus;

/* Issue command */
	WRITE_NAND_CLE(NAND_CMD_READ_STATUS);
	nReadStatusCount = 0;
	while(nReadStatusCount < MAX_READ_STATUS_COUNT)
	{
		/* Read status byte */
		READ_NAND_BYTE(ucStatus);
		/* Check status */
		if((ucStatus & STATUS_BIT_6) == STATUS_BIT_6)  /* If status bit 6 = 1 device is ready */
		{
			if((ucStatus & STATUS_BIT_0) == 0)	/* If status bit 0 = 0 the last operation was succesful */
				return NAND_IO_RC_PASS;
			else
				return NAND_IO_RC_FAIL;
		}			
		nReadStatusCount++;
	}
	return NAND_IO_RC_TIMEOUT;
} /* NAND_ReadStatus */




/* ================================================================================================
Function : NAND_ReadUniqueID

	Parameters:
	a_usReadSizeByte : Number of byte to read
	ucReadUniqueID : Read data buffer

	Description:
        This function reads the Unique ID information from the NAND device.  The 16 byte Unique ID information 
		starts at byte 0 of the page and is followed by 16 bytes which are the complement of the first 16 bytes.  
		These 32 bytes of data are then repeated 16 times to form the 512 byte Unique ID> 	

	Return code:
		NAND_IO_RC_PASS =		0 : The function completes operation successfully.
		NAND_IO_RC_FAIL =		1 : The function does not complete operation successfully.
		NAND_IO_RC_TIMEOUT =	2 : The function times out before operation completes.

==================================================================================================== */
short NAND_ReadUniqueID(unsigned short a_usReadSizeByte,
unsigned char *ucReadUniqueID)
{
	short rc;
//	unsigned char ucData;
//	Clear_CE(); 		// Enable Chip Select
	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_CE;

/* Issue Unique ID commands */
	WRITE_NAND_CLE(NAND_CMD_PAGE_READ_CYCLE2);
	WRITE_NAND_CLE(NAND_CMD_READ_UNIQUE_ID);	
	WRITE_NAND_CLE(NAND_CMD_PAGE_READ_CYCLE1);

/*Issue Unique ID address;*/
	WRITE_NAND_ALE(0x00);
	WRITE_NAND_ALE(0x00);
	WRITE_NAND_ALE(0x02);
	WRITE_NAND_ALE(0x00);
	WRITE_NAND_ALE(0x00);
	
/* Issue Unique ID commands */
	WRITE_NAND_CLE(NAND_CMD_PAGE_READ_CYCLE2);

	rc = NAND_ReadStatus();						/* Wait for status */                                        
	if(rc != NAND_IO_RC_PASS)
		return rc;

	/* Issue Command - Set device to read from data register. */
	WRITE_NAND_CLE(NAND_CMD_PAGE_READ_CYCLE1);					

	READ_NAND_ARRAY(ucReadUniqueID, a_usReadSizeByte);

//	Set_CE(); 			// Disable Chip Select
	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_CE;

	return rc;
} /* NAND_ReadUniqueID */




/* ================================================================================================
Function : NAND_ReadPage

	Parameters:
	a_uiPageNum : Page number for reading
	a_usColNum : Column number for reading
	a_usReadSizeByte : Number of byte to read
	a_pucReadBuf : Read data buffer

	Description:
        This function reads data from the input page a_uiPageNum and column a_usColNum number into the 
		buffer a_pucReadBuf for a_usReadSizeByte number of bytes.	

	Return code:
		NAND_IO_RC_PASS =		0 : The function completes operation successfully.
		NAND_IO_RC_FAIL =		1 : The function does not complete operation successfully.
		NAND_IO_RC_TIMEOUT =	2 : The function times out before operation completes.

==================================================================================================== */
//int NAND_ReadPage(
//unsigned int a_uiPageNum,
//unsigned short a_usColNum,
//unsigned short a_usReadSizeByte,
//unsigned char *a_pucReadBuf)

short NAND_ReadPage( struct NANDAddress structNandAddress, unsigned short a_usReadSizeByte, unsigned char *a_pucReadBuf)
{
	short rc;
	unsigned char ucnandAddressByte; 
//	Clear_CE(); 		// Enable Chip Select
	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_CE;

/* Issue command */
	WRITE_NAND_CLE(NAND_CMD_PAGE_READ_CYCLE1);					/* Issue page read command cycle 1 */

/* Issue Address */
	ucnandAddressByte = (structNandAddress.usColNum) & 0xFF;
	WRITE_NAND_ALE(ucnandAddressByte);		/* Set column address byte 0 */
	ucnandAddressByte = (structNandAddress.usColNum >> 8) & 0x0F;
	WRITE_NAND_ALE(ucnandAddressByte);		/* Set column address byte 1 */
	
	ucnandAddressByte = (structNandAddress.ucPageNum ) & 0x3F;
	ucnandAddressByte |= (unsigned char)((structNandAddress.usBlockNum  & 0x0003) << 6);

	WRITE_NAND_ALE(ucnandAddressByte);		/* Set page address byte 0 */ 
	ucnandAddressByte |= (unsigned char)(((structNandAddress.usBlockNum  >> 3) & 0x00FF));
	WRITE_NAND_ALE(ucnandAddressByte);		/* Set page address byte 1 */
	ucnandAddressByte |= (unsigned char)(((structNandAddress.usBlockNum  >> 11) & 0x0007));
	WRITE_NAND_ALE(ucnandAddressByte);		/* Set page address byte 2 */

/* Issue command */
	WRITE_NAND_CLE(NAND_CMD_PAGE_READ_CYCLE2);

	rc = NAND_ReadStatus();										                                      
	if(rc != NAND_IO_RC_PASS)
		return rc;

	/* Issue Command - Set device to read from data register. */
	WRITE_NAND_CLE(NAND_CMD_PAGE_READ_CYCLE1);					

	READ_NAND_ARRAY(a_pucReadBuf, a_usReadSizeByte);

//	Set_CE(); 			// Disable Chip Select
	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_CE;


	return rc;
} /* NAND_ReadPage */

/* **************************************************************************************************************
 * Function : NAND_ProgramPage
 * 
 * Parameters:
 * 				1. a_uiPageNum : Programming will occur at this page address.
 * 				2. a_usColNum :  Programming will begin at this column address.
 * 				3. a_usReadSizeByte : Number of bytes to program.
 * 				4. a_pucReadBuf : Data buffer with data which will be programmed to flash.
 * 
 * Description:
 * 				The NAND_ProgramPage function is used program a page of data into the NAND device.
 * 
 * Return code:
 * 				NAND_IO_RC_PASS =		0 : The function completes operation successfully.
 * 				NAND_IO_RC_FAIL =		1 : The function does not complete operation successfully.
 * 				NAND_IO_RC_TIMEOUT =	2 : The function times out before operation completes.
 **************************************************************************************************************/ 
short NAND_ProgramPage( struct NANDAddress structNandAddress, unsigned short a_usReadSizeByte, unsigned char *a_pucReadBuf)
{
	short rc;
	unsigned char ucnandAddressByte; 

//	Clear_CE(); 		// Enable Chip Select
	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_CE;

	/* Issue command */
	WRITE_NAND_CLE(NAND_CMD_PAGE_PROGRAM_CYCLE1);					/* Issue 0x80 command */

/* Issue Address */
	ucnandAddressByte = (structNandAddress.usColNum) & 0xFF;
	WRITE_NAND_ALE(ucnandAddressByte);		/* Set column address byte 0 */
	ucnandAddressByte = (structNandAddress.usColNum >> 8) & 0x0F;
	WRITE_NAND_ALE(ucnandAddressByte);		/* Set column address byte 1 */
	
	ucnandAddressByte = (structNandAddress.ucPageNum ) & 0x3F;
	ucnandAddressByte |= (unsigned char)((structNandAddress.usBlockNum  & 0x0003) << 6);

	WRITE_NAND_ALE(ucnandAddressByte);		/* Set page address byte 0 */ 
	ucnandAddressByte |= (unsigned char)(((structNandAddress.usBlockNum  >> 3) & 0x00FF));
	WRITE_NAND_ALE(ucnandAddressByte);		/* Set page address byte 1 */
	ucnandAddressByte |= (unsigned char)(((structNandAddress.usBlockNum  >> 11) & 0x0007));
	WRITE_NAND_ALE(ucnandAddressByte);		/* Set page address byte 2 */

/* Issue Command */
	WRITE_NAND_ARRAY(a_pucReadBuf,a_usReadSizeByte );				/* Input data to NAND device */	

	WRITE_NAND_CLE(NAND_CMD_PAGE_PROGRAM_CYCLE2);					/* Issue 0x10 command */

	rc = NAND_ReadStatus();											/* Wait for status */                                        

//	Set_CE(); 			// Disable Chip Select
	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_CE;

	return rc;
} /* NAND_ProgramPage */

/* ================================================================================================
Function : NAND_ReadPageRandomStart

	Parameters:
	a_uiPageNum : Page number for reading
	a_usColNum : Column number for reading
	a_usReadSizeByte : Number of bytes to read
	a_pucReadBuf : Read data buffer

	Description:
        The NAND_ReadPageRandomStart and the NAND_ReadPageRandom functions are used to perform random reads 
		within a page.  NAND_ReadPageRandomStart function begins the process by issuing 0x00 and 0x30 commands
		to the device along with the address (a_uiPageNum and column a_usColNum) of the page to be read.
		Data is read into the buffer a_pucReadBuf for a_usReadSizeByte number of bytes.	

	Return code:
		NAND_IO_RC_PASS =		0 : The function completes operation successfully.
		NAND_IO_RC_FAIL =		1 : The function does not complete operation successfully.
		NAND_IO_RC_TIMEOUT =	2 : The function times out before operation completes.

==================================================================================================== */
short NAND_ReadPageRandomStart( struct NANDAddress structNandAddress, unsigned short a_usReadSizeByte, unsigned char *a_pucReadBuf)
{
	short rc;
//	int i;
	unsigned char ucnandAddressByte; 

//	Clear_CE(); 								// Enable Chip Select
	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_CE;

/* Issue command */
	WRITE_NAND_CLE(NAND_CMD_PAGE_READ_CYCLE1);					/* Issue page read command cycle 1 */

/* Issue Address */
	ucnandAddressByte = (structNandAddress.usColNum) & 0xFF;
	WRITE_NAND_ALE(ucnandAddressByte);		/* Set column address byte 0 */
	ucnandAddressByte = (structNandAddress.usColNum >> 8) & 0x0F;
	WRITE_NAND_ALE(ucnandAddressByte);		/* Set column address byte 1 */
	
	ucnandAddressByte = (structNandAddress.ucPageNum ) & 0x3F;
	ucnandAddressByte |= (unsigned char)((structNandAddress.usBlockNum  & 0x0003) << 6);

	WRITE_NAND_ALE(ucnandAddressByte);		/* Set page address byte 0 */ 
	ucnandAddressByte |= (unsigned char)(((structNandAddress.usBlockNum  >> 3) & 0x00FF));
	WRITE_NAND_ALE(ucnandAddressByte);		/* Set page address byte 1 */
	ucnandAddressByte |= (unsigned char)(((structNandAddress.usBlockNum  >> 11) & 0x0007));
	WRITE_NAND_ALE(ucnandAddressByte);		/* Set page address byte 2 */

/* Issue command */
	WRITE_NAND_CLE(NAND_CMD_PAGE_READ_CYCLE2);

	rc = NAND_ReadStatus();						/* Wait for ready status */                                        

	if(rc != NAND_IO_RC_PASS)
		return rc;

	WRITE_NAND_CLE(NAND_CMD_PAGE_READ_CYCLE1);		/* Set device to read data mode by issuing a page read command */


	READ_NAND_ARRAY(a_pucReadBuf, a_usReadSizeByte);


	return rc;
} /* NAND_ReadPageRandomStart */



/* ================================================================================================
Function : NAND_ReadPageRandom

	Parameters:
	a_usColNum : Column number for reading
	a_usReadSizeByte : Number of byte to read
	a_pucReadBuf : Read data buffer

	Description:
        The NAND_ReadPageRandomStart and the NAND_ReadPageRandom functions are used to perform random reads 
		within a page.  The NAND_ReadPageRandom function sends commands 0x05 and 0xe0 along with the column address.
		Only the column address is needed becasue the page number does not change in a random read operation.  
		Data is read into the buffer a_pucReadBuf for a_usReadSizeByte number of bytes.	

  	Return code:
		None

==================================================================================================== */
short NAND_ReadPageRandom(
unsigned short a_usColNum,
unsigned short a_usReadSizeByte, 
unsigned char *a_pucReadBuf)
{
//	unsigned int i; 
//	Clear_CE(); 								// Enable Chip Select
	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_CE;

/* Issue command */
	WRITE_NAND_CLE(NAND_CMD_RANDOM_DATA_READ_CYCLE1);		/* Issue 0x05 command. */

/* Issue Column Address */
	WRITE_NAND_ALE((unsigned char)(a_usColNum & 0xFF));
	WRITE_NAND_ALE((unsigned char)((a_usColNum >> 8) & 0x0F));	/* Set column address byte 1 */

/* Issue command */
	WRITE_NAND_CLE(NAND_CMD_RANDOM_DATA_READ_CYCLE2);		/* Issue 0xE0 command. */

	READ_NAND_ARRAY(a_pucReadBuf, a_usReadSizeByte);

//	Set_CE(); 			// Disable Chip Select
	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_CE;
	return 0;
} /* NAND_ReadPageRandom */

/* ================================================================================================
Function : NAND_ProgramPageRandomStart

	Parameters:
	a_uiPageNum : Page number for reading
	a_usColNum : Column number for reading
	a_usReadSizeByte : Number of bytes to program
	a_pucReadBuf : Data buffer

	Description:
        The NAND_ProgramPageRandomStart, NAND_ProgramPageRandom, and the NAND_ProgramPageRandomLast 
		functions are used to perform random data input in a page (random programming).  The 
		NAND_ProgramPageRandomStart function begins the process by issuing an 0x80 command, followed 
		by a 5 cycle address (page = a_uiPageNum, column = a_usColNum) of the page to be programmed, 
		and finally the data to be programmed which is contained in the a_pucReadBuf buffer.

  	Return code:
		None

==================================================================================================== */
short NAND_ProgramPageRandomStart( struct NANDAddress structNandAddress, unsigned short a_usReadSizeByte, unsigned char *a_pucReadBuf)
{
	unsigned char ucnandAddressByte; 
	
//		  P2OUT |= BIT7;			//Debug 			
	
//	Clear_CE(); 								// Enable Chip Select
	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_CE;
	WRITE_NAND_CLE(NAND_CMD_PAGE_PROGRAM_CYCLE1);					/* Issue 0x80 command */

/* Issue Address */
	ucnandAddressByte = (structNandAddress.usColNum) & 0xFF;
	WRITE_NAND_ALE(ucnandAddressByte);		/* Set column address byte 0 */
	ucnandAddressByte = (structNandAddress.usColNum >> 8) & 0x0F;
	WRITE_NAND_ALE(ucnandAddressByte);		/* Set column address byte 1 */
	
	ucnandAddressByte = (structNandAddress.ucPageNum ) & 0x3F;
	ucnandAddressByte |= (unsigned char)((structNandAddress.usBlockNum  & 0x0003) << 6);

	WRITE_NAND_ALE(ucnandAddressByte);		/* Set page address byte 0 */ 
	ucnandAddressByte |= (unsigned char)(((structNandAddress.usBlockNum  >> 3) & 0x00FF));
	WRITE_NAND_ALE(ucnandAddressByte);		/* Set page address byte 1 */
	ucnandAddressByte |= (unsigned char)(((structNandAddress.usBlockNum  >> 11) & 0x0007));
	WRITE_NAND_ALE(ucnandAddressByte);		/* Set page address byte 2 */

	/* Write Data */
	
	//WRITE_NAND_ARRAY1(a_pucReadBuf,a_usReadSizeByte );	/* Input data to NAND device */
	Flash_MT298G08AAAWP_Nand_Write_8Bytes(  a_pucReadBuf, a_usReadSizeByte);
		
//	Set_CE(); 			// Disable Chip Select
	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_CE;

//		  P2OUT &= ~BIT7;		// Debug 			

	return 0;
} //NAND_ProgramPageRandomStart



/* ================================================================================================
Function : NAND_ProgramPageRandom

	Parameters:
	usColNum : Column number for reading
	a_usReadSizeByte : Number of bytes to program
	a_pucReadBuf : Data buffer

	Description:
        The NAND_ProgramPageRandomStart, NAND_ProgramPageRandom, and the NAND_ProgramPageRandomLast 
		functions are used to perform random data input in a page (random programming).  The 
		NAND_ProgramPageRandom function issues an 0x85 command, followed by a 2 cycle address 
		(column = usColNum) of the page to be programmed, and finally the data to be programmed 
		which is contained in the a_pucReadBuf buffer.
		Only the column address is needed becasue the page number does not change in a random data input
		operation.

  	Return code:
		None

==================================================================================================== */
short NAND_ProgramPageRandom( unsigned short usColNum, unsigned short a_usReadSizeByte, unsigned char *a_pucReadBuf)
{

//	Clear_CE(); 								// Enable Chip Select
	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_CE;
	/* Issue Command */
	WRITE_NAND_CLE(NAND_CMD_RANDOM_DATA_INPUT);					/* Issue 0x85 command */

	/* Issue Address */
	WRITE_NAND_ALE((unsigned char)(usColNum & 0xFF));			/* Set column address byte 0 */
	WRITE_NAND_ALE((unsigned char)((usColNum >> 8) & 0x0F));		/* Set column address byte 1 */

//	WRITE_NAND_ARRAY1(a_pucReadBuf,a_usReadSizeByte );	/* Input data to NAND device */	
	Flash_MT298G08AAAWP_Nand_Write_8Bytes(  a_pucReadBuf, a_usReadSizeByte);

//	Set_CE(); 			// Disable Chip Select
	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_CE;


	return 0;
} /* NAND_ProgramPageRandom */



/* ================================================================================================
Function : NAND_ProgramPageRandomLast

	Parameters:
		None

	Description:
        The NAND_ProgramPageRandomStart, NAND_ProgramPageRandom, and the NAND_ProgramPageRandomLast 
		functions are used to perform random data input in a page (random programming).  
		The NAND_ProgramPageRandomLast function issues an 0x10 command to close out the random programming
		operation.

	Return code:
		NAND_IO_RC_PASS =		0 : The function completes operation successfully.
		NAND_IO_RC_FAIL =		1 : The function does not complete operation successfully.
		NAND_IO_RC_TIMEOUT =	2 : The function times out before operation completes.

==================================================================================================== */
short NAND_ProgramPageRandomLast(void)
{
	short rc;
//	Clear_CE(); 								// Enable Chip Select
	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_CE;

	WRITE_NAND_CLE(NAND_CMD_PAGE_PROGRAM_CYCLE2);

	rc = NAND_ReadStatus();                                     
//	Set_CE(); 			// Disable Chip Select
	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_CE;

	return rc;

} //NAND_ProgramPageRandomLast



/* =============================================================================================
Function : NAND_EraseBlock

	Parameters:
	usBlockNum : Block number to be erased.

	Description:
        This function erases (returns all bytes in the block to 0xFF) a block of data in the 
		NAND device	

	Return code:
		NAND_IO_RC_PASS = 0 : This function completes its operation successfully
		NAND_IO_RC_FAIL = 1 : This function does not complete its operation successfully
		NAND_IO_RC_TIMEOUT

============================================================================================== */
short NAND_EraseBlock(unsigned short usBlockNum)
{
	short rc;
	unsigned short ucnandAddressByte; 
//	Clear_CE(); 								// Enable Chip Select
	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_CE;

	/* Issue command */
	WRITE_NAND_CLE(NAND_CMD_BLOCK_ERASE_CYCLE1);

	/* Issue address */
	
	ucnandAddressByte = usBlockNum & 0x3;
	ucnandAddressByte = ucnandAddressByte << 6;
	
	WRITE_NAND_ALE((unsigned char)ucnandAddressByte);		/* Set page address byte 0 */ 
	
	ucnandAddressByte = usBlockNum >> 0x2;
	ucnandAddressByte = ucnandAddressByte & 0xff;
	
	WRITE_NAND_ALE((unsigned char)ucnandAddressByte);		/* Set page address byte 1 */
	 
	ucnandAddressByte = usBlockNum >> 10;
	ucnandAddressByte = ucnandAddressByte & 0x07;
	

	WRITE_NAND_ALE((unsigned char)ucnandAddressByte);	/* Set page address byte 2 */

	/* Issue command */
	WRITE_NAND_CLE(NAND_CMD_BLOCK_ERASE_CYCLE2);

	rc = NAND_ReadStatus();		/* Wait for ready status */                                        

//	Set_CE(); 			// Disable Chip Select
	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_CE;

	return rc;
} /* NAND_EraseBlock */
/* =============================================================================================
 * Function : Send_Recorded_ECG_Samples_to_USB
 * 
 * 	Parameters:
 * 		None.
 *
 * 	Description:
 *         This function reads recorded data from Nand FLASH memory send to USB port packet by packet.
 * 			
 * 
 * 	Return code:
 * 	None
 * 
 * ============================================================================================== */

void Send_Recorded_ECG_Samples_to_USB(void)
{
		unsigned char TrnasferComplete = 0, StreamCount,Loop_i;
		unsigned char *ptr;
		WORD bytesSent,bytesReceived;
		
		if ( Read_Recorder_NANDAddress.usColNum == 0)
		{
			NAND_ReadPageRandomStart(Read_Recorder_NANDAddress, 32, &Dwn_NandReadBuf[Dwn_head<<5] );
			if (strncmp((const char *) &Dwn_NandReadBuf[Dwn_head<<5], (const char *) HeadrPacket,20) !=0)
			{
				TrnasferComplete = 1;
			}
			if ( TrnasferComplete == 0)
			{
				Read_Recorder_NANDAddress.usColNum += 64;
				NAND_ReadPageRandom(Read_Recorder_NANDAddress.usColNum,32,&Dwn_NandReadBuf[Dwn_head<<5]);
			}
		}	
		else{
			NAND_ReadPageRandom(Read_Recorder_NANDAddress.usColNum,32,&Dwn_NandReadBuf[Dwn_head<<5]);
		}
		Dwn_head++;
		Read_Recorder_NANDAddress.usColNum += 32;
		if ( Read_Recorder_NANDAddress.usColNum == PAGE_SIZE)
		{
			Read_Recorder_NANDAddress.usColNum = 0;
			Read_Recorder_NANDAddress.ucPageNum++;
			if (Read_Recorder_NANDAddress.ucPageNum == PAGES_PER_BLOCK)
			{
				Read_Recorder_NANDAddress.ucPageNum =0;
				Read_Recorder_NANDAddress.usBlockNum++;
				if (Read_Recorder_NANDAddress.usBlockNum==MAX_BLOCKS)
				{
					// Exit data send
					TrnasferComplete = 1;
				}
				
			}
			
		}
		if (((Dwn_head - Dwn_tail) > 1) || (TrnasferComplete !=0))
		{
			//while(CdcWriteCtrl[0]->nCdcBytesToSendLeft != 0);
		   	StreamCount = 0;
			ptr = &Dwn_NandReadBuf[Dwn_tail<<5];
		   	ECGTxPacket[StreamCount++]= START_DATA_HEADER;
		   	ECGTxPacket[StreamCount++]= DATA_DOWNLOAD_COMMAND;
		   	if ( TrnasferComplete ==0)
		   	{
				ECGTxPacket[StreamCount++]= 0;
		   	}
		   	else
		   	{
		   		ECGTxPacket[StreamCount++]= 0xFF;
		   	}
			
			ECGTxPacket[StreamCount++]= *ptr++;		// Packet Start header
		   	ECGTxPacket[StreamCount++]= *ptr++;		// Packet Start header
		   	
		   	/* Create Acquire packet*/
			for (Loop_i = 0; Loop_i < 8;Loop_i++)
			{
				ECGTxPacket[StreamCount++]=*ptr++;
				ECGTxPacket[StreamCount++]=*ptr++;
				ECGTxPacket[StreamCount++]=*ptr++;
			
				ECGTxPacket[StreamCount++]=*ptr++;
				ECGTxPacket[StreamCount++]=*ptr++;
				ECGTxPacket[StreamCount++]=*ptr++;
				ptr++;
				ptr++;
				//ECGTxPacket[StreamCount++] = 0;
			}
		    ECGTxPacket[StreamCount++]= END_DATA_HEADER;
		    ECGTxPacket[StreamCount++]= '\n';
			while(USBCDC_intfStatus(0,&bytesSent,&bytesReceived) & kUSBCDC_waitingForSend);
		   	/* send packet to PC application*/
			cdc_sendDataInBackground((BYTE*)&ECGTxPacket,StreamCount,0,0);          // Send the response over USB
			Dwn_tail +=2;
			if (Dwn_tail == 4)
			{
				Dwn_tail =0;
			}
		}		
		if (Dwn_head == 4)
		{
			Dwn_head =0;
		}
		if (TrnasferComplete )
		{
			Read_Recorder_NANDAddress.usBlockNum = 10;
			Read_Recorder_NANDAddress.ucPageNum = 0;
			Read_Recorder_NANDAddress.usColNum = 0;
			ECGTxPacket[2] = 0xFF;
			ECG_Recoder_state.state = IDLE_STATE;			
		}

} // Send_Recorded_ECG_Samples_to_USB
/* =============================================================================================
 * Function : Store_Processed_ECG_Samples_to_Flash
 *
 * 	Parameters:
 * 	a_pucReadBuf : Array to be stored.
 *  EndPacket : to end of packet in page
 * 
 * 	Description:
 *         This function stores32 bytes in the FLASH NAND device
 *
 * 	Return code:
 * 		NONE
 * 
 * ============================================================================================== */
void Store_Processed_ECG_Samples_to_Flash(unsigned char a_pucReadBuf[])
{
//		packetCounter = 0;
								
	if ( Recorder_NANDAddress.usColNum == 0)
	{
		HeadrPacket[22] = ADS1x9xRegVal[0];
		NAND_ProgramPageRandomStart(Recorder_NANDAddress, 32, HeadrPacket);
		Recorder_NANDAddress.usColNum +=32;
		NAND_ProgramPageRandom(Recorder_NANDAddress.usColNum,32,a_pucReadBuf);
	}
	else
	{
		NAND_ProgramPageRandom(Recorder_NANDAddress.usColNum,32,a_pucReadBuf);
	}
	Recorder_NANDAddress.usColNum +=32;
	
	if ( Recorder_NANDAddress.usColNum == PAGE_SIZE)
	{
		Recorder_NANDAddress.usColNum=0;
		Recorder_NANDAddress.ucPageNum++;
		if ( Recorder_NANDAddress.ucPageNum == PAGES_PER_BLOCK)
		{
			Recorder_NANDAddress.ucPageNum = 0;
			Recorder_NANDAddress.usBlockNum++;
			if (Recorder_NANDAddress.usBlockNum == MAX_BLOCKS)
			{
				Recorder_NANDAddress.usBlockNum = MAX_BLOCKS-1;
				Recorder_NANDAddress.ucPageNum = PAGES_PER_BLOCK-1;
			}
		}
		NAND_ProgramPageRandomLast();
	}
	
}

/* =============================================================================================
Function : Get_Raw_ECG_Samples_Data_pointer

	Parameters: None
	
	Description:
        This function computes current pointer for recording stores in a structure Recorder_NANDAddress. 	

	Return code:
		None
============================================================================================== */
void Get_Raw_ECG_Samples_Data_pointer(void)
{
	unsigned short i;
	unsigned char NandSysName[32];
	//unsigned char SysName[32] = {"ADS1x9x_ECG_Recorder\n           "};
	unsigned char SysNameBlank[32] = {0xFF, 0xFF, 0xFF, 0xFF,0xFF, 0xFF, 0xFF, 0xFF,
									  0xFF, 0xFF, 0xFF, 0xFF,0xFF, 0xFF, 0xFF, 0xFF,
									  0xFF, 0xFF, 0xFF, 0xFF,0xFF, 0xFF, 0xFF, 0xFF,
									  0xFF, 0xFF, 0xFF, 0xFF,0xFF, 0xFF, 0xFF, 0xFF};
	unsigned char Erase_Required = 0;									  

	Recorder_NANDAddress.usBlockNum = RECORD_START_BLOCK;
	Recorder_NANDAddress.ucPageNum = 0;
	Recorder_NANDAddress.usColNum = 0;
	
	for (i = RECORD_START_BLOCK; i < MAX_BLOCKS; i++)
	{
		
		NAND_ReadPage( Recorder_NANDAddress, 32, NandSysName);
		//memcpy(SysName, NandSysName, strlen("ADS1x9x_ECG_Recorder"));
		if ( strncmp((const char *) NandSysName, (const char *) HeadrPacket,22) == 0)			// Check for valid Stored data 
		{
			Recorder_NANDAddress.usBlockNum++;
			//copy to usb
		}
		else if(strncmp((const char *) NandSysName, (const char *) SysNameBlank,22) == 0)	// Check for blank page
		{
			Recorder_NANDAddress.usBlockNum--;
			break;
		}
		else {								// Erase if not blank
			
			Erase_Required = 1;
			
			break;				
		}
	}
	
	if ( Erase_Required == 1)
	{
		for ( i= Recorder_NANDAddress.usBlockNum; i < MAX_BLOCKS; i++)
		{
			NAND_EraseBlock(i);		// Erase blank  
		}
		Erase_Required = 0;
	}
	for (i = 0; i < PAGES_PER_BLOCK; i++)
	{
		
		NAND_ReadPage( Recorder_NANDAddress, 32, NandSysName);
		//memcpy(SysName, NandSysName, strlen("ADS1x9x_ECG_Recorder"));
		if ( strncmp((const char *) NandSysName, (const char *) HeadrPacket,22) == 0)			// Check for valid Stored data 
		{
			Recorder_NANDAddress.ucPageNum++;
			if ( Recorder_NANDAddress.ucPageNum==PAGES_PER_BLOCK-1)
			{
				Recorder_NANDAddress.usBlockNum++;
				Recorder_NANDAddress.ucPageNum=0;
			} 
			//copy to usb
		}
		else if(strncmp((const char *) NandSysName, (const char *) SysNameBlank,22) == 0)	// Check for blank page
		{
			//Recorder_NANDAddress.ucPageNum--;
			break;
		}
		else {								// Erase if not blank
			
			break;				
		}

	}
}
/* =============================================================================================
 * Function : Erase_NAND_Flash
 * 
 * 	Parameters:
 * 		None.
 *
 * 	Description:
 *         This function Erases recorded data from Nand FLASH memory.
 * 			
 * 
 * 	Return code:
 * 	None
 * MT29F8G08FACWP
 * MT29F8G08AAAWP
 * ============================================================================================== */

void Erase_NAND_Flash(void)
{
	unsigned short BlockCount =0;	
		for ( BlockCount= 1; BlockCount < MAX_BLOCKS; BlockCount++)
		{
			NAND_EraseBlock(BlockCount);		// Erase blank  
		}
} //Erase_NAND_Flash

/* =============================================================================================
 * Function : Store_Acquired_ECG_Samples_to_Flash
 *
 * 	Parameters:
 * 	a_pucReadBuf : Array to be stored.
 * 
 * 	Description:
 *         This function stores 32 bytes in the FLASH NAND device
 *
 * 	Return code:
 * 		NONE
 * 
 * ============================================================================================== */
void Store_Acquired_ECG_Samples_to_Flash(unsigned char a_pucReadBuf[], unsigned short NumPackets)
{
								
		if ( Acquire_NANDAddress.usColNum == 0)
		{
			/* Start of page write command  + 64 byte data*/
			NAND_ProgramPageRandomStart(Acquire_NANDAddress, 64, a_pucReadBuf);
		}
		else
		{
			/* write 64 byte data*/
			NAND_ProgramPageRandom(Acquire_NANDAddress.usColNum,64,a_pucReadBuf);
		}
		Acquire_NANDAddress.usColNum +=64;			// Increment mem pointer by 64		
		if ( Acquire_NANDAddress.usColNum == PAGE_SIZE) 	// Check for page boundery
//		if ( Acquire_NANDAddress.usColNum == 2048) 	// Check for page boundery
		{
			Acquire_NANDAddress.usColNum = 0;
			Acquire_NANDAddress.ucPageNum++;			// increment page number
			if ( Acquire_NANDAddress.ucPageNum == PAGES_PER_BLOCK)	// Check for block boundery
			{
				Acquire_NANDAddress.ucPageNum = 0;		// Reset page
				Acquire_NANDAddress.usBlockNum++;		// Increment block number
														// boundery condition not checked
			}
			/* End of page write command */
			NAND_ProgramPageRandomLast();
		}
		if ( (Acquire_NANDAddress.usColNum != 0) && (NumPackets == 0))
		{
			NAND_ProgramPageRandomLast();
		}
}


/* =============================================================================================
 * Function : Send_Acquired_ECG_Samples_to_USB
 * 
 * 	Parameters:
 * 		None.
 *
 * 	Description:
 *         This function reads recorded data from Nand FLASH memory send to USB port packet by packet.
 * 			
 * 
 * 	Return code:
 * 	None
 * 
 * ============================================================================================== */

void Send_Acquired_ECG_Samples_to_USB(unsigned short NumSamples)
{
	unsigned char NandReadBuf[64], StreamCount,Loop_i, Loop_j;
	WORD bytesSent,bytesReceived;
	unsigned short SendCount = 0;

	while ( SendCount < NumSamples )
	{
		if ( Acquire_NANDAddress.usColNum == 0)
		{
			NAND_ReadPageRandomStart(Acquire_NANDAddress, 64, NandReadBuf );
		}	
		else
		{
			NAND_ReadPageRandom(Acquire_NANDAddress.usColNum, 64, NandReadBuf);
		}
		Acquire_NANDAddress.usColNum += 64;				// Increment mem colum adress by 64 bytes 
		if ( Acquire_NANDAddress.usColNum == PAGE_SIZE)	// Check for page boundery
//		if ( Acquire_NANDAddress.usColNum == 2048)	// Check for page boundery
		{
			Acquire_NANDAddress.usColNum = 0;			// Reset colum
			Acquire_NANDAddress.ucPageNum++;			// increment page number
			if (Acquire_NANDAddress.ucPageNum == PAGES_PER_BLOCK)	// Check for block boundery
			{
				Acquire_NANDAddress.ucPageNum =0;		// Reset page
				Acquire_NANDAddress.usBlockNum++;		// Increment block number
														// boundery condition not checked
			}
		}
		//while(CdcWriteCtrl[0]->nCdcBytesToSendLeft != 0);
		while((USBCDC_intfStatus(0,&bytesSent,&bytesReceived) & kUSBCDC_waitingForSend) );
	   	StreamCount = 0;									// Init pointer

	   	ECGTxPacket[StreamCount++]= START_DATA_HEADER;		// Packet Start header
	   	ECGTxPacket[StreamCount++]= ACQUIRE_DATA_PACKET;	// Acquire Packet header
	   	ECGTxPacket[StreamCount++]= NandReadBuf[57];		// Packet Start header
	   	ECGTxPacket[StreamCount++]= NandReadBuf[58];		// Packet Start header
	   	
	   	/* Create Acquire packet*/
		for (Loop_i = 0,Loop_j=0; Loop_i < 8;Loop_i++)
		{
			Loop_j +=2;
			ECGTxPacket[StreamCount++]=NandReadBuf[Loop_j++];
			ECGTxPacket[StreamCount++]=NandReadBuf[Loop_j++];
			ECGTxPacket[StreamCount++]=NandReadBuf[Loop_j++];
			ECGTxPacket[StreamCount++]=NandReadBuf[Loop_j++];
			ECGTxPacket[StreamCount++]=NandReadBuf[Loop_j++];
			ECGTxPacket[StreamCount++]=NandReadBuf[Loop_j++];
			//ECGTxPacket[StreamCount++] = 0;
		}
	   	ECGTxPacket[StreamCount++]= END_DATA_HEADER;		// End of packet
	   	ECGTxPacket[StreamCount++]= '\n';

	   	/* send packet to PC application*/
		cdc_sendDataInBackground((BYTE*)&ECGTxPacket,ECG_ACQUIRE_PACKET_LENGTH,0,0);          // Send the response over USB
		  
		 SendCount += 8;
		}	
} // Send_Acquired_ECG_Samples_to_USB

/* =============================================================================================*/

