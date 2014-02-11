/**************************************************************************************************
  Filename:       Accelprofile.h
  Revision:       1
**************************************************************************************************/

#ifndef ACCELPROFILE_H
#define ACCELPROFILE_H

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
#define ACCELPROFILE_CONTROL                  0  // RW uint8 < Device State. Default is 0.
#define ACCELPROFILE_ACCELDATA                1  // RO uint8
  
// ACCEL Keys Profile Services bit fields
#define ACCELPROFILE_SERVICE                  0x00000001
  
// ACCEL Profile Service UUID
#define ACCELPROFILE_SERV_UUID                0xFFF0
  
// Key Pressed UUID
#define ACCELPROFILE_CONTROL_UUID             0xFFF1
#define ACCELPROFILE_ACCELDATA_UUID           0xFFF2
  
// Length of Characteristic 1 in bytes (2 bytes per axis, 3 axis)
#define ACCELPROFILE_ACCELDATA_LEN           6

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
typedef NULL_OK void (*AccelProfileChange_t)( uint8 paramID );

typedef struct
{
  AccelProfileChange_t        pfnAccelProfileChange;  // Called when characteristic value changes
} AccelProfileCBs_t;

    

/*********************************************************************
 * API FUNCTIONS 
 */


/*
 * AccelProfile_AddService- Initializes the Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 */

extern bStatus_t AccelProfile_AddService( uint32 services );

/*
 * AccelProfile_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t AccelProfile_RegisterAppCBs( AccelProfileCBs_t *appCallbacks );

/*
 * AccelProfile_SetParameter - Set a profile parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 */
extern bStatus_t AccelProfile_SetParameter( uint8 param, uint8 len, void *value );
  
/*
 * AccelProfile_GetParameter - Get a profile parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 */
extern bStatus_t AccelProfile_GetParameter( uint8 param, void *value );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ACCELPROFILE_H */
