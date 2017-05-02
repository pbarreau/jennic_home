/****************************************************************************
 *
 * MODULE:             JenNet End Device
 * COMPONENT:          $RCSfile: EndDevice.c,v $
 *
 * VERSION:
 *
 * REVISION:           0.0
 *
 * DATED:              2007/05/29 15:40:19
 *
 * STATUS:             Exp
 *
 * AUTHOR:             MM
 *
 * DESCRIPTION:
 *
 *
 * LAST MODIFIED BY:   $Author: jahme $
 *                     $Modtime: $
 *
 ****************************************************************************
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
 * Copyright Jennic Ltd 2008. All rights reserved
 *
 ****************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include <AppApi.h>
#include <AppHardwareApi.h>
#include <Jenie.h>
#include <JPI.h>

#include <JenNetApi.h>

#include <string.h>
#include <Printf.h>

#include "Utils.h"
#include "m_config.h"
#include "e_config.h"


/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void PBAR_SetModeKey(void);
//PRIVATE void vDIOInterruptCallback (uint32 u32Device, uint32 u32ItemBitmap);

PRIVATE bool_t VerifierExistanceBoite(PBAR_E_KeyMode eMode,
		PBAR_KIT_8046 eKey,uint8 BoxId);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern PUBLIC PBAR_KIT_8046 eKeyTouched;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

PRIVATE tsAppData 	sAppData;

PRIVATE uint8 IncomingBoxId;
PRIVATE bool_t bConfStart = FALSE;
PRIVATE bool_t OneIt20 = FALSE;

PRIVATE bool_t  mbConfigNecessaire = FALSE;


PRIVATE uint32	u32ServiceRq; //service request
PRIVATE uint32	u32ServiceRp; //service responce

PRIVATE uint8 	box;					// Id de la box pour avoir addresse

PUBLIC  PBAR_E_KeyMode eKeyMode = E_CLAV_MODE_NOT_SET;
PUBLIC  PBAR_E_KeyMode eModeOnConf = E_CLAV_MODE_DEFAULT;
PRIVATE bpeClav eTypeUsage = E_CLAV_EN_NON_DEFINI;

PUBLIC bool_t				 bModePgmOn = FALSE;

PRIVATE uint16 tempoMode = 0;


// A l'initialisation
PRIVATE uint8 	pBuff[2]; // pointeur In & Read du buffer
PRIVATE uint8 	bufReception[PBAR_RBUF_SIZE]; // Buffer circulaire reception
PRIVATE uint64	bufAddr[PBAR_RBUF_SIZE];

// Gestion antirebond touche
PRIVATE bool_t bStartTimerMode = FALSE;

PUBLIC	uint8 tstMsg1[]={0x01,0x00,0x00};
PUBLIC bool_t gOneIt20 = FALSE;

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
	gJenie_Channel          = PBAR_CHANNEL;
	gJenie_NetworkApplicationID = PBAR_NID;
	gJenie_PanID            = PBAR_PAN_ID;

	gJenie_EndDevicePollPeriod  = PBAR_E_POLL_PER;
	gJenie_RoutingEnabled       = FALSE;

	/* Initialise flash interface */
	//bAHI_FlashInit(E_FL_CHIP_AUTO, NULL);


	//vApi_RegScanSortCallback(PBAR_SCANEnd);
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
	u32AHI_Init();
	vUART_printInit();
	vUtils_Init();

	vAHI_DioSetDirection(PBAR_CFG_NUMPAD_IN,0);
	vAHI_DioSetDirection(0,PBAR_CFG_LED);
	vAHI_DioSetPullup(PBAR_CFG_NUMPAD_IN, PBAR_CFG_LED);
	vAHI_DioSetOutput(0,PBAR_CFG_LED);

	vAHI_DioInterruptEdge(
			PBAR_CFG_NUMPAD_IN &
			(~(E_JPI_DIO19_INT)),
			E_JPI_DIO19_INT
	);

	vAHI_DioInterruptEdge(0,E_JPI_DIO20_INT);

	vAHI_SysCtrlRegisterCallback(vDIOInterruptCallback);

	memset(&sAppData, 0, sizeof(sAppData));
	memset(&au8Led, 0, sizeof(sbpLed));

	au8Led[0].actif = TRUE;
	au8Led[0].pio = C_LED1;

#if 0
	if(!bAHI_FullFlashRead(FLASH_START, sizeof(bpsFlash), (uint8 *) &eeprom))
	{
		vPrintf("Pb Lors de la lecture de la config Flash\n");
	}
	// --------------------------
#endif


	vUtils_Debug("Jenie Init End Device");
	vPrintf("nb Boites:%d\n",eeprom.nbBoite);

	if(eJenie_Start(E_JENIE_END_DEVICE) != E_JENIE_SUCCESS)
	{
		vUtils_Debug("!!Failed to start Jenie!!");
		while(1);
	}

}
#if 0
PUBLIC void vJenie_CbInit(bool_t bWarmStart)
{

	//BA7_InitFlash();
	vUtils_Init();
	/* Initialise utilities */
	u32AHI_Init();
	vUART_printInit();
	vAHI_UartDisable(E_AHI_UART_1);

	/* Set SW1(dio11..20) to input and LEDs(dio10,9,8) to output */
	vAHI_DioSetDirection(PBAR_CFG_NUMPAD_IN,0);
	vAHI_DioSetDirection(0,PBAR_CFG_LED);

	/* enable pullups on all inputs/unused pins  */
	vAHI_DioSetPullup(PBAR_CFG_NUMPAD_IN, PBAR_CFG_LED);

	/* turn LED off */
	vAHI_DioSetOutput(0,PBAR_CFG_LED);


	/* set interrupt for DIO11-20 to occur on button release - rising edge */
	// Sauf pour IO 19  Front descendant
	//vPrintf("a:%x,b:%x\n",PBAR_CFG_NUMPAD_IN,PBAR_CFG_NUMPAD_IN &(~E_JPI_DIO19_INT));
	vAHI_DioInterruptEdge(
			PBAR_CFG_NUMPAD_IN &
			(~(E_JPI_DIO19_INT)),
			E_JPI_DIO19_INT
	);

	vPrintf("Taille donnees :%d\n", sizeof(bpsFlash));
	vAHI_DioInterruptEdge(0,E_JPI_DIO20_INT);

	/* enable interrupt for DIO11-20 */
	//vAHI_DioInterruptEnable(PBAR_CFG_NUMPAD_IN,0);


	/* register interrupt handling function */
	vAHI_SysCtrlRegisterCallback(vDIOInterruptCallback);




	if(!bWarmStart)
	{
		// Demarrage a froid
		// ie: apres coupure secteur/1er Branchement

		//Reinitialiser tables des correspondances
		// ----- LECTURE FLASH --------
		// ----------------------------
		if(!bAHI_FullFlashRead(FLASH_START, sizeof(bpsFlash), (uint8 *) &eeprom))
		{
			vPrintf("Pb Lors de la lecture de la config Flash\n");
		}
		// --------------------------



		vUtils_Debug("Jenie Init End Device");
		vPrintf("nb cold Boites:%d\n",eeprom.nbBoite);
	}
	else
	{
		// Cas reveil du noeud apres sleep
		vUtils_Debug("Tic .. Tic Toc");
		vPrintf("nb warm Boites:%d\n",eeprom.nbBoite);
	}

	// Reinitialiser etat du pgm
	memset(&sAppData, 0, sizeof(sAppData));
	memset(&au8Led, 0, sizeof(sbpLed));

	/* Set interrupts on buttons to wake us up */
	//vJPI_DioWake(PBAR_CFG_NUMPAD_IN, 0, 0, PBAR_CFG_NUMPAD_IN);
	/* Set sleep period */
	eJenie_SetSleepPeriod(PERIOD_SLEEP);

	au8Led[0].actif = TRUE;
	au8Led[0].pio = C_LED1;

	if(eJenie_Start(E_JENIE_END_DEVICE) != E_JENIE_SUCCESS)
	{
		vUtils_Debug("!!Failed to start Jenie!!");
		while(1);
	}
}
#endif

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
	//eJenie_SetSleepPeriod(1000); /* 1 seconFIRSTd */

	teJenieStatusCode eStatus;
	uint8 valThisBox = 0;
	uint8 useBox =0;
	uint8 i;
	PRIVATE bool_t unPassage = FALSE;

	/* regular watchdog reset */
#ifdef WATCHDOG_ENABLED
	vAHI_WatchdogRestart();
#endif

	switch(sAppData.eAppState)
	{
	case APP_STATE_WAITING_FOR_NETWORK:
		au8Led[0].mode = E_FLASH_RECHERCHE_RESEAU;
		break;

	case APP_STATE_NETWORK_UP:
	{
		/* as we are an end device, disallow nodes to associate with us */
		vPrintf("> Desactivation autorisation association\n\n");
		eJenie_SetPermitJoin(FALSE);

		// La flash a t elle une config ?
		if(eeprom.nbBoite == 0xFF || eeprom.nbBoite == 0){
			vPrintf("Configuration liens necessaire\n");
			memset(&eeprom, 0, sizeof(eeprom));

			// Non indiquer par voyant
			// necessite de proframmer ce clavier
			au8Led[1].actif = TRUE;
			au8Led[1].pio = C_LED3;
			au8Led[1].mode = E_FLASH_EN_ATTENTE_TOUCHE_BC;
		}
		else
		{
			vPrintf("Une config existe deja\n");
			au8Led[1].actif = FALSE;
		}

		au8Led[0].mode = E_FLASH_RESEAU_ACTIF;


		/* enable interrupt for DIO11-20 */
		vAHI_DioInterruptEnable(PBAR_CFG_NUMPAD_IN,0);

		eTypeUsage = E_CLAV_EN_USAGE;

		eKeyMode = E_CLAV_MODE_DEFAULT;
		eModeOnConf = E_CLAV_MODE_DEFAULT;

		sAppData.eAppState = APP_STATE_RUNNING;

	}
	break;

	case APP_STATE_CONF_NEEDED:
	{
		// Initialiser la memoire
		// memset(&eeprom.netConf,0,sizeof(bpsConfReseau));

		vPrintf("\n> Transmettre passage en mode conf:%x\n",SRV_INTER);

		if(eJenie_RegisterServices((uint32)SRV_INTER)== E_JENIE_DEFERRED){
			vPrintf("   PATIENCE je transmets a mon pere\n");
			sAppData.eAppState = APP_STATE_WAITING_FOR_CONF_SERVICE;

		}
		else{
			vPrintf("!!Passage clavier en mode conf a revoir\n");
		}
	}
	break;

	case APP_STATE_WAITING_FOR_CONF_SERVICE:
	{
		; // Attendre
		//vJenie_CbStackMgmtEvent, E_JENIE_REG_SVC_RSP
	}
	break;

	case APP_STATE_CONF_START:
	{
		vPrintf("\nVoyant: mode configuration\n");
		vPrintf(" En attente de configs\n");

		eKeyTouched = E_KPD_NONE;


		// indiquer attente de config
		au8Led[0].actif = TRUE;
		au8Led[0].pio = C_LED1;
		au8Led[0].mode = E_FLASH_EN_ATTENTE_TOUCHE_BC;


		/* enable interrupt for DIO11-20 */
		//vAHI_DioInterruptEnable(PBAR_CFG_NUMPAD_IN,0);

		sAppData.eAppState = APP_STATE_ATTENDRE_BOITES;
	}
	break;

	case APP_STATE_ATTENDRE_BOITES:
	{
		// Quitter le mode Programmation clavier ?
		if (eKeyTouched == E_KPD_MODE )
		{
			if (PBAR_TraiterClavier(
					&sAppData.eAppState,
					&eTypeUsage,
					eKeyTouched))
			{
				//vPrintf("Fin de Programmation\n");

				eKeyTouched = E_KPD_NONE;
			}

		}

		; // ATTENTE E_JENIE_DATA
		// vJenie_CbStackDataEvent
	}
	break;

	case APP_STATE_ATTENDRE_SEL_MODE_AND_KBD:
	{
		if((eKeyTouched == E_KPD_MODE)){
			vPrintf("Touche Mode pressed val:%d\n Step1\n",eKeyMode);
			if (PBAR_TraiterClavier(
					&sAppData.eAppState,
					&eTypeUsage,
					eKeyTouched))
			{
				;// Rien
			}
			eKeyTouched = E_KPD_NONE;

		}

		else if( (eKeyTouched != E_KPD_NONE) && bConfStart == TRUE){

			if(eModeOnConf == E_CLAV_MODE_NOT_SET){
				vPrintf("A Verifier\n");
				eModeOnConf =  E_CLAV_MODE_DEFAULT;
			}


			vPrintf("Programmation touche clavier:%d, mode:%d\n",eKeyTouched,eModeOnConf);

			// verifier si cette touche connait la boite a configurer
			// si non rajouter cette boite a la touche
			//-------------ICI
			VerifierExistanceBoite(eModeOnConf,eKeyTouched,IncomingBoxId);
			tstMsg1[0]= E_MSG_CFG_LIENS;
			tstMsg1[1]= (eModeOnConf-E_CLAV_MODE_DEFAULT)<<4| eKeyTouched;
			tstMsg1[2]= eeprom.netConf.boxData[(eModeOnConf-E_CLAV_MODE_DEFAULT)][eKeyTouched][IncomingBoxId];

			vPrintf("> Msg %x,%x,%x a [%x;%x]\n",tstMsg1[0],
					tstMsg1[1],
					tstMsg1[2],
					(uint32)(eeprom.BoxAddr[IncomingBoxId] >> 32),
					(uint32)(eeprom.BoxAddr[IncomingBoxId] &  0xFFFFFFFF)
			);
			eJenie_SendData(eeprom.BoxAddr[IncomingBoxId],
					tstMsg1, 3,
					TXOPTION_SILENT);
			sAppData.eAppState = APP_STATE_ATTENDRE_RETOUR_CFG_BOITE;
			eKeyTouched = E_KPD_NONE;

		}

	}
	break;

	case APP_STATE_ATTENDRE_RETOUR_CFG_BOITE:
	{
		; // ATTENTE E_JENIE_DATA
		// vJenie_CbStackDataEvent
	}
	break;

	case APP_STATE_REGISTERING_SERVICE:
	{
		/* Liste des services */
		u32ServiceRq=SRV_INTER;

		vPrintf("-> Indiquer Service:%x\n",u32ServiceRq);
		eStatus = eJenie_RegisterServices(u32ServiceRq);
		switch (eStatus)
		{
		case E_JENIE_SUCCESS:
			vPrintf(" Enregistrement rapide ok\n");
			break;
		case E_JENIE_DEFERRED:
			vPrintf("    PATIENCE je transmets mes capacites\n");
			sAppData.eAppState = APP_STATE_WAITING_FOR_REGISTER_SERVICE;
			break;
		case E_JENIE_ERR_STACK_BUSY:
			vPrintf(" Erreur de Pile\n");
			break;
		case E_JENIE_ERR_UNKNOWN:
			vPrintf(" Erreur critique\n");
			break;
		default:
			vPrintf(" Erreur inconue:%d\n",eStatus);
			break;
		}
	}
	break;

	case APP_STATE_WAITING_FOR_REGISTER_SERVICE:
		break;

	case APP_STATE_SERVICE_REGISTERED:
	{
		/* Services now registered with parent */
		vPrintf("Activation lecture clavier\n");

		/* enable interrupt for DIO11-20 */
		vAHI_DioInterruptEnable(PBAR_CFG_NUMPAD_IN,0);
		eKeyTouched = E_KPD_NONE;

		/* go to the running state */
		sAppData.eAppState = APP_STATE_RUNNING;

	}
	break;

	case APP_STATE_REMOVE_SVC:
	{
		vPrintf("> Retirer clavier comme etant a programmer !\n");
		eStatus = eJenie_RegisterServices(0);
		sAppData.eAppState = APP_STATE_ATTENDRE_FIN_REMOVE_SVC;
	}
	break;


	case APP_STATE_ATTENDRE_FIN_REMOVE_SVC:
	{
		; //Rien on attent evenement Data
	}
	break;

	case APP_STATE_SVC_IS_REMOVE:
	{
		eKeyTouched = E_KPD_NONE;
		sAppData.eAppState =APP_STATE_RUNNING;
	}
	break;

	case APP_STATE_CHANGE_MOD:
	{
		PBAR_SetModeKey();
		eKeyTouched = E_KPD_NONE;

		if(eTypeUsage == E_CLAV_EN_USAGE)
		{
			// Utilisation normale du clavier
			if(unPassage == TRUE){
				unPassage = FALSE;
				vPrintf("Suppression Service\n");
				sAppData.eAppState = APP_STATE_REMOVE_SVC;
			}
			else{
				vPrintf("Maintien Service\n");
				sAppData.eAppState = APP_STATE_RUNNING;
			}
		}
		else
		{
			vPrintf("Mode Prog Service\n");

			if(unPassage == FALSE){
				unPassage = TRUE;

				if(au8Led[1].actif == TRUE){
					vPrintf(" Led off\n");
					// Laisser les voyant mode utilisable
					au8Led[1].actif = FALSE;
				}
				vPrintf("Conf need\n");
				// Obliger le passage en configuration
				sAppData.eAppState= APP_STATE_CONF_NEEDED;
			}
			else
			{
				vPrintf("Attendre boite\n");
				sAppData.eAppState= APP_STATE_ATTENDRE_BOITES;
			}
		}
	}
	break;


	case APP_STATE_EXECUTER_TOUCHE_KBD:
	{
		vPrintf("  Traiter touche:%d mode %d\n",eKeyTouched, eKeyMode);
		// A ton une demande valide
		if(
				(eKeyMode > E_CLAV_MODE_NOT_SET) &&
				(eKeyMode < E_CLAV_MODE_END) &&
				(eKeyTouched < E_KPD_NONE)
		)
		{
			// Regarder le tableau ptr_destination
			// Max boucle == C_MAX_BOXES
			for(box=0;box<C_MAX_BOXES+1;box++)
			{
				if(eeprom.netConf.boxList[eKeyMode-E_CLAV_MODE_DEFAULT][eKeyTouched][box] == C_CLEF_VIDE){
					//suivant de box = box
					// sortir de la boucle
					vPrintf("  Arret recherche id box niveau:%d\n\n",box);
					break;
				}
				else{
					// il y a une boite configuree
					useBox = eeprom.netConf.boxList[eKeyMode-E_CLAV_MODE_DEFAULT][eKeyTouched][box];
					vPrintf("  Use box %d (m:%d, k:%d, ptr:%d)\n",
							useBox,
							eKeyMode-E_CLAV_MODE_DEFAULT,
							eKeyTouched,
							box);

					vPrintf("  bit:%x\n",eeprom.netConf.boxData[eKeyMode-E_CLAV_MODE_DEFAULT][eKeyTouched][useBox]);

					if(eeprom.netConf.boxData[eKeyMode-E_CLAV_MODE_DEFAULT][eKeyTouched][useBox])
					{
						vPrintf("  Io [%x] avec boite %d\n",
								eeprom.netConf.boxData[eKeyMode-E_CLAV_MODE_DEFAULT][eKeyTouched][useBox],
								useBox);
						// Oui alors a t on en memoire l'@ de cette boite
						if(eeprom.BoxAddr[useBox]){
							// Oui on a une addresse connue
							sAppData.u64ServiceAddress = eeprom.BoxAddr[useBox];

							// envoyer le message à cette boite
							if(eKeyTouched != E_KPD_ALL){
								//Demande sous forme de bascule
								tstMsg1[0]=E_MSG_DATA_SELECT;
								tstMsg1[2]=0x00;
							}
							else{
								// Demande Globale impossee
								tstMsg1[0]=E_MSG_DATA_ALL;
								if(eSetAllOff == TRUE){
									// On doit tout eteindre
									tstMsg1[2]=0x00;
								}
								else
								{
									// On doit tout allumer
									tstMsg1[2]=0xFF;
								}
							}
							tstMsg1[1]=eeprom.netConf.boxData[eKeyMode-E_CLAV_MODE_DEFAULT][eKeyTouched][useBox];

							vPrintf
							("  --> MSG (%x,%x,%x) a [%x:%x]\n",
									tstMsg1[0],
									tstMsg1[1],
									tstMsg1[2],
									(uint32) (sAppData.u64ServiceAddress >> 32),
									(uint32) (sAppData.u64ServiceAddress & 0xFFFFFFFF)
							);

							eJenie_SendData(sAppData.u64ServiceAddress,
									tstMsg1, 3,
									TXOPTION_SILENT);
						}
						else
						{
							vPrintf("Erreur dans la memorisation des @ boites\n");
						}

					}
				}
			}
		}
		eKeyTouched = E_KPD_NONE;
		sAppData.eAppState = APP_STATE_RUNNING;
	}
	break;

	case APP_STATE_MK_KBD_ALL:
	{
		// rajout de cette boite a la liste de celle du clavier
		vPrintf("Une B:%d de + a  key:%d ?\n",IncomingBoxId,eKeyTouched);
		VerifierExistanceBoite(eModeOnConf,eKeyTouched,IncomingBoxId);

		// on va parcourir toute les boites de cette touche
		// et creer une commande ALL
		vPrintf("\nCreation cmd pour kbd ALL\n");
		// Parcourir toute les touche de la boite en config
		valThisBox = 0;
		for(i=0;i<C_MAX_KEYS;i++)
		{
			// recuperer la config du clavier et la stocker
			// dans la touche ALL
			valThisBox |= eeprom.netConf.boxData[eModeOnConf-E_CLAV_MODE_DEFAULT][i][IncomingBoxId];
			vPrintf(" box:%d key:%d, val:%x\n",useBox,i,valThisBox);
		}

		// sauvegarder cette valeur dans la touche ALL de la boite en cours
		// On sauve la touche all
		eeprom.netConf.boxData[eModeOnConf-E_CLAV_MODE_DEFAULT][C_MAX_KEYS][IncomingBoxId] = valThisBox;

		// Dire que Touche ALL position ptr++ a une Boite configuree
		VerifierExistanceBoite(eModeOnConf,E_KPD_ALL,IncomingBoxId);

		// sauvegarder la config dans la flash !!!
#if 0
		bAHI_FlashEraseSector(FLASH_SECTOR);
		bAHI_FullFlashProgram(FLASH_START, sizeof(bpsFlash), (uint8 *) &eeprom);

		// Test LEcture
		bAHI_FullFlashRead(FLASH_START, sizeof(bpsFlash), (uint8 *) &eeprom);
		vPrintf("Relecture boite=%d\n",eeprom.nbBoite);
#endif

		if(!bAHI_FlashEraseSector(FLASH_SECTOR)){
			vPrintf("Pb lors effacement Sector 3\n");
		}
		else{
			// Programmation

			if(!bAHI_FullFlashProgram(FLASH_START, sizeof(bpsFlash), (uint8 *) &eeprom)
			)
			{
				vPrintf("Pb Lors de la sauvegarde en Flash\n");
			}
			else
			{
				vPrintf("Sauvegarde sur Flash terminee\n");
			}
		}
		// montrer mode pgm next box ..?
		au8Led[0].mode=E_FLASH_EN_ATTENTE_TOUCHE_BC;

		bConfStart = FALSE;
		sAppData.eAppState = APP_STATE_ATTENDRE_BOITES;
	}
	break;

	case APP_STATE_RUNNING:
	{
		if (eKeyTouched != E_KPD_NONE)
		{
			if (PBAR_TraiterClavier(
					&sAppData.eAppState,
					&eTypeUsage,
					eKeyTouched))
			{
				; // Gestion ailleur
			}

		}

		// Bouton Pgm appuye ??
		if ((u8JPI_PowerStatus() & 0x10) == 0)
		{
			vPrintf("Effacement de la config Clavier\n");

#if 0
			// Oui alors effacer donnees de la flash
			bAHI_FlashEraseSector(7);
			memset(&eeprom,0x00,sizeof(eeprom));
			bAHI_FullFlashProgram(FLASH_START, sizeof(bpsFlash), (uint8 *) &eeprom);
#endif

			bAHI_FlashEraseSector(7);
			//memset(&eeprom,0xFF,sizeof(eeprom));
			//bAHI_FullFlashProgram(FLASH_START, sizeof(bpsFlash), (uint8 *) &eeprom);

			// Attendre relachement bouton pour continuer
			while ((u8JPI_PowerStatus() & 0x10) == 0){
				// Tick Timer
				/* regular watchdog reset */
#ifdef WATCHDOG_ENABLED
				vAHI_WatchdogRestart();
#endif

			}

#if 0
			// Vidage buffer UART
			while ((u8JPI_UartReadLineStatus(E_JPI_UART_0) & (E_JPI_UART_LS_THRE | E_JPI_UART_LS_TEMT)) !=
					(E_JPI_UART_LS_THRE | E_JPI_UART_LS_TEMT));
#endif
			// effectuer un reset du clavier
			vAHI_SwReset ();
		}

		if(tempoMode == TEMPS_MODE){
			bStartTimerMode = FALSE;
			tempoMode = 0;
			eKeyMode = E_CLAV_MODE_END;
			sAppData.eAppState = APP_STATE_CHANGE_MOD;
		}
#if 0
		// Vidage buffer UART
		while ((u8JPI_UartReadLineStatus(E_JPI_UART_0) & (E_JPI_UART_LS_THRE | E_JPI_UART_LS_TEMT)) !=
				(E_JPI_UART_LS_THRE | E_JPI_UART_LS_TEMT));

		/* do all necessary processing here */
		//eJenie_Sleep(E_JENIE_SLEEP_OSCON_RAMON);
#endif
	}
	break;

	default:
		vUtils_DisplayMsg("!!Unknown state!!", sAppData.eAppState);
		while(1);
		break;
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
	tsSvcReqRsp *psStackMgmtData =(tsSvcReqRsp *)pvEventPrim;


	switch(eEventType)
	{
	case E_JENIE_NETWORK_UP:
	{
		bp_CommunStackMgmtEvent(&sAppData.eAppState,
				eEventType,
				pvEventPrim);

	}
	break;

	case E_JENIE_REG_SVC_RSP:
	{
		switch(sAppData.eAppState)
		{
		case APP_STATE_ATTENDRE_FIN_REMOVE_SVC:
		{
			vPrintf("< Clavier n'est plus reference\n");
			sAppData.eAppState = APP_STATE_SVC_IS_REMOVE;
		}
		break;

		case APP_STATE_WAITING_FOR_CONF_SERVICE:
			vPrintf("< Le reseau est informe\n");
			if(eeprom.nbBoite == 0){
				sAppData.eAppState = APP_STATE_CONF_START;
			}
			else
			{
				sAppData.eAppState =APP_STATE_ATTENDRE_BOITES;
			}
			break;

		case APP_STATE_WAITING_FOR_REGISTER_SERVICE:
			sAppData.eAppState = APP_STATE_SERVICE_REGISTERED;
			break;

		default:
			vPrintf("SVC resp error, state is:%d\n", sAppData.eAppState);
			break;
		}
	}
	break;

	case E_JENIE_SVC_REQ_RSP:
		u32ServiceRp = psStackMgmtData->u32Services;
		{vPrintf
			(" \n<-Reponse de [%x:%x],svc=%x\n",
					(uint32) (psStackMgmtData->u64SrcAddress >> 32),
					(uint32) (psStackMgmtData->u64SrcAddress & 0xFFFFFFFF),
					u32ServiceRp);

		switch(sAppData.eAppState){
		case APP_STATE_WAITING_FOR_REQUEST_SERVICE:
			if((psStackMgmtData->u32Services) & u32ServiceRq)
			{
				sAppData.u64ServiceAddress = psStackMgmtData->u64SrcAddress;
				sAppData.eAppState = APP_STATE_SERVICE_REQUEST_RETURNED;
			}
			else
			{
				vUtils_Debug("<-wrong service");
			}
			break;

		default:
			vUtils_DisplayMsg("--wrong state", sAppData.eAppState);
			break;
		}
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
		// Mise a 0 des addresses
		//memset(eeprom.BoxAddr, 0, sizeof(eeprom.BoxAddr));
		//eeprom.nbBoite = 0;
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
	tsData *psData = (tsData *) pvEventPrim;
	teAppState eState = APP_STATE_ATTENDRE_BOITES;
	uint8 mode = 0;
	uint8 clav = 0;
	uint8 conf= 0;

	switch(eEventType)
	{
	case E_JENIE_DATA:{
		vPrintf("< Msg de[%x:%x] sur %d octets\n",
				(uint32)(psData->u64SrcAddress >> 32),
				(uint32)(psData->u64SrcAddress &  0xFFFFFFFF),
				psData->u16Length
		);

		switch(sAppData.eAppState)
		{
		case APP_STATE_ATTENDRE_BOITES:
		{
			if(psData->u16Length){
				// Recuperation de la boite Id
				IncomingBoxId = psData->pau8Data[psData->u16Length-1];

				// est elle valide
				if(IncomingBoxId < C_MAX_BOXES){
					// verifier que l'on ne connait pas deja cette @
					//if(eeprom.BoxAddr[IncomingBoxId]==0xffffffffffffffffULL){
					if(eeprom.BoxAddr[IncomingBoxId]==0ULL){
						vPrintf("    Cette boite [%d] est nouvelle\n",IncomingBoxId);
						vPrintf("    Je la memorise\n");

						eeprom.BoxAddr[IncomingBoxId]=psData->u64SrcAddress;
						// Une boite de plus dans le clavier
						eeprom.nbBoite++;
					}
					else
					{
						vPrintf("    Je connais a priori cette boite\n");
						if(eeprom.BoxAddr[IncomingBoxId]!=psData->u64SrcAddress)
						{
							vPrintf("     La config a changee\n");
							eeprom.BoxAddr[IncomingBoxId]=psData->u64SrcAddress;
						}
						else
						{
							vPrintf("     C'est bien celle que je connais\n");
						}
					}
					vPrintf("    Selectionner une touche\n\n");
					bConfStart = TRUE;
					au8Led[0].mode=E_FLASH_BP_EN_CONFIGURATION_SORTIES;
					eState = APP_STATE_ATTENDRE_SEL_MODE_AND_KBD;
				}
				else
				{
					vPrintf("Ce n'est pas une box id valide\n");
					vPrintf("On attend toujours une box\n");
					//eState =
				}
			}
			sAppData.eAppState=eState;
		}
		break;

		case APP_STATE_ATTENDRE_RETOUR_CFG_BOITE:
		{
			//bConfStart = TRUE;

			switch(psData->pau8Data[psData->u16Length-3])
			{
			case E_MSG_CFG_LIENS:
			{
				mode = psData->pau8Data[psData->u16Length-2]>>4 &0x0F;
				clav = psData->pau8Data[psData->u16Length-2] & 0x0F;
				conf = psData->pau8Data[psData->u16Length-1];

				vPrintf("    Reception config liens\n");
				vPrintf("     Box:%d, Mode:%d, clav:%d, conf:%x\n\n",
						IncomingBoxId,mode,clav,conf);

				eeprom.netConf.boxData[mode][clav][IncomingBoxId] = conf;
				sAppData.eAppState = APP_STATE_ATTENDRE_SEL_MODE_AND_KBD;
			}
			break;

			default:
			{
				vPrintf("!!Msg de la boite non compris\n\n");
			}
			break;
			}
		}
		break;

		case APP_STATE_ATTENDRE_SEL_MODE_AND_KBD:
		{
			//bConfStart = TRUE;
			if(psData->u16Length == 3){

				switch(psData->pau8Data[psData->u16Length-3])
				{

				case E_MSG_CFG_BOX_END:
				{
					vPrintf("     Fin config boite %d\n\n",IncomingBoxId);
					sAppData.eAppState =APP_STATE_MK_KBD_ALL;
				}
				break;

				default:
				{
					vPrintf("FIN CFG Msg de la boite non compris\n");
				}
				break;
				}

			}

		}
		break;

		default:
		{
			vPrintf("App State confuse:%d\n",sAppData.eAppState);
		}
		break;
		}
	}
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
	PRIVATE uint8 it20tick = 0;

	switch (u32DeviceId)
	{
	case E_JPI_DEVICE_TICK_TIMER:
	{
		// Gestion Led
		PBAR_ClignoteLed_1();

		// touche mode appuyee
		if(OneIt20)
		{

			it20tick++;
			if(it20tick==30)
			{
				it20tick = 0;

				// Lire etat clavier
				if(!IsBitSet(PBAR_ScanKey(),9))
				{
					vAHI_DioInterruptEdge(E_JPI_DIO20_INT,0);
					TimingIo_20 = 0;
					bStartTimerIo_20 = TRUE;
				}
				else{
					vAHI_DioInterruptEdge(0,E_JPI_DIO20_INT);
					bStartTimerIo_20 = FALSE;
				}
				OneIt20 = FALSE;
				eKeyTouched = E_KPD_MODE;
			}
		}

		if (bStartTimerIo_20)
			TimingIo_20++;

		switch(sAppData.eAppState)
		{
		case APP_STATE_RUNNING:
		{
			//PBAR_ClignoterLedNFois(mLedId,1);

			if(bStartTimerIo_19)
				TimingIo_19++;


			if(bStartTimerMode)
				tempoMode++;
		}
		break;

		default:
		{
			; //rien
		}
		break;
		}
	}
	break;

	default:
	{
		vUtils_DisplayMsg("HWint: ", u32DeviceId);
	}
	break;
	}
}

PRIVATE void PBAR_SetModeKey(void)
{
	PBAR_E_KeyMode *doMode;

	PRIVATE char const *msg[2]=
	{
			"usr:",
			"cfg:"
	};


	if(au8Led[1].actif == FALSE){

		if(eTypeUsage != E_CLAV_EN_CONFIG)
		{
			doMode = &eKeyMode;
		}
		else
		{
			doMode = &eModeOnConf;
		}

		switch(*doMode)
		{
		case E_CLAV_MODE_NOT_SET:
		{
			vPrintf("%s DEFAULT set\n",msg[eTypeUsage-1]);
			*doMode=E_CLAV_MODE_DEFAULT;
		}
		break;

		case E_CLAV_MODE_DEFAULT:
		{
			vPrintf("%s DEFAULT -> M1\n",msg[eTypeUsage-1]);
			*doMode=E_CLAV_MODE_1;
			vAHI_DioSetOutput(C_BAR_LED_2,0);
			vAHI_DioSetOutput(0,C_BAR_LED_3);
		}
		break;

		case E_CLAV_MODE_1:
			vPrintf("%s M1 -> M2\n",msg[eTypeUsage-1]);
			*doMode=E_CLAV_MODE_2;
			vAHI_DioSetOutput(0,C_BAR_LED_2);
			vAHI_DioSetOutput(C_BAR_LED_3,0);
			break;

		case E_CLAV_MODE_2:
			vPrintf("%s M2 -> M3\n",msg[eTypeUsage-1]);
			*doMode=E_CLAV_MODE_3;
			vAHI_DioSetOutput(C_BAR_LED_2,0);
			vAHI_DioSetOutput(C_BAR_LED_3,0);
			break;

		case E_CLAV_MODE_3:
			vPrintf("%s M3 -> DEFAULT\n",msg[eTypeUsage-1]);
			*doMode=E_CLAV_MODE_DEFAULT;
			break;

		case E_CLAV_MODE_END:
			vPrintf("\n!!%s Retour en mode default\n",msg[eTypeUsage-1]);
			*doMode=E_CLAV_MODE_DEFAULT;
			break;

		default:
			vPrintf("%s Erreur clavier mode %d!!\n Retour en mode par defaut\n"
					,msg[eTypeUsage-1],
					eKeyMode);
			*doMode=E_CLAV_MODE_DEFAULT;
			break;
		}

		if((eTypeUsage != E_CLAV_EN_CONFIG )&& (*doMode!=E_CLAV_MODE_DEFAULT)){

			// etait on deja dans un mode particulier
			if(bStartTimerMode)
				tempoMode = 0;

			bStartTimerMode = TRUE;
		}

		if (*doMode == E_CLAV_MODE_DEFAULT){
			// eteindre les leds de mode
			vAHI_DioSetOutput(0,C_BAR_LED_2|C_BAR_LED_3);
		}
	}
	else
	{
		vPrintf(" Ce clavier n'est pas configure\n");
	}

}



PUBLIC void vDIOInterruptCallback (uint32 u32Device, uint32 u32ItemBitmap)
{
	PRIVATE bool_t bPsgIoUp19 = FALSE;

	if (u32Device == E_JPI_DEVICE_SYSCTRL){
		switch(u32ItemBitmap)
		{
		case E_JPI_DIO11_INT:
		{
			eKeyTouched = E_KPD_A;
		}
		break;

		case E_JPI_DIO12_INT:
		{
			eKeyTouched = E_KPD_B;
		}
		break;
		case E_JPI_DIO13_INT:
		{
			eKeyTouched = E_KPD_C;
		}
		break;
		case E_JPI_DIO14_INT:
		{
			eKeyTouched = E_KPD_D;
		}
		break;
		case E_JPI_DIO15_INT:
		{
			eKeyTouched = E_KPD_1;
		}
		break;
		case E_JPI_DIO16_INT:
		{
			eKeyTouched = E_KPD_2;
		}
		break;
		case E_JPI_DIO17_INT:
		{
			eKeyTouched = E_KPD_3;
		}
		break;
		case E_JPI_DIO18_INT:
		{
			eKeyTouched = E_KPD_4;
		}
		break;

		case E_JPI_DIO19_INT:
		{
			if((bStartTimerIo_19 == FALSE) && (TimingIo_19 == 0)){
				vPrintf(" IO_19 Down\n");
				// Demarrer compteur
				bStartTimerIo_19 = TRUE;
				bPsgIoUp19 = FALSE;
				// Positionner it sur front montant
				vAHI_DioInterruptEdge(E_JPI_DIO19_INT,0);
			}
			else{
				//vPrintf("Timing:%d\n",TimingIo_19);
				if(TimingIo_19>TEMPS_IT_19 && (bPsgIoUp19 == FALSE)){
					vPrintf(" IO_19 Up\n");
					// Antirebond relachement
					bPsgIoUp19 = TRUE;
					// Arret compteur
					bStartTimerIo_19 = FALSE;
					// Positionner it sur front descendant
					vAHI_DioInterruptEdge(0,E_JPI_DIO19_INT);
				}
			}
			eKeyTouched = E_KPD_ALL;
		}
		break;

		case E_JPI_DIO20_INT:
		{

			if (OneIt20 == FALSE){
				OneIt20 = TRUE;
			}

		}
		break;

		default:
		{
			eKeyTouched = E_KPD_NONE;
		}
		break;
		}
	}
}

// ------------------------------------------------------------
// ------------------------------------------------------------

PRIVATE bool_t VerifierExistanceBoite(PBAR_E_KeyMode eMode,
		PBAR_KIT_8046 eKey,uint8 BoxId)
{
	bool_t eReturn = FALSE;
	uint8 i;
	uint8 nbBox = 0;
	uint8 useBox = 0;

	vPrintf(" Box:%d, Mode:%d connue ?\n", BoxId,eMode-E_CLAV_MODE_DEFAULT);

	// Mode et clavir valide ?
	if( (eMode>E_CLAV_MODE_NOT_SET) &&
			(eMode<E_CLAV_MODE_END && eKey < E_KPD_NONE)){

		nbBox = eeprom.netConf.ptr_boxList[eMode-E_CLAV_MODE_DEFAULT][eKey];
		vPrintf(" Recherche\n");
		// Ok
		// A t on une boite associe a la touche dans le mode
		for(i=0;(i<=nbBox)&& (i<C_MAX_BOXES);i++)
		{
			useBox = eeprom.netConf.boxList[(eMode-E_CLAV_MODE_DEFAULT)][eKey][i];
			vPrintf("  Pos:%d, boxid : %d\n",i,useBox);

			if(useBox == 0x00){
				// On a parcouru toute la liste
				// sauvegarde de la boite associe a la touche
				vPrintf("   Sauvegarde necessaire!\n");
				eeprom.netConf.boxList[eMode-E_CLAV_MODE_DEFAULT][eKey][i]=IncomingBoxId;
				eeprom.netConf.ptr_boxList[eMode-E_CLAV_MODE_DEFAULT][eKey]++;
				break;
			}
			else
			{
				if(useBox == IncomingBoxId){
					vPrintf("   Connait déja!\n");
					eReturn = TRUE;
					break;
				}
				else{
					vPrintf("  On regarde avec la suivante\n");
					continue;
				}
			}
		}
		vPrintf(" Recherche Terminee\n\n");
	}

	return eReturn;
}
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
