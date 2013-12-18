/**************************************************************************************************
  Filename:       simpleGATTprofile.h
  Revised:        $Date: 2010-08-06 08:56:11 -0700 (Fri, 06 Aug 2010) $
  Revision:       $Revision: 23333 $

  Description:    This file contains the Simple GATT profile definitions and
                  prototypes.

  Copyright 2010 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, 
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE, 
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com. 
**************************************************************************************************/

#ifndef ECGPROFILE_H
#define ECGPROFILE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */
  
// Profile Parameters
#define ECGPROFILE_CONTROL                0  // RW uint8 < Device State. Default is 0.
#define ECGPROFILE_ECGDATA                1  // RO uint8[ECGPROFILE_CHAR1_LEN] < ECG Data. Default is all 0s.
  
//#define ECGPROFILE_R2_DEC_RATE             2  // RW uint 8 < this parameter controls the R2 decimiation rate register of ADS for filter
//#define ECGPROFILE_R3_DEC_RATE_CHL1        3  // RW uint8 < this parameter controls the R2 decimiation rate register of ADS for filter
//#define ECGPROFILE_ERROR_STATUS            4  // RO uint8 < error status << may not be needed
  
// ECG Keys Profile Services bit fields
#define ECGPROFILE_SERVICE               0x00000001
  
// ECG Profile Service UUID
#define ECGPROFILE_SERV_UUID               0xFFF0
  
// Key Pressed UUID
#define ECGPROFILE_CONTROL_UUID            0xFFF1
#define ECGPROFILE_ECGDATA_UUID            0xFFF2
  
// Length of Characteristic 1 in bytes (6 samples x 3 bytes per sample)
#define ECGPROFILE_ECGDATA_LEN           18  

/*********************************************************************
 * TYPEDEFS
 */

  
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * Profile Callbacks
 */

// Callback when a characteristic value has changed
typedef NULL_OK void (*ECGProfileChange_t)( uint8 paramID );

typedef struct
{
  ECGProfileChange_t        pfnECGProfileChange;  // Called when characteristic value changes
} ECGProfileCBs_t;

    

/*********************************************************************
 * API FUNCTIONS 
 */


/*
 * SimpleProfile_AddService- Initializes the Simple GATT Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 */

extern bStatus_t ECGProfile_AddService( uint32 services );

/*
 * SimpleProfile_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t ECGProfile_RegisterAppCBs( ECGProfileCBs_t *appCallbacks );

/*
 * SimpleProfile_SetParameter - Set a Simple GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 */
extern bStatus_t ECGProfile_SetParameter( uint8 param, uint8 len, void *value );
  
/*
 * SimpleProfile_GetParameter - Get a Simple GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 */
extern bStatus_t ECGProfile_GetParameter( uint8 param, void *value );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEGATTPROFILE_H */
