/****************************************************************************
 *
 * MODULE:             JenNet End Device
 *
 * LAST MODIFIED BY:   $Author: $
 *                     $Modtime: $
 *
 ****************************************************************************
 *
 *
 ****************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include <string.h>

#include <Jenie.h>
#include <JPI.h>
#include "Utils.h"
#include "config.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

typedef enum
{
    APP_STATE_WAITING_FOR_NETWORK,
    APP_STATE_NETWORK_UP,
    APP_STATE_REGISTERING_SERVICE,
    APP_STATE_WAITING_FOR_REGISTER_SERVICE,
    APP_STATE_SERVICE_REGISTERED,
    APP_STATE_REQUEST_SERVICES,
    APP_STATE_WAITING_FOR_REQUEST_SERVICE,
    APP_STATE_SERVICE_REQUEST_RETURNED,
    APP_STATE_RUNNING
} teAppState;

typedef struct
{
    teAppState eAppState;
    uint64     u64ServiceAddress;
} tsAppData;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

PRIVATE tsAppData sAppData;

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
    /* Change default network config */
    gJenie_Channel          = CHANNEL;
    gJenie_NetworkApplicationID = SERVICE_PROFILE_ID;
    gJenie_PanID            = PAN_ID;

    gJenie_EndDevicePollPeriod  = POLL_PERIOD;
    gJenie_RoutingEnabled       = FALSE;
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
    /* Initialise utilities */
    vUtils_Init();

    if(!bWarmStart)
    {
        memset(&sAppData, 0, sizeof(sAppData));
        vUtils_Debug("Jenie Init End Device");
    }
    else
    {
        vUtils_Debug("waking from sleep");
    }

    if(eJenie_Start(E_JENIE_END_DEVICE) != E_JENIE_SUCCESS)
    {
        vUtils_Debug("!!Failed to start Jenie!!");
        while(1);
    }
}

/****************************************************************************
 *
 * NAME: gJenie_CbMainFIRST
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
    eJenie_SetSleepPeriod(1000); /* 1 seconFIRSTd */

    static int i = 0;

    /* regular watchdog reset */
    #ifdef WATCHDOG_ENABLED
       vAHI_WatchdogRestart();
    #endif

    switch(sAppData.eAppState)
    {
    case APP_STATE_WAITING_FOR_NETWORK:
        break;

    case APP_STATE_NETWORK_UP:
        /* as we are an end device, disallow nodes to associate with us */
        vUtils_Debug("disabling association");
        eJenie_SetPermitJoin(FALSE);

        sAppData.eAppState = APP_STATE_REGISTERING_SERVICE;
        break;

    case APP_STATE_REGISTERING_SERVICE:
        /* we provide THIRD_SERVICE */
        vUtils_Debug("registering service");
        eJenie_RegisterServices(THIRD_SERVICE_MASK);

        sAppData.eAppState = APP_STATE_WAITING_FOR_REGISTER_SERVICE;
        i = 5;
        break;

    case APP_STATE_WAITING_FOR_REGISTER_SERVICE:
        if(i == 0)
        {
            /* when we wake up, go back to the previous state */
            sAppData.eAppState = APP_STATE_REGISTERING_SERVICE;
        }
        i--;
        eJenie_Sleep(E_JENIE_SLEEP_OSCON_RAMON);
        break;

    case APP_STATE_SERVICE_REGISTERED:
        /* Services now registered with parent */

        /* go to the running state */
        sAppData.eAppState = APP_STATE_RUNNING;

        /* Or request services from router */
        //sAppData.eAppState = APP_STATE_REQUEST_SERVICES;
        break;

    case APP_STATE_REQUEST_SERVICES:
        /* we use service SECOND_SERVICE on a remote node */
        vUtils_Debug("requesting service");
        eJenie_RequestServices(SECOND_SERVICE_MASK, TRUE);

        sAppData.eAppState = APP_STATE_WAITING_FOR_REQUEST_SERVICE;
        i = 5;
        break;

    case APP_STATE_WAITING_FOR_REQUEST_SERVICE:
        if(i == 0)
        {
            /* when we wake up, go back to the previous state */
            sAppData.eAppState = APP_STATE_SERVICE_REGISTERED;
        }
        i--;
        eJenie_Sleep(E_JENIE_SLEEP_OSCON_RAMON);
        break;

    case APP_STATE_SERVICE_REQUEST_RETURNED:
        /* bind local THIRD_SERVICE to remote SECOND_SERVICE */
        vUtils_Debug("binding service");
        if (eJenie_BindService(THIRD_SERVICE, sAppData.u64ServiceAddress, SECOND_SERVICE) == E_JENIE_SUCCESS)
        {
            uint8 au8Data[] = "THIRD->SECOND";
            eJenie_SendDataToBoundService(THIRD_SERVICE, au8Data, 14, TXOPTION_ACKREQ);
        }

        sAppData.eAppState = APP_STATE_RUNNING;
        break;

    case APP_STATE_RUNNING:
        /* do all necessary processing here */
        break;

    default:
        vUtils_DisplayMsg("!!Unknown state!!", sAppData.eAppState);
        while(1);
    }
}
/****************************************************************************
 *
 * NAME: vJenie_CbStackMgmtEvent
 *
 * DESCRIPTION:
 * Used to receive stack management events
 *
 * PARAMETERS:      Name                    RW  Usage
 *                  *psStackMgmtEvent       R   Pointer to event structure
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vJenie_CbStackMgmtEvent(teEventType eEventType, void *pvEventPrim)
{
    switch(eEventType)
    {
    case E_JENIE_NETWORK_UP:
        /* Indicates stack is up and running */
        vUtils_Debug("network up");
        if(sAppData.eAppState == APP_STATE_WAITING_FOR_NETWORK)
        {
            sAppData.eAppState = APP_STATE_NETWORK_UP;
        }
        break;

    case E_JENIE_REG_SVC_RSP:
        vUtils_Debug("Reg service response");
        if(sAppData.eAppState == APP_STATE_WAITING_FOR_REGISTER_SERVICE)
        {
            sAppData.eAppState = APP_STATE_SERVICE_REGISTERED;
        }
        else
        {
            vUtils_DisplayMsg("wrong state", sAppData.eAppState);
        }
        break;

    case E_JENIE_SVC_REQ_RSP:
        vUtils_Debug("Service req response");
        if(sAppData.eAppState == APP_STATE_WAITING_FOR_REQUEST_SERVICE)
        {
            if(((tsSvcReqRsp *)pvEventPrim)->u32Services & SECOND_SERVICE_MASK)
            {
                sAppData.u64ServiceAddress = ((tsSvcReqRsp *)pvEventPrim)->u64SrcAddress;
                sAppData.eAppState = APP_STATE_SERVICE_REQUEST_RETURNED;
            }
            else
            {
                vUtils_Debug("wrong service");
            }
        }
        else
        {
            vUtils_DisplayMsg("wrong state", sAppData.eAppState);
        }
        break;

    case E_JENIE_POLL_CMPLT:
        break;

    case E_JENIE_PACKET_SENT:
        vUtils_Debug("Packet sent");
        break;

    case E_JENIE_PACKET_FAILED:
        vUtils_Debug("Packet failed");
        break;

    case E_JENIE_STACK_RESET:
        vUtils_Debug("Stack Reset");
        sAppData.eAppState = APP_STATE_WAITING_FOR_NETWORK;
        break;

    default:
        /* Unknown data event type */
        vUtils_DisplayMsg("!!Unknown Mgmt Event!!", eEventType);
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
        vUtils_Debug("Data event");
        break;

    case E_JENIE_DATA_TO_SERVICE:
        vUtils_Debug("Data to service event");
        break;

    case E_JENIE_DATA_ACK:
        vUtils_Debug("Data ack");
        break;

    case E_JENIE_DATA_TO_SERVICE_ACK:
        vUtils_Debug("Data to service ack");
        break;

    default:
        // Unknown data event type
        vUtils_DisplayMsg("!!Unknown Data Event!!", eEventType);
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
    switch (u32DeviceId)
    {
        case E_JPI_DEVICE_TICK_TIMER:
            /* regular 10ms tick generated here */
            break;

        default:
            vUtils_DisplayMsg("HWint: ", u32DeviceId);
            break;
    }
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
