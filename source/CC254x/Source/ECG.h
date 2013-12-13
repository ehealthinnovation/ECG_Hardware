/**************************************************************************************************
  Filename:       ecg.h
  Revised:        $Date: 
  Revision:       $Revision: 
**************************************************************************************************/

#ifndef ECG_H
#define ECG_H

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

// PERCFG (0xF1) - Peripheral Control
#define PERCFG_U0CFG                  0x01               // USART 0 I/O location
#define PERCFG_U0CFG_ALT1                 0x00          // Alternative 1 location
  
// P2DIR (0xFF) – Port 2 Direction and Port 0 Peripheral Priority Control
#define P2DIR_PRIP0_USART0                (0x00 << 6) // USART 0 has priority, then USART 1, then Timer 1
  
// Simple BLE Peripheral Task Events
#define SBP_START_DEVICE_EVT                              0x0001
#define SBP_PERIODIC_EVT                                  0x0002
#define SBP_ADV_IN_CONNECTION_EVT                         0x0004
#define SBP_LED_EVT                                       0x0008
  
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the BLE Application
 */
extern void ECG_Init( uint8 task_id );

/*
 * Task Event Processor for the BLE Application
 */
extern uint16 ECG_ProcessEvent( uint8 task_id, uint16 events );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ECG_H */
