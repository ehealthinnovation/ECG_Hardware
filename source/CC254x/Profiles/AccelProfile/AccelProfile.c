/**************************************************************************************************
  Filename:       Accelprofile.c
  Revision:       1
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

#include "AccelProfile.h"

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
// Profile Service UUID: 0xFFF0
CONST uint8 AccelProfileServUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(ACCELPROFILE_SERV_UUID), HI_UINT16(ACCELPROFILE_SERV_UUID)
};

// Characteristic 1 CONTROL UUID: 0xFFF1
CONST uint8 AccelProfileCONTROLUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(ACCELPROFILE_CONTROL_UUID), HI_UINT16(ACCELPROFILE_CONTROL_UUID)
};

// Characteristic 2 ACCEL DATA UUID: 0xFFF2
CONST uint8 AccelProfileACCELDATAUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(ACCELPROFILE_ACCELDATA_UUID), HI_UINT16(ACCELPROFILE_ACCELDATA_UUID)
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

static AccelProfileCBs_t *AccelProfile_AppCBs = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

// Accel Profile Service attribute
static CONST gattAttrType_t AccelProfileService = { ATT_BT_UUID_SIZE, AccelProfileServUUID };


// Accel Profile CONTROL Properties
static uint8 AccelProfileCONTROLProps = GATT_PROP_READ | GATT_PROP_WRITE;

// CONTROL Value
static uint8 AccelProfileCONTROL = 0;

// Accel Profile CONTROL User Description
static uint8 AccelProfileCONTROLUserDesp[8] = "CONTROL\0";


// Accel Profile ACCELDATA Properties
static uint8 AccelProfileACCELDATAProps = GATT_PROP_NOTIFY;

// ACCELDATA Value
static uint8 AccelProfileACCELDATA[ACCELPROFILE_ACCELDATA_LEN] = { 0 };

// Accel Profile ACCELDATA User Description
static uint8 AccelProfileACCELDATAUserDesp[10] = "ACCELDATA\0";

// Accel Profile Characteristic Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t AccelProfileACCELDATAConfig[GATT_MAX_NUM_CONN];


/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t AccelProfileAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] = 
{
  // Accel Profile Service
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&AccelProfileService             /* pValue */
  },

    // CONTROL Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &AccelProfileCONTROLProps 
    },

      // CONTROL Value
      { 
        { ATT_BT_UUID_SIZE, AccelProfileCONTROLUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
        0, 
        &AccelProfileCONTROL 
      },

      // CONTROL User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        AccelProfileCONTROLUserDesp 
      },      

    // ACCELDATA Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &AccelProfileACCELDATAProps 
    },

      // ACCELDATA Value
      { 
        { ATT_BT_UUID_SIZE, AccelProfileACCELDATAUUID },
        0, 
        0, 
        AccelProfileACCELDATA
      },

      // ACCELDATA configuration
      { 
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
        0, 
        (uint8 *)AccelProfileACCELDATAConfig
      },
          
      // ACCELDATA User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        AccelProfileACCELDATAUserDesp 
      },             
};


/*********************************************************************
 * LOCAL FUNCTIONS
 */
static uint8 AccelProfile_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                            uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen );
static bStatus_t AccelProfile_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                 uint8 *pValue, uint8 len, uint16 offset );
static void AccelProfile_HandleConnStatusCB( uint16 connHandle, uint8 changeType );
/*********************************************************************
 * PROFILE CALLBACKS
 */
// Accel Profile Service Callbacks
CONST gattServiceCBs_t AccelProfileCBs =
{
  AccelProfile_ReadAttrCB,   // Read callback function pointer
  AccelProfile_WriteAttrCB,  // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      AccelProfile_AddService
 *
 * @brief   Initializes the Accel Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t AccelProfile_AddService( uint32 services )
{
  uint8 status = SUCCESS;

  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, AccelProfileACCELDATAConfig );

  // Register with Link DB to receive link status change callback
  VOID linkDB_Register( AccelProfile_HandleConnStatusCB );  
  
  if ( services & ACCELPROFILE_SERVICE )
  {
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( AccelProfileAttrTbl, 
                                          GATT_NUM_ATTRS( AccelProfileAttrTbl ),
                                          &AccelProfileCBs );
  }

  return ( status );
}


/*********************************************************************
 * @fn      AccelProfile_RegisterAppCBs
 *
 * @brief   Registers the application callback function. Only call 
 *          this function once.
 *
 * @param   callbacks - pointer to application callbacks.
 *
 * @return  SUCCESS or bleAlreadyInRequestedMode
 */
bStatus_t AccelProfile_RegisterAppCBs( AccelProfileCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    AccelProfile_AppCBs = appCallbacks;
    
    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}
  

/*********************************************************************
 * @fn      AccelProfile_SetParameter
 *
 * @brief   Set a profile parameter.
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
bStatus_t AccelProfile_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case ACCELPROFILE_CONTROL:
      if ( len == sizeof ( uint8 ) ) 
      {
        AccelProfileCONTROL = *((uint8*)value);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case ACCELPROFILE_ACCELDATA:
      if ( len == ACCELPROFILE_ACCELDATA_LEN ) 
      {
        VOID osal_memcpy( AccelProfileACCELDATA, value, ACCELPROFILE_ACCELDATA_LEN );
        
         // See if Notification has been enabled
        GATTServApp_ProcessCharCfg( AccelProfileACCELDATAConfig, AccelProfileACCELDATA, FALSE,
                                    AccelProfileAttrTbl, GATT_NUM_ATTRS( AccelProfileAttrTbl ),
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
 * @fn      AccelProfile_GetParameter
 *
 * @brief   Get a profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to put.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t AccelProfile_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case ACCELPROFILE_CONTROL:
      *((uint8*)value) = AccelProfileCONTROL;
      break;

    case ACCELPROFILE_ACCELDATA:
      VOID osal_memcpy( value, AccelProfileACCELDATA, ACCELPROFILE_ACCELDATA_LEN );
      break;      
      
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn          AccelProfile_ReadAttrCB
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
static uint8 AccelProfile_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
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

      case ACCELPROFILE_CONTROL_UUID:
        *pLen = 1;
        pValue[0] = *pAttr->pValue;
        break;
      
      case ACCELPROFILE_ACCELDATA_UUID:
        *pLen = ACCELPROFILE_ACCELDATA_LEN;
        VOID osal_memcpy( pValue, pAttr->pValue, ACCELPROFILE_ACCELDATA_LEN );
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
 * @fn      AccelProfile_WriteAttrCB
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
static bStatus_t AccelProfile_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
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
      case ACCELPROFILE_CONTROL_UUID:

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

          if( pAttr->pValue == &AccelProfileCONTROL )
          {
            notifyApp = ACCELPROFILE_CONTROL;        
          }
        }
             
        break;

      case GATT_CLIENT_CHAR_CFG_UUID:
        status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_NOTIFY );
        break;
        
      default:
        // Should never get here!
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
  if ( (notifyApp != 0xFF ) && AccelProfile_AppCBs && AccelProfile_AppCBs->pfnAccelProfileChange )
  {
   AccelProfile_AppCBs->pfnAccelProfileChange( notifyApp );  
  }
  
  return ( status );
}

/*********************************************************************
 * @fn          AccelProfile_HandleConnStatusCB
 *
 * @brief       profile link status change handler function.
 *
 * @param       connHandle - connection handle
 * @param       changeType - type of change
 *
 * @return      none
 */
static void AccelProfile_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{ 
  // Make sure this is not loopback connection
  if ( connHandle != LOOPBACK_CONNHANDLE )
  {
    // Reset Client Char Config if connection has dropped
    if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
         ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) && 
           ( !linkDB_Up( connHandle ) ) ) )
    { 
      GATTServApp_InitCharCfg( connHandle, AccelProfileACCELDATAConfig );
    }
  }
}


/*********************************************************************
*********************************************************************/