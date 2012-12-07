// (c)2009 by Texas Instruments Incorporated, All Rights Reserved.
/*----------------------------------------------------------------------------+
|                                                                             |
|                              Texas Instruments                              |
|                                                                             |
|                          MSP430 USB-Example                                 |
|                                                                             |
+-----------------------------------------------------------------------------+
|  Source: types.h, v1.1 2009/06/29                                           |
|  Author: Rostyslav Stolyar                                                  |
|                                                                             |
|  WHO          WHEN         WHAT                                             |
|  ---          ----------   ------------------------------------------------ |
|  R.Stolyar    2008/09/03   born                                             |
|  R.Stolyar    2009/06/24   Change to _tiny version                          |
+----------------------------------------------------------------------------*/
#ifndef _TYPES_H_
#define _TYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*----------------------------------------------------------------------------+
| Include files                                                               |
+----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
| Function Prototype                                                          |
+----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
| Type Definition & Macro                                                     |
+----------------------------------------------------------------------------*/
typedef char            CHAR;
typedef unsigned char   UCHAR;
typedef int             INT;
typedef unsigned int    UINT;
typedef short           SHORT;
typedef unsigned short  USHORT;
typedef long            LONG;
typedef unsigned long   ULONG;
typedef void            VOID;
typedef unsigned long   HANDLE;
typedef char *          PSTR;
typedef int             BOOL;
typedef double          DOUBLE;
//typedef unsigned char   BYTE;
typedef unsigned char*  PBYTE;
typedef unsigned int    WORD;
typedef unsigned long   DWORD;
typedef unsigned long*  PDWORD;
#define VOID void
#if 0
// DEVICE_REQUEST Structure
typedef struct _tDEVICE_REQUEST
{
    BYTE    bmRequestType;              // See bit definitions below
    BYTE    bRequest;                   // See value definitions below
    WORD    wValue;                    // Meaning varies with request type
    WORD    wIndex;                    // Meaning varies with request type
    WORD    wLength;                   // Number of bytes of data to transfer
} tDEVICE_REQUEST, *ptDEVICE_REQUEST;


//----------------------------------------------------------------------------
typedef enum
{
    STATUS_ACTION_NOTHING,
    STATUS_ACTION_DATA_IN,
    STATUS_ACTION_DATA_OUT
} tSTATUS_ACTION_LIST;

typedef enum
{
    DISABLE,
    ENABLE
} tSTATUS_EN_DISABLED;

typedef enum
{
    FALSE,
    TRUE
} tBOOL;
#endif
/*----------------------------------------------------------------------------+
| Constant Definition                                                         |
+----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------+
| End of header file                                                          |
+----------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
#endif /* _TYPES_H_ */
/*------------------------ Nothing Below This Line --------------------------*/
