/*****************************************************************************
 *
 * MODULE:      Utilities
 *
 * COMPONENT:   Printf.h
 *
 * AUTHOR:      LJM
 *
 * DESCRIPTION: Printf header file
 *
 * $HeadURL: http://svn/sware/Projects/Jeneric/Modules/Utilities/Tags/JENERIC_UTILITIES_1v1_RC2/Include/Printf.h $
 *
 * $Revision: 14257 $
 *
 * $LastChangedBy: jahme $
 *
 * $LastChangedDate: 2009-06-12 14:23:59 +0100 (Fri, 12 Jun 2009) $
 *
 * $Id: Printf.h 14257 2009-06-12 13:23:59Z jahme $
 *
 *****************************************************************************
 *
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
 * Copyright Jennic Ltd. 2009 All rights reserved
 *
 ****************************************************************************/

#ifndef PRINTF_H_INCLUDED
#define PRINTF_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <jendefs.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

PUBLIC void vUART_printInit(void);

PUBLIC void vInitPrintf(void (*fp)(char c));
PUBLIC void vPrintf(const char *fmt, ...);

PUBLIC void vPutC(unsigned char c);
PUBLIC void vUART_Init(bool bWaitForKey);

#if defined __cplusplus
}
#endif

#endif /* PRINTF_H_INCLUDED */


