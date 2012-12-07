/***********************************************************************************************************
 *  MT298G08AAAWP: 8GBits Nand Flash is used to store the ECG data.
 * This file contains three mdules which are as below
 * Flash_MT298G08AAAWP_init()
 * 			: which initialzes the MT298G08AAAWP interface and GPIO regs.
 * Flash_MT298G08AAAWP_Page_Read()
 * 			: which reads the MT298G08AAAWP page of size 2048 bytes.
 * Flash_MT298G08AAAWP_Page_Write()
 * 			: which writes the MT298G08AAAWP page of size 2048 bytes.
 * 
 * 	The MT298G08AAAWP is interfaced with MSP430 as below,
 * 
 * 	____________]-------------------[________________
 * 			P7.0]------------------>[!CE 			]
 * 			P7.1]------------------>[CLE			]
 * 			P7.2]------------------>[ALE    		]
 * 			P7.3]------------------>[!WE    		]
 * 				]					[				]
 * 			P7.4]------------------>[!RE 			]
 * 			P7.5]------------------>[!CE2			]
 * 				]					[				]
 * 			P1.2]------------------>[!RB			]
 * 			P1.4]------------------>[!RB2			]
 * 				]					[				]
 * MSP430		]					[ MT298G08AAAWP ]
 * 				]					[				]
 * 				]					[				]
 * 				]-------------------[				]
 * 		Port6   ]<Port6<-->I/O bus >[ D0-D7			]
 * 				]-------------------[				]
 * 				]					[				]
 * 				]					[				]
 *[ ____________]-------------------[_______________]
 * 				
 * 			
 *******************************************************************************************************/

/***********************************************************************************************************
 * 
 *  Include files.
 * 
 * *********************************************************************************************************/
#include "ADS1x9x_Nand_LLD.h"
//unsigned char ucPageReadBuf[2112];
//unsigned char ucPageProgBuf[2112];
//unsigned char FlashData2[64];

/***********************************************************************************************************
 * 	Function Name	: Flash_MT298G08AAAWP_init()
 * 	Purpose			: This function Initializaes flash and memory pointer.
 *					: Low level driver code to initialize memory chip. 
 * 
 * 
 * 
 * 
 * 
 * 
 ***********************************************************************************************************/
void Flash_MT298G08AAAWP_init(void)
{

	NAND_FALSH_DATA_DIR = 0x00;		// set as input port
	P7DIR |= 0x3F;					// Set CE2,RE,CE,CLE,ALE and WE as out pins
//	Disable_CS1();					
// 	Clear_CLE();
	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_CLE;	// Disable Command Latch Enable
// 	Set_CE();
 	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_CE;	// Disable chip selects
// 	Set_CE2();
 	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_CE2;	// Disable Command Latch Enable Block2
// 	Set_RE();
	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_RE;	// Disable Read Enable
// 	Set_WE();
	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_WE;	// Disable Write Enable
// 	Clear_ALE();
 	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_ALE;	// Disable Address Latch Enable

} /* Flash_MT298G08AAAWP_init */

/***********************************************************************************************************
 * 	Function Name	: 	Flash_MT298G08AAAWP_Nand_Write_Cmd()
 * 	Purpose			:  Low level driver code to write a command to NAND flash.
 * 
 * 
 * 
 ***********************************************************************************************************/
void Flash_MT298G08AAAWP_Nand_Write_Cmd( unsigned long a_ulOffset, unsigned char a_ucCmd)
{

//Set_IO_DIR_Out();									// Set DIR as output
	P6DIR = 0xFF;									// Set DIR as output

// 	Set_CLE();
	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_CLE;	// Enable Command Latch
 	
// 	Clear_WE();
	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_WE;	// Set write pin to low
	
/* Load  command on Data bus */
 	P6OUT = a_ucCmd;								// Issue Command 1 

// 	Set_WE();
	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_WE;	// Disable !WE
	
// 	Clear_CLE();
	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_CLE; // Disable Command Latch

}/* Flash_MT298G08AAAWP_Nand_Write_Cmd */

/***********************************************************************************************************
 * 	Function Name	: 	Flash_MT298G08AAAWP_Nand_Write_Addr()
 * 	Purpose			: This function Initializaes flash and memory pointer.
 * 
 * 
 * 
 ***********************************************************************************************************/

void Flash_MT298G08AAAWP_Nand_Write_Addr( unsigned long a_ulOffset, unsigned char a_ucAddr)
{
	/* Add low level driver code to write an address to NAND flash. */
	//Set_IO_DIR_Out();								// Set DIR as output
	P6DIR = 0xFF;				// Set DIR output
// 	Set_ALE();
	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_ALE;
// 	Clear_WE();
	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_WE;
 	P6OUT = a_ucAddr;					// Issue address
// 	Set_WE();
	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_WE;
// 	Clear_ALE();
	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_ALE;

}/* Flash_MT298G08AAAWP_Nand_Write_Addr */

/***********************************************************************************************************
 * 	Function Name	: 	Flash_MT298G08AAAWP_Nand_Write()
 * 	Purpose			: This function Initializaes flash and memory pointer.
 * 
 * 
 * 
 ***********************************************************************************************************/

void Flash_MT298G08AAAWP_Nand_Write( unsigned long a_ulOffset, void *a_pBuf, unsigned short a_usLen)
{
	int uiLenth;
	unsigned char *ucBuf_address = (unsigned char *)a_pBuf;
	
	/* Add low level driver code to write data to NAND flash. */
	
	//Set_IO_DIR_Out();									// Set DIR as output
	P6DIR = 0xFF;										// Set DIR output
	for (uiLenth = 0; uiLenth < a_usLen; uiLenth++)
	{
// 		Clear_WE();
		P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_WE;
	 	P6OUT = *ucBuf_address++;						// Write data to data bus.
// 		Set_WE();
		P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_WE;
	}
}/* Flash_MT298G08AAAWP_Nand_Write */

/***********************************************************************************************************
 * 	Function Name	: 	Flash_MT298G08AAAWP_Nand_Write()
 * 	Purpose			: This function Initializaes flash and memory pointer.
 * 
 * 
 * 
 ***********************************************************************************************************/

void Flash_MT298G08AAAWP_Nand_Write_8Bytes(  void *a_pBuf, unsigned short a_usLen)
{
	unsigned short uiLenth,length;
	unsigned char *ucBuf_address = (unsigned char *)a_pBuf;
	length = a_usLen >> 3;
	/* Add low level driver code to write data to NAND flash. */
	
	//Set_IO_DIR_Out();									// Set DIR as output
	P6DIR = 0xFF;										// Set DIR output
	for (uiLenth = 0; uiLenth < length; uiLenth++)
	{
// 		Clear_WE();
		P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_WE;
	 	P6OUT = *ucBuf_address++;						// Write data to data bus.
// 		Set_WE();
		P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_WE;

// 		Clear_WE();
		P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_WE;
	 	P6OUT = *ucBuf_address++;						// Write data to data bus.
// 		Set_WE();
		P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_WE;
		
// 		Clear_WE();
		P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_WE;
	 	P6OUT = *ucBuf_address++;						// Write data to data bus.
// 		Set_WE();
		P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_WE;// 		Clear_WE();
		
// 		Clear_WE();
		P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_WE;
	 	P6OUT = *ucBuf_address++;						// Write data to data bus.
// 		Set_WE();
		P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_WE;		

// 		Clear_WE();
		P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_WE;
	 	P6OUT = *ucBuf_address++;						// Write data to data bus.
// 		Set_WE();
		P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_WE;

// 		Clear_WE();
		P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_WE;
	 	P6OUT = *ucBuf_address++;						// Write data to data bus.
// 		Set_WE();
		P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_WE;

// 		Clear_WE();
		P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_WE;
	 	P6OUT = *ucBuf_address++;						// Write data to data bus.
// 		Set_WE();
		P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_WE;

// 		Clear_WE();
		P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_WE;
	 	P6OUT = *ucBuf_address++;						// Write data to data bus.
// 		Set_WE();
		P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_WE;

	}
}/* Flash_MT298G08AAAWP_Nand_Write */

/***********************************************************************************************************
 * 	Function Name	: 	Flash_MT298G08AAAWP_Nand_Write_byte()
 * 	Purpose			: This function Initializaes flash and memory pointer.
 * 
 * 
 * 
 ***********************************************************************************************************/
void Flash_MT298G08AAAWP_Nand_Write_byte( unsigned char a_pucBuf, unsigned short a_usLen)
{
	/* Add low level driver code to write data to NAND flash. */
	//Set_IO_DIR_Out();								// Set DIR as output
	P6DIR = 0xFF;				// Set DIR output

// 	Clear_WE();
	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_WE;
 	P6OUT = a_pucBuf;
// 	Set_WE();
	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_WE;

}/* Flash_MT298G08AAAWP_Nand_Write_byte */

/***********************************************************************************************************
 * 	Function Name	: 	Flash_MT298G08AAAWP_Nand_Read()
 * 	Purpose			: This function Initializaes flash and memory pointer.
 * 
 * 
 * 
 ***********************************************************************************************************/
void Flash_MT298G08AAAWP_Nand_Read( unsigned long a_ulOffset, void *a_pBuf, unsigned short a_usLen)
{

	/* Add low level driver code to read data from NAND flash. */
int uiLenth;
unsigned char *ucBuf_address = (unsigned char *)a_pBuf;
//	Set_IO_DIR_In();								// Set DIR as input
	P6DIR = 0x00;				// Set DIR input
	/* Add low level driver code to write data to NAND flash. */
	for (uiLenth = 0; uiLenth < a_usLen; uiLenth++)
	{
//	 	Clear_RE();
		P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_RE;
	 	*ucBuf_address++ = P6IN;
//	 	Set_RE();
		P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_RE;
		
	}
}/* Flash_MT298G08AAAWP_Nand_Read */

/***********************************************************************************************************
 * 	Function Name	: 	Flash_MT298G08AAAWP_init()
 * 	Purpose			: This function Initializaes flash and memory pointer.
 * 
 * 
 * 
 * 
 ***********************************************************************************************************/
void Flash_MT298G08AAAWP_Nand_Read_byte( unsigned long a_ulOffset, void *a_pBuf, unsigned short a_usLen)
{
	volatile unsigned char *ucBuf_address = (unsigned char *)a_pBuf;

	/* Low level driver code to read data from NAND flash. */
	P6DIR = 0x00;				// Set BUS DIR as input

	/* Set Read Enable Pin to Low for NAND flash. */

	P7OUT &= ~(enum PORT7_FALSH_CONTROL)FLASH_RE;	// Enable Chip
	
	/* Read data fro NAND flash. */

 	*ucBuf_address = P6IN;

	/* Set Read Enable Pin to High for NAND flash. */

	P7OUT |= (enum PORT7_FALSH_CONTROL)FLASH_RE; 	// Disable Chip

}/* Flash_MT298G08AAAWP_Nand_Read_byte */


