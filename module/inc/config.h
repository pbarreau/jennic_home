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

#ifndef  CONFIG_H_INCLUDED
#define  CONFIG_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include "services.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/* Network parameters - these MUST be changed to suit the target application */
//#define PAN_ID                  0x1234U
//#define CHANNEL                 12
//#define POLL_PERIOD             10 /* in 10ths of a second */

/* UTILS config */
#define UTILS_UART              E_AHI_UART_0
#define UTILS_UART_BAUD_RATE    E_AHI_UART_RATE_115200

/* Specify which serial port to use when displaying setup menu (tag only) */
#define SETUP_PORT                      E_AHI_UART_0
/* Specify which serial port to use when outputting debug information */
#define DEBUG_PORT                      E_AHI_UART_1
/* Specifiy size of routing table in coordinator and routers */
#define ROUTING_TABLE_SIZE              32
/* Define the maximum ping period at which a sensor can operate */
#define SENSOR_MAX_PING_PERIOD_ms       30000UL
/* Define the maximum ping period at which a sensor can operate */
#define SENSOR_MIN_PING_PERIOD_ms       100
/* Define how often we check the sensors to see if they are srill sending data */
#define SENSOR_TIMEOUT_POLL_PERIOD_ms   10000
/* Define maximum time between received frame before we mark sensor as failed */
#define SENSOR_TX_TIMEOUT_ms            61000

/* Specify the default radio channel to be used by tags, readers and gateway */
#define CHANNEL_SCAN_MASK               (1UL << 26)
/* Specify the PAN ID to be used by tags, readers and gateway */
#define PAN_ID                          0x87EA
/* Specify the network application ID, used to uniquely identify the network */
#define NETWORK_ID                      0x012ED5B3UL

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* CONFIG_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
