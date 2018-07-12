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
/** @file
 *
 * @defgroup
 *
 */
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include <AppHardwareApi.h>

#include "Utils.h"

#include "intr.h"
#include "uart.h"
#include "queue.h"
#include "serial.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define SERIAL_MAX_PORTS    2

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct
{
    tsQueue sRxQueue;
    tsQueue sTxQueue;
}tsSerialPort;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vSerial_RxComplete(uint8 u8Port, uint8 u8Char);
PRIVATE bool_t bSerial_TxComplete(uint8 u8Port, uint8 *pu8Byte);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE tsSerialPort asPort[SERIAL_MAX_PORTS];

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: vSerial_Init
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC void vSerial_Init(tsSerialPortSetup *psPort)
{
    /* Initialise serial queues */
    vQueue_Init(&asPort[psPort->u8Port].sRxQueue, psPort->pu8RxBuff, psPort->u16RxBuffLen, sizeof(uint8));
    vQueue_Init(&asPort[psPort->u8Port].sTxQueue, psPort->pu8TxBuff, psPort->u16TxBuffLen, sizeof(uint8));

    /* Initialise the UART */
    vUART_Init(psPort->u8Port,
               psPort->u32BaudRate,
               psPort->u8DataBits,
               psPort->u8Parity,
               psPort->u8StopBits,
               vSerial_RxComplete,
               bSerial_TxComplete);
}

/****************************************************************************
 *
 * NAME: vSerial_Close
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC bool_t bSerial_Close(uint8 u8Port)
{
    bool_t bResult = FALSE;

    /* Check if output queue is empty */
    if (u16Queue_ItemsPresent(&asPort[u8Port].sTxQueue) == 0)
    {
        /* Attempt to close UART, only happens if transmit is complete */
        if (bUART_Close(u8Port))
        {
            bResult = TRUE;
        }
    }
    return (bResult);
}

/****************************************************************************
 *
 * NAME: bSerial_TxChar
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC bool_t bSerial_TxChar(uint8 u8Port, uint8 u8Chr)
{
    bool_t bRetValue = FALSE;

	DISABLE_INTERRUPTS();
    {
        if (bQueue_AddItem(&asPort[u8Port].sTxQueue, (void *)&u8Chr))
        {
            bRetValue = TRUE;

            vUART_StartTx(u8Port);
        }
    }
    ENABLE_INTERRUPTS();

    return (bRetValue);
}

/****************************************************************************
 *
 * NAME: vSerial_TxChar
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC void vSerial_TxString(uint8 u8Port, uint8 *pu8String)
{
    while(*pu8String)
    {
        (void)bSerial_TxChar(u8Port, *pu8String);
        pu8String++;
    }
}

/****************************************************************************
 *
 * NAME: i16Serial_RxChar
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC int16 i16Serial_RxChar(uint8 u8Port)
{
    int16 i16Result;

    if(bQueue_RemoveItem(&asPort[u8Port].sRxQueue, (void *)&i16Result))
    {
        i16Result >>= 8;
    }
    else
    {
        i16Result = -1;
    }
    return(i16Result);
}

/****************************************************************************
 *
 * NAME: u32Serial_RxString
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * Blocks until a carriage return character is received.
 ****************************************************************************/
PUBLIC uint32 u32Serial_RxString(uint8 u8Port, uint8 *pu8RxBuffer, uint32 u32BuffSize)
{
	int16 i16RxChar = 0;
	uint32 u32CharCount = 0;

	while ((uint8)i16RxChar != 0x0D)
	{
		while((i16RxChar = i16Serial_RxChar(u8Port)) < 0);

		if (u32CharCount < u32BuffSize)
		{
            pu8RxBuffer[u32CharCount++] = (uint8)i16RxChar;
            // Afficher le charactere
            vAHI_UartWriteData(UTILS_UART, (uint8)i16RxChar);
            while ((u8AHI_UartReadLineStatus(UTILS_UART) & 0x20) == 0);

		}
		else
		{
            break;
		}
	}
	return(u32CharCount);
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: vSerial_RxComplete
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 *
 ****************************************************************************/
PRIVATE void vSerial_RxComplete(uint8 u8Port, uint8 u8Char)
{
    (void)bQueue_AddItem(&asPort[u8Port].sRxQueue, (void *)&u8Char);
}

/****************************************************************************
 *
 * NAME: vSerial_TxComplete
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 *
 ****************************************************************************/
PRIVATE bool_t bSerial_TxComplete(uint8 u8Port, uint8 *pu8Byte)
{
    return (bQueue_RemoveItem(&asPort[u8Port].sTxQueue, (void *)pu8Byte));
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
