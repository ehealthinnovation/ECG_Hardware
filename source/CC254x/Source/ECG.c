/**************************************************************************************************
  Filename:       ECG.c
  Revised:        $Date: 2010-08-06 08:56:11 -0700 (Fri, 06 Aug 2010) $
  Revision:       $Revision: 23333 $

  Description:    This file contains the Simple BLE Peripheral sample application
                  for use with the CC2540 Bluetooth Low Energy Protocol Stack.

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

//CC2541EM, CC2543EM, CC2545EM: 
//Connect from each board:

//- MISO:  P0_2  (PIN9 on Debug Connector P18)
//- MOSI:  P0_3  (PIN11 on Debug Connector P18)
//- SSN:   P0_4  (PIN13 on Debug Connector P18)
//- SCK:   P0_5  (PIN15 on Debug Connector P18)
//- GND:         (PIN20 on Debug Connector P18)

/*********************************************************************
 * INCLUDES
 */

#include <stdio.h> 
#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "OnBoard.h"
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_lcd.h"

#include "gatt.h"

#include "hci.h"

#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"

#include "peripheral.h"

#include "gapbondmgr.h"

#include "ECG.h"

#if defined FEATURE_OAD
  #include "oad.h"
  #include "oad_target.h"
#endif

#include "ccdefines.h"

#include "ECGProfile.h"
#include "AccelProfile.h"
#include "TI_ADS1293.h"
#include "TI_ADS1293_register_settings.h"

#include "lis3dh.h"


/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// How long an LED stays on, defined in milliseconds
#define ECG_LED_FAST_ON_PERIOD            500

#define ECG_LED_SLOW_ON_PERIOD            2000

// Number of times an LED will blink in response to an event
#define ECG_LED_NUMBER_OF_BLINKS              3

// What is the advertising interval when device is discoverable (units of 625us, 160=1000ms)
#define DEFAULT_ADVERTISING_INTERVAL          160

// General discoverable mode advertises indefinitely
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL

// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     8

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     10

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          1000

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         FALSE

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         6

// Company Identifier: Texas Instruments Inc. (13)
#define TI_COMPANY_ID                         0x000D

#define INVALID_CONNHANDLE                    0xFFFF

// Length of bd addr as a string
#define B_ADDR_STR_LEN                        15

#if defined ( PLUS_BROADCASTER )
  #define ADV_IN_CONN_WAIT                    500 // delay 500 ms
#endif


/* SPI Mode = UASRT 0 Alt1*/
#define MISO            P0_2
#define MOSI            P0_3
#define ADS1293_CS      P0_4 //CS
#define SCK             P0_5 //CLK

#define LIS3DH_CS       P1_0

#define CS_DISABLED     1
#define CS_ENABLED      0

//ADS1293 CONTROL register state values
#define ADS1293_START_CONVERSION 0x01
#define ADS1293_IDLE 0x02
#define ADS1293_POWERDOWN 0x04

//LIS3DH control values
#define LIS3DH_POWERDOWN 0x00
#define LIS3DH_STREAM 0x01

#define CH_DATA_SIZE 3 // 3 bytes

#define RED_LED_PIN     P2_0
#define GREEN_LED_PIN   P1_6
#define BLUE_LED_PIN    P1_7

#define ADS1293_VCC     P1_5

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
uint8 registers[20];
uint8 ecg_data[18]; 
uint8 byteCounter = 0;
uint8 adv_enabled;

lis3dhData_t data;
uint8 buffer = 0x00;

uint8 I2CSlaveBuffer[6];

static uint8 simpleBLEPeripheral_TaskID;   // Task ID for internal task/event processing

static gaprole_States_t gapProfileState = GAPROLE_INIT;

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[] =
{
  // complete name
  0x05,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
  0x45,   // 'E'
  0x43,   // 'C'
  0x47,   // 'G'
  0x38,   // '8'
  
  // connection interval range
  0x05,   // length of this data
  GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
  LO_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),
  HI_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),
  LO_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),
  HI_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),

  // Tx power level
  0x02,   // length of this data
  GAP_ADTYPE_POWER_LEVEL,
  0       // 0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8 advertData[] =
{
  // Flags; this sets the device to use limited discoverable
  // mode (advertises for 30 seconds at a time) instead of general
  // discoverable mode (advertises indefinitely)
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

  // service UUID, to notify central devices what services are included
  // in this peripheral
  0x03,   // length of this data
  GAP_ADTYPE_16BIT_MORE,      // some of the UUID's, but not all
  LO_UINT16( ECGPROFILE_SERV_UUID ),
  HI_UINT16( ECGPROFILE_SERV_UUID ),
};

// GAP GATT Attributes
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "ECG";

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void simpleBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void peripheralStateNotificationCB( gaprole_States_t newState );
static void simpleProfileChangeCB( uint8 paramID );

static void SPIInitialize();
static void ADS1293_Initialize();
static void ADS1293_Control(uint8 value);

uint8 TI_ADS1293_ReadReg(uint8 addr, uint8 *pVal);
void TI_ADS1293_WriteReg(uint8 addr, uint8 value);

void spiWriteByte(uint8 write);
void spiReadByte(uint8 *read, uint8 write);

void EnableDRDYInterrupt();
void ADS1293_ReadDataStream();

static char *bdAddr2Str ( uint8 *pAddr );

uint8 LIS3DH_Initialize();

/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t simpleBLEPeripheral_PeripheralCBs =
{
  peripheralStateNotificationCB,  // Profile State Change Callbacks
  NULL                            // When a valid RSSI is read from controller (not used by application)
};

// GAP Bond Manager Callbacks
static gapBondCBs_t simpleBLEPeripheral_BondMgrCBs =
{
  NULL,                     // Passcode callback (not used by application)
  NULL                      // Pairing / Bonding state Callback (not used by application)
};

// Simple GATT Profile Callbacks
static ECGProfileCBs_t simpleBLEPeripheral_ECGProfileCBs =
{
  simpleProfileChangeCB    // Charactersitic value change callback
};

/*********************************************************************
 * INTERRUPTS
 */

#pragma vector = P0INT_VECTOR
__interrupt void p0_ISR(void)
{
    /* Note that the order in which the following flags are cleared is important.
       For level triggered interrupts (port interrupts) one has to clear the module
       interrupt flag prior to clearing the CPU interrupt flags. */
  
    // Clear CPU interrupt status flag for P0.
    P0IF = 0;
  
    if (P0IFG & BIT1)
    {
      // Clear status flag for pin with R/W0 method, see datasheet.
      P0IFG = ~BIT1;
    
      ADS1293_ReadDataStream();
      
      if(byteCounter == 18)
      {
        byteCounter = 0;
         
        ECGProfile_SetParameter( ECGPROFILE_ECGDATA, 18, &ecg_data);
      }
    }
}


/*********************************************************************
 * @fn      ECG_Init
 *
 * @brief   Initialization function for the Simple BLE Peripheral App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void ECG_Init( uint8 task_id )
{
  simpleBLEPeripheral_TaskID = task_id;

  // Setup the GAP
  VOID GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL );
  
  // Setup the GAP Peripheral Role Profile
  {
    // For other hardware platforms, device starts advertising upon initialization
    uint8 initial_advertising_enable = FALSE;
   
    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16 gapRole_AdvertOffTime = 0;

    // Use a single channel for advertising (useful for debugging with BLE sniffer)
    //uint8 advChannel = GAP_ADVCHAN_37;
    uint8 advChannel = GAP_ADVCHAN_ALL;
    
    uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;

    // Set the GAP Role Parameters
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
    GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, sizeof( uint16 ), &gapRole_AdvertOffTime );

    GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanRspData ), scanRspData );
    GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );

    GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE, sizeof( uint8 ), &enable_update_request );
    GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, sizeof( uint16 ), &desired_min_interval );
    GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, sizeof( uint16 ), &desired_max_interval );
    GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, sizeof( uint16 ), &desired_slave_latency );
    GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER, sizeof( uint16 ), &desired_conn_timeout );
    
    // Set advertising channel(s)
    GAPRole_SetParameter(GAPROLE_ADV_CHANNEL_MAP, sizeof(advChannel), &advChannel);
  }

  // Set the GAP Characteristics
  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName );

  // Set advertising interval
  {
    uint16 advInt = DEFAULT_ADVERTISING_INTERVAL;

    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt );
  }

  // Setup the GAP Bond Manager
  {
    uint32 passkey = 0; // passkey "000000"
    uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    uint8 mitm = FALSE;
    uint8 ioCap = GAPBOND_IO_CAP_NO_INPUT_NO_OUTPUT;
    uint8 bonding = FALSE;
    GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
    GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
    GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
    GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
    GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
  }

  // Initialize GATT attributes
  GGS_AddService( GATT_ALL_SERVICES );            // GAP
  GATTServApp_AddService( GATT_ALL_SERVICES );    // GATT attributes
  DevInfo_AddService();                           // Device Information Service
  ECGProfile_AddService( GATT_ALL_SERVICES );  // Simple GATT Profile
#if defined FEATURE_OAD
  VOID OADTarget_AddService();                    // OAD Profile
#endif

  // Setup the SimpleProfile Characteristic Values
  /*
  {
    uint8 charValue1 = 1;
    uint8 charValue2 = 2;
    uint8 charValue3 = 3;
    uint8 charValue4 = 4;
    uint8 charValue5[SIMPLEPROFILE_CHAR5_LEN] = { 1, 2, 3, 4, 5 };
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR1, sizeof ( uint8 ), &charValue1 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR2, sizeof ( uint8 ), &charValue2 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR3, sizeof ( uint8 ), &charValue3 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR4, sizeof ( uint8 ), &charValue4 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR5, SIMPLEPROFILE_CHAR5_LEN, charValue5 );
  }
  */

//#if defined( CC2540_MINIDK )

  //SK_AddService( GATT_ALL_SERVICES ); // Simple Keys Profile

  // Register for all key events - This app will handle all key events
  //RegisterForKeys( simpleBLEPeripheral_TaskID );

  // makes sure LEDs are off
  //HalLedSet( (HAL_LED_1 | HAL_LED_2), HAL_LED_MODE_OFF );

  // For keyfob board set GPIO pins into a power-optimized state
  // Note that there is still some leakage current from the buzzer,
  // accelerometer, LEDs, and buttons on the PCB.

  P0SEL = 0; // Configure Port 0 as GPIO
  P1SEL = 0; // Configure Port 1 as GPIO
  P2SEL = 0; // Configure Port 2 as GPIO

  P0DIR = 0xFC; // Port 0 pins P0.0 and P0.1 as input (buttons),
                // all others (P0.2-P0.7) as output
  P1DIR = 0xFF; // All port 1 pins (P1.0-P1.7) as output
  P2DIR = 0x1F; // All port 1 pins (P2.0-P2.4) as output

  P0 |= 0x03;   // All pins on port 0 to low, except for P0.0 and P0.1
  P1 |= 0xC0;   // All pins on port 1 to low, except P1_6, and P1_7
  P2 |= 0x01;   // All pins on port 2 to low, except P2_0

  //Testing
  //ADS1293_VCC = 1;
  
  //TESTING LED FUNCTIONALITY
  //P2INP = 0x80; //BLUE LED
  //P1_7 = 1;
  //P1 = 0x7F;

  
  
//#endif // #if defined( CC2540_MINIDK )

#if (defined HAL_LCD) && (HAL_LCD == TRUE)

#if defined FEATURE_OAD
  #if defined (HAL_IMAGE_A)
    HalLcdWriteStringValue( "BLE Peri-A", OAD_VER_NUM( _imgHdr.ver ), 16, HAL_LCD_LINE_1 );
  #else
    HalLcdWriteStringValue( "BLE Peri-B", OAD_VER_NUM( _imgHdr.ver ), 16, HAL_LCD_LINE_1 );
  #endif // HAL_IMAGE_A
#else
  HalLcdWriteString( "BLE Peripheral", HAL_LCD_LINE_1 );
#endif // FEATURE_OAD

#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

  // Register callback with SimpleGATTprofile
  VOID ECGProfile_RegisterAppCBs( &simpleBLEPeripheral_ECGProfileCBs );

  // Enable clock divide on halt
  // This reduces active current while radio is active and CC254x MCU
  // is halted
  HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT );

#if defined ( DC_DC_P0_7 )

  // Enable stack to toggle bypass control on TPS62730 (DC/DC converter)
  HCI_EXT_MapPmIoPortCmd( HCI_EXT_PM_IO_PORT_P0, HCI_EXT_PM_IO_PORT_PIN7 );

#endif // defined ( DC_DC_P0_7 )

  //Initialize the SPI bus
  SPIInitialize();
    
  //Initialize the ADS1293
  //ADS1293_Initialize();
  //LIS3DH_Initialize();
   
  // Setup a delayed profile startup
  osal_set_event( simpleBLEPeripheral_TaskID, ECG_START_DEVICE_EVT );

}

/*********************************************************************
 * @fn      ECG_ProcessEvent
 *
 * @brief   Simple BLE Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16 ECG_ProcessEvent( uint8 task_id, uint16 events )
{

  VOID task_id; // OSAL required parameter that isn't used in this function

  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( simpleBLEPeripheral_TaskID )) != NULL )
    {
      simpleBLEPeripheral_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );

      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & ECG_START_DEVICE_EVT )
  {
    // Start the Device
    VOID GAPRole_StartDevice( &simpleBLEPeripheral_PeripheralCBs );

    // Start Bond Manager
    VOID GAPBondMgr_Register( &simpleBLEPeripheral_BondMgrCBs );

    GREEN_LED_PIN ^= 1;
    osal_start_timerEx( simpleBLEPeripheral_TaskID, ECG_POWERON_LED_EVT, ECG_LED_SLOW_ON_PERIOD );
    
    return ( events ^ ECG_START_DEVICE_EVT );
  }
  
  if ( events & ECG_POWERON_LED_EVT )
  {
    GREEN_LED_PIN = 1;
    osal_stop_timerEx( simpleBLEPeripheral_TaskID, ECG_POWERON_LED_EVT );

    adv_enabled = TRUE;
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &adv_enabled );
    
    return ( events ^ ECG_POWERON_LED_EVT );
  }

  if ( events & ECG_ADVERTISING_LED_EVT )
  {
    BLUE_LED_PIN ^= 1;
    osal_start_timerEx( simpleBLEPeripheral_TaskID, ECG_ADVERTISING_LED_EVT, ECG_LED_FAST_ON_PERIOD );

    return ( events ^ ECG_ADVERTISING_LED_EVT );
  }


#if defined ( PLUS_BROADCASTER )
  if ( events & SBP_ADV_IN_CONNECTION_EVT )
  {
    uint8 turnOnAdv = TRUE;
    // Turn on advertising while in a connection
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &turnOnAdv );

    return (events ^ SBP_ADV_IN_CONNECTION_EVT);
  }
#endif // PLUS_BROADCASTER

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * @fn      simpleBLEPeripheral_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void simpleBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {
  #if defined( CC2540_MINIDK )
    case KEY_CHANGE:
      simpleBLEPeripheral_HandleKeys( ((keyChange_t *)pMsg)->state, ((keyChange_t *)pMsg)->keys );
      break;
  #endif // #if defined( CC2540_MINIDK )

  default:
    // do nothing
    break;
  }
}

#if defined( CC2540_MINIDK )
/*********************************************************************
 * @fn      simpleBLEPeripheral_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void simpleBLEPeripheral_HandleKeys( uint8 shift, uint8 keys )
{
  uint8 SK_Keys = 0;

  VOID shift;  // Intentionally unreferenced parameter

  if ( keys & HAL_KEY_SW_1 )
  {
    SK_Keys |= SK_KEY_LEFT;
  }

  if ( keys & HAL_KEY_SW_2 )
  {

    SK_Keys |= SK_KEY_RIGHT;

    // if device is not in a connection, pressing the right key should toggle
    // advertising on and off
    if( gapProfileState != GAPROLE_CONNECTED )
    {
      uint8 current_adv_enabled_status;
      uint8 new_adv_enabled_status;

      //Find the current GAP advertisement status
      GAPRole_GetParameter( GAPROLE_ADVERT_ENABLED, &current_adv_enabled_status );

      if( current_adv_enabled_status == FALSE )
      {
        new_adv_enabled_status = TRUE;
      }
      else
      {
        new_adv_enabled_status = FALSE;
      }

      //change the GAP advertisement status to opposite of current status
      GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &new_adv_enabled_status );
    }

  }

  // Set the value of the keys state to the Simple Keys Profile;
  // This will send out a notification of the keys state if enabled
  SK_SetParameter( SK_KEY_ATTR, sizeof ( uint8 ), &SK_Keys );
}
#endif // #if defined( CC2540_MINIDK )

/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateNotificationCB( gaprole_States_t newState )
{
  if( gapProfileState == GAPROLE_CONNECTED )
  {
    if ( newState != GAPROLE_CONNECTED)
    {
      // Disconnected
      TI_ADS1293_WriteReg(TI_ADS1293_CONFIG_REG,TI_ADS1293_CONFIG_REG_VALUE);
    }
  }
  
  switch ( newState )
  {
    case GAPROLE_STARTED:
      {
        uint8 ownAddress[B_ADDR_LEN];
        uint8 systemId[DEVINFO_SYSTEM_ID_LEN];

        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

        // use 6 bytes of device address for 8 bytes of system ID value
        systemId[0] = ownAddress[0];
        systemId[1] = ownAddress[1];
        systemId[2] = ownAddress[2];

        // set middle bytes to zero
        systemId[4] = 0x00;
        systemId[3] = 0x00;

        // shift three bytes up
        systemId[7] = ownAddress[5];
        systemId[6] = ownAddress[4];
        systemId[5] = ownAddress[3];

        DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);

        printf("Addr: %s",bdAddr2Str( ownAddress ));
      }
      break;

    case GAPROLE_ADVERTISING:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Advertising",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      
      //TI_ADS1293_WriteReg(TI_ADS1293_CONFIG_REG,TI_ADS1293_CONFIG_REG_VALUE);
      
      BLUE_LED_PIN ^= 1;
      osal_start_timerEx( simpleBLEPeripheral_TaskID, ECG_ADVERTISING_LED_EVT, ECG_LED_FAST_ON_PERIOD );
        
      break;

    case GAPROLE_CONNECTED:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Connected",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

        osal_stop_timerEx( simpleBLEPeripheral_TaskID, ECG_ADVERTISING_LED_EVT );
      
        // For testing purposes only, will draw too much power
        //BLUE_LED_PIN = 0;

        BLUE_LED_PIN = 1; //turn off blue led
        
        // Turn on ADS1293
        ADS1293_VCC = 1;
      }
      break;

    case GAPROLE_WAITING:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Disconnected",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    case GAPROLE_WAITING_AFTER_TIMEOUT:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Timed Out",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    case GAPROLE_ERROR:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Error",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    default:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

  }

  gapProfileState = newState;

#if !defined( CC2540_MINIDK )
  VOID gapProfileState;     // added to prevent compiler warning with
                            // "CC2540 Slave" configurations
#endif


}


/*********************************************************************
 * @fn      simpleProfileChangeCB
 *
 * @brief   Callback from SimpleBLEProfile indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void simpleProfileChangeCB( uint8 paramID )
{
  uint8 ECGControlValue;

  switch( paramID )
  {
    case ECGPROFILE_CONTROL:
      ECGProfile_GetParameter( ECGPROFILE_CONTROL, &ECGControlValue );
  
      ADS1293_Control(ECGControlValue);
      break;

    default:
      // should not reach here!
      break;
  }
}

/*********************************************************************
 * @fn      bdAddr2Str
 *
 * @brief   Convert Bluetooth address to string. Only needed when
 *          LCD display is used.
 *
 * @return  none
 */
char *bdAddr2Str( uint8 *pAddr )
{
  uint8       i;
  char        hex[] = "0123456789ABCDEF";
  static char str[B_ADDR_STR_LEN];
  char        *pStr = str;

  *pStr++ = '0';
  *pStr++ = 'x';

  // Start from end of addr
  pAddr += B_ADDR_LEN;

  for ( i = B_ADDR_LEN; i > 0; i-- )
  {
    *pStr++ = hex[*--pAddr >> 4];
    *pStr++ = hex[*pAddr & 0x0F];
  }

  *pStr = 0;

  return str;
}


/* */
void EnableDRDYInterrupt()
{
  // Clear interrupt flags for P1.7
  P0IFG = ~BIT1;              // Clear status flag for pin.
  P0IF = 0;                   // Clear CPU interrupt status flag for P0.
    
  // Set individual interrupt enable bit in the peripherals SFR
  P0IEN |= BIT1;                              // Enable interrupt from pin.

  // Enable P0 interrupts.
  IEN1 |= IEN1_P0IE;  
}


/*********************************************************************
 * @fn      SPIInitialize
 *
 * @brief   Initialize the SPI port configurations
 *
 * @return  none
 */
static void SPIInitialize()
{
  // Configure USART0 for Alternative 1 => Port P0 (PERCFG.U0CFG = 0).
  PERCFG = (PERCFG & ~PERCFG_U0CFG) | PERCFG_U0CFG_ALT1; 
  
  // Give priority to USART 0 over Timer 1 for port 0 pins.
  P2DIR &= P2DIR_PRIP0_USART0;
  
  // Set pins 2, 3 and 5 as peripheral I/O and pin 4 as GPIO output.
  P0SEL = (P0SEL & ~BIT4) | BIT5 | BIT3 | BIT2;
  P0DIR |= BIT4;

  //Make sure CS pins are disabled
  ADS1293_CS = CS_DISABLED;
  LIS3DH_CS = CS_DISABLED;

  // SPI master mode
  U0CSR = 0x00;

  // SCK frequency = 4MHz, MSB
  //U0GCR |= 0x70;
  //U0BAUD = 0xFF;
  
  U0GCR = 0x20;
  U0GCR |= 0x11;
  U0BAUD = 0x0;    
}


/* */
void ADS1293_Initialize()
{
  //TI_ADS1293_ReadReg(TI_ADS1293_REVID_REG, &registers[0]);  
    
  //set the config register to 0x02 idle
  TI_ADS1293_WriteReg(TI_ADS1293_CONFIG_REG,TI_ADS1293_CONFIG_REG_VALUE);
  TI_ADS1293_ReadReg(TI_ADS1293_CONFIG_REG, &registers[1]);
    
  //Set flex ch1
  TI_ADS1293_WriteReg(TI_ADS1293_FLEX_CH1_CN_REG,TI_ADS1293_FLEX_CH1_CN_REG_VALUE);
  TI_ADS1293_ReadReg(TI_ADS1293_FLEX_CH1_CN_REG, &registers[2]);
  
  //disable lead off detect
  TI_ADS1293_WriteReg(TI_ADS1293_LOD_CN_REG,TI_ADS1293_LOD_CN_REG_VALUE);
  TI_ADS1293_ReadReg(TI_ADS1293_LOD_CN_REG, &registers[3]);
  
  //enable common mode detector
  TI_ADS1293_WriteReg(TI_ADS1293_CMDET_EN_REG,TI_ADS1293_LOD_CN_REG_VALUE);
  TI_ADS1293_ReadReg(TI_ADS1293_CMDET_EN_REG, &registers[4]);
  
  //connect RL amp to pin 4
  TI_ADS1293_WriteReg(TI_ADS1293_RLD_CN_REG,TI_ADS1293_RLD_CN_REG_VALUE);
  TI_ADS1293_ReadReg(TI_ADS1293_RLD_CN_REG, &registers[5]);
  
  //use external xtal
  TI_ADS1293_WriteReg(TI_ADS1293_OSC_CN_REG,TI_ADS1293_OSC_CN_REG_VALUE);
  TI_ADS1293_ReadReg(TI_ADS1293_OSC_CN_REG, &registers[6]);
  
  // Switch clock source to CLK pin (OSC)
  //TI_ADS1293_WriteReg(TI_ADS1293_OSC_CN_REG,0x02);
  //TI_ADS1293_ReadReg(TI_ADS1293_OSC_CN_REG, &registers[6]);
  
  //shut down unused channel(s)
  TI_ADS1293_WriteReg(TI_ADS1293_AFE_SHDN_CN_REG,TI_ADS1293_AFE_SHDN_CN_REG_VALUE);
  TI_ADS1293_ReadReg(TI_ADS1293_AFE_SHDN_CN_REG, &registers[7]);
  
  TI_ADS1293_WriteReg(TI_ADS1293_AFE_FAULT_CN_REG,TI_ADS1293_AFE_FAULT_CN_REG_VALUE);
  TI_ADS1293_ReadReg(TI_ADS1293_AFE_FAULT_CN_REG, &registers[8]);
  
  TI_ADS1293_WriteReg(TI_ADS1293_AFE_PACE_CN_REG,TI_ADS1293_AFE_PACE_CN_REG_VALUE);
  TI_ADS1293_ReadReg(TI_ADS1293_AFE_PACE_CN_REG, &registers[9]);
  
  //configure r2 desimation rates
  TI_ADS1293_WriteReg(TI_ADS1293_R2_RATE_REG,TI_ADS1293_R2_RATE_REG_VALUE);
  TI_ADS1293_ReadReg(TI_ADS1293_R2_RATE_REG, &registers[10]);
  
  //configure r3 desimation rates
  TI_ADS1293_WriteReg(TI_ADS1293_R3_RATE1_REG,TI_ADS1293_R3_RATE1_REG_VALUE);
  TI_ADS1293_ReadReg(TI_ADS1293_R3_RATE1_REG, &registers[11]);
  
  //disable chnl2&3 ECG filters
  TI_ADS1293_WriteReg(TI_ADS1293_DIS_EFILTER_REG,TI_ADS1293_DIS_EFILTER_REG_VALUE);
  TI_ADS1293_ReadReg(TI_ADS1293_DIS_EFILTER_REG, &registers[12]);
  
  //configure DRDYB to channel 1
  TI_ADS1293_WriteReg(TI_ADS1293_DRDYB_SRC_REG,TI_ADS1293_DRDYB_SRC_REG_VALUE);
  TI_ADS1293_ReadReg(TI_ADS1293_DRDYB_SRC_REG, &registers[13]);
  
  //configure filter alarms
  TI_ADS1293_WriteReg(TI_ADS1293_ALARM_FILTER_REG,TI_ADS1293_ALARM_FILTER_REG_VALUE);
  TI_ADS1293_ReadReg(TI_ADS1293_ALARM_FILTER_REG, &registers[14]);
  
  //enable ch1for loop readback mode
  TI_ADS1293_WriteReg(TI_ADS1293_CH_CNFG_REG,TI_ADS1293_CH_CNFG_REG_VALUE);
  TI_ADS1293_ReadReg(TI_ADS1293_CH_CNFG_REG, &registers[15]);
  
  // Enable the clock
  //TI_ADS1293_WriteReg(TI_ADS1293_OSC_CN_REG,0x06);
  //TI_ADS1293_ReadReg(TI_ADS1293_OSC_CN_REG, &registers[16]);
}


static void ADS1293_Control(uint8 value)
{
  switch(value)
  {
    case ADS1293_POWERDOWN:
      //P1_5 = 0;
      ADS1293_VCC = 0;
      
      TI_ADS1293_WriteReg(TI_ADS1293_CONFIG_REG,TI_ADS1293_CONFIG_REG_VALUE);
      
      GAPRole_TerminateConnection();
      break;
    case ADS1293_START_CONVERSION:
      //ensure chip is initialized properly
      //P1_5 = 1;
      
      ADS1293_VCC = 1;
      
      ADS1293_Initialize();
        
      EnableDRDYInterrupt();
      
      TI_ADS1293_WriteReg(TI_ADS1293_CONFIG_REG,value);
      break;
    case ADS1293_IDLE:
      break;
  }
}


/**************************************************************************//**
* @fn       spiWriteByte(uint8 write)
*
* @brief    Write one byte to SPI interface
*
* @param    write   Value to write
******************************************************************************/
void spiWriteByte(uint8 write)
{
  U0CSR &= ~0x02;                 // Clear TX_BYTE
  U0DBUF = write;
  while (!(U0CSR & 0x02));        // Wait for TX_BYTE to be set
}


/**************************************************************************//**
* @fn       spiReadByte(uint8 *read, uint8 write)
*
* @brief    Read one byte from SPI interface
*
* @param    read    Read out value
* @param    write   Value to write
******************************************************************************/
void spiReadByte(uint8 *read, uint8 write)
{
        U0CSR &= ~0x02;                 // Clear TX_BYTE
        U0DBUF = write;                 // Write address to accelerometer
        while (!(U0CSR & 0x02));        // Wait for TX_BYTE to be set
        *read = U0DBUF;                 // Save returned value
}


/* */
uint8 TI_ADS1293_ReadReg(uint8 addr, uint8 *pVal)
{
  uint8 inst;
    
  inst = ADS1293_READ_BIT | addr;
  
  ADS1293_CS = CS_ENABLED;
    
  spiWriteByte(inst);

  spiReadByte(pVal, 0x00);
  
  ADS1293_CS = CS_DISABLED;   
  
  return(0);
}


/* */
void TI_ADS1293_WriteReg(uint8 addr, uint8 value)
{
  uint8 inst;
  
  inst = ADS1293_WRITE_BIT & addr;
  
  ADS1293_CS = CS_ENABLED;
  
  spiWriteByte(inst);
  
  spiWriteByte(value);
  
  ADS1293_CS = CS_DISABLED;
}


/* */
void ADS1293_ReadDataStream()
{
  uint8 inst;
  
  inst = ADS1293_READ_BIT | TI_ADS1293_DATA_LOOP_REG;
  
  ADS1293_CS = CS_ENABLED;
  
  spiWriteByte(inst);

  for(uint8 i=0;i<3;i++)
  {
    spiReadByte(&ecg_data[byteCounter], 0x00);
    byteCounter++;
  }
  
  ADS1293_CS = CS_DISABLED;   
}


uint8 LIS3DH_ReadReg(uint8 addr, uint8 *pVal)
{
  uint8 inst;
  
  inst = LIS3DH_READ_BIT | addr;
  
  LIS3DH_CS = CS_ENABLED;
    
  spiWriteByte(inst);

  spiReadByte(pVal, 0x00);
  
  LIS3DH_CS = CS_DISABLED;   
  
  return(0);
}


void LIS3DH_WriteReg(uint8 addr, uint8 value)
{
  uint8 inst;
  
  inst = LIS3DH_WRITE_BIT & addr;
  
  LIS3DH_CS = CS_ENABLED;
  
  spiWriteByte(inst);
  
  spiWriteByte(value);
  
  LIS3DH_CS = CS_DISABLED;
}


/**************************************************************************/
/*!
    @brief  Reads three signed 16 bit values over I2C
*/
/**************************************************************************/
void LIS3DH_ReadXYZ(uint8 addr, uint16 *x, uint16 *y, uint16 *z)
{
  uint8 inst;
  
  inst = LIS3DH_READ_BIT | LIS3DS_MULTIPLE_RW_BIT | addr;
  
  LIS3DH_CS = CS_ENABLED;
    
  spiWriteByte(inst);

  for(uint8 i=0;i<6;i++)
  {
    spiReadByte(&I2CSlaveBuffer[i], 0x00);
  }
  
  *x = (uint16)(I2CSlaveBuffer[1] << 8 | I2CSlaveBuffer[0]);
  *y = (uint16)(I2CSlaveBuffer[3] << 8 | I2CSlaveBuffer[2]);
  *z = (uint16)(I2CSlaveBuffer[5] << 8 | I2CSlaveBuffer[4]);
    
  LIS3DH_CS = CS_DISABLED;   
  
}

/**************************************************************************/
/*!
    @brief  Polls the device for a new X/Y/Z reading
*/
/**************************************************************************/
uint8 LIS3DH_Poll(lis3dhData_t* data)
{
  /* Check the status register until a new X/Y/Z sample is ready */
  //do
  //{
    LIS3DH_ReadReg(LIS3DH_REGISTER_STATUS_REG2, &buffer);
  //} while (!(buffer & LIS3DH_STATUS_REG_ZYXDA));
  
  /* For now, always read data even if it hasn't changed */
  LIS3DH_ReadXYZ(LIS3DH_REGISTER_OUT_X_L, &(data->x), &(data->y), &(data->z));
  
  // Send all bytes as-is for testing  
  AccelProfile_SetParameter( ACCELPROFILE_ACCELDATA, 6, &I2CSlaveBuffer);
  
  return(0);
}


uint8 LIS3DH_Initialize()
{  
  registers[0] = 0x00;
  
  P1_4 = 1;
  
  LIS3DH_ReadReg(LIS3DH_REGISTER_WHO_AM_I, &registers[0]);
  
  /* Normal mode, 50Hz */
  LIS3DH_WriteReg(LIS3DH_REGISTER_CTRL_REG1,
    LIS3DH_CTRL_REG1_DATARATE_50HZ |    
    LIS3DH_CTRL_REG1_XYZEN);
    
  /* Enable block update and set range to +/-2G */
  LIS3DH_WriteReg(LIS3DH_REGISTER_CTRL_REG4,
    LIS3DH_CTRL_REG4_BLOCKDATAUPDATE |  /* Enable block update */
    LIS3DH_CTRL_REG4_SCALE_2G);        /* +/-2G measurement range */
  
  //do { 
  //  LIS3DH_Poll(&data);
  //} while(1);
  
  return(0);
}
