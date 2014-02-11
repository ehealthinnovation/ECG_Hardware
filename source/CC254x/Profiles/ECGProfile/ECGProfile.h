/**************************************************************************************************
  Filename:       ECGProfile.h
  Revision:       1
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
#define ECGPROFILE_CONTROL                   0  // RW uint8 < Device State. Default is 0.
#define ECGPROFILE_ECGDATA                   1  // RO uint8[ECGPROFILE_CHAR1_LEN] < ECG Data. Default is all 0s.
  
//#define ECGPROFILE_R2_DEC_RATE             2  // RW uint 8 < this parameter controls the R2 decimiation rate register of ADS for filter
//#define ECGPROFILE_R3_DEC_RATE_CHL1        3  // RW uint8 < this parameter controls the R2 decimiation rate register of ADS for filter
//#define ECGPROFILE_ERROR_STATUS            4  // RO uint8 < error status << may not be needed
  
// ECG Keys Profile Services bit fields
#define ECGPROFILE_SERVICE                   0x00000001
  
// ECG Profile Service UUID
#define ECGPROFILE_SERV_UUID                 0xFFF0
  
// Key Pressed UUID
#define ECGPROFILE_CONTROL_UUID              0xFFF1
#define ECGPROFILE_ECGDATA_UUID              0xFFF2
  
// Length of Characteristic 1 in bytes (6 samples x 3 bytes per sample)
#define ECGPROFILE_ECGDATA_LEN               18  

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
 * ECGProfile_AddService- Initializes the profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 */

extern bStatus_t ECGProfile_AddService( uint32 services );

/*
 * ECGProfile_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t ECGProfile_RegisterAppCBs( ECGProfileCBs_t *appCallbacks );

/*
 * ECGProfile_SetParameter - Set a profile parameter.
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
 * ECGProfile_GetParameter - Get a profile parameter.
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

#endif /* ECGGATTPROFILE_H */
