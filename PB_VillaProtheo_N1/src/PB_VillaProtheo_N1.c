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

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include <string.h>
#include <Jenie.h>
#include <JPI.h>
#include <JenNetApi.h>
#include "Utils.h"
#include "config.h"

#include <Utilities.h>

/* Project includes */
#include "ip.h"
#include "time.h"
#include "setup.h"
#include <rtc.h>
#include <config.h>
#include <sensordata.h>
#include "serial.h"


/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/* Address of ROM based un-aligment access exception handler */
#define UNALIGNED_ACCESS *((volatile uint32 *)(0x4000008))

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

typedef enum
{
    E_STATE_IDLE,
    E_STATE_NWK_STARTED
}teAppState;

typedef enum
{
    E_EVENT_NWK_STARTED,
    E_EVENT_DATA_RECEIVED,
    E_EVENT_POLL,
}teAppEvent;

typedef struct
{
    uint64          u64Addr;
    tsSensorData    sData;
}tsSensor;

typedef struct
{
    uint16          u16TickCounts;
    tsSetup         sSetup;
    teAppState      eAppState;
    uint32          u32LastSensorPollTime;
}tsAppData;
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vSetState(teAppState eNewState);
PRIVATE void vProcessEvent(teAppEvent eEvent, void *pvParam);
PUBLIC void vUnalignedAccessHandler (void);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern void unaligned_access_handler(uint32 u32HeapAddr, uint32 u32Vector);
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE tsAppData sAppData;
PUBLIC  tsSensorList sSensorList;
PRIVATE tsJenieRoutingTable asRoutingTable[ROUTING_TABLE_SIZE];

/* Version/build information. */
PRIVATE uint8 au8Version[] __attribute__ ((used))   = "Gateway Monitor (Coordinator) - v1.0.0";
PRIVATE uint8 au8BuildDate[] __attribute__ ((used)) = __DATE__;
PRIVATE uint8 au8BuildTime[] __attribute__ ((used)) = __TIME__;


/****************************************************************************
 *
 * NAME: gJenie_CbNetworkApplicationID
 *
 * DESCRIPTION:
 * Entry point for application from boot loader.
 * CALLED AT COLDSTART ONLY
 * Allows application to initialises network paramters before stack starts.
 *
 * RETURNS:
 * Nothing
 *
 ****************************************************************************/
PUBLIC void vJenie_CbConfigureNetwork(void)
{
  /* Install ROM based unaligned access handler. This is a patch to fix some
     alignment issues with the uIP stack */
  UNALIGNED_ACCESS = (uint32) vUnalignedAccessHandler;

  /* Configure stack parameters */
  gJenie_PanID                            = PAN_ID;
  gJenie_NetworkApplicationID             = NETWORK_ID;
  gJenie_Channel                          = 0;
  gJenie_ScanChannels                     = CHANNEL_SCAN_MASK;
  gJenie_MaxChildren                      = 16;
  gJenie_MaxSleepingChildren              = 8;
  gJenie_MaxBcastTTL                      = 5;
  gJenie_MaxFailedPkts                    = 3;
  gJenie_RoutingEnabled                   = TRUE;
  gJenie_RoutingTableSize                 = ROUTING_TABLE_SIZE;
  gJenie_RoutingTableSpace                = (void *)asRoutingTable;
  gJenie_RouterPingPeriod                 = ((SENSOR_MAX_PING_PERIOD_ms / 2UL) / 100UL); /* Units of 100ms */
  gJenie_EndDeviceChildActivityTimeout    = ((SENSOR_MAX_PING_PERIOD_ms * 3UL) / 100UL); /* Units of 100ms */
  gJenie_RecoverFromJpdm                  = FALSE;
  gJenie_RecoverChildrenFromJpdm          = FALSE;

}

/****************************************************************************
 *
 * NAME: vJenie_CbInit
 *
 * DESCRIPTION:
 * Entry point for application after stack init.
 * Called after stack has initialised. Whether warm or cold start
 *
 * RETURNS:
 * Nothing
 *
 ****************************************************************************/

PUBLIC void vJenie_CbInit(bool_t bWarmStart)
{
  teJenieStatusCode eResult;
    /* Initialise utilities */
    vUtils_Init();
    vAHI_WatchdogStop();

    /* Initialise application data */
    vSetState(E_STATE_IDLE);

    /* Read configration from flash */
    (void)bSetup_Read(&sAppData.sSetup);

    /* Initialise generic timer module */
    vTime_Init(TIME_TIMER_0);

  /* Initialise IP interface */
  vIP_Init(&sAppData.sSetup.sLocalAddr, &sAppData.sSetup.sGatewayAddr, &sAppData.sSetup.sSubnetMask);

    /* Initialise RTC */
  vRTC_Init(RTC_WAKE_TIMER_NONE, RTC_CLK_SRC_APP);
  vRTC_SetDate(&sAppData.sSetup.sDate);
  vRTC_SetTime(&sAppData.sSetup.sTime);
  sAppData.u16TickCounts = 0;

  /* Start the Jenie stack */
  eResult = eJenie_Start(E_JENIE_COORDINATOR);
  if(eResult != E_JENIE_SUCCESS)
  {
    /* Try again... */
    vJPI_SwReset();
  }
}

/****************************************************************************
 *
 * NAME: gJenie_CbMain
 *
 * DESCRIPTION:
 * Main user routine. This is called by the stack at regular intervals.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vJenie_CbMain(void)
{
  uint8 u8Status;
    /* regular watchdog reset */
    //#ifdef WATCHDOG_ENABLED
       //vAHI_WatchdogRestart();
    //#endif
    vProcessEvent(E_EVENT_POLL, NULL);
    /* Read the UART status */
    u8Status = u8AHI_UartReadLineStatus(SETUP_PORT);
    /* If data in Receive FIFO */
    if (u8Status & E_AHI_UART_LS_DR)
  {
      u8Status = u8AHI_UartReadData (SETUP_PORT);
      /* If character = S or s then*/
    if ((u8Status == 'S') ||(u8Status == 's'))
    {
      /* Display the setup menu, this function does not return. */
      vSetup_Task(au8Version, au8BuildDate, au8BuildTime);
      /* Restart the JN5148 */
      vAHI_SwReset();
      }
  }
}
/****************************************************************************
 *
 * NAME: vJenie_CbStackMgmtEvent
 *
 * DESCRIPTION:
 * Used to receive stack management events
 *
 * PARAMETERS:      Name            RW  Usage
 *                  eEventType      R   Enumeration to indicate event type
 *                  *pvEventPrim    R   Pointer to data structure containing
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vJenie_CbStackMgmtEvent(teEventType eEventType, void *pvEventPrim)
{
  switch (eEventType)
  {
    case E_JENIE_NETWORK_UP:
      vProcessEvent(E_EVENT_NWK_STARTED, NULL);
      break;

    case E_JENIE_CHILD_JOINED:
      break;

    case E_JENIE_CHILD_LEAVE:
      break;

    default:
      break;
  }
}

/****************************************************************************
 *
 * NAME: vJenie_CbStackDataEvent
 *
 * DESCRIPTION:
 * Used to receive stack data events
 *
 * PARAMETERS:      Name                    RW  Usage
 *                  *psStackDataEvent       R   Pointer to data structure
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vJenie_CbStackDataEvent(teEventType eEventType, void *pvEventPrim)
{
  switch(eEventType)
  {
    case E_JENIE_DATA:
      /* We have received a data frame, pass to state machine for processing */
      vProcessEvent(E_EVENT_DATA_RECEIVED, pvEventPrim);
      break;

    case E_JENIE_DATA_ACK:
      break;

    default:
      break;
  }
}

/****************************************************************************
 *
 * NAME: vJenie_CbHwEvent
 *
 * DESCRIPTION:
 * Adds events to the hardware event queue.
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u32Device       R   Peripheral responsible for interrupt e.g DIO
 *                  u32ItemBitmap   R   Source of interrupt e.g. DIO bit map
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vJenie_CbHwEvent(uint32 u32DeviceId,uint32 u32ItemBitmap)
{
  if (u32DeviceId == E_AHI_DEVICE_TICK_TIMER)
  {
    if (++sAppData.u16TickCounts >= 100)
    {
      vRTC_Tick(32768);
      sAppData.u16TickCounts = 0;
    }
  }
}
/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: vProcessEvent
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
PRIVATE void vProcessEvent(teAppEvent eEvent, void *pvParam)
{
    switch (sAppData.eAppState)
    {
        case E_STATE_IDLE:
            if (eEvent == E_EVENT_NWK_STARTED)
            {
                /* Cleat list of sensors */
                sSensorList.u8NbrSensors = 0;
                sAppData.u32LastSensorPollTime = u32Time_TimeNowMs();

                /* Enable route purging */
                vApi_SetPurgeRoute(TRUE);
                vApi_SetPurgeInterval((SENSOR_MAX_PING_PERIOD_ms * 2UL) / 100UL);

                vSetState(E_STATE_NWK_STARTED);
            }
            break;

        case E_STATE_NWK_STARTED:
            if (eEvent == E_EVENT_POLL)
            {
                uint8 i;
        /* Allow internet interface to send and receive */
        vIP_Poll();

                /* Check if it is timer to check the sensors */
                if (u16Time_ElapsedTimeSinceMs(sAppData.u32LastSensorPollTime) > SENSOR_TIMEOUT_POLL_PERIOD_ms)
                {
                    /* Check for sensors that are no longer sending data */
                    for (i = 0; i < MAX_SENSORS; i++)
                    {
                        if (u16Time_ElapsedTimeSinceMs(sSensorList.asSensor[i].u32LastRxTime) > SENSOR_TX_TIMEOUT_ms)
                        {
                            sSensorList.asSensor[i].u8Status = STATUS_FAILED;
                        }
                    }
                    sAppData.u32LastSensorPollTime = u32Time_TimeNowMs();
                }
            }
            else if (eEvent == E_EVENT_DATA_RECEIVED)
            {
                uint8 i;
                bool_t bFound = FALSE;
                tsData *psRxData = (tsData *)pvParam;

                /* Check if we already have an entry in the list for this sensor */
                for (i = 0; i < MAX_SENSORS; i++)
                {
                    if (psRxData->u64SrcAddress == sSensorList.asSensor[i].u64Addr)
                    {
                        /* We have this sensor in the list so just update the entry */
                        memcpy(&sSensorList.asSensor[i], &psRxData->pau8Data[0], sizeof(tsSensorData));

                        /* Fill in time and date fields */
                        vRTC_GetDate(&sSensorList.asSensor[i].sDate);
                        vRTC_GetTime(&sSensorList.asSensor[i].sTime);

                        /* Add internal timestamp that will be used to check if
                           sensor has stopped transmitting */
                        sSensorList.asSensor[i].u32LastRxTime = u32Time_TimeNowMs();

                        /* Update status */
                        sSensorList.asSensor[i].u8Status = STATUS_OK;

                        bFound = TRUE;
                        break;
                    }
                }

                if (!bFound)
                {
                    /* Could not find this sensor in the list, check if we have space to add it */
                    if (sSensorList.u8NbrSensors < MAX_SENSORS)
                    {
                        /* Space available add to list */
                        memcpy(&sSensorList.asSensor[sSensorList.u8NbrSensors], &psRxData->pau8Data[0], sizeof(tsSensorData));

                        /* Add internal timestamp that will be used to check if
                           sensor has stopped transmitting */
                        sSensorList.asSensor[sSensorList.u8NbrSensors].u32LastRxTime = u32Time_TimeNowMs();

                        /* Fill in time and date fields */
                        vRTC_GetDate(&sSensorList.asSensor[sSensorList.u8NbrSensors].sDate);
                        vRTC_GetTime(&sSensorList.asSensor[sSensorList.u8NbrSensors].sTime);

                        /* Update status */
                        sSensorList.asSensor[sSensorList.u8NbrSensors].u8Status = STATUS_OK;
                        sSensorList.u8NbrSensors++;
                    }
                }
            }
            break;

        default:
            break;
    }
}

/****************************************************************************
 *
 * NAME: vSetState
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
PRIVATE void vSetState(teAppState eNewState)
{
    sAppData.eAppState = eNewState;
}
/****************************************************************************
 *
 * NAME: vUnalignedAccessHandler
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
PUBLIC void vUnalignedAccessHandler (void)
{
  volatile uint32 u32BusyWait = 1600000;
  // Display the exception
  vUtils_Debug("vUnalignedAccessHandler");
  // wait for the UART write to complete
  while(u32BusyWait--){}
  vAHI_SwReset ();
}
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
