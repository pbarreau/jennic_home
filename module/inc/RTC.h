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
#ifndef  RTC_H_INCLUDED
#define  RTC_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define RTC_WAKE_TIMER_NONE		0
#define RTC_WAKE_TIMER_0		1
#define RTC_WAKE_TIMER_1		2

#define RTC_CLK_SRC_APP			0
#define RTC_CLK_SRC_WKT_INT		1
#define RTC_CLK_SRC_WKT_EXT		2

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct
{
    uint8  u8WakeTimer;
    uint32 u32LastCount;
    uint8  u8Hours;
    uint8  u8Minutes;
    uint8  u8Seconds;
    uint32 u32ClockTicks;
    uint16 u16Year;
    uint8  u8Month;
    uint8  u8Day;
}tsRTC;

typedef struct
{
    uint8  u8Hours;
    uint8  u8Minutes;
    uint8  u8Seconds;
}tsTime;

typedef struct
{
    uint16 u16Year;
    uint8  u8Month;
    uint8  u8Day;
}tsDate;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC void vRTC_Start(void);
PUBLIC void vRTC_Update(void);
PUBLIC void vRTC_SetTime(tsTime *psTime);
PUBLIC void vRTC_GetTime(tsTime *psTime);
PUBLIC void vRTC_SetDate(tsDate *psDate);
PUBLIC void vRTC_GetDate(tsDate *psDate);
PUBLIC void vRTC_Tick(uint32 u32ElapsedTicks);
PUBLIC void vRTC_Init(uint8 u8WakeTimer, uint8 u8ClockSource);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* RTC_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
