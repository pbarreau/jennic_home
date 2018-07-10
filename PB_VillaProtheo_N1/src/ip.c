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

/* uIP includes */
#include <uip.h>
#include <uip_arp.h>
#include <timer.h>

/* Project includes */
#include "ip.h"
#include "network-device.h"
#include <m_config.h>

#ifdef SERIAL_DEBUG
    #include <serial.h>
    #include <xsprintf.h>
#endif

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define BUF     ((struct uip_eth_hdr *)&uip_buf[0])

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
PRIVATE uip_ipaddr_t ipaddr;
PRIVATE struct timer periodic_timer, arp_timer;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * RETURNS:
 *
 ****************************************************************************/
PUBLIC void vIP_Init(tsIPAddr *psLocalAddr, tsIPAddr *psGatewayAddr, tsIPAddr *psSubnetMask)
{
    clock_init();
    timer_set(&periodic_timer, CLOCK_SECOND / 2);
    timer_set(&arp_timer, CLOCK_SECOND * 10);

    network_device_init();
    uip_init();

    /* set up our own IP address */
    uip_ipaddr(ipaddr, psLocalAddr->au8[0], psLocalAddr->au8[1], psLocalAddr->au8[2], psLocalAddr->au8[3]);
    uip_sethostaddr(ipaddr);

    /* Set subnet mask */
    uip_ipaddr(ipaddr, psSubnetMask->au8[0], psSubnetMask->au8[1], psSubnetMask->au8[2], psSubnetMask->au8[3]);
    uip_setnetmask(ipaddr);

    /* Set the default gateways address */
    uip_ipaddr(ipaddr, psGatewayAddr->au8[0], psGatewayAddr->au8[1], psGatewayAddr->au8[2], psGatewayAddr->au8[3]);
    uip_setdraddr(ipaddr);

    httpd_init();
}

/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * RETURNS:
 *
 ****************************************************************************/
PUBLIC void vIP_Poll(void)
{
    uint8 i;

    uip_len = network_device_read();

    if(uip_len > 0)
    {
        if(BUF->type == htons(UIP_ETHTYPE_IP))
        {
            uip_arp_ipin();
            uip_input();

            /* If the above function invocation resulted in data that
               should be sent out on the network, the global variable
               uip_len is set to a value > 0. */
            if(uip_len > 0)
            {
                uip_arp_out();
                network_device_send();
            }
        }
        else if(BUF->type == htons(UIP_ETHTYPE_ARP))
        {
            uip_arp_arpin();

            /* If the above function invocation resulted in data that
               should be sent out on the network, the global variable
               uip_len is set to a value > 0. */
            if(uip_len > 0)
            {
                network_device_send();
            }
        }
    }
    else if(timer_expired(&periodic_timer))
    {
        timer_reset(&periodic_timer);

        for(i = 0; i < UIP_CONNS; i++)
        {
            uip_periodic(i);

            /* If the above function invocation resulted in data that
               should be sent out on the network, the global variable
               uip_len is set to a value > 0. */
            if(uip_len > 0)
            {
                uip_arp_out();
                network_device_send();
            }
        }

        /* Call the ARP timer function every 10 seconds. */
        if(timer_expired(&arp_timer))
        {
            timer_reset(&arp_timer);
            uip_arp_timer();
        }
    }
}

#ifdef SERIAL_DEBUG
/****************************************************************************
 *
 * NAME: uip_log
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
PUBLIC void uip_log(char *msg)
{

    vSerial_TxString(DEBUG_PORT, (uint8 *)msg);
}
#endif

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
