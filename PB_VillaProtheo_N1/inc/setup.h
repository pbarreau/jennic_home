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
#ifndef  SETUP_H_INCLUDED
#define  SETUP_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include "ip.h"
//#include "uip-conf.h"
#include <rtc.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct
{
  //u16_t usePort;
  uint16 usePort;
  tsTime   sTime;
  tsDate   sDate;
  tsIPAddr sLocalAddr;
  tsIPAddr sGatewayAddr;
  tsIPAddr sSubnetMask;
}tsSetup;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC bool_t bSetup_Read(tsSetup *psSetup);
PUBLIC bool_t bSetup_Write(tsSetup *psSetup);
PUBLIC void vSetup_Task(uint8 *pu8Version, uint8 *pu8BuildDate, uint8 *pu8BuildTime);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* SETUP_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
