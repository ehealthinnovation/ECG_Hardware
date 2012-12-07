/*******************************************************************************
*
*    File Name:  ADS1x9x_NAND_FLASH_H.h
*     Revision:  3.0
*
*  Description:  Micron NAND I/O Driver
*
*
* ---  ---------------	----------	-------------------------------
*/


#ifndef ADS1x9x_NAND_FLASH_H
#define ADS1x9x_NAND_FLASH_H


/*------------------*/
/* NAND command set */
/*------------------*/
#define NAND_CMD_PAGE_READ_CYCLE1 			0x00
#define	NAND_CMD_PAGE_READ_CYCLE2 			0x30
#define NAND_CMD_READ_DATA_MOVE 			0x35
#define NAND_CMD_RESET 						0xFF
#define	NAND_CMD_PAGE_PROGRAM_CYCLE1 		0x80
#define NAND_CMD_PAGE_PROGRAM_CYCLE2 		0x10
#define NAND_CMD_CACHE_PRGM_CONFIRM 		0x15
#define NAND_CMD_PRGM_DATA_MOVE				0x85
#define NAND_CMD_BLOCK_ERASE_CYCLE1 		0x60
#define NAND_CMD_BLOCK_ERASE_CYCLE2 		0xD0
#define NAND_CMD_RANDOM_DATA_INPUT 			0x85
#define NAND_CMD_RANDOM_DATA_READ_CYCLE1	0x05
#define NAND_CMD_RANDOM_DATA_READ_CYCLE2	0xE0
#define NAND_CMD_READ_STATUS 				0x70
#define NAND_CMD_READ_CACHE_START 			0x31
#define NAND_CMD_READ_CACHE_LAST 			0X3F
#define NAND_CMD_READ_UNIQUE_ID 			0x65

#define NAND_CMD_DS 						0xB8

#define	NAND_CMD_READ_ID 					0x90
#define NAND_CMD_READ_UNIQUE_ID 			0x65

#define	NAND_CMD_PROGRAM_OTP 				0xA0
#define	NAND_CMD_PROTECT_OTP 				0xA5
#define	NAND_CMD_READ_OTP 					0xAF

#define	NAND_CMD_BLOCK_UNLOCK_CYCLE1 		0x23
#define	NAND_CMD_BLOCK_UNLOCK_CYCLE2 		0x24
#define	NAND_CMD_BLOCK_LOCK 				0x2A
#define	NAND_CMD_BLOCK_LOCK_TIGHT 			0x2C
#define	NAND_CMD_BLOCK_LOCK_STATUS 			0x7A


/*---------------------------------*/
/* Maximum read status retry count */
/*---------------------------------*/
#define MAX_READ_STATUS_COUNT 				100000

/*----------------------*/
/* NAND status bit mask */
/*----------------------*/
#define STATUS_BIT_0 						0x01
#define STATUS_BIT_1 						0x02
#define STATUS_BIT_5 						0x20
#define STATUS_BIT_6 						0x40

/*----------------------*/
/* NAND status response */
/*----------------------*/
#define NAND_IO_RC_PASS 					0
#define NAND_IO_RC_FAIL 					1
#define NAND_IO_RC_TIMEOUT 					2


/*-------------------------*/
/* NAND device information */
/*-------------------------*/
#define NAND_PAGE_SIZE_BYTE 				2112
#define NAND_PAGE_COUNT_PER_BLOCK 			64
#define NAND_BLOCK_COUNT 					8192
#define NAND_OTP_PAGES 						0x03  //Higest Page ADDR starting from 0x02 = 10


#define NAND_SECTOR_BYTES 					512
#define NAND_SECTOR_SPARE_BYTES 			16

#define RECORD_START_BLOCK 					10
struct NANDAddress{
	unsigned short usBlockNum;
	unsigned char ucPageNum;
	unsigned short usColNum;
};


short NAND_Init(void);

short NAND_Reset(void);

short NAND_ReadID(unsigned char *a_ReadID);

short NAND_ReadUniqueID(unsigned short a_usReadSizeByte,unsigned char *ucReadUniqueID);

short NAND_ReadPage( struct NANDAddress structNandAddress, unsigned short a_usReadSizeByte, unsigned char *a_pucReadBuf);

short NAND_ReadPageRandomStart( struct NANDAddress structNandAddress, unsigned short a_usReadSizeByte, unsigned char *a_pucReadBuf);

short NAND_ReadPageRandom( unsigned short a_usColNum, unsigned short a_usReadSizeByte, unsigned char *a_pucReadBuf);

short NAND_ReadPageCacheStart( unsigned int a_uiPageNum, unsigned short a_usColNum);

short NAND_ReadPageCache( unsigned short a_usReadSizeByte, unsigned char *a_pucReadBuf);

short NAND_ReadPageCacheLast( unsigned short a_usReadSizeByte, unsigned char *a_pucReadBuf);

short NAND_ProgramPage( struct NANDAddress structNandAddress, unsigned short a_usReadSizeByte, unsigned char *a_pucReadBuf);

short NAND_ProgramPageRandomStart( struct NANDAddress structNandAddress, unsigned short a_usReadSizeByte,
	unsigned char *a_pucReadBuf);

short NAND_ProgramPageRandom( unsigned short a_usColNum, 	unsigned short a_usReadSizeByte, unsigned char *a_pucReadBuf);

short NAND_ProgramPageRandomLast(void);

short NAND_ProgramPageCache( unsigned int a_uiPageNum, unsigned short a_usColNum, unsigned short a_usReadSizeByte, unsigned char *a_pucReadBuf);

short NAND_ProgramPageCacheLast( unsigned int a_uiPageNum, unsigned short a_usColNum, unsigned short a_usReadSizeByte, unsigned char *a_pucReadBuf);

short NAND_ReadInternalDataMove( unsigned int a_uiPageNum, unsigned short a_usColNum);

short NAND_ProgramInternalDataMove( unsigned int a_uiPageNum, unsigned short a_usColNum);

short NAND_ProgramInternalDataMoveRandomStart( unsigned int a_uiPageNum, unsigned short a_usColNum, unsigned short a_usReadSizeByte, unsigned char *a_pucReadBuf);

short NAND_ProgramInternalDataMoveRandom( unsigned short a_usColNum, unsigned short a_usReadSizeByte, unsigned char *a_pucReadBuf);

short NAND_ProgramInternalDataMoveRandomLast(void);

short NAND_EraseBlock(unsigned short a_uiBlockNum);

short NAND_ReadDS( unsigned char *a_pucReadBuf);

short NAND_ProgDS( unsigned char a_pucReadBuf);

short NAND_ProgramOTP( unsigned int a_uiPageNum, unsigned short a_usColNum, unsigned short a_usProgSizeByte, unsigned char *a_pucReadBuf);

short NAND_ProtectOTP(void);

short NAND_ReadOTP( unsigned int a_uiPageNum, unsigned short a_usColNum, unsigned short a_usReadSizeByte, unsigned char *a_pucReadBuf);

short NAND_ProgramInternalDataMoveRandomLast(void);

short NAND_EraseBlock(unsigned short a_uiBlockNum);

short NAND_BlockLock(void);

short NAND_BlockLockTight(void);

short NAND_BlockUnlock(unsigned int a_usBlockLow, unsigned int a_usBlockHigh, unsigned int a_usInvert);

short NAND_BlockLockStatus( unsigned int a_usBlock, unsigned char *ucReadStatus);

void Send_Recorded_ECG_Samples_to_USB(void);

void Store_Processed_ECG_Samples_to_Flash(unsigned char *ECG_Data );
void Get_Raw_ECG_Samples_Data_pointer(void);
void Erase_NAND_Flash(void);
void Send_Acquired_ECG_Samples_to_USB(unsigned short NumSamples);
void Store_Acquired_ECG_Samples_to_Flash(unsigned char a_pucReadBuf[], unsigned short NumPackets);
#endif /* ADS1x9x_NAND_FLASH_H */
