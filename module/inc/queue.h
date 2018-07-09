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
#ifndef  QUEUE_H_INCLUDED
#define  QUEUE_H_INCLUDED

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

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct
{
	uint8  *pu8Head;    /* Pointer to start of queue storage area */
	uint8  *pu8Tail;    /* Pointer to end of queue storage area */
	uint8  *pu8Write;   /* Pointer to the free next place in the storage area */
	uint8  *pu8Read;    /* Pointer to the last place that a queued item was read from */
	uint16 u16Items;    /* Number of items current held by queue */
	uint16 u16Length;   /* Maximum number of items the queue can hold */
	uint32 u32ItemSize; /* Size (in bytes) of each item to be stored */
}tsQueue;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC uint16 u16Queue_ItemsPresent(tsQueue *psQueue);
PUBLIC bool_t bQueue_AddItem(tsQueue *psQueue, void *pvItem);
PUBLIC bool_t bQueue_RemoveItem(tsQueue *psQueue, void *pvItem);
PUBLIC void vQueue_Init(tsQueue *psQueue, uint8 *pu8Buffer, uint16 u16Length, uint32 u32ItemSize);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* QUEUE_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
