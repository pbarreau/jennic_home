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
#include <AppApi.h>
#include <AppHardwareApi.h>

#include "dhcpc.h"
#include <uip.h>
#include <uip_arp.h>

#include "ENC28J60.h"
#include "network-device.h"

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
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME:
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
PUBLIC void network_device_init(void)
{
    struct uip_eth_addr ethaddr;

    /* Microchip OUI is 0x0004A3, this is used as the first 3 bytes of the
       Ethernet MAC address. The final 3 bytes are made up from the bottom
       3 bytes of the 64-bit MAC address of the Jennic module. This should
       give a fairly unique Ethernet MAC address.... */
    ethaddr.addr[0] = 0x00;
    ethaddr.addr[1] = 0x04;
    ethaddr.addr[2] = 0xA3;
    ethaddr.addr[3] = (uint8)(*((uint32 *)pvAppApiGetMacAddrLocation() + 1) >> 16);
    ethaddr.addr[4] = (uint8)(*((uint32 *)pvAppApiGetMacAddrLocation() + 1) >> 8);
    ethaddr.addr[5] = (uint8)(*((uint32 *)pvAppApiGetMacAddrLocation() + 1));

    uip_setethaddr(ethaddr);

    (void)bENC28J60_Init(&ethaddr.addr[0]);
}

/****************************************************************************
 *
 * NAME:
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
PUBLIC uint16 network_device_read(void)
{
    uint16 u16PktLen = 0;

    if (u8ENC28J60_RxPacketCount() > 0)
    {
        u16PktLen = u16ENC28J60_Receive(uip_buf);
    }
    return u16PktLen;
}

/****************************************************************************
 *
 * NAME:
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
PUBLIC void network_device_send(void)
{
    vENC28J60_Transmit(uip_buf, uip_len);
}


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
