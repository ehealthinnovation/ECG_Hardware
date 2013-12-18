/**************************************************************************************************
  Filename:       simpleGATTprofile.c
  Revised:        $Date: 2013-05-06 13:33:47 -0700 (Mon, 06 May 2013) $
  Revision:       $Revision: 34153 $

  Description:    This file contains the Simple GATT profile sample GATT service 
                  profile for use with the BLE sample application.

  Copyright 2010 - 2013 Texas Instruments Incorporated. All rights reserved.

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

/*********************************************************************
 * INCLUDES
 */
#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

#include "ECGProfile.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#define SERVAPP_NUM_ATTR_SUPPORTED        8

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// Simple GATT Profile Service UUID: 0xFFF0
CONST uint8 ECGProfileServUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(ECGPROFILE_SERV_UUID), HI_UINT16(ECGPROFILE_SERV_UUID)
};

// Characteristic 1 CONTROL UUID: 0xFFF1
CONST uint8 ECGProfileCONTROLUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(ECGPROFILE_CONTROL_UUID), HI_UINT16(ECGPROFILE_CONTROL_UUID)
};

// Characteristic 2 ECGDATA UUID: 0xFFF2
CONST uint8 ECGProfileECGDATAUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(ECGPROFILE_ECGDATA_UUID), HI_UINT16(ECGPROFILE_ECGDATA_UUID)
};

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static ECGProfileCBs_t *ECGProfile_AppCBs = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

// ECG Profile Service attribute
static CONST gattAttrType_t ECGProfileService = { ATT_BT_UUID_SIZE, ECGProfileServUUID };


// ECG Profile CONTROL Properties
static uint8 ECGProfileCONTROLProps = GATT_PROP_READ | GATT_PROP_WRITE;

// CONTROL Value
static uint8 ECGProfileCONTROL = 0;

// ECG Profile CONTROL User Description
static uint8 ECGProfileCONTROLUserDesp[8] = "CONTROL\0";


// ECG Profile ECGDATA Properties
static uint8 ECGProfileECGDATAProps = GATT_PROP_NOTIFY;

// ECGDATA Value
static uint8 ECGProfileECGDATA[ECGPROFILE_ECGDATA_LEN] = { 0 };

// ECG Profile ECGDATA User Description
static uint8 ECGProfileECGDATAUserDesp[8] = "ECGDATA\0";

// ECG Profile Characteristic 4 Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t ECGProfileECGDATAConfig[GATT_MAX_NUM_CONN];


/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t ECGProfileAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] = 
{
  // ECG Profile Service
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&ECGProfileService            /* pValue */
  },

    // CONTROL Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &ECGProfileCONTROLProps 
    },

      // CONTROL Value
      { 
        { ATT_BT_UUID_SIZE, ECGProfileCONTROLUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
        0, 
        &ECGProfileCONTROL 
      },

      // CONTROL User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        ECGProfileCONTROLUserDesp 
      },      

    // ECGDATA Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &ECGProfileECGDATAProps 
    },

      // ECGDATA Value
      { 
        { ATT_BT_UUID_SIZE, ECGProfileECGDATAUUID },
        0, 
        0, 
        ECGProfileECGDATA
      },

      // ECGDATA configuration
      { 
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
        0, 
        (uint8 *)ECGProfileECGDATAConfig
      },
          
      // ECGDATA User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        ECGProfileECGDATAUserDesp 
      },             
};


/*********************************************************************
 * LOCAL FUNCTIONS
 */
static uint8 ECGProfile_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                            uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen );
static bStatus_t ECGProfile_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                 uint8 *pValue, uint8 len, uint16 offset );
static void ECGProfile_HandleConnStatusCB( uint16 connHandle, uint8 changeType );
/*********************************************************************
 * PROFILE CALLBACKS
 */
// ECG Profile Service Callbacks
CONST gattServiceCBs_t ECGProfileCBs =
{
  ECGProfile_ReadAttrCB,  // Read callback function pointer
  ECGProfile_WriteAttrCB, // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      ECGProfile_AddService
 *
 * @brief   Initializes the ECG Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t ECGProfile_AddService( uint32 services )
{
  uint8 status = SUCCESS;

  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, ECGProfileECGDATAConfig );

  // Register with Link DB to receive link status change callback
  VOID linkDB_Register( ECGProfile_HandleConnStatusCB );  
  
  if ( services & ECGPROFILE_SERVICE )
  {
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( ECGProfileAttrTbl, 
                                          GATT_NUM_ATTRS( ECGProfileAttrTbl ),
                                          &ECGProfileCBs );
  }

  return ( status );
}


/*********************************************************************
 * @fn      ECGProfile_RegisterAppCBs
 *
 * @brief   Registers the application callback function. Only call 
 *          this function once.
 *
 * @param   callbacks - pointer to application callbacks.
 *
 * @return  SUCCESS or bleAlreadyInRequestedMode
 */
bStatus_t ECGProfile_RegisterAppCBs( ECGProfileCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    ECGProfile_AppCBs = appCallbacks;
    
    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}
  

/*********************************************************************
 * @fn      ECGProfile_SetParameter
 *
 * @brief   Set a Simple Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to right
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t ECGProfile_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case ECGPROFILE_CONTROL:
      if ( len == sizeof ( uint8 ) ) 
      {
        ECGProfileCONTROL = *((uint8*)value);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case ECGPROFILE_ECGDATA:
      if ( len == ECGPROFILE_ECGDATA_LEN ) 
      {
        VOID osal_memcpy( ECGProfileECGDATA, value, ECGPROFILE_ECGDATA_LEN );
        
         // See if Notification has been enabled
        GATTServApp_ProcessCharCfg( ECGProfileECGDATAConfig, ECGProfileECGDATA, FALSE,
                                    ECGProfileAttrTbl, GATT_NUM_ATTRS( ECGProfileAttrTbl ),
                                    INVALID_TASK_ID );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;
    
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn      ECGProfile_GetParameter
 *
 * @brief   Get a ECG Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to put.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t ECGProfile_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case ECGPROFILE_CONTROL:
      *((uint8*)value) = ECGProfileCONTROL;
      break;

    case ECGPROFILE_ECGDATA:
      VOID osal_memcpy( value, ECGProfileECGDATA, ECGPROFILE_ECGDATA_LEN );
      break;      
      
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn          ECGProfile_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 *
 * @return      Success or Failure
 */
static uint8 ECGProfile_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                            uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen )
{
  bStatus_t status = SUCCESS;

  // If attribute permissions require authorization to read, return error
  if ( gattPermitAuthorRead( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  // Make sure it's not a blob operation (no attributes in the profile are long)
  if ( offset > 0 )
  {
    return ( ATT_ERR_ATTR_NOT_LONG );
  }
 
  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    switch ( uuid )
    {
      // No need for "GATT_SERVICE_UUID" or "GATT_CLIENT_CHAR_CFG_UUID" cases;
      // gattserverapp handles those reads

      case ECGPROFILE_CONTROL_UUID:
        *pLen = 1;
        pValue[0] = *pAttr->pValue;
        break;
      
      case ECGPROFILE_ECGDATA_UUID:
        *pLen = ECGPROFILE_ECGDATA_LEN;
        VOID osal_memcpy( pValue, pAttr->pValue, ECGPROFILE_ECGDATA_LEN );
        break;
        
      default:
        // Should never get here! (characteristics 3 and 4 do not have read permissions)
        *pLen = 0;
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
    }
  }
  else
  {
    // 128-bit UUID
    *pLen = 0;
    status = ATT_ERR_INVALID_HANDLE;
  }

  return ( status );
}

/*********************************************************************
 * @fn      ECGProfile_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 *
 * @return  Success or Failure
 */
static bStatus_t ECGProfile_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                 uint8 *pValue, uint8 len, uint16 offset )
{
  bStatus_t status = SUCCESS;
  uint8 notifyApp = 0xFF;
  
  // If attribute permissions require authorization to write, return error
  if ( gattPermitAuthorWrite( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    switch ( uuid )
    {
      case ECGPROFILE_CONTROL_UUID:

        //Validate the value
        // Make sure it's not a blob oper
        if ( offset == 0 )
        {
          if ( len != 1 )
          {
            status = ATT_ERR_INVALID_VALUE_SIZE;
          }
        }
        else
        {
          status = ATT_ERR_ATTR_NOT_LONG;
        }
        
        //Write the value
        if ( status == SUCCESS )
        {
          uint8 *pCurValue = (uint8 *)pAttr->pValue;        
          *pCurValue = pValue[0];

          if( pAttr->pValue == &ECGProfileCONTROL )
          {
            notifyApp = ECGPROFILE_CONTROL;        
          }
        }
             
        break;

      case GATT_CLIENT_CHAR_CFG_UUID:
        status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_NOTIFY );
        break;
        
      default:
        // Should never get here! (characteristics 2 (ECGDATA) and 4 does not have write permissions)
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
    }
  }
  else
  {
    // 128-bit UUID
    status = ATT_ERR_INVALID_HANDLE;
  }

  // If a charactersitic value changed then callback function to notify application of change
  if ( (notifyApp != 0xFF ) && ECGProfile_AppCBs && ECGProfile_AppCBs->pfnECGProfileChange )
  {
   ECGProfile_AppCBs->pfnECGProfileChange( notifyApp );  
  }
  
  return ( status );
}

/*********************************************************************
 * @fn          simpleProfile_HandleConnStatusCB
 *
 * @brief       Simple Profile link status change handler function.
 *
 * @param       connHandle - connection handle
 * @param       changeType - type of change
 *
 * @return      none
 */
static void ECGProfile_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{ 
  // Make sure this is not loopback connection
  if ( connHandle != LOOPBACK_CONNHANDLE )
  {
    // Reset Client Char Config if connection has dropped
    if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
         ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) && 
           ( !linkDB_Up( connHandle ) ) ) )
    { 
      GATTServApp_InitCharCfg( connHandle, ECGProfileECGDATAConfig );
    }
  }
}


/*********************************************************************
*********************************************************************/