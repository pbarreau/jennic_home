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

#include "time.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define TICK_PERIOD_ms                  1UL
#define CPU_CLOCK_FREQ_HZ               16000000UL

#define TICK_TIMER_TICK_PERIOD_COUNT    (16000UL * TICK_PERIOD_ms)

/* Timer clocked at (16MHz / (2^TIMER_PRESCALE)) */
#define TIMER_PRESCALE                  0UL

/* Maximum value is 65535, use prescale if this does not give sufficient period */
#define TIMERx_TICK_PERIOD_COUNT       (CPU_CLOCK_FREQ_HZ / (TICK_PERIOD_ms * 1000))

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vTime_TimerISR(uint32 u32Device, uint32 u32ItemBitmap);

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE uint32 u32TimerTicks;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: vTime_Init
 *
 * DESCRIPTION:
 * Initialises JN5121 tick timer to provide periodic interrupt that is used
 * used to implement a system tick. All timing provided by this module is based
 * on this tick period, defined by TICK_PERIOD_ms above.
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
PUBLIC void vTime_Init(uint8 u8Timer)
{
    u32TimerTicks = 0;

    if (u8Timer == TIME_TICK_TIMER)
    {
        /* Initialise tick timer to give periodic interrupt */
        vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_DISABLE);
        vAHI_TickTimerWrite(0);
        vAHI_TickTimerInit(vTime_TimerISR);
        vAHI_TickTimerInterval(TICK_TIMER_TICK_PERIOD_COUNT);
        vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_RESTART);
        vAHI_TickTimerIntEnable(TRUE);
    }
    else if (u8Timer == TIME_TIMER_0)
    {
        vAHI_TimerEnable(E_AHI_TIMER_0, TIMER_PRESCALE, FALSE, TRUE, FALSE);
        vAHI_TimerClockSelect(E_AHI_TIMER_0, FALSE, TRUE);
        vAHI_Timer0RegisterCallback(vTime_TimerISR);
        vAHI_TimerStartRepeat(E_AHI_TIMER_0, 0, TIMERx_TICK_PERIOD_COUNT);
        vAHI_TimerDIOControl(E_AHI_TIMER_0, FALSE);
    }
    else if (u8Timer == TIME_TIMER_1)
    {
        vAHI_TimerEnable(E_AHI_TIMER_1, TIMER_PRESCALE, FALSE, TRUE, FALSE);
        vAHI_TimerClockSelect(E_AHI_TIMER_1, FALSE, TRUE);
        vAHI_Timer1RegisterCallback(vTime_TimerISR);
        vAHI_TimerStartRepeat(E_AHI_TIMER_1, 0, TIMERx_TICK_PERIOD_COUNT);
        vAHI_TimerDIOControl(E_AHI_TIMER_1, FALSE);
    }
}

/****************************************************************************
 *
 * NAME: u32Time_TimeNowMs
 *
 * DESCRIPTION:
 * Returns the current value of the tick count.
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * uint32 - The current value of the tick count.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC uint32 u32Time_TimeNowMs(void)
{
    return(u32TimerTicks);
}

/****************************************************************************
 *
 * NAME: u16Time_ElapsedTimeSinceMs
 *
 * DESCRIPTION:
 * Provides a time difference (in milliseconds) between the time passed and
 * the current time (value of timer tick parameter).
 *
 * PARAMETERS:      Name            RW  Usage
 * uint16 u16ReferenceTime - Start time for comparision with current time.
 *
 * RETURNS:
 * uint16 - Time difference (in milliseconds).
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC uint16 u16Time_ElapsedTimeSinceMs(uint16 u16ReferenceTime)
{
    uint16 u16ElapsedTime;

    if (u32TimerTicks >= u16ReferenceTime)
    {
        u16ElapsedTime = u32TimerTicks - u16ReferenceTime;
    }
    else
    {
        u16ElapsedTime = (0xFFFFFFFFUL - u16ReferenceTime) + u32TimerTicks;
    }
    return(u16ElapsedTime);
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: vTime_TimerISR
 *
 * DESCRIPTION:
 * Timer interrupt service routine. Increments value of tick count every
 * time it is called.
 *
 * PARAMETERS: Name        RW  Usage
 * uint32 u32Device     - Not used, provided for compatibility with generic
 * uint32 u32ItemBitmap - interrupt handling implemented by the queue API.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PRIVATE void vTime_TimerISR(uint32 u32Device, uint32 u32ItemBitmap)
{
    u32TimerTicks++;
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
