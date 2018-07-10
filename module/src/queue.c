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

#include "mem.h"
#include "intr.h"
#include "queue.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: vQueue_Init
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
PUBLIC void vQueue_Init(tsQueue *psQueue, uint8 *pu8Buffer, uint16 u16Length, uint32 u32ItemSize)
{
	psQueue->pu8Head     = pu8Buffer;
	psQueue->pu8Tail     = psQueue->pu8Head + (u16Length * u32ItemSize);
	psQueue->pu8Write    = pu8Buffer;
	psQueue->pu8Read     = psQueue->pu8Head + ((u16Length - 1) * u32ItemSize);
	psQueue->u16Items    = 0;
	psQueue->u16Length   = u16Length;
	psQueue->u32ItemSize = u32ItemSize;
}

/****************************************************************************
 *
 * NAME: bQueue_AddItem
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
bool_t bQueue_AddItem(tsQueue *psQueue, void *pvItem)
{
    uint32 u32IntState;
    bool_t bResult = FALSE;

    /* Check if there is room available on the queue */
    if(psQueue->u16Items < psQueue->u16Length)
    {
        /* Copy item onto queue */
        memcpy((void *)psQueue->pu8Write, pvItem, psQueue->u32ItemSize);

        MICRO_DISABLE_AND_SAVE_INTERRUPTS(u32IntState);
        {
            /* Record another item has been added */
            psQueue->u16Items++;
        }
        MICRO_RESTORE_INTERRUPTS(u32IntState);

        /* Move write pointer to start of next available free space */
        psQueue->pu8Write += psQueue->u32ItemSize;

        if(psQueue->pu8Write >= psQueue->pu8Tail)
        {
            psQueue->pu8Write = psQueue->pu8Head;
        }
        bResult = TRUE;																						\
    }
	return (bResult);
}

/****************************************************************************
 *
 * NAME: bQueue_RemoveItem
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
PUBLIC bool_t bQueue_RemoveItem(tsQueue *psQueue, void *pvItem)
{
    uint32 u32IntState;
    bool_t bResult = FALSE;

    /* Check if there are any items on the queue */
    if(psQueue->u16Items > 0)
    {
        /* Move read point to next item to remove from queue */
        psQueue->pu8Read += psQueue->u32ItemSize;

        if(psQueue->pu8Read >= psQueue->pu8Tail)
        {
            psQueue->pu8Read = psQueue->pu8Head;
        }

        MICRO_DISABLE_AND_SAVE_INTERRUPTS(u32IntState);
        {
          psQueue->u16Items--;
        }
        MICRO_RESTORE_INTERRUPTS(u32IntState);

        memcpy((void *)pvItem, (void *)psQueue->pu8Read, psQueue->u32ItemSize);

        bResult = TRUE;
    }
    return bResult;
}

/****************************************************************************
 *
 * NAME: bQueue_RemoveItem
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
PUBLIC uint16 u16Queue_ItemsPresent(tsQueue *psQueue)
{
    uint16 u16Result;

    u16Result = psQueue->u16Items;

	return u16Result;
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
