/****************************************************************************
 * $Rev::                   $: Revision of last commit
 * $Author::                $: Author of last commit
 * $Date::                  $: Date of last commit
 * $HeadURL:                $
 ****************************************************************************
 * This software is owned by Jennic and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on Jennic products. You, and any third parties must reproduce
 * the copyright and warranty notice and any other legend of ownership on each
 * copy or partial copy of the software.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS". JENNIC MAKES NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * ACCURACY OR LACK OF NEGLIGENCE. JENNIC SHALL NOT, IN ANY CIRCUMSTANCES,
 * BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT LIMITED TO, SPECIAL,
 * INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON WHATSOEVER.
 *
 * Copyright Jennic Ltd 2010. All rights reserved
 ****************************************************************************/
#ifndef  SERIAL_H_INCLUDED
#define  SERIAL_H_INCLUDED

#if defined __cplusplus
extern "C"
{
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include <jendefs.h>

#include "queue.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct
{
    uint8   u8Port;
    uint8   u8DataBits;
    uint8   u8Parity;
    uint8   u8StopBits;
    uint32  u32BaudRate;
    uint8   *pu8RxBuff;
    uint8   *pu8TxBuff;
    uint16  u16RxBuffLen;
    uint16  u16TxBuffLen;
}tsSerialPortSetup;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC bool_t bSerial_Close(uint8 u8Port);
PUBLIC void vSerial_Init(tsSerialPortSetup *psPort);
PUBLIC int16 i16Serial_RxChar(uint8 u8Port);
PUBLIC bool_t bSerial_TxChar(uint8 u8Port, uint8 u8Chr);
PUBLIC void vSerial_TxString(uint8 u8Port, uint8 *pu8String);
PUBLIC uint32 u32Serial_RxString(uint8 u8Port, uint8 *pu8RxBuffer, uint32 u32BuffSize);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* SERIAL_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
