/**
 * \addtogroup httpd
 * @{
 */

/**
 * \file
 *         Web server script interface
 * \author
 *         Adam Dunkels <adam@sics.se>
 *
 */

/*
 * Copyright (c) 2001-2006, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: httpd-cgi.c,v 1.2 2006/06/11 21:46:37 adam Exp $
 *
 */

#include "uip.h"
#include "psock.h"
#include "httpd.h"
#include "httpd-cgi.h"
#include "httpd-fs.h"

#include <stdio.h>
#include <string.h>
#include <jenie.h>

#include <sensordata.h>

HTTPD_CGI_CALL(floorone, "floor-one", f1_temps);
HTTPD_CGI_CALL(floortwo, "floor-two", f2_temps);

static uint8 u8UpdateTableEntry(uint8 u8TableIndex, uint16 u16LineIndex);

extern tsSensorList sSensorList;

static const struct httpd_cgi_call *calls[] = { &floorone, &floortwo, NULL };

/*---------------------------------------------------------------------------*/
static PT_THREAD(nullfunction(struct httpd_state *s, char *ptr))
{
    PSOCK_BEGIN(&s->sout);
    PSOCK_END(&s->sout);
}

/*---------------------------------------------------------------------------*/
httpd_cgifunction httpd_cgi(char *name)
{
    const struct httpd_cgi_call **f;

    /* Find the matching name in the table, return the function. */
    for(f = calls; *f != NULL; ++f)
    {
        if(strncmp((*f)->name, name, strlen((*f)->name)) == 0)
        {
            return (*f)->function;
        }
    }
    return nullfunction;
}

/*---------------------------------------------------------------------------*/
static unsigned short generate_floor1_temps(void *arg)
{
    uint8 i;
    uint16 u16LineLen = 0;

    for (i = 0; i < MAX_SENSORS; i++)
    {
        if (sSensorList.asSensor[i].u8Floor == 1)
        {
            u16LineLen += u8UpdateTableEntry(i, u16LineLen);
        }
    }
    /* If no sensors present */
    if (u16LineLen == 0)
    {
        u16LineLen = snprintf((char *)uip_appdata, UIP_APPDATA_SIZE, "<TR><TD COLSPAN=7 ALIGN=CENTER>No Sensors</TD></TR>");
    }
    return u16LineLen;
}

/*---------------------------------------------------------------------------*/
static unsigned short generate_floor2_temps(void *arg)
{
    uint8 i;
    uint16 u16LineLen = 0;

    for (i = 0; i < MAX_SENSORS; i++)
    {
        if (sSensorList.asSensor[i].u8Floor == 2)
        {
            u16LineLen += u8UpdateTableEntry(i, u16LineLen);
        }
    }
    /* If no sensors present */
    if (u16LineLen == 0)
    {
        u16LineLen = snprintf((char *)uip_appdata, UIP_APPDATA_SIZE, "<TR><TD COLSPAN=7 ALIGN=CENTER>No Sensors</TD></TR>");
    }
    return u16LineLen;
}

/*---------------------------------------------------------------------------*/
static uint8 u8UpdateTableEntry(uint8 u8TableIndex, uint16 u16LineIndex)
{
    uint8 u8LineLen = 0;

    u8LineLen += snprintf((char *)&uip_appdata[u16LineIndex + u8LineLen], UIP_APPDATA_SIZE,
                           "<TR><TD>%s</TD>",
                           sSensorList.asSensor[u8TableIndex].au8Name);

    u8LineLen += snprintf((char *)&uip_appdata[u16LineIndex + u8LineLen], UIP_APPDATA_SIZE,
                           "<TD>0x%08X%08X</TD>",
                           sSensorList.asSensor[u8TableIndex].u64Addr,
                           sSensorList.asSensor[u8TableIndex].u64Addr >> 32);

    u8LineLen += snprintf((char *)&uip_appdata[u16LineIndex + u8LineLen], UIP_APPDATA_SIZE,
                          "<TD>%dF</TD>",
                          sSensorList.asSensor[u8TableIndex].u8TemperatureF);

    if (sSensorList.asSensor[u8TableIndex].u8Type == E_JENIE_ROUTER)
    {
        u8LineLen += snprintf((char *)&uip_appdata[u16LineIndex + u8LineLen], UIP_APPDATA_SIZE,
                              "<TD>AC</TD>");
    }
    else
    {
        u8LineLen += snprintf((char *)&uip_appdata[u16LineIndex + u8LineLen], UIP_APPDATA_SIZE,
                              "<TD>%dmV</TD>",
                              sSensorList.asSensor[u8TableIndex].u16SupplyVoltage);
    }

    if (sSensorList.asSensor[u8TableIndex].u8Status == STATUS_OK)
    {
        u8LineLen += snprintf((char *)&uip_appdata[u16LineIndex + u8LineLen], UIP_APPDATA_SIZE,
                              "<TD>OK</TD>");
    }
    else
    {
        u8LineLen += snprintf((char *)&uip_appdata[u16LineIndex + u8LineLen], UIP_APPDATA_SIZE,
                              "<TD>FAILED</TD>");
    }

    u8LineLen += snprintf((char *)&uip_appdata[u16LineIndex + u8LineLen], UIP_APPDATA_SIZE,
                          "<TD>%d</TD>", sSensorList.asSensor[u8TableIndex].u32NwkReJoinAttempts);

    u8LineLen += snprintf((char *)&uip_appdata[u16LineIndex + u8LineLen], UIP_APPDATA_SIZE,
                           "<TD>%02d/%02d/%04d - %02d:%02d:%02d</TD></TR>",
                           sSensorList.asSensor[u8TableIndex].sDate.u8Month,
                           sSensorList.asSensor[u8TableIndex].sDate.u8Day,
                           sSensorList.asSensor[u8TableIndex].sDate.u16Year,
                           sSensorList.asSensor[u8TableIndex].sTime.u8Hours,
                           sSensorList.asSensor[u8TableIndex].sTime.u8Minutes,
                           sSensorList.asSensor[u8TableIndex].sTime.u8Seconds);
    return u8LineLen;
}

/*---------------------------------------------------------------------------*/
static PT_THREAD(f1_temps(struct httpd_state *s, char *ptr))
{
    PSOCK_BEGIN(&s->sout);

    PSOCK_GENERATOR_SEND(&s->sout, generate_floor1_temps, s);

    PSOCK_END(&s->sout);
}

/*---------------------------------------------------------------------------*/
static PT_THREAD(f2_temps(struct httpd_state *s, char *ptr))
{
    PSOCK_BEGIN(&s->sout);

    PSOCK_GENERATOR_SEND(&s->sout, generate_floor2_temps, s);

    PSOCK_END(&s->sout);
}

/*---------------------------------------------------------------------------*/
/** @} */
