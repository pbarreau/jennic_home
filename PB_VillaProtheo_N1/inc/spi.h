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
#ifndef  SPI_H_INCLUDED
#define  SPI_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include <jendefs.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/* SPI clock rate is 16MHz divided by 2 x clock divider (specified below). */
#define SPI_CLOCK_8MHz		1
#define SPI_CLOCK_4MHz		2
#define SPI_CLOCK_2MHz		4
#define SPI_CLOCK_1MHz		8
#define SPI_CLOCK_500KHz	16
#define SPI_CLOCK_250KHz	32

/* SPI Modes */
#define SPI_MODE_0			0
#define SPI_MODE_1			1
#define SPI_MODE_2			2
#define SPI_MODE_3			3

/* SPI Chip Select lines */
#define SPI_CS_NONE         0x00
#define SPI_CS_1			0x02
#define SPI_CS_2			0x04
#define SPI_CS_3			0x08
#define SPI_CS_4			0x10

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC uint8 u8SPI_Read8(void);
PUBLIC void vSPI_Write8(uint8 u8Data);
PUBLIC void vSPI_SelectChip(uint8 u8CS);
PUBLIC void vSPI_Init(uint8 u8Mode, uint8 u8ClockRate);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* SPI_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
