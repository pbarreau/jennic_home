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
#include <JPI.h>

#include "led.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/* Define which DIO are connected to the bi-color LED */
#define DIO_LED_ANODE_GREEN         5UL
#define DIO_LED_ANODE_RED           1UL
#define DIO_MASK_LED_ANODE_GREEN    (1UL << DIO_LED_ANODE_GREEN)
#define DIO_MASK_LED_ANODE_RED      (1UL << DIO_LED_ANODE_RED)

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vLED_FlashISR(uint32 u32DeviceId, uint32 u32ItemBitmap);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE bool_t bLedOn;
PRIVATE teLedColor eLedColor;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: vLED_Init
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
PUBLIC void vLED_Init(void)
{
    /* Set DIO's connected to bi-color LED as outputs and drive them low to
       turn LED off */
    vJPI_DioSetDirection(0, (DIO_MASK_LED_ANODE_GREEN + DIO_MASK_LED_ANODE_RED));
    vJPI_DioSetOutput(0, (DIO_MASK_LED_ANODE_GREEN + DIO_MASK_LED_ANODE_RED));

    /* Set up timer that will be used to flash the LED, set timer clock to
       approximately 1KHz so we can set flash period in milliseconds */
    vJPI_TimerEnable(E_JPI_TIMER_0,
                     14,
                     E_JPI_TIMER_INT_PERIOD,
                     FALSE, FALSE,
                     E_JPI_TIMER_CLOCK_INTERNAL_NORMAL);
    /* Register the function that will flash the LED */
    vJPI_Timer0RegisterCallback(vLED_FlashISR);
}

/****************************************************************************
 *
 * NAME: vLED_On
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
PUBLIC void vLED_On(teLedColor eColor)
{
    /* Stop LED flash timer, just in case it is running */
    vJPI_TimerStop(E_JPI_TIMER_0);

    switch (eColor)
    {
        case E_LED_COLOR_RED:
            /* Turn on red LED by setting red anode high and green anode low */
            vJPI_DioSetOutput(DIO_MASK_LED_ANODE_RED, DIO_MASK_LED_ANODE_GREEN);
            break;

        case E_LED_COLOR_AMBER:
            /* Turn on amber LED by turning on both red and green at the same time */
            vJPI_DioSetOutput((DIO_MASK_LED_ANODE_GREEN + DIO_MASK_LED_ANODE_RED), 0);
            break;

        case E_LED_COLOR_GREEN:
            /* Turn on green LED by setting green anode high and red anode low */
            vJPI_DioSetOutput(DIO_MASK_LED_ANODE_GREEN, DIO_MASK_LED_ANODE_RED);
            break;

        default:
            /* Unrecognised color, turn LED off */
            vJPI_DioSetOutput(0, (DIO_MASK_LED_ANODE_GREEN + DIO_MASK_LED_ANODE_RED));
            break;
    }
}

/****************************************************************************
 *
 * NAME: vLED_Flash
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
PUBLIC void vLED_Flash(teLedColor eColor, uint16 u16Periodms)
{
    /* Store the color required for use by the ISR when the flash timer fires */
    eLedColor = eColor;

    /* Start the flashing timer with the required period */
    vJPI_TimerStart(E_JPI_TIMER_0, E_JPI_TIMER_MODE_REPEATING, 0, u16Periodms);
}

/****************************************************************************
 *
 * NAME: vLED_Off
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
PUBLIC void vLED_Off(void)
{
    /* Stop LED flash timer, just in case it is running */
    vJPI_TimerStop(E_JPI_TIMER_0);

    /* Turn off LED by setting both anode connections low */
    vJPI_DioSetOutput(0, (DIO_MASK_LED_ANODE_GREEN + DIO_MASK_LED_ANODE_RED));
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: vLED_FlashISR
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
PRIVATE void vLED_FlashISR(uint32 u32DeviceId, uint32 u32ItemBitmap)
{
    /* Toggle the state of the LED */
    if (bLedOn)
    {
        /* Turn off LED by setting both anode connections low */
        vJPI_DioSetOutput(0, (DIO_MASK_LED_ANODE_GREEN + DIO_MASK_LED_ANODE_RED));
    }
    else
    {
        switch (eLedColor)
        {
            case E_LED_COLOR_RED:
                /* Turn on red LED by setting red anode high and green anode low */
                vJPI_DioSetOutput(DIO_MASK_LED_ANODE_RED, DIO_MASK_LED_ANODE_GREEN);
                break;

            case E_LED_COLOR_AMBER:
                /* Turn on amber LED by turning on both red and green at the same time */
                vJPI_DioSetOutput((DIO_MASK_LED_ANODE_GREEN + DIO_MASK_LED_ANODE_RED), 0);
                break;

            case E_LED_COLOR_GREEN:
                /* Turn on green LED by setting green anode high and red anode low */
                vJPI_DioSetOutput(DIO_MASK_LED_ANODE_GREEN, DIO_MASK_LED_ANODE_RED);
                break;

            default:
                /* Unrecognised color, turn LED off */
                vJPI_DioSetOutput(0, (DIO_MASK_LED_ANODE_GREEN + DIO_MASK_LED_ANODE_RED));
                break;
        }
    }
    bLedOn = !bLedOn;
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
