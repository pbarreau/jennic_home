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

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vPRT_TraiterChangementEntree(uint32 val);
PRIVATE uint8 showDipSwitch(void);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
PUBLIC ebpLedInfo mNetOkTypeFlash = E_FLASH_RESEAU_ACTIF;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/* Routing table storage */
PRIVATE tsJenieRoutingTable asRoutingTable[100];

PRIVATE uint8 	pBuff[2]; // pointeur In & Read du buffer
PRIVATE uint8 	bufReception[PBAR_RBUF_SIZE]; // Buffer circulaire reception
PRIVATE uint64	bufAddr[PBAR_RBUF_SIZE];
PRIVATE uint8 	buf2[3] ={0,0,0};

// Pour config Clavier distant
PRIVATE uint8 prevConf = 0;

PRIVATE uint16 TimeRechercheClavier = 0;

PRIVATE uint8 	etatSorties; // reflet des ios actuel



/****************************************************************************/
PRIVATE uint8 showDipSwitch(void)
{
  uint32 val = 0L;
  uint8 uboxid = 0;

  // Set IO In
  vAHI_DioSetDirection(E_JPI_DIO0_INT |\
      E_JPI_DIO1_INT |\
      E_JPI_DIO8_INT |\
      E_JPI_DIO9_INT,0);

  val = u32AHI_DioReadInput();
  // Recuperer la valeur de conf de la boite
  uboxid = ((uint8)((val>>6)&0x0C) |((uint8)val&0x03));

#if !NO_DEBUG_ON
  /* Open UART for printf use {v2} */
  vUART_printInit();
  /* Initialise utilities */
  vUtils_Init();
#endif

  // Detection type de boite
  vPrintf("!!Box Id set by dip switch : %d\n\n", uboxid);

  return(uboxid);
}


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
  uThisBox_Id = showDipSwitch();

  /* Change default network config */
  if(!uThisBox_Id)
  {
    gJenie_Channel              = 0;
    gJenie_NetworkApplicationID = 0xbadebade;
    gJenie_PanID                = 0x1111;
  }
  else
  {
    gJenie_Channel              = PBAR_CHANNEL;
    gJenie_NetworkApplicationID = PBAR_NID;
    gJenie_PanID                = PBAR_PAN_ID;
  }

  /* Configure stack with routing table data */
  gJenie_RoutingEnabled    = TRUE;
  gJenie_RoutingTableSize  = PBAR_RTBL_SIZE;
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
  teJenieStatusCode eStatus; /* Jenie status code */
  teJenieDeviceType eDevType = E_JENIE_ROUTER;

  // Reset Buffer Application
  memset(&bufReception, 0, sizeof(bufReception));
  memset(&pBuff, 0, sizeof(pBuff));

  // Reset APP_STATES
  memset(&sAppData, 0, sizeof(sAppData));

  u32AHI_Init();

#if !NO_DEBUG_ON
  vUtils_Init();
  vUART_printInit();
#endif

  vPRT_Init_IosOfCard(E_BUS_400_KH);

  switch(uThisBox_Id)
  {
    case 0:
    {
      vPrintf("Mode test des sorties\n");
      eDevType = E_JENIE_COORDINATOR;
    }
    break;
    case 1:
    {
      vPrintf("Mode Installation Coordonateur\n");
      eDevType = E_JENIE_COORDINATOR;
    }
    break;

    case 2:
    {
      vPrintf("Mode Installation Passerelle\n");
    }
    break;

    default :
    {
      vPrintf("Mode Installation Routeur\n");
    }
    break;
  }

  // Activation de la led status
  au8Led[C_LID_1].actif = TRUE;
  au8Led[C_LID_1].pio = C_LPID_1;


  if((eStatus=eJenie_Start(eDevType)) != E_JENIE_SUCCESS)
  {
    au8Led[C_LID_1].mode = E_FLASH_ERREUR_DECTECTEE;

    vPrintf("!!Jenie err: %d\n", eStatus);
    while(1);
  }
  else
  {
    vPrintf("Mise en place du reseau !!\n\n");
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
  teJenieStatusCode eStatus;
  uint8 keep;
  uint8 mask;
  uint8 valu;

  /* regular watchdog reset */
#ifdef WATCHDOG_ENABLED
  vAHI_WatchdogRestart();
#endif

  switch(sAppData.eAppState)
  {
    case APP_STATE_WAITING_FOR_NETWORK:
      /* nothing to do till network is up and running */
      au8Led[0].mode = E_FLASH_RECHERCHE_RESEAU;
      break;

    case APP_STATE_NETWORK_UP:
      if(uThisBox_Id == 0)
      {
        au8Led[0].mode = E_FLASH_BP_TEST_SORTIES;
        sAppData.eAppState = APP_STATE_TST_START_LUMIERES;
      }
      else
      {
        vPrintf(" Possibilite d'association activee\n\n");
        eJenie_SetPermitJoin(TRUE);

        // Reseau actif on peut recevoir des donnees !!
        au8Led[0].mode = mNetOkTypeFlash;

        // Pas de clavier distant
        LaBasId = 0;

        /* register services */
        sAppData.eAppState = APP_STATE_REGISTERING_SERVICE;

      }
      break;

    case APP_STATE_REGISTERING_SERVICE:
    {
      /* services disponible aux autres */
      vPrintf(" Enregistrement du service:%x, ",SRV_LUMIR|SRV_VOLET);
      eStatus = eJenie_RegisterServices(SRV_LUMIR|SRV_VOLET);
      switch (eStatus)
      {
        case E_JENIE_SUCCESS:
          vPrintf("ok\n");
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

      /* go to the running state */
      sAppData.eAppState = APP_STATE_RUNNING;
    }
    break;

    case APP_STATE_TRAITER_INPUT_MESSAGE:
    {
      if(pBuff[1] == PBAR_RBUF_SIZE)
        pBuff[1]=0;
      // 1er octet 0 -> impose
      // 2em octet id bit sur lesquel agir
      // 3em octet config des bits
      mask = bufReception[pBuff[1]+1];
      valu = bufReception[pBuff[1]+2];

      vPrintf(" Ptr Lecture:%d\n",pBuff[1]);

      vPrintf(" Buffer:%x,%x,%x\n",
          bufReception[pBuff[1]],
          bufReception[pBuff[1]+1],
          bufReception[pBuff[1]+2]);

      switch(bufReception[pBuff[1]])
      {
        case E_MSG_DATA_ALL:
        {
          vPrintf(" Data ALL ios actuel:%x\n",etatSorties);

          keep = (etatSorties & (~mask))|(mask & valu);
          vPrintf(" Mask:%x,Value:%x\n",mask,valu);
          vPrintf(" Nouvelle config ios:%x\n",keep);
          etatSorties = keep;

          // Configuer les sorties
          vPRT_DioSetOutput(etatSorties<<PBAR_DEBUT_IO,(~etatSorties)<<PBAR_DEBUT_IO);
        }
        break;

        case E_MSG_DATA_SELECT:
        {
          vPrintf(" Data Spe ios actuel:%x\n",etatSorties);

          keep = (etatSorties ^ mask);
          vPrintf(" Mask:%x,Value:%x\n",mask,valu);
          vPrintf(" Nouvelle config ios:%x\n\n",keep);
          etatSorties = keep;

          // Configuer les sorties
          vPRT_DioSetOutput(etatSorties<<PBAR_DEBUT_IO,(~etatSorties)<<PBAR_DEBUT_IO);
        }
        break;

        case E_MSG_ASK_ID_BOX:
        {
          bufEmission[0] = E_MSG_RSP_ID_BOX;
          bufEmission[1] = bufReception[pBuff[1]+1];
          bufEmission[2] = uThisBox_Id;

          // etablissement de lien
          cbUnClavierActif = TRUE;

          vPrintf("Rsp a demande Box Id:\n D:[%x:%x], Msg:%x, %x, %x\n",
              (uint32) (bufAddr[pBuff[1]] >> 32),
              (uint32) (bufAddr[pBuff[1]] & 0xFFFFFFFF),
              bufEmission[0],
              bufEmission[1],
              bufEmission[2]
          );
          // renvoyer la reponse
          eJenie_SendData(bufAddr[pBuff[1]],
              bufEmission, 3,
              TXOPTION_SILENT);
        }
        break;

        default:
        {
          vPrintf("Erreur sur type message recu\n");
        }
        break;
      }
      pBuff[1]+=3;

      sAppData.eAppState = APP_STATE_RUNNING;
    }
    break;

    case APP_STATE_RECHERCHE_CLAVIER:
    {
      vPrintf("\nRecherche d'un boitier de commande disponible\n");

      // Recherche de la boite ayant le service:
      // clavier_conf positionne
      eJenie_RequestServices(SRV_INTER, TRUE);
      au8Led[0].mode= E_FLASH_RECHERCHE_BC;

      sAppData.eAppState = APP_STATE_ATTENTE_CLAV_RSP;
    }
    break;

    case APP_STATE_ATTENTE_CLAV_RSP:
    {
      ;//Rien
    }
    break;

    case APP_STATE_REPONSE_CLAVIER_TROP_LONG:
    {
      vPrintf("Attente boitier commande trop longue!!\n");
      vPrintf("Verifier si en mode programmation\n");
      vPrintf("Reour BP en mode normal\n");
      au8Led[0].mode=mNetOkTypeFlash;
      sAppData.eAppState = APP_STATE_RUNNING;
    }
    break;


    case APP_STATE_CLAV_READY:
    {
      au8Led[0].mode=E_FLASH_LIAISON_BP_BC_ON;
      PBAR_LireBtnPgm();
    }
    break;

    case APP_STATE_SET_MY_OUTPUT:
    {


      vPrintf(" Config actuelle:%x\n",config);
      // Mettre les sorties au niveau de config
      vPRT_DioSetOutput(config<<PBAR_DEBUT_IO,(~config)<<PBAR_DEBUT_IO);

      vPrintf(" En attente de changement programmation\n");
      au8Led[0].mode= E_FLASH_BP_EN_CONFIGURATION_SORTIES;
      ePgmMode = E_CLAV_MODE_1;
      sAppData.eAppState = APP_STATE_ATTENDRE_FIN_CFG_LOCAL;
    }
    break;

    case APP_STATE_ATTENDRE_FIN_CFG_LOCAL:
    {
      PBAR_LireBtnPgm();
    }
    break;

    case APP_STATE_FIN_CFG_BOX:
    {
      vPrintf("Deconnection du boitier de commande\n");
      vPrintf("Retour de BP en mode usage courant\n");

      // On Montre mode user
      au8Led[0].mode= E_FLASH_RESEAU_ACTIF;

      ePgmMode = E_CLAV_MODE_NOT_SET;

      bufEmission[0] = E_MSG_CFG_BOX_END;
      bufEmission[1]= 0;
      bufEmission[2]= 0;

      eJenie_SendData(LaBasId,
          bufEmission, 3,
          TXOPTION_SILENT);

      LaBasId = 0;
      sAppData.eAppState = APP_STATE_RUNNING;

    }
    break;

    case APP_STATE_RUNNING:
    {
      ; // Rien attendre evenement reseau ou clavier
      PBAR_LireBtnPgm();
    }
    break;

    case APP_STATE_TST_START_LUMIERES:
    {
      PBAR_DecodeBtnPgm(&valu);
    }
    break;

    case APP_STATE_TST_STOP_LUMIERES:
    {
    }
    break;

    default:
#if !NO_DEBUG_ON
      vUtils_DisplayMsg("!!Unknown state!!", sAppData.eAppState);
      while(1);
#endif
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
  tsChildJoined *psStackMgmtData = (tsChildJoined *) pvEventPrim;

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
      vPrintf("Enregistrement differre ??\n");
    }
    break;

    case E_JENIE_SVC_REQ_RSP:
      vPrintf(" > Trouve : ");
      {
        if(sAppData.eAppState == APP_STATE_ATTENTE_CLAV_RSP)
        {
          // Stop Timer
          cbStartTempoRechercheClavier = FALSE;
          TimeRechercheClavier = 0;

          vPrintf
          ("Boitier [%x:%x]\n",
              (uint32) (psStackMgmtData->u64SrcAddress >> 32),
              (uint32) (psStackMgmtData->u64SrcAddress & 0xFFFFFFFF)
          );

          // Memorisation du @ clavier
          LaBasId = psStackMgmtData->u64SrcAddress;

          // Indiquer mon numero de boite au clavier
          vPrintf("    Envoie de mon box id:%d\n\n",uThisBox_Id);
          vPrintf("En attente d'une touche du Boitier de commande\n");
          bufEmission[0]=uThisBox_Id;
          eJenie_SendData(LaBasId,
              bufEmission, 1,TXOPTION_SILENT);
          /// ZZZZZZZZZZZZZZZZZZZ
          /// Avec ack ou sans ack ? TXOPTION_ACKREQ);
          // On montre Led
          au8Led[0].mode= E_FLASH_RECHERCHE_BC;

          sAppData.eAppState = APP_STATE_CLAV_READY;

        }

      }
      break;


    case E_JENIE_PACKET_SENT:
      vPrintf("> Packet sent\n");
      break;

    case E_JENIE_PACKET_FAILED:
      vPrintf("Packet failed\n");
      break;

    case E_JENIE_CHILD_JOINED:
    {
      vPrintf
      ("> Arrivage d'un fils ->[%x:%x]\n",
          (uint32) (psStackMgmtData->u64SrcAddress >> 32),
          (uint32) (psStackMgmtData->u64SrcAddress & 0xFFFFFFFF));
    }
    break;

    case E_JENIE_CHILD_LEAVE:
      vPrintf("Child Leave\n");
      break;

    case E_JENIE_CHILD_REJECTED:
      vPrintf("Child Rejected\n");
      break;

    case E_JENIE_STACK_RESET:
      vPrintf("Stack Reset\n");
      sAppData.eAppState = APP_STATE_WAITING_FOR_NETWORK;
      break;

    default:
      /* Unknown data event type */
#if !NO_DEBUG_ON
      vUtils_DisplayMsg("!!Unknown Mgmt Event!!", eEventType);
      while(1);
#endif
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
 * Impose
 ****************************************************************************/
PUBLIC void vJenie_CbStackDataEvent(teEventType eEventType, void *pvEventPrim)
{
#if !NO_DEBUG_ON
  tsDataToService *psStackEventData = (tsDataToService *) pvEventPrim;
#endif

  tsData *psData = (tsData *) pvEventPrim;

  switch(eEventType)
  {
    case E_JENIE_PACKET_SENT:
      vPrintf("Mesg envoyee\n");
      break;

    case E_JENIE_PACKET_FAILED:
      vPrintf("Mesg echoue\n");
      break;

    case E_JENIE_DATA:
    {
      /* Get pointer to correct primitive structure */
      /* Output to UART */
      vPrintf("\nMsg du noeud[%x:%x] sur %d octets\n",
          (uint32)(psData->u64SrcAddress >> 32),
          (uint32)(psData->u64SrcAddress &  0xFFFFFFFF),
          psData->u16Length
      );

      switch(sAppData.eAppState)
      {
        case APP_STATE_CLAV_READY:
        {
          if(psData->u16Length == 3){
            buf2[0]= psData->pau8Data[psData->u16Length-3];
            buf2[1]= psData->pau8Data[psData->u16Length-2];
            buf2[2]= psData->pau8Data[psData->u16Length-1];

            vPrintf(" Msg: %x,%x,%x\n",buf2[0],buf2[1],buf2[2]);

            switch(buf2[0]){
              case E_MSG_CFG_LIENS:
              {
                config=buf2[2];
                prevConf = config;
                LabasKbd = buf2[1] & 0x0F;
                LabasMod = buf2[1]>>4 & 0xF;
                vPrintf("  ie: touche '%d', mode '%d'\n",LabasKbd,LabasMod);

                sAppData.eAppState = APP_STATE_SET_MY_OUTPUT;
              }
              break;

              default:
              {
                vPrintf("Message clavier non compris\n");
              }
              break;

            }
          }
        }
        break;

        case APP_STATE_RUNNING:
        {
          // Mettre info dans buffer circulaire pour traitement
          if(pBuff[0] == PBAR_RBUF_SIZE)
            pBuff[0]=0;

          // Addresse emetteur
          bufAddr[pBuff[0]]=psData->u64SrcAddress;

          // Debut format message (sur 3 octets)
          // octet 0
          bufReception[pBuff[0]]=psData->pau8Data[psData->u16Length-3];
          pBuff[0]++;

          // octel 1
          bufReception[pBuff[0]]=psData->pau8Data[psData->u16Length-2];
          pBuff[0]++;

          // octel 2
          bufReception[pBuff[0]]=psData->pau8Data[psData->u16Length-1];
          pBuff[0]++;
          // Fin format message

          // Traitement
          sAppData.eAppState = APP_STATE_TRAITER_INPUT_MESSAGE;

        }
        break;
        default:
        {
          vPrintf("Reception donnee non prevu !!\n");
        }
        break;
      }

    }
    break;

    case E_JENIE_DATA_TO_SERVICE:
      vPrintf("Data to service event\n");

      vPrintf("S:%d -> d:%d\n",psStackEventData->u8SrcService,psStackEventData->u8DestService);
      break;

    case E_JENIE_DATA_ACK:
    {
      //vPrintf("\n> Data ack");
      switch(sAppData.eAppState)
      {
        case APP_STATE_ATTENDRE_FIN_CFG_LOCAL:
        {
          vPrintf("Configuration touche terminee\n");
          vPrintf("En attente autre touche du boitier de commande\n");

			// On efface la config visible
			// Mettre les sorties a 0
			config = 0;
			// On quitte le mode test: eteindre les lumieres
			// vPrintf("switch off evrything\n");
			vPRT_DioSetOutput((config)<<PBAR_DEBUT_IO,(~config)<<PBAR_DEBUT_IO);

			// on reinitialise les registre interne
			etatSorties = 0;

          // On remet la led en normal
          au8Led[0].mode= E_FLASH_EN_ATTENTE_TOUCHE_BC;

          ledId = 0;
          ePgmMode = E_CLAV_MODE_NOT_SET;
          sAppData.eAppState = APP_STATE_CLAV_READY;
        }
        break;

        default:
        {
          vPrintf("Ack non prevu\n");
        }
        break;
      }
    }
    break;

    case E_JENIE_DATA_TO_SERVICE_ACK:
      vPrintf("Data to service ack\n");
      break;

    default:
      // Unknown data event type
#if !NO_DEBUG_ON
      vUtils_DisplayMsg("!!Unknown Data Event!!", eEventType);
      while(1);
#endif
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
  uint32 val = 0;

  switch (u32DeviceId)
  {
    case E_JPI_DEVICE_TICK_TIMER:
    {
      IHM_ClignoteLed();
      PBAR_LireBtnPgm();

      /* regular 10ms tick generated here */
      if (bStartPgmTimer){
        TimePgmPressed++;
      }


      /* regular 10ms tick generated here */

      if(cbStartTempoRechercheClavier){
        TimeRechercheClavier++;

        if(TimeRechercheClavier == 400){
          // On arrete de chercher
          cbStartTempoRechercheClavier = FALSE;
          TimeRechercheClavier = 0;
          sAppData.eAppState = APP_STATE_REPONSE_CLAVIER_TROP_LONG;
        }
      }
    }
    break;

    default:
#if !NO_DEBUG_ON
      vUtils_DisplayMsg("HWint: ", u32DeviceId);
      while(1);
#endif
      break;
  }
}

PRIVATE void vPRT_TraiterChangementEntree(uint32 val)
{
}
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
