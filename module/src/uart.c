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
 * Copyright Jennic Ltd 2007,2008. All rights reserved
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

#include "intr.h"
#include "uart.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define UART_MAX_UARTS      2
#define UART_0_START_ADR  	0x02003000UL
#define UART_1_START_ADR  	0x02004000UL
#define UART_LCR_OFFSET 	0x0C
#define UART_DLM_OFFSET 	0x04

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct
{
    void   (*pfRxComplete)(uint8, uint8);
    bool_t (*pfTxComplete)(uint8, uint8 *);
}tsUART;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vUART_SetBuadRate(uint8 u8Port, uint32 u32BaudRate);
PRIVATE void vUART_HandleUartInterrupt(uint32 u32Device, uint32 u32ItemBitmap);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE tsUART asUART[UART_MAX_UARTS];

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: vUART_Init
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
PUBLIC void vUART_Init(uint8  u8Port,
                       uint32 u32BaudRate,
                       uint8  u8DataBits,
                       uint8  u8Parity,
                       uint8  u8StopBits,
                       void   (*pfRxComplete)(uint8, uint8),
                       bool_t (*pfTxComplete)(uint8, uint8 *))
{
	bool_t bEvenParity   = FALSE;
	bool_t bEnableParity = FALSE;
	bool_t bOneStopBit   = FALSE;

    /* Enable and reset required UART */
    vAHI_UartEnable(u8Port);
    vAHI_UartReset(u8Port, TRUE, TRUE);
    vAHI_UartReset(u8Port, FALSE, FALSE);

    /* Register higher layer handlers */
    asUART[u8Port].pfRxComplete = pfRxComplete;
    asUART[u8Port].pfTxComplete = pfTxComplete;

    /* Register function that will handle UART interrupts */
    if (u8Port == 0)
    {
        vAHI_Uart0RegisterCallback(vUART_HandleUartInterrupt);
    }
    else
    {
        vAHI_Uart1RegisterCallback(vUART_HandleUartInterrupt);
    }

    /* Set the clock divisor register to give required buad, this has to be done
       directly as the normal routines (in ROM) do not support all baud rates */
    vUART_SetBuadRate(u8Port, u32BaudRate);

	if (u8StopBits == 1)
	{
		bOneStopBit = TRUE;
	}

    vAHI_UartSetControl(u8Port, bEvenParity, bEnableParity, E_AHI_UART_WORD_LEN_8, bOneStopBit, FALSE);
    vAHI_UartSetInterrupt(u8Port, FALSE, FALSE, TRUE, TRUE, E_AHI_UART_FIFO_LEVEL_1);
    vAHI_UartSetRTSCTS(u8Port, FALSE); /* Allow RTS/CTS to be used as DIO */

}

/****************************************************************************
 *
 * NAME: vUART_StartTx
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
PUBLIC void vUART_StartTx(uint8 u8Port)
{
  	/* Has interrupt driven transmit stalled (tx fifo is empty) */
   	if (u8AHI_UartReadLineStatus(u8Port) & E_AHI_UART_LS_THRE)
   	{
        uint8 u8Byte;

        if (asUART[u8Port].pfTxComplete(u8Port, &u8Byte))
        {
            vAHI_UartWriteData(u8Port, u8Byte);
        }
  	}
}

/****************************************************************************
 *
 * NAME: vUART_Disable
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 *
 ****************************************************************************/
PUBLIC bool_t bUART_Close(uint8 u8UART)
{
    bool_t bResult = FALSE;

    /* Read the UART status */
    uint8 u8Status = u8AHI_UartReadLineStatus(u8UART);

    /* Check if the transmit shift register is empty */
    if (u8Status & E_AHI_UART_LS_TEMT)
    {
        /* Check if the transmit FIFO is empty */
        if (u8Status & E_AHI_UART_LS_THRE)
        {
            /* UART has finished transmitting and so can be closed */
            vAHI_UartDisable(u8UART);

            bResult = TRUE;
        }
    }
    return (bResult);
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: vUART_SetBuadRate
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 *
 ****************************************************************************/
PRIVATE void vUART_SetBuadRate(uint8 u8Port, uint32 u32BaudRate)
{
    uint8 *pu8Reg;
    uint8  u8TempLcr;
    uint16 u16Divisor;
    uint32 u32Remainder;
    uint32 u32UARTStartAddr;

    if (u8Port == 0)
    {
        u32UARTStartAddr = UART_0_START_ADR;
    }
    else
    {
        u32UARTStartAddr = UART_1_START_ADR;
    }

    /* Put UART into clock divisor setting mode */
    pu8Reg    = (uint8 *)(u32UARTStartAddr + UART_LCR_OFFSET);
    u8TempLcr = *pu8Reg;
    *pu8Reg   = u8TempLcr | 0x80;

    /* Write to divisor registers:
       Divisor register = 16MHz / (16 x baud rate) */
    u16Divisor = (uint16)(16000000UL / (16UL * u32BaudRate));

    /* Correct for rounding errors */
    u32Remainder = (uint32)(16000000UL % (16UL * u32BaudRate));

    if (u32Remainder >= ((16UL * u32BaudRate) / 2))
    {
        u16Divisor += 1;
    }

    pu8Reg  = (uint8 *)u32UARTStartAddr;
    *pu8Reg = (uint8)(u16Divisor & 0xFF);
    pu8Reg  = (uint8 *)(u32UARTStartAddr + UART_DLM_OFFSET);
    *pu8Reg = (uint8)(u16Divisor >> 8);

    /* Put back into normal mode */
    pu8Reg    = (uint8 *)(u32UARTStartAddr + UART_LCR_OFFSET);
    u8TempLcr = *pu8Reg;
    *pu8Reg   = u8TempLcr & 0x7F;
}

/****************************************************************************
 *
 * NAME: vUART_HandleUart0Interrupt
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
PRIVATE void vUART_HandleUartInterrupt(uint32 u32Device, uint32 u32ItemBitmap)
{
    uint8 u8UART;

    if (u32Device == E_AHI_DEVICE_UART0)
    {
        u8UART = 0;
    }
    else if (u32Device == E_AHI_DEVICE_UART1)
    {
        u8UART = 1;
    }
    else
    {
        return;
    }

    if ((u32ItemBitmap & 0x000000FF) == E_AHI_UART_INT_RXDATA || (u32ItemBitmap & 0x000000FF) ==  E_AHI_UART_INT_TIMEOUT)
    {
        if (asUART[u8UART].pfRxComplete != NULL)
        {
            while(u8AHI_UartReadLineStatus(u8UART) & E_AHI_UART_LS_DR)
            {
                asUART[u8UART].pfRxComplete(u8UART, u8AHI_UartReadData(u8UART));
            }
        }
    }
    else if (u32ItemBitmap & E_AHI_UART_INT_TX)
    {
        if (asUART[u8UART].pfTxComplete != NULL)
        {
            uint8 i;

            for(i = 0; i < 16; i++)
            {
                uint8 u8Byte;

                if (asUART[u8UART].pfTxComplete(u8UART, &u8Byte))
                {
                    vAHI_UartWriteData(u8UART, u8Byte);
                }
            }
        }
    }
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
