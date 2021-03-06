/*
 * main.c
 *
 *  Created on: 11 juil. 2013
 *      Author: RapidOs
 */

#include <jendefs.h>
#include <JenNetApi.h>
#include <JPI.h>
#include <Jenie.h>

#include <Utilities.h>

#if !NO_DEBUG_ON
#include <Printf.h>
#include "Utils.h"
#endif

#include "m_config.h"
#include "clav2pc.h"
#include "e_config.h"

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void InitAFroid(void);
PRIVATE void APP_ConfigIoJennic(void);
PRIVATE void PBAR_ISR_Clavier_c3(uint32 u32Device, uint32 u32ItemBitmap);
PRIVATE void CLAV_GestionIts(void);
//PRIVATE etInUsingkey CLAV_AnalyseIts(uint8 *position);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
PUBLIC tsClavData AppData;
PUBLIC bpsFlash eeprom;

PUBLIC bool_t start_timer_of_mode = FALSE;
PUBLIC uint16 compter_duree_mode = 0;
PUBLIC eLedInfo mNetOkTypeFlash = E_FLASH_RESEAU_ACTIF;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE bool_t b_recherche_touche_en_cours = FALSE;
PRIVATE uint32 memo_its_down = 0UL;
//PRIVATE uint32 memo_its_up = 0UL;
PRIVATE eStatusClavier liaison_clavier = E_AUTONOMOUS;

PRIVATE bool_t anti_rebond_it = FALSE;
//PRIVATE bool_t b_It_tempo_enable = FALSE;

PRIVATE bool_t b_it_detect_front_descendant = FALSE;
//PRIVATE uint16 tempo_rebond = 0;

PRIVATE bool_t b_start_press_count = FALSE;
PUBLIC uint16 timer_appuie_touche = 0;
PUBLIC uint32 NEW_memo_delay_touche = 0;

PRIVATE const uint8 pioClavOut[] = { C_CLAV_PIO_OUT_1, C_CLAV_PIO_OUT_2,
C_CLAV_PIO_OUT_3, C_CLAV_PIO_OUT_4 };

PRIVATE const uint8 ligne_colonne[] = {

0xEE, 0xED, 0xEB, 0xE7, 0xDE, 0xDD, 0xDB, 0xD7, 0xBE, 0xBD, 0xBB, 0xB7, 0x7E,
    0x7D, 0x7B, 0x77

};
PRIVATE const etInUsingkey key_code[] = { E_KEY_NUM_1, E_KEY_NUM_4, E_KEY_NUM_7,
    E_KEY_NUM_ETOILE, E_KEY_NUM_2, E_KEY_NUM_5, E_KEY_NUM_8, E_KEY_NUM_0,
    E_KEY_NUM_3, E_KEY_NUM_6, E_KEY_NUM_9, E_KEY_NUM_DIESE, E_KEY_NUM_MOD_1,
    E_KEY_NUM_MOD_2, E_KEY_NUM_MOD_3, E_KEY_NUM_MOD_4 };
PUBLIC uint16 timer_touche[E_KEYS_NUM_END] = { 0 };

#if !NO_DEBUG_ON
PUBLIC const uint8 code_ascii[] = "147*2580369#ABCD";
#endif

// -----------------------------------------------------
// Bloc Jennic

// Etape 1
PUBLIC void vJenie_CbConfigureNetwork(void)
{
  /* Change default network config */
  gJenie_NetworkApplicationID = PBAR_NID;
  gJenie_PanID = PBAR_PAN_ID;

  gJenie_Channel = PBAR_CHANNEL;
  gJenie_ScanChannels = PBAR_SCAN_CHANNELS;

  gJenie_RoutingEnabled = FALSE;
  gJenie_EndDevicePollPeriod = 5;
  gJenie_EndDeviceScanSleep = 1000;
}

// Etape 2
PUBLIC void vJenie_CbInit(bool_t bWarmStart)
{

  teJenieStatusCode eStatus;

  // Initialisation API
  u32AHI_Init();

  // debug ?
#if !NO_DEBUG_ON
  vUART_printInit();
  vUtils_Init();
#endif

  if (bWarmStart == FALSE)
  {
    InitAFroid();
  }

  eStatus = eJenie_Start(E_JENIE_END_DEVICE);
  if (eStatus != E_JENIE_SUCCESS)
  {
    vPrintf("Impossible de demarrer pile Jenie -> Err=%d !!\n", eStatus);
    while (1)
      ;
  }
  else
  {
    vPrintf("Recherche reseau!\n");
    AppData.pgl = E_PGL_RECHERCHE_RESEAU;
    // Montrer la led d'activite reseau
    au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_RECHERCHE_RESEAU;
    au8Led_clav[C_CLAV_LED_INFO_1].actif = TRUE;

    // PAr defaut autre led utilise mais eteintes
    au8Led_clav[C_CLAV_LED_INFO_2].actif = TRUE;
    au8Led_clav[C_CLAV_LED_INFO_3].actif = TRUE;

    au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_OFF;
    au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_OFF;

  }

}

PUBLIC void vJenie_CbMain(void)
{
  /* regular watchdog reset */
#ifdef WATCHDOG_ENABLED
  vAHI_WatchdogRestart();
#endif

  switch (AppData.pgl)
  {
    case E_PGL_RECHERCHE_RESEAU:
      ; // Rien attendre
    break;

    case E_PGL_RESEAU_ETABLI:
      vPrintf("Connection Reseau etablit\n");
      // Montrer par flash que l'on est connecte
      au8Led_clav[C_CLAV_LED_INFO_1].mode = mNetOkTypeFlash;

      // Autoriser la prise en compte des its clavier
      vAHI_DioInterruptEnable(PBAR_CFG_NUMPAD_IN, 0);

      // etape suivante
      AppData.pgl = E_PGL_BOUCLE_PRINCIPALE;

    break;

    case E_PGL_BOUCLE_PRINCIPALE:
    {
      // Test pour voir si connection a un pc
      if (liaison_clavier == E_CONNECTED_TO_PC)
      {
        CLAV_AnalyserPc(AppData.ePcState);
      }
      else
      {
        // Usage autonome
        if (compter_duree_mode >= TEMPS_MODE)
        {
          start_timer_of_mode = FALSE;
          compter_duree_mode = 0;
          CLAV_GererMode(E_KEY_NUM_MOD_1);
        }
        CLAV_AnalyserEtat(AppData.stp);
      }
    }
    break;

    default:
      vPrintf("Machine a etat fini en boucle\n");
      while (1)
        ;
    break;

  }
}

PUBLIC void vJenie_CbStackMgmtEvent(teEventType eEventType, void *pvEventPrim)
{
  //tsSvcReqRsp *psStackMgmtData =(tsSvcReqRsp *)pvEventPrim;

  bp_CommunMsgReseau(&AppData.pgl, eEventType, pvEventPrim);

  switch (eEventType)
  {
    case E_JENIE_NETWORK_UP:
    {
      AppData.pgl = E_PGL_RESEAU_ETABLI;
    }
    break;

    case E_JENIE_POLL_CMPLT:
      // A traiter
    break;

    case E_JENIE_REG_SVC_RSP:
    {
      if (AppData.eClavState == E_KS_STP_SERVICE_ON)
      {
        vPrintf(" Le service est bien enregistre chez le pere\n\n");
        vPrintf("En attente de demande d'une boite\n\n");
        AppData.eClavState = E_KS_STP_ATTENDRE_BOITE;
      }
      else if (AppData.eClavState == E_KS_STP_SERVICE_OFF)
      {
        vPrintf(" Le service est bien Retire chez le pere\n\n");
        vPrintf("En attente de commandes utilisateur\n\n");
        AppData.eClavState = E_KS_STP_ATTENTE_TOUCHE;
      }
      else
      {
        vPrintf("ERR:E_JENIE_REG_SVC_RSP -> %s",
            dbg_teClavState[AppData.eClavState]);
      }

    }
    break;

    default:
    {
      vPrintf("ERR:vJenie_CbStackMGMTEvent ->%s\n",
          dbg_teEventType[eEventType]);
    }
    break;
  }
}

PUBLIC void vJenie_CbStackDataEvent(teEventType eEventType, void *pvEventPrim)
{
  tsData *psData = (tsData *) pvEventPrim;

  bp_CommunMsgReseau(&AppData.pgl, eEventType, pvEventPrim);

  switch (eEventType)
  {
    case E_JENIE_DATA:
    {
      switch (AppData.pgl)
      {
        case E_PGL_BOUCLE_PRINCIPALE:
        {
          CLAV_NetMsgInput(psData);
        }
        break;

        default:
        {
          vPrintf("ERR:E_JENIE_DATA, AppData.eAppState -> %d\n", AppData.pgl);
        }
        break;
      }

    }
    break;

    case E_JENIE_PACKET_SENT:
      vPrintf("Msg emis\n");
    break;

    default:
    {
      vPrintf("ERR:vJenie_CbStackDATAEvent ->%s\n",
          dbg_teEventType[eEventType]);
    }
    break;

  }

}

PUBLIC void vJenie_CbHwEvent(uint32 u32DeviceId, uint32 u32ItemBitmap)
{
  static uint16 tempo_enb_it = 0;
  static uint16 temps_bip = 0;

  switch (u32DeviceId)
  {
    case E_JPI_DEVICE_TICK_TIMER:
    {
      IHM_ClignoteLed();

      if (b_start_press_count)
      {
        timer_appuie_touche++;
      }

      if (start_timer_of_mode == TRUE)
      {
        compter_duree_mode++;
      }

      if (b_activer_bip == TRUE)
      {
        vAHI_DioSetOutput(C_CLAV_BUZER,0);
        temps_bip++;
        if (temps_bip >= C_TEMPS_BIP)
        {
          // Eteindre buzzer
          b_activer_bip = FALSE;
          temps_bip = 0;
          vAHI_DioSetOutput(0, C_CLAV_BUZER);
        }
      }

      if (anti_rebond_it)
      {
        tempo_enb_it++;
        if (tempo_enb_it >= C_TEMPS_ANTI_REBOND)
        {
          tempo_enb_it = 0;
          anti_rebond_it = FALSE;
          CLAV_GestionIts();
        }

      }
    }
    break;
    default:
      vPrintf("Evenement hardware no prevu:%x\n", u32DeviceId);
    break;
  }
}

// -----------------------------------------------------------------
PRIVATE void APP_ConfigIoJennic(void)
{
  // Sorties pour led
  vAHI_DioSetDirection(0, PBAR_CFG_LED);
  // Activation pull up
  vAHI_DioSetPullup(PBAR_CFG_LED, 0);
  // Mettre a un on pour eteindre les leds
  vAHI_DioSetOutput(PBAR_CFG_LED, 0);

  // Sortie du buzzer
  vAHI_DioSetDirection(0, C_CLAV_BUZER);
  // Activation pull up
  vAHI_DioSetPullup(0, C_CLAV_BUZER);
  // Mettre a un on pour eteindre le buzzer
  vAHI_DioSetOutput(0, C_CLAV_BUZER);

  // Gestion clavier
  vAHI_DioSetDirection(PBAR_CFG_NUMPAD_IN, PBAR_CFG_NUMPAD_OUT);
  // Activation pull up sur entrees
  vAHI_DioSetPullup(PBAR_CFG_NUMPAD_IN, PBAR_CFG_NUMPAD_OUT);
  // Mise a off sur le port colonne du clavier
  vAHI_DioSetOutput(0, PBAR_CFG_NUMPAD_OUT);
  // Preparation Detection Front descendant sur entrees
  vAHI_DioInterruptEdge(0, PBAR_CFG_NUMPAD_IN);
  // Mise en place de l'isr
  vAHI_SysCtrlRegisterCallback(PBAR_ISR_Clavier_c3);
  // L'activation de lecture des fronts est faite ailleurs

}

// Routine de lecture du clavier matriciel sous IT
PRIVATE void PBAR_ISR_Clavier_c3(uint32 u32Device, uint32 u32ItemBitmap)
{

  if (u32Device == E_AHI_DEVICE_SYSCTRL)
  {
    switch (u32ItemBitmap)
    {
      case E_AHI_DIO12_INT:
      case E_AHI_DIO13_INT:
      case E_AHI_DIO14_INT:
      case E_AHI_DIO15_INT:
      {
        switch (AppData.stp)
        {
          case E_KS_STP_ATTENTE_TOUCHE:
          {
            vPrintf("1:Down\n");
            memo_its_down = u32ItemBitmap;
            AppData.stp = E_KS_STP_DEBUT_IT;
            timer_antirebond_dow = 0;
            b_DebutIt = TRUE;
          }
          break;

          case E_KS_STP_COMPTER_DUREE_PRESSION:
          {
            b_compter_pression = FALSE;
            NEW_memo_delay_touche = timer_duree_pression / 100;
            // Informer a gerer
            vPrintf("2:UP->time used '%d'\n", NEW_memo_delay_touche);
            AppData.stp = E_KS_STP_REBOND_HAUT_COMMENCE;
            timer_antirebond_up = 0;
            b_FinIt = TRUE;
          }
          break;

          case E_KS_STP_DEBUT_IT:
          case E_KS_STP_REBOND_HAUT_COMMENCE:
          case E_KS_STP_TRAITER_TOUCHE:
          {
            ; // It Parasite
          }
          break;

          default:
          {
            vPrintf("Step sous it non prevu:%d\n", AppData.stp);
          }
          break;

        }

      }
      break;

      default:
        vPrintf("It non prevue:%x\n", u32ItemBitmap);
      break;
    }
  }
}

PRIVATE void CLAV_GestionIts(void)
{
  etInUsingkey la_touche = E_KEY_NON_DEFINI;
  uint8 posCodeAscii = 0;

  if (b_it_detect_front_descendant == TRUE)
  {
    la_touche = CLAV_AnalyseIts(&posCodeAscii);
    if (la_touche != E_KEY_NON_DEFINI)
    {
      vPrintf("Touche '%c' trouvee, code dans pgm:%d\n",
          code_ascii[posCodeAscii], la_touche);
      AppData.key = la_touche;
      AppData.ukey = posCodeAscii;

      vPrintf("Debut mesure du temps d'appuie\n");
      b_start_press_count = TRUE;

      AppData.eClavState = E_KS_STP_TRAITER_IT;

      // Detection passage down -> up (front montant)
      vAHI_DioInterruptEdge(PBAR_CFG_NUMPAD_IN, 0);
    }
    else
    {
      vPrintf("la_touche == E_NO_KEYS\n");
      b_it_detect_front_descendant = FALSE;
      AppData.eClavState = E_KS_STP_ATTENTE_TOUCHE;
    }
  }
  else
  {
    b_start_press_count = FALSE;
    // a 't on un temps minimum d'appui
    if (timer_appuie_touche >= C_MIN_DURE_PRESSION)
    {
      // Calcul du temps d'appuie de la touche + bip + traitement
      timer_touche[AppData.key - 1] = timer_appuie_touche;
      vPrintf(" Duree maintient de touche '%c'= %d\n\n",
          code_ascii[AppData.ukey], timer_touche[AppData.key - 1]);

      // Memorisation de l'etat avant traitement de la touche
      //AppData.ePrevClav = AppData.eClavState;

      // Un appui puis un relachement on ete detectee et minutee
      //on effectue une action
      AppData.eClavState = E_KS_STP_TRAITER_TOUCHE;
    }
    else
    {
      vPrintf("Temps Pression insuffisant:%d\n", timer_appuie_touche);
      vPrintf("Retour en attente pression touche\n");
      // Ignorer la touche
      AppData.key = E_KEY_NON_DEFINI;
      AppData.ukey = 0;
      AppData.eClavState = E_KS_STP_ATTENTE_TOUCHE;

    }
    timer_appuie_touche = 0;
    // Detection passage up -> down (front descendant)
    vAHI_DioInterruptEdge(0, PBAR_CFG_NUMPAD_IN);
  }
}

PUBLIC etInUsingkey CLAV_AnalyseIts(uint8 *position)
{
  uint8 byte = 0;
  uint8 tentative = 0;
  uint8 i = 0;
  uint8 depart = 0;
  uint32 val = 0;
  bool_t trouve = FALSE;
  etInUsingkey ret_val = E_KEY_NON_DEFINI;

  vPrintf(" Traitement it:");

  // Pour la recherche faire defiler a un chacune des sortie
  // et lire la valeur en entree
  switch (memo_its_down)
  {
    case E_AHI_DIO12_INT:
    {
      vPrintf("IO_12\n");
      depart = 0;
    }
    break;
    case E_AHI_DIO13_INT:
    {
      vPrintf("IO_13\n");
      depart = C_CLAV_LGN_OUT;
    }
    break;
    case E_AHI_DIO14_INT:
    {
      vPrintf("IO_14\n");
      depart = 2 * C_CLAV_LGN_OUT;
    }
    break;
    case E_AHI_DIO15_INT:
    {
      vPrintf("IO_15\n");
      depart = 3 * C_CLAV_LGN_OUT;
    }
    break;

    default:
      vPrintf("It non prevu:%x\n", memo_its_down);
      au8Led_clav[C_CLAV_LED_INFO_2].actif = TRUE;
      au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_ERREUR_DECTECTEE;
      depart = 4 * C_CLAV_LGN_OUT;
    break;
  }

  // Recherche de la touche clavier
  if (depart != (4 * C_CLAV_LGN_OUT))
  {
    // Desactivation des its clavier
    //vAHI_DioInterruptEnable(0,PBAR_CFG_NUMPAD_IN);
    b_recherche_touche_en_cours = TRUE;
    vPrintf(" Recherche de la touche ayant declenchee l'it\n");

    //val = u32AHI_DioReadInput();
    //byte = (uint8)(val >> 8);
    //vPrintf("  Depart:%d, Val=%x, byte=%x\n", depart, val, byte);
    tentative = 0;
    do
    {
#ifdef WATCHDOG_ENABLED
      vAHI_WatchdogRestart();
#endif
      // mettre a 1 les 4 lignes out du clavier
      vAHI_DioSetOutput(0x0F << C_CLAV_PIO_OUT_1, 0);
      for (i = depart; i < depart + C_CLAV_LGN_OUT; i++)
      {
        // Mettre a 0 uniquement la ligne en cours
        //vAHI_DioSetOutput(~(1<<pioClavOut[i-depart]),(1<<pioClavOut[i-depart]));
        vAHI_DioSetOutput(0, (1UL << pioClavOut[i - depart]));

        // Lecture resultat
        val = u32AHI_DioReadInput();
        byte = (uint8) (val >> 8);
        vPrintf("  s%d:Val=%x, byte=%x, ref:%x\n", i - depart, val, byte,
            ligne_colonne[i]);

        // Valeur connue ?
        if (ligne_colonne[i] == byte)
        {
          vPrintf("\n  Touche trouvee\n");
          ret_val = key_code[i];
          *position = i;

          trouve = TRUE;

          break;
        }
        // Remettre a 1 la ligne
        vAHI_DioSetOutput((1 << pioClavOut[i - depart]), 0);

      }
      tentative++;
    } while ((!trouve) && (tentative < C_MAX_TENTATIVES));

    //Reset des lignes de sorties
    vAHI_DioSetOutput(0, PBAR_CFG_NUMPAD_OUT);

    //Reactivation des its clavier
    //vAHI_DioInterruptEnable(PBAR_CFG_NUMPAD_IN,0);
    b_recherche_touche_en_cours = FALSE;

    if (tentative >= C_MAX_TENTATIVES)
    {
      vPrintf("  Le decodage de touche a echoue\n");

      ///CLAV_ResetLecture();
    }

  }
  vPrintf("Fin traitement it down\n\n");

  return ret_val;
}

PUBLIC void CLAV_ResetLecture(void)
{
  vPrintf("ERR : Attente front montant trop long\n");
  vPrintf("Retour attente pression touche\n");

  // Remettre decodage front descendant
  vAHI_DioInterruptEdge(0, PBAR_CFG_NUMPAD_IN);
  b_it_detect_front_descendant = FALSE;

  b_start_press_count = FALSE;
  timer_appuie_touche = 0;
  AppData.ukey = 0;
  AppData.key = E_KEY_NON_DEFINI;
  AppData.eClavState = E_KS_STP_ATTENTE_TOUCHE;

}

PRIVATE void InitAFroid(void)
{
  /* Initialise flash interface */
  if (bAHI_FlashInit(E_FL_CHIP_AUTO, NULL) == FALSE)
  {
    vPrintf("Echec Initialisation API flash !!\n");
  }
  else
  {
    vPrintf("API Memoire flash OK\n");
  }

  // Config io de cette appli
  APP_ConfigIoJennic();

  // Type utilisation par defaut
  liaison_clavier = E_AUTONOMOUS;

  // Initialisation de la machine a etat fini
  memset(&AppData, 0, sizeof(AppData));

  // Reflet memoire eeprom
  memset(&eeprom, 0, sizeof(bpsFlash));

  if (!bAHI_FullFlashRead(FLASH_START, sizeof(bpsFlash), (uint8 *) &eeprom))
  {
    vPrintf("Pb Lors de la lecture de la config Flash\n");
  }
  // La flash a t elle une config ?
  if (eeprom.nbBoite == 0xFF || eeprom.nbBoite == 0x00)
  {
    vPrintf("Configuration liens necessaire\n");
    memset(&eeprom, 0, sizeof(eeprom));
  }

  vPrintf("Taille Eeprom necessaire %d\n", sizeof(bpsFlash));
  vPrintf("Nb boites connues %x\n", eeprom.nbBoite);

}
