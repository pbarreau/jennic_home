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
#ifndef  SENSOR_DATA_H_INCLUDED
#define  SENSOR_DATA_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include <AppHardwareApi.h>

#include <rtc.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define MAX_NAME_LEN    16
#define MAX_SENSORS     8

#define STATUS_OK       0
#define STATUS_FAILED   1

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct
{
    uint8  u8Type;
    uint64 u64Addr;
    uint8  au8Name[MAX_NAME_LEN];   /* Sensor name (ASCII string) */
    uint8  u8Floor;                 /* Floor number (0-9) */
    uint8  u8TemperatureF;          /* Temperature degrees F */
    uint16 u16SupplyVoltage;        /* Supply voltage millivolts */
    uint8  u8Status;
    uint32 u32LastRxTime;
    uint32 u32NwkReJoinAttempts;
    tsDate sDate;                   /* Time and date of last update */
    tsTime sTime;
}tsSensorData;

typedef struct
{
    uint8           u8NbrSensors;
    tsSensorData    asSensor[MAX_SENSORS];
}tsSensorList;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* SENSOR_DATA_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
