/****************************************************************************
 *
 * MODULE:             JenNet Coordinator Node
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
#include <Printf.h>
#include "Utils.h"

#include "c_config.h"
#include "bit.h"
#include "led.h"
#include "i2c_9555.h"

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
	APP_STATE_RUNNING
} teAppState;

typedef struct
{
	teAppState eAppState;
} tsAppData;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vPRT_LireBtnPgm(void);
PRIVATE void vPRT_TraiterChangementEntree(uint32 val);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE bool_t bStartPgmTimer = FALSE;
PRIVATE uint16 TimePgmPressed = 0;

PRIVATE tsAppData sAppData;

/* Routing table storage */
PRIVATE tsJenieRoutingTable asRoutingTable[100];


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
	gJenie_Channel              = CHANNEL;
	gJenie_NetworkApplicationID = SERVICE_PROFILE_ID;
	gJenie_PanID                = PAN_ID;

	/* Configure stack with routing table data */
	gJenie_RoutingEnabled    = TRUE;
	gJenie_RoutingTableSize  = 100;
	gJenie_RoutingTableSpace = (void *)asRoutingTable;

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

	u32AHI_Init();
	vUtils_Init();
	vUART_printInit();
	vPRT_Init_9555(E_BUS_400_KH);


#if !NO_DEBUG_ON

#if SHOW_TEST_1
	PCA_9555_test_01();
#endif //SHOW_TEST_1

#if SHOW_TEST_2
	PCA_9555_test_02();
#endif //SHOW_TEST_2

#if SHOW_TEST_3
	PCA_9555_test_03();
#endif //SHOW_TEST_3
#endif // !NO_DEBUG_ON

#if 0
	/* Initialise utilities */
	bool_t status = FALSE;
	vUtils_Init();
	vUART_printInit();

	//status = Init_PCA_9555(E_BUS_100_KH);
	I2CWrite(0x40,0x06,0x00);

	vPrintf("stat:%d\n",status);
#endif

	memset(&sAppData, 0, sizeof(sAppData));
	vUtils_Debug("Jenie demarrage Coordinateur");

	if(eJenie_Start(E_JENIE_COORDINATOR) != E_JENIE_SUCCESS)
	{
		vUtils_Debug("!!Failed to start Jenie!!");
		while(1);
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
#if 0
PUBLIC void vJenie_CbMain(void)
{
	/* regular watchdog reset */
#ifdef WATCHDOG_ENABLED
	vAHI_WatchdogRestart();
#endif

	switch(sAppData.net)
	{
		case E_JEN_RECHERCHE_RESEAU:
		{
			au8Led[CST_LED_INFO_1].actif = TRUE;
			au8Led[CST_LED_INFO_1].pio = PIO_LED_INFO_1;
			au8Led[CST_LED_INFO_1].mode = E_LED_MSG_1;

			au8Led[CST_LED_INFO_2].actif = TRUE;
			au8Led[CST_LED_INFO_2].pio = PIO_LED_INFO_2;
			au8Led[CST_LED_INFO_2].mode = E_LED_OFF;

			sAppData.net = E_JEN_ATTENTE_RESEAU;
		}
		break;

		case E_JEN_ATTENTE_RESEAU:
		{
			; //rien
		}
		break;

		case E_JEN_RESEAU_PRESENT:
		{
			au8Led[CST_LED_INFO_1].mode = E_LED_MSG_2;
			sAppData.net = E_JEN_RUN_APP;
		}
		break;

		case E_JEN_RUN_APP:
		{
			if(sAppData.use_pwr)
			{
				GererMEF1(sAppData.pwr);
			}

			if(sAppData.clv)
			{
				GererMEF2(sAppData.clv);
			}
		}
		break;

		default:
		{
			vPrintf("Erreur MEF0\n");

		}
		break;
	}
}
#endif

PUBLIC void vJenie_CbMain(void)
{
	/* regular watchdog reset */
#ifdef WATCHDOG_ENABLED
	vAHI_WatchdogRestart();
#endif

	switch(sAppData.eAppState)
	{
		case APP_STATE_WAITING_FOR_NETWORK:
			/* nothing to do till network is up and running */
			break;

		case APP_STATE_NETWORK_UP:
			/* as we are a coordinator, allow nodes to associate with us */
			vUtils_Debug("enabling association");
			eJenie_SetPermitJoin(TRUE);

			/* register services */
			sAppData.eAppState = APP_STATE_RUNNING;
			break;

		case APP_STATE_REGISTERING_SERVICE:
			/* we provide FIRST_SERVICE */
			vUtils_Debug("registering service");
			//eJenie_RegisterServices(FIRST_SERVICE_MASK);

			/* go to the running state */
			sAppData.eAppState = APP_STATE_RUNNING;
			break;

		case APP_STATE_RUNNING:
			/* do all necessary processing here */
			vPRT_LireBtnPgm();
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
			break;

		case E_JENIE_SVC_REQ_RSP:
			vUtils_Debug("Service req response");
			break;

		case E_JENIE_PACKET_SENT:
			vUtils_Debug("Packet sent");
			break;

		case E_JENIE_PACKET_FAILED:
			vUtils_Debug("Packet failed");
			break;

		case E_JENIE_CHILD_JOINED:
			vUtils_Debug("Child Joined");
			break;

		case E_JENIE_CHILD_LEAVE:
			vUtils_Debug("Child Leave");
			break;

		case E_JENIE_CHILD_REJECTED:
			vUtils_Debug("Child Rejected");
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
	static uint8 timer_it_dio11 = 0;
	static uint8 timer_it_dio12 = 0;
	uint32 val = 0;

	switch (u32DeviceId)
	{
		case E_JPI_DEVICE_TICK_TIMER:
		{
			IHM_ClignoteLed();

			/* regular 10ms tick generated here */
			if (bStartPgmTimer){
				TimePgmPressed++;
			}

			if (bIt_DIO11){
				timer_it_dio11++;

				if(timer_it_dio11 == CST_ANTI_REBOND_IT)
				{
					timer_it_dio11 = 0;
					bIt_DIO11=FALSE;
					val = vPRT_DioReadInput();
					vPrintf("It read = %x\n",val);
					vPRT_TraiterChangementEntree(val);
				}
			}

			if (bIt_DIO12)
			{

				// Supression anti rebond
				timer_it_dio12++;

				if(timer_it_dio12 == CST_ANTI_REBOND_IT)
				{
					// A priori on a plus d'it de rebond interrupteur
					timer_it_dio12 = 0;

					// Autoriser une nouvelle It
					bIt_DIO12=FALSE;
				}
			}

			if(bMessureDureePressionDio12)
			{
				// Mesure du temps d'appui sur bouton
				timer_pression_DIO12++;
			}
		}
		break;

		default:
			vUtils_DisplayMsg("HWint: ", u32DeviceId);
			break;
	}
}

PRIVATE void vPRT_LireBtnPgm(void)
{
	PRIVATE bool_t passage = FALSE;

	// Bouton Pgm appuye ??
	if ((u8JPI_PowerStatus() & 0x10) == 0)
	{
		bStartPgmTimer = TRUE;
	}
	else
	{
		bStartPgmTimer = FALSE;
		if(TimePgmPressed)
		{
			if (TimePgmPressed<30)
			{
				if(!passage){
					vPRT_DioSetOutput(bit0,bit7);
				}
				else
				{
					vPRT_DioSetOutput(bit7,bit0);
				}
				passage = !passage;

			}
			TimePgmPressed = 0;
		}
	}
}

PRIVATE void vPRT_TraiterChangementEntree(uint32 val)
{
	uint16 lesEntrees = (val>>16 & 0xFF00) | (((uint16)val>>8) & 0x00FF);
	uint16 tmp = prvCnf_I_9555 ^ lesEntrees;
	uint8 un_port = 0;
	uint8 val_port = 0;
	uint8 une_entree = 0;
	uint32 req_on = ((prvCnf_O_9555 << 8) & 0x00FF0000 )| ((uint8)prvCnf_O_9555 & 0x00FF00FF);
	uint32 req_off = ((~prvCnf_O_9555 << 8) & 0x00FF0000 )| (~(uint8)prvCnf_O_9555 & 0x00FF00FF);

	vPrintf("\nConfig entrees -> previous:%x, now:%x\n", prvCnf_I_9555, lesEntrees);
	vPrintf(" Depart Req_on:%x, Req_off:%x\n",req_on,req_off);

	if(tmp)
	{
		vPrintf("\nConfig sorties -> previous:%x\n",prvCnf_O_9555);

		for(un_port=0;un_port<2;un_port++)
		{
			val_port = (uint8)tmp>>(8*un_port);
			//out_cnf = (uint8)prevConfOutputs>>(8*un_port);

			for(une_entree = 0; une_entree< 8; une_entree++)
			{
				if(IsBitSet(val_port, une_entree))
				{
					vPrintf(" Changement sur l'entree %d du port %d\n", une_entree, un_port);

					// Memorisation du changement
					prvCnf_I_9555 = lesEntrees;

					// prendre la valeur du bit concerne pour faire on ou off
					if(IsBitSet(lesEntrees,(une_entree + (un_port*8))))
					{
						// Si on a un 1 -> pull up active
						// interrupteur branche a la masse
						vPrintf("  Passage a OFF\n");
						BitNset(req_on,(une_entree + ((un_port)*16)));
						BitNclr(req_off,(une_entree + ((un_port)*16)));
					}
					else
					{
						vPrintf("  Passage a ON\n");
						BitNset(req_off,(une_entree + ((un_port)*16)));
						BitNclr(req_on,(une_entree + ((un_port)*16)));
					}
				}
			}

		}
		// envoyer la commande de config des sorties
		vPrintf(" Modif Req_on:%x, Req_off:%x\n",req_on,req_off);
		vPRT_DioSetOutput(req_on,req_off);
	}
}
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
