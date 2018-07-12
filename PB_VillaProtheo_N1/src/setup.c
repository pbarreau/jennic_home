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
#include <AppHardwareApi.h>
#include <stdlib.h>
#include <string.h>
#include <Utilities.h>
#include <m_config.h>
#include <serial.h>
#include <xsprintf.h>

#include "setup.h"
#include "spi.h"
#include "RTC.h"

#include "uip_arp.h"

#include "c_config.h"
#include "led.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/* This number is written to the first 4 bytes of flash sector 3 to signify
   that a custom setup is present */
#define FLASH_MAGIC_NUMBER              0x55AA3399UL

/* Default setup values */
#define SETUP_DEFAULT_LOCAL_ADDR        0xC0A80163UL    /* 192.168.1.99 */
#define SETUP_DEFAULT_GATEWAY_ADDR      0xC0A80101UL    /* 192.168.1.1 */
#define SETUP_DEFAULT_SUBNET_MASK       0xFFFFFF00UL    /* 255.255.255.0 */

#define SETUP_DEFAULT_DATE_YEAR         2018            /* 01/01/2000 */
#define SETUP_DEFAULT_DATE_MONTH        01
#define SETUP_DEFAULT_DATE_DAY          01
#define SETUP_DEFAULT_TIME_HOURS        12              /* 12:00:00 */
#define SETUP_DEFAULT_TIME_MINUTES      00
#define SETUP_DEFAULT_TIME_SECONDS      00


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vSetup_ClearScreen(void);
PRIVATE bool_t bSetup_SetDefault(void);
PRIVATE void vSetup_DisplayVersion(uint8 *pu8Version, uint8 *pu8BuildDate, uint8 *pu8BuildTime);
PRIVATE void vSetupDisplayMacc(void);
PRIVATE void vSetup_DisplayOptions(tsSetup *psSetup);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE tsSerialPortSetup sCOM1;
PRIVATE uint8 au8RxBuff[32];
PRIVATE uint8 au8TxBuff[512];
PRIVATE uint8 au8SetupMsg[128];
PUBLIC uint8 u8Reset;

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
PUBLIC void vSetup_Task(uint8 *pu8Version, uint8 *pu8BuildDate, uint8 *pu8BuildTime)
{
  tsSetup sSetup;
  u8Reset = 0;

  /* Initialise serial port for configuration input/output */
  sCOM1.u8Port        = SETUP_PORT;
  sCOM1.u8DataBits    = 8;
  sCOM1.u8Parity      = 0;
  sCOM1.u8StopBits    = 1;
  sCOM1.u32BaudRate   = 19200;//115200;
  sCOM1.pu8RxBuff     = au8RxBuff;
  sCOM1.pu8TxBuff     = au8TxBuff;
  sCOM1.u16RxBuffLen  = sizeof(au8RxBuff);
  sCOM1.u16TxBuffLen  = sizeof(au8TxBuff);

  vSerial_Init(&sCOM1);

  /* Read current configuration so it can be displayed along with options */
  (void)bSetup_Read(&sSetup);

  while(u8Reset == 0)
  {
    /* Clear Screen */
    vSetup_ClearScreen();

    /* Display version information */
    vSetup_DisplayVersion(pu8Version, pu8BuildDate, pu8BuildTime);

    // Montrer addresse mac
    vSetupDisplayMacc();

    /* Display options */
    vSetup_DisplayOptions(&sSetup);
  }
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
PUBLIC bool_t bSetup_Read(tsSetup *psSetup)
{
  /* Check the first byte of flash sector 3, if it is not 0xff then a custom
       configuration must be present */
  if (bAHI_FlashInit(E_FL_CHIP_AUTO, NULL) == TRUE)
  {
    uint32 u32MagicNbr;

    if (bAHI_FullFlashRead(0x30000, sizeof(uint32), (uint8 *)&u32MagicNbr))
    {
      if (u32MagicNbr == FLASH_MAGIC_NUMBER)
      {
        if (bAHI_FullFlashRead(0x30000+sizeof(tsSetup), sizeof(tsSetup), (uint8 *)psSetup))
        {
          /* Configuration present in flash and read successfully */
          return TRUE;
        }
      }
    }
  }
  /* Setup not available in flash, or we failed to read it so just load
       the default values */
  psSetup->sLocalAddr.u32     = SETUP_DEFAULT_LOCAL_ADDR;
  psSetup->sGatewayAddr.u32   = SETUP_DEFAULT_GATEWAY_ADDR;
  psSetup->sSubnetMask.u32    = SETUP_DEFAULT_SUBNET_MASK;
  psSetup->sDate.u16Year      = SETUP_DEFAULT_DATE_YEAR;
  psSetup->sDate.u8Month      = SETUP_DEFAULT_DATE_MONTH;
  psSetup->sDate.u8Day        = SETUP_DEFAULT_DATE_DAY;
  psSetup->sTime.u8Hours      = SETUP_DEFAULT_TIME_HOURS;
  psSetup->sTime.u8Minutes    = SETUP_DEFAULT_TIME_MINUTES;
  psSetup->sTime.u8Seconds    = SETUP_DEFAULT_TIME_SECONDS;

  return FALSE;
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
PUBLIC bool_t bSetup_Write(tsSetup *psSetup)
{
  bool_t bResult = FALSE;

  if (bAHI_FlashInit(E_FL_CHIP_AUTO, NULL) == TRUE)
  {
    /* Flash must be erased before we can write to it */
    if (bAHI_FlashEraseSector(3) == TRUE)
    {
      uint32 u32MagicNbr = FLASH_MAGIC_NUMBER;

      /* Write magic number to signify configuration data is present */
      if (bAHI_FullFlashProgram(0x30000, sizeof(uint32), (uint8 *)&u32MagicNbr))
      {
        /* Now write the configuration data into flash */
        if (bAHI_FullFlashProgram(0x30000+sizeof(tsSetup), sizeof(tsSetup), (uint8 *)psSetup))
        {
          bResult = TRUE;
        }
      }
    }
  }
  return bResult;
}

/****************************************************************************/
/***        Local Functions                                               ***/
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
PRIVATE void vSetupDisplayMacc(void)
{
  /* Display ethernet address */
  xsprintf((char *)au8SetupMsg, "\n\rMAC %02X-%02X-%02X-%02X-%02X-%02X\n\r",
      uip_ethaddr.addr[0],
      uip_ethaddr.addr[1],
      uip_ethaddr.addr[2],
      uip_ethaddr.addr[3],
      uip_ethaddr.addr[4],
      uip_ethaddr.addr[5]
  );
  vSerial_TxString(SETUP_PORT, au8SetupMsg);
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
PRIVATE void vSetup_DisplayOptions(tsSetup *psSetup)
{
  volatile uint32 i;
  bool_t bDone = FALSE;
  uint8 au8StrInpBuff[32];

  /* Display available options and current configuration in [] brackets */
  xsprintf((char *)au8SetupMsg, "\n\r1. Set local IP Address  [%d.%d.%d.%d]",
      psSetup->sLocalAddr.au8[0],
      psSetup->sLocalAddr.au8[1],
      psSetup->sLocalAddr.au8[2],
      psSetup->sLocalAddr.au8[3]);
  vSerial_TxString(SETUP_PORT, au8SetupMsg);

  xsprintf((char *)au8SetupMsg, "\n\r2. Set default gateway   [%d.%d.%d.%d]",
      psSetup->sGatewayAddr.au8[0],
      psSetup->sGatewayAddr.au8[1],
      psSetup->sGatewayAddr.au8[2],
      psSetup->sGatewayAddr.au8[3]);
  vSerial_TxString(SETUP_PORT, au8SetupMsg);

  xsprintf((char *)au8SetupMsg, "\n\r3. Set subnet mask       [%d.%d.%d.%d]",
      psSetup->sSubnetMask.au8[0],
      psSetup->sSubnetMask.au8[1],
      psSetup->sSubnetMask.au8[2],
      psSetup->sSubnetMask.au8[3]);
  vSerial_TxString(SETUP_PORT, au8SetupMsg);

  xsprintf((char *)au8SetupMsg, "\n\r4. Set date              [%02d/%02d/%04d]",
      psSetup->sDate.u8Month,
      psSetup->sDate.u8Day,
      psSetup->sDate.u16Year);
  vSerial_TxString(SETUP_PORT, au8SetupMsg);

  xsprintf((char *)au8SetupMsg, "\n\r5. Set time              [%02d:%02d:%02d]",
      psSetup->sTime.u8Hours,
      psSetup->sTime.u8Minutes,
      psSetup->sTime.u8Seconds);
  vSerial_TxString(SETUP_PORT, au8SetupMsg);

  xsprintf((char *)au8SetupMsg, "\n\r6. Exit  ");
  vSerial_TxString(SETUP_PORT, au8SetupMsg);

  xsprintf((char *)au8SetupMsg, "\n\rR. Restore defaults");
  vSerial_TxString(SETUP_PORT, au8SetupMsg);

  xsprintf((char *)au8SetupMsg, "\n\rS. Save Settings");
  vSerial_TxString(SETUP_PORT, au8SetupMsg);

  xsprintf((char *)au8SetupMsg, "\n\r\n\rEnter option: ");
  vSerial_TxString(SETUP_PORT, au8SetupMsg);

  /* Capture user input */
  while (!bDone)
  {
    int16 i16Char;

    au8Led[C_LID_2].mode = E_FLASH_OFF;
    au8Led[C_LID_3].mode = E_FLASH_ALWAYS;

#ifdef WATCHDOG_ENABLED
    vAHI_WatchdogRestart();
#endif

    if ((i16Char = i16Serial_RxChar(SETUP_PORT)) > 0)
    {
      switch((uint8)i16Char)
      {
        case '1':
          xsprintf((char *)au8SetupMsg, "\n\rEnter local IP address (aaa.bbb.ccc.ddd): ");
          vSerial_TxString(SETUP_PORT, au8SetupMsg);

          if (u32Serial_RxString(SETUP_PORT, au8StrInpBuff, sizeof(au8StrInpBuff)) <= 17)
          {
            psSetup->sLocalAddr.au8[0] = atoi((char *)&au8StrInpBuff[0]);
            psSetup->sLocalAddr.au8[1] = atoi((char *)&au8StrInpBuff[4]);
            psSetup->sLocalAddr.au8[2] = atoi((char *)&au8StrInpBuff[8]);
            psSetup->sLocalAddr.au8[3] = atoi((char *)&au8StrInpBuff[12]);
          }
          bDone = TRUE;
          break;

        case '2':
          xsprintf((char *)au8SetupMsg, "\n\rEnter default gateway address (aaa.bbb.ccc.ddd): ");
          vSerial_TxString(SETUP_PORT, au8SetupMsg);

          if (u32Serial_RxString(SETUP_PORT, au8StrInpBuff, sizeof(au8StrInpBuff)) <= 17)
          {
            psSetup->sGatewayAddr.au8[0] = atoi((char *)&au8StrInpBuff[0]);
            psSetup->sGatewayAddr.au8[1] = atoi((char *)&au8StrInpBuff[4]);
            psSetup->sGatewayAddr.au8[2] = atoi((char *)&au8StrInpBuff[8]);
            psSetup->sGatewayAddr.au8[3] = atoi((char *)&au8StrInpBuff[12]);
          }
          bDone = TRUE;
          break;

        case '3':
          xsprintf((char *)au8SetupMsg, "\n\rEnter subnet mask (aaa.bbb.ccc.ddd): ");
          vSerial_TxString(SETUP_PORT, au8SetupMsg);

          if (u32Serial_RxString(SETUP_PORT, au8StrInpBuff, sizeof(au8StrInpBuff)) <= 17)
          {
            psSetup->sSubnetMask.au8[0] = atoi((char *)&au8StrInpBuff[0]);
            psSetup->sSubnetMask.au8[1] = atoi((char *)&au8StrInpBuff[4]);
            psSetup->sSubnetMask.au8[2] = atoi((char *)&au8StrInpBuff[8]);
            psSetup->sSubnetMask.au8[3] = atoi((char *)&au8StrInpBuff[12]);
          }
          bDone = TRUE;
          break;

        case '4':
          xsprintf((char *)au8SetupMsg, "\n\rEnter date (mm/dd/yyyy): ");
          vSerial_TxString(SETUP_PORT, au8SetupMsg);

          if (u32Serial_RxString(SETUP_PORT, au8StrInpBuff, sizeof(au8StrInpBuff)) <= 12)
          {
            psSetup->sDate.u8Month = atoi((char *)&au8StrInpBuff[0]);
            psSetup->sDate.u8Day   = atoi((char *)&au8StrInpBuff[3]);
            psSetup->sDate.u16Year = atoi((char *)&au8StrInpBuff[6]);

          }
          bDone = TRUE;
          break;

        case '5':
          xsprintf((char *)au8SetupMsg, "\n\rEnter time (hh:mm:ss): ");
          vSerial_TxString(SETUP_PORT, au8SetupMsg);

          if (u32Serial_RxString(SETUP_PORT, au8StrInpBuff, sizeof(au8StrInpBuff)) <= 10)
          {
            psSetup->sTime.u8Hours   = atoi((char *)&au8StrInpBuff[0]);
            psSetup->sTime.u8Minutes = atoi((char *)&au8StrInpBuff[3]);
            psSetup->sTime.u8Seconds = atoi((char *)&au8StrInpBuff[6]);
          }
          bDone = TRUE;
          break;

        case '6':
          /* Reset ENC28J60 */
          vSPI_Write8(0xFF);
          /* Clear Screen */
          vSetup_ClearScreen();
          xsprintf((char *)au8SetupMsg, "Redemarrage Gateway..\n\r");
          vSerial_TxString(SETUP_PORT, au8SetupMsg);
          for (i = 0; i < 75000; i++);
          u8Reset = 1;
          bDone = TRUE;
          break;

        case 'r':
        case 'R':
          xsprintf((char *)au8SetupMsg, "\n\rRestoring Defaults.. ");
          vSerial_TxString(SETUP_PORT, au8SetupMsg);

          if (bSetup_SetDefault())
          {
            (void)bSetup_Read(psSetup);

            xsprintf((char *)au8SetupMsg, "OK");
            vSerial_TxString(SETUP_PORT, au8SetupMsg);
          }
          else
          {
            xsprintf((char *)au8SetupMsg, "ERROR");
            vSerial_TxString(SETUP_PORT, au8SetupMsg);
          }
          /* Short delay so that message is displayed for a few seconds */
          for (i = 0; i < 750000; i++);

          bDone = TRUE;
          break;

        case 's':
        case 'S':
          xsprintf((char *)au8SetupMsg, "\n\rSaving Settings.. ");
          vSerial_TxString(SETUP_PORT, au8SetupMsg);

          if (bSetup_Write(psSetup) == TRUE)
          {
            xsprintf((char *)au8SetupMsg, "OK");
            vSerial_TxString(SETUP_PORT, au8SetupMsg);
          }
          else
          {
            xsprintf((char *)au8SetupMsg, "ERROR");
            vSerial_TxString(SETUP_PORT, au8SetupMsg);
          }
          /* Short delay so that message is displayed for a few seconds */
          for (i = 0; i < 750000; i++);

          bDone = TRUE;
          break;

        default:
          bDone = TRUE;
          break;
      }
    }
  }
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
PRIVATE void vSetup_ClearScreen(void)
{
  xsprintf((char *)au8SetupMsg, "\x1b[2J");
  vSerial_TxString(SETUP_PORT, au8SetupMsg);

  xsprintf((char *)au8SetupMsg, "\x1b[H");
  vSerial_TxString(SETUP_PORT, au8SetupMsg);
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
PRIVATE void vSetup_DisplayVersion(uint8 *pu8Version, uint8 *pu8BuildDate, uint8 *pu8BuildTime)
{
  uint8 u8hPos, u8hLen;

  /* Work out the length of the string to be displayed */
  u8hLen = xsprintf((char *)au8SetupMsg, "%s Build %s %s", pu8Version, pu8BuildDate, pu8BuildTime);
  u8hLen += 2; /* Add box sides */

  /* Draw top of box for menu screen header */
  xsprintf((char *)au8SetupMsg, "\xc9");
  vSerial_TxString(SETUP_PORT, au8SetupMsg);

  for (u8hPos = 0; u8hPos < u8hLen; u8hPos++)
  {
    xsprintf((char *)au8SetupMsg, "\xcd");
    vSerial_TxString(SETUP_PORT, au8SetupMsg);
  }
  xsprintf((char *)au8SetupMsg, "\xbb");
  vSerial_TxString(SETUP_PORT, au8SetupMsg);

  xsprintf((char *)au8SetupMsg, "\n\r\xba %s Build %s %s \xba", pu8Version, pu8BuildDate, pu8BuildTime);
  vSerial_TxString(SETUP_PORT, au8SetupMsg);

  /* Draw bottom of box for menu screen */
  xsprintf((char *)au8SetupMsg, "\n\r\xc8");
  vSerial_TxString(SETUP_PORT, au8SetupMsg);

  for (u8hPos = 0; u8hPos < u8hLen; u8hPos++)
  {
    xsprintf((char *)au8SetupMsg, "\xcd");
    vSerial_TxString(SETUP_PORT, au8SetupMsg);
  }
  xsprintf((char *)au8SetupMsg, "\xbc");
  vSerial_TxString(SETUP_PORT, au8SetupMsg);

  xsprintf((char *)au8SetupMsg, "\n");
  vSerial_TxString(SETUP_PORT, au8SetupMsg);
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
PRIVATE bool_t bSetup_SetDefault(void)
{
  bool_t bResult = FALSE;

  /* Setting the config to default values simply involves erasing flash
       sector 3. The next time we read the config none will be found in flash
       and the default values will be used */
  if (bAHI_FlashInit(E_FL_CHIP_AUTO, NULL) == TRUE)
  {
    /* Flash must be erased before we can write to it */
    if (bAHI_FlashEraseSector(3) == TRUE)
    {
      bResult = TRUE;
    }
  }
  return bResult;
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
