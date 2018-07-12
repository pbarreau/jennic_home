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

#include "rtc.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/* Clock frequency of RTC oscillator */
#define RTC_CLOCK_FREQ_Hz           32768
#define RTC_CLOCK_TICKS_PER_SECOND  RTC_CLOCK_FREQ_Hz

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE uint32 u32RTC_CalcElapsedTime(void);
PRIVATE bool_t bRTC_IsLeapYear(uint16 u16Year);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE tsRTC sRTC;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/**
 *
 * @param
 * @return
 *
 */
PUBLIC void vRTC_Init(uint8 u8WakeTimer, uint8 u8ClockSource)
{
	uint8 status;

    /* Set default local time and date */
    sRTC.u8Hours       = 12;
    sRTC.u8Minutes     = 0;
    sRTC.u32ClockTicks = 0;
    sRTC.u16Year       = 2000;
    sRTC.u8Month       = 1;
    sRTC.u8Day         = 1;

    if (u8ClockSource == RTC_CLK_SRC_WKT_EXT)
    {
        /* Use external oscillator */
        //vAHI_ExternalClockEnable(TRUE);
        bAHI_Set32KhzClockMode(E_AHI_XTAL);
    }

    if (u8WakeTimer == RTC_WAKE_TIMER_0)
    {
    	/* Enable wake timer used by RTC */
    	vAHI_WakeTimerEnable(E_AHI_WAKE_TIMER_0, FALSE);
	    /* Store details of wake timer to be used for tracking time/date */
	    sRTC.u8WakeTimer = E_AHI_WAKE_TIMER_0;
	}
	else if (u8WakeTimer == RTC_WAKE_TIMER_1)
	{
    	/* Enable wake timer used by RTC */
    	vAHI_WakeTimerEnable(E_AHI_WAKE_TIMER_1, FALSE);
	    /* Store details of wake timer to be used for tracking time/date */
	    sRTC.u8WakeTimer = E_AHI_WAKE_TIMER_1;
	}
}

/**
 *
 * @param
 * @return
 *
 */
PUBLIC void vRTC_Start(void)
{
    if ((sRTC.u8WakeTimer == E_AHI_WAKE_TIMER_0) || (sRTC.u8WakeTimer == E_AHI_WAKE_TIMER_1))
    {
    	/* Start the wake timer that will be used to track time between tick points */
   	 	vAHI_WakeTimerStart(sRTC.u8WakeTimer, 0xFFFFFFFFUL);
 	}
    sRTC.u32LastCount = 0xFFFFFFFFUL;
}

/**
 *
 * @param
 * @return
 *
 */
PUBLIC void vRTC_Update(void)
{
    vRTC_Tick(u32RTC_CalcElapsedTime());
}

/**
 *
 * @param
 * @return
 *
 */
PUBLIC void vRTC_SetTime(tsTime *psTime)
{
    sRTC.u8Hours       = psTime->u8Hours;
    sRTC.u8Minutes     = psTime->u8Minutes;
    sRTC.u8Seconds     = psTime->u8Seconds;
    sRTC.u32ClockTicks = 0;
}

/**
 *
 * @param
 * @return
 *
 */
PUBLIC void vRTC_GetTime(tsTime *psTime)
{
    psTime->u8Hours   = sRTC.u8Hours;
    psTime->u8Minutes = sRTC.u8Minutes;
    psTime->u8Seconds = sRTC.u8Seconds;
}

/**
 *
 * @param
 * @return
 *
 */
PUBLIC void vRTC_SetDate(tsDate *psDate)
{
    sRTC.u16Year = psDate->u16Year;
    sRTC.u8Month = psDate->u8Month;
    sRTC.u8Day   = psDate->u8Day;
}

/**
 *
 * @param
 * @return
 *
 */
PUBLIC void vRTC_GetDate(tsDate *psDate)
{
    psDate->u16Year = sRTC.u16Year;
    psDate->u8Month = sRTC.u8Month;
    psDate->u8Day   = sRTC.u8Day;
}

/**
 *
 * @param
 * @return
 *
 */
PUBLIC void vRTC_Tick(uint32 u32ElapsedTicks)
{
    if ((sRTC.u32ClockTicks + u32ElapsedTicks) >= RTC_CLOCK_TICKS_PER_SECOND)
    {
        /* More than one second has elapsed since we last updated the RTC so
           update second counter */
        sRTC.u8Seconds += ((sRTC.u32ClockTicks + u32ElapsedTicks) / RTC_CLOCK_TICKS_PER_SECOND);
        sRTC.u32ClockTicks = (sRTC.u32ClockTicks + u32ElapsedTicks) % RTC_CLOCK_TICKS_PER_SECOND;

        if (sRTC.u8Seconds > 59)
        {
            sRTC.u8Seconds -= 60;

            /* Second count has rolled over so update minute counter */
            if (++sRTC.u8Minutes == 60)
            {
                sRTC.u8Minutes = 0;

                if (++sRTC.u8Hours == 24)
                {
                    sRTC.u8Hours = 0;

                    if (++sRTC.u8Day == 32)
                    {
                        sRTC.u8Month++;
                        sRTC.u8Day = 1;
                    }
                    else if (sRTC.u8Day == 31)
                    {
                        if ((sRTC.u8Month == 4) || (sRTC.u8Month == 6) || (sRTC.u8Month == 9) || (sRTC.u8Month == 11))
                        {
                            sRTC.u8Month++;
                            sRTC.u8Day = 1;
                        }
                    }
                    else if (sRTC.u8Day == 30)
                    {
                        if(sRTC.u8Month == 2)
                        {
                            sRTC.u8Month++;
                            sRTC.u8Day = 1;
                        }
                    }
                    else if (sRTC.u8Day == 29)
                    {
                        if((sRTC.u8Month == 2) && (!bRTC_IsLeapYear(sRTC.u16Year)))
                        {
                            sRTC.u8Month++;
                            sRTC.u8Day = 1;
                        }
                    }
                    if (sRTC.u8Month == 13)
                    {
                        sRTC.u8Month = 1;
                        sRTC.u16Year++;
                    }
                }
            }
        }
    }
    else
    {
        sRTC.u32ClockTicks += u32ElapsedTicks;
    }
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/**
 *
 * @param
 * @return
 *
 */
PRIVATE uint32 u32RTC_CalcElapsedTime(void)
{
    uint32 u32ElapsedTime;
    uint32 u32WakeTimerCount;

    /* Read current value of free running timer */
    u32WakeTimerCount = u32AHI_WakeTimerRead(sRTC.u8WakeTimer);

    /* Calculate how much time has elapsed since the last time the RTC was updated */
    if (u32WakeTimerCount < sRTC.u32LastCount)
    {
        u32ElapsedTime = sRTC.u32LastCount - u32WakeTimerCount;
    }
    else
    {
        /* Wake timer has rolled over since we last checked */
        u32ElapsedTime = u32WakeTimerCount + (0xFFFFFFFFUL - sRTC.u32LastCount);
    }
    sRTC.u32LastCount = u32WakeTimerCount;

    return (u32ElapsedTime);
}

/**
 *
 * @param
 * @return
 *
 */
PRIVATE bool_t bRTC_IsLeapYear(uint16 u16Year)
{
    bool_t bResult = FALSE;

    if (!(u16Year % 100))
    {
        if ((u16Year % 400) != 0)
        {
            bResult = TRUE;
        }
    }
    else
    {
        if ((u16Year % 4) != 0)
        {
            bResult = TRUE;
        }
    }
    return (bResult);
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
