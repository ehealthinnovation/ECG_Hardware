#ifndef ADS1x9x_NAND_LLD_H_
#define ADS1x9x_NAND_LLD_H_
/*************************************************************************************************
 * 
 *  Include files
 * 
 * ***********************************************************************************************/
#include "..\Common\device.h"
#include "..\Common\types.h"          // Basic Type declarations
//#include "ADS1x9x_Nand_Flash.h"

/*************************************************************************************************
 * 
 *  Constants
 * 
 * ***********************************************************************************************/
 #define NAND_FALSH_DATA_DIR P6DIR
/*****************************************************************************************************************
 *  Control signals to port 7
 * P7.0 --Chip select 1 for first 512 MB data space
 * P7.1 --Clock
 * P7.2 --Address latch enable
 * P7.3 --Write enable
 * P7.4 --Read Enable
 * P7.5 --Chip select 2 for Second 512 MB data space
 *****************************************************************************************************************/
enum PORT7_FALSH_CONTROL
{
	FLASH_CE = 1,
	FLASH_CLE = 2,
	FLASH_ALE = 4,
	FLASH_WE = 8,
	FLASH_RE = 16,
	FLASH_CE2 = 32
};
/*****************************************************************************************************************
 *  Control signals to port 1
 * 
 * P1.2 -- Bussy interrupt for first 512 MB
 * P1.4 -- Bussy interrupt for Second 512 MB
 * 
 *****************************************************************************************************************/
enum PORT1_FALSH_CONTROL
{
	FLASH_RB = 4,
	FLASH_RB2 = 16
};
/*****************************************************************************************************************/


#define CLE_OFFSET 0xFFFFF	/*The offset value is for example purpose only */
#define ALE_OFFSET 0xFFFFE  /*The offset value is for example purpose only */
#define RW_OFFSET  0xFFFFD  /*The offset value is for example purpose only */

#define WRITE_NAND_CLE(command)(Flash_MT298G08AAAWP_Nand_Write_Cmd(CLE_OFFSET, command))
#define WRITE_NAND_ALE(address)(Flash_MT298G08AAAWP_Nand_Write_Addr(ALE_OFFSET, address))
#define WRITE_NAND_BYTE(data) (Flash_MT298G08AAAWP_Nand_Write_byte(RW_OFFSET, 1))
#define WRITE_NAND_ARRAY(data,n) (Flash_MT298G08AAAWP_Nand_Write(RW_OFFSET, data, n))
#define READ_NAND_BYTE(data)(Flash_MT298G08AAAWP_Nand_Read_byte(RW_OFFSET, &(data), 1))
#define READ_NAND_ARRAY(data,n) (Flash_MT298G08AAAWP_Nand_Read(RW_OFFSET, data, n))
#define WRITE_NAND_ARRAY1(data,n) (Flash_MT298G08AAAWP_Nand_Write_8Bytes(RW_OFFSET, data, n))


/***********************************************************************************************************
 * 
 *  Global Functions.
 * 
 * *********************************************************************************************************/

void Flash_MT298G08AAAWP_init(void);
void Flash_MT298G08AAAWP_Nand_Write_Cmd( unsigned long a_ulOffset, unsigned char a_ucCmd);
void Flash_MT298G08AAAWP_Nand_Write_Addr(unsigned long a_ulOffset, unsigned char a_ucAddr);
void Flash_MT298G08AAAWP_Nand_Write( unsigned long a_ulOffset, void *a_pBuf, unsigned short a_usLen);
void Flash_MT298G08AAAWP_Nand_Write_byte( unsigned char a_pucBuf, unsigned short a_usLen);
void Flash_MT298G08AAAWP_Nand_Read( unsigned long a_ulOffset, void *a_pBuf, unsigned short a_usLen);
void Flash_MT298G08AAAWP_Nand_Read_byte( unsigned long a_ulOffset, void *a_pBuf, unsigned short a_usLen);
void Flash_MT298G08AAAWP_Nand_Write_8Bytes(void *a_pBuf, unsigned short a_usLen);
#endif /*ADS1x9x_NAND_LLD_H_*/
