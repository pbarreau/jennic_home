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

#include "spi.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME:
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
PUBLIC void vSPI_Init(uint8 u8Mode, uint8 u8ClockRate)
{
	switch (u8Mode)
	{
		case SPI_MODE_0:
			vAHI_SpiConfigure(1, FALSE, FALSE, FALSE, u8ClockRate, FALSE, FALSE);
			break;
		case SPI_MODE_1:
			vAHI_SpiConfigure(1, FALSE, FALSE, TRUE, u8ClockRate, FALSE, FALSE);
			break;
		case SPI_MODE_2:
			vAHI_SpiConfigure(1, FALSE, TRUE, FALSE, u8ClockRate, FALSE, FALSE);
			break;
		case SPI_MODE_3:
			vAHI_SpiConfigure(1, FALSE, TRUE, TRUE, u8ClockRate, FALSE, FALSE);
			break;
		default:
			break;
	}
}

/****************************************************************************
 *
 * NAME:
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
PUBLIC void vSPI_SelectChip(uint8 u8CS)
{
    vAHI_SpiSelect(u8CS);
}

/****************************************************************************
 *
 * NAME:
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
PUBLIC void vSPI_Write8(uint8 u8Data)
{
    /* Output data */
    vAHI_SpiStartTransfer8(u8Data);

    /* Wait until transfer is complete */
    vAHI_SpiWaitBusy();
}

/****************************************************************************
 *
 * NAME:
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
PUBLIC uint8 u8SPI_Read8(void)
{
	uint8 u8Data;

    /* Output dummy data */
    vAHI_SpiStartTransfer8(0);

    /* Wait until transfer is complete */
    vAHI_SpiWaitBusy();

    /* Read input data */
	u8Data = u8AHI_SpiReadTransfer8();

    return (u8Data);
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
