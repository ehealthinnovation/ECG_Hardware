#include "..\Common\device.h"
#include "..\Common\types.h"          // Basic Type declarations

#include "..\USB_Common\descriptors.h"
#include "..\USB_Common\usb.h"        // USB-specific functions

//#include "main.h"
#ifdef _CDC_
    #include "..\USB_CDC_API\UsbCdc.h"
#endif
#ifdef _HID_
    #include "..\USB_HID_API\UsbHid.h"
#endif

#include <intrinsics.h>
#include "USB_constructs.h"




//*********************************************************************************************
// Please see the MSP430 CDC/HID USB API Programmer's Guide for a full description of these 
// functions, how they work, and how to use them.  
//*********************************************************************************************



#ifdef _HID_
// This call assumes no previous send operation is underway; also assumes size is non-zero.  
// Returns zero if send completed; non-zero if it failed.  
BYTE hid_sendDataWaitTilDone(BYTE* dataBuf, WORD size, BYTE intfNum, ULONG ulTimeout)
{
  ULONG sendCounter = 0;
  WORD bytesSent, bytesReceived;

  switch(USBHID_sendData(dataBuf,size,intfNum))
  {
     case kUSBHID_sendStarted:
          break;
     case kUSBHID_busNotAvailable:
          return 2;
     case kUSBHID_intfBusyError:
          return 3;
     case kUSBHID_generalError:
          return 4;
     default:;                                  
  }
  
  // If execution reaches this point, then the operation successfully started.  Now wait til it's finished.  
  while(1)                                      
  {
    BYTE ret = USBHID_intfStatus(intfNum,&bytesSent,&bytesReceived);
    if(ret & kUSBHID_busNotAvailable)                // This may happen at any time
      return 2;
    if(ret & kUSBHID_waitingForSend)
    {
      if(ulTimeout && (sendCounter++ >= ulTimeout))  // Incr counter & try again
        return 1 ;                                   // Timed out
    }
    else 
      return 0;                                      // If neither busNotAvailable nor waitingForSend, it succeeded
  }
}


// This call assumes a previous send operation might be underway; also assumes size is non-zero.  
// Returns zero if send completed; non-zero if it failed, with 1 = timeout and 2 = bus is gone.  
BYTE hid_sendDataInBackground(BYTE* dataBuf, WORD size, BYTE intfNum, ULONG ulTimeout)
{
  ULONG sendCounter = 0; 
  WORD bytesSent, bytesReceived;
  
  while(USBHID_intfStatus(intfNum,&bytesSent,&bytesReceived) & kUSBHID_waitingForSend)
  {
    if(ulTimeout && ((sendCounter++)>ulTimeout))  // A send operation is underway; incr counter & try again
      return 1;                                   // Timed out               
  }
  
  // The interface is now clear.  Call sendData().  
  switch(USBHID_sendData(dataBuf,size,intfNum))
  {
     case kUSBHID_sendStarted:
          return 0;
     case kUSBHID_busNotAvailable:
          return 2;
     default:
          return 4;
  }
}                                  

  
                         
// This call assumes a prevoius receive operation is NOT underway.  It only retrieves what data is waiting in the buffer
// It doesn't check for kUSBHID_busNotAvailable, b/c it doesn't matter if it's not.  size is the maximum that
// is allowed to be received before exiting; i.e., it is the size allotted to dataBuf.  
// Returns the number of bytes received.  
WORD hid_receiveDataInBuffer(BYTE* dataBuf, WORD size, BYTE intfNum)
{
  WORD bytesInBuf;
  BYTE* currentPos=dataBuf;
 
  while(bytesInBuf = USBHID_bytesInUSBBuffer(intfNum))
  {
    if((WORD)(currentPos-dataBuf+bytesInBuf) > size) 
      break;
 
    USBHID_receiveData(currentPos,bytesInBuf,intfNum); 
    currentPos += bytesInBuf;
  } 
  return (currentPos-dataBuf);
}
#endif

//*********************************************************************************************
// Please see the MSP430 USB CDC API Programmer's Guide Sec. 9 for a full description of these 
// functions, how they work, and how to use them.  
//*********************************************************************************************

#ifdef _CDC_
// This call assumes no previous send operation is underway; also assumes size is non-zero.  
// Returns zero if send completed; non-zero if it failed.  
BYTE cdc_sendDataWaitTilDone(BYTE* dataBuf, WORD size, BYTE intfNum, ULONG ulTimeout)
{
  ULONG sendCounter = 0;
  WORD bytesSent, bytesReceived;

  switch(USBCDC_sendData(dataBuf,size,intfNum))
  {
     case kUSBCDC_sendStarted:
          break;
     case kUSBCDC_busNotAvailable:
          return 2;
     case kUSBCDC_intfBusyError:
          return 3;
     case kUSBCDC_generalError:
          return 4;
     default:;                                  
  }
  
  // If execution reaches this point, then the operation successfully started.  Now wait til it's finished.  
  while(1)                                     
  {
    BYTE ret = USBCDC_intfStatus(intfNum,&bytesSent,&bytesReceived);
    if(ret & kUSBCDC_busNotAvailable)                // This may happen at any time
      return 2;
    if(ret & kUSBCDC_waitingForSend)
    {
      if(ulTimeout && (sendCounter++ >= ulTimeout))  // Incr counter & try again
        return 1 ;                                   // Timed out
    }
    else 
      return 0;                                   // If neither busNotAvailable nor waitingForSend, it succeeded
  }
}



// This call assumes a previous send operation might be underway; also assumes size is non-zero.  
// Returns zero if send completed; non-zero if it failed, with 1 = timeout and 2 = bus is gone.  
BYTE cdc_sendDataInBackground(BYTE* dataBuf, WORD size, BYTE intfNum, ULONG ulTimeout)
{
  ULONG sendCounter = 0; 
  WORD bytesSent, bytesReceived;
  
  while(USBCDC_intfStatus(intfNum,&bytesSent,&bytesReceived) & kUSBCDC_waitingForSend)
  {
    if(ulTimeout && ((sendCounter++)>ulTimeout))  // A send operation is underway; incr counter & try again
      return 1;                                   // Timed out               
  }
  
  // The interface is now clear.  Call sendData().  
  switch(USBCDC_sendData(dataBuf,size,intfNum))
  {
     case kUSBCDC_sendStarted:
          return 0;
     case kUSBCDC_busNotAvailable:
          return 2;
     default:
          return 4;
  }
}                                  

                         
                         
// This call assumes a prevoius receive operation is NOT underway.  It only retrieves what data is waiting in the buffer
// It doesn't check for kUSBCDC_busNotAvailable, b/c it doesn't matter if it's not.  size is the maximum that
// is allowed to be received before exiting; i.e., it is the size allotted to dataBuf.  
// Returns the number of bytes received.  
WORD cdc_receiveDataInBuffer(BYTE* dataBuf, WORD size, BYTE intfNum)
{
  WORD bytesInBuf;
  BYTE* currentPos=dataBuf;
 
  while(bytesInBuf = USBCDC_bytesInUSBBuffer(intfNum))
  {
    if((WORD)(currentPos-dataBuf+bytesInBuf) > size) 
      break;
 
    USBCDC_receiveData(currentPos,bytesInBuf,intfNum); 
    currentPos += bytesInBuf;
  } 
  return (currentPos-dataBuf);
}
#endif
