/*
 * main.c
 *
 *  Created on: 11 juil. 2013
 *      Author: RapidOs
 */

#include <JPI.h>
#include <AppApi.h>
#include <AppHardwareApi.h>

#include <JenNetApi.h>
#include <Jenie.h>

#if !NO_DEBUG_ON
#include <Printf.h>
#include <Utilities.h>
#include "Utils.h"
#endif

#include "bit.h"
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
PRIVATE void NEW_CLAV_GestionIts(void);

PUBLIC etCLAV_keys CLAV_AnalyseIts(uint8 *position);

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
PRIVATE uint32 memo_its_up = 0UL;
PRIVATE eStatusClavier liaison_clavier = E_AUTONOMOUS;

PRIVATE bool_t anti_rebond_it = FALSE;
//PRIVATE bool_t b_It_tempo_enable = FALSE;

PRIVATE bool_t b_it_detect_front_descendant = FALSE;
//PRIVATE uint16 tempo_rebond = 0;
PRIVATE bool_t b_it_detecte = FALSE;
PUBLIC bool_t NEW_traiter_It = FALSE;

PRIVATE bool_t b_start_press_count = FALSE;
PUBLIC uint16 timer_appuie_touche = 0;
PUBLIC uint16 NEW_timer_appuie_touche = 0;
PUBLIC uint16 NEW_memo_delay_touche = 0;

PRIVATE b_it_20_down_start = TRUE;
#ifdef CLAV_IS_VELLMAN
PRIVATE const uint16 ligne_colonne[] = { 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40,
    0x80, 0x200, 0x100 };

PUBLIC etCLAV_keys key_code[] = { E_KEY_NUM_0, E_KEY_NUM_1, E_KEY_NUM_2,
    E_KEY_NUM_3, E_KEY_NUM_4, E_KEY_NUM_5, E_KEY_NUM_6, E_KEY_NUM_7,
    E_KEY_NUM_DIESE, E_KEY_NUM_ETOILE };
PUBLIC uint16 timer_touche[sizeof(key_code) / sizeof(etCLAV_keys)] = { 0 };

PUBLIC const etCLAV_keys R_Key_modes[CST_NB_MODES] = { E_KEY_NUM_MOD_1,

E_KEY_NUM_MOD_2, E_KEY_NUM_MOD_3, E_KEY_NUM_MOD_4 };

#if !NO_DEBUG_ON
PUBLIC const uint8 code_ascii[] = "ABCD1234#*";
#endif

#else
PRIVATE const uint8 pioClavOut[] =
{ C_CLAV_PIO_OUT_1, C_CLAV_PIO_OUT_2,
  C_CLAV_PIO_OUT_3, C_CLAV_PIO_OUT_4};

PRIVATE const uint8 ligne_colonne[] =
{
  0xEE, 0xED, 0xEB, 0xE7, 0xDE, 0xDD, 0xDB, 0xD7, 0xBE, 0xBD, 0xBB, 0xB7, 0x7E,
  0x7D, 0x7B, 0x77
};
PRIVATE const etCLAV_keys key_code[] =
{ E_KEY_NUM_1, E_KEY_NUM_4, E_KEY_NUM_7,
  E_KEY_NUM_ETOILE, E_KEY_NUM_2, E_KEY_NUM_5, E_KEY_NUM_8, E_KEY_NUM_0,
  E_KEY_NUM_3, E_KEY_NUM_6, E_KEY_NUM_9, E_KEY_NUM_DIESE, E_KEY_NUM_MOD_1,
  E_KEY_NUM_MOD_2, E_KEY_NUM_MOD_3, E_KEY_NUM_MOD_4};
PUBLIC uint16 timer_touche[sizeof(key_code) / sizeof(etCLAV_keys)] =
{ 0};

#if !NO_DEBUG_ON
PRIVATE const uint8 code_ascii[] = "147*2580369#ABCD";
#endif
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
  //u32AppApiInit(NULL, NULL, NULL, NULL, NULL, NULL); // Pour gestion its AN/AP-1040
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
    AppData.eAppState = APP_RECHERCHE_RESEAU;
    // Montrer la led d'activite reseau
    au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_RECHERCHE_RESEAU;
    au8Led_clav[C_CLAV_LED_INFO_1].actif = TRUE;

    // PAr defaut autre led utilise mais eteintes
    au8Led_clav[C_CLAV_LED_INFO_2].actif = TRUE;
    au8Led_clav[C_CLAV_LED_INFO_3].actif = TRUE;

#ifdef CLAV_IS_VELLMAN
    au8Led_clav[C_CLAV_LED_INFO_2].mode = ~E_FLASH_OFF;
    au8Led_clav[C_CLAV_LED_INFO_3].mode = ~E_FLASH_OFF;
#else
    au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_OFF;
    au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_OFF;
#endif

  }

}

PUBLIC void vJenie_CbMain(void)
{
  /* regular watchdog reset */
#ifdef WATCHDOG_ENABLED
  vAHI_WatchdogRestart();
#endif

  switch (AppData.eAppState)
  {
    case APP_RECHERCHE_RESEAU:
      ; // Rien attendre
    break;

    case APP_RESEAU_ETABLI:
      vPrintf("Connection Reseau etablit\n");
      // Montrer par flash que l'on est connecte
      au8Led_clav[C_CLAV_LED_INFO_1].mode = mNetOkTypeFlash;

      // Autoriser la prise en compte des its clavier
      vAHI_DioInterruptEnable(PBAR_CFG_NUMPAD_IN, 0);

      // etape suivante
      AppData.eAppState = APP_BOUCLE_PRINCIPALE;

    break;

    case APP_BOUCLE_PRINCIPALE:
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
        CLAV_AnalyserEtat(AppData.eClavState);
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

  bp_CommunMsgReseau(&AppData.eAppState, eEventType, pvEventPrim);

  switch (eEventType)
  {
    case E_JENIE_NETWORK_UP:
    {
      AppData.eAppState = APP_RESEAU_ETABLI;
    }
    break;

    case E_JENIE_POLL_CMPLT:
      // A traiter
    break;

    case E_JENIE_REG_SVC_RSP:
    {
      if (AppData.eNetState == E_KS_NET_CONF_START)
      {
        vPrintf(" Le service est bien enregistre chez le pere\n\n");
        vPrintf("En attente de demande d'une boite\n\n");
        AppData.eNetState = E_KS_NET_WAIT_CLIENT;
      }
      else if (AppData.eNetState == E_KS_NET_CONF_END)
      {
        vPrintf(" Le service est bien Retire chez le pere\n\n");
        vPrintf("En attente de commandes utilisateur\n\n");
        AppData.eNetState = E_KS_NET_CLAV_ON;
      }
      else
      {
        vPrintf("ERREUR RESEAU CLAVIER\n");
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

  bp_CommunMsgReseau(&AppData.eAppState, eEventType, pvEventPrim);

  switch (eEventType)
  {
    case E_JENIE_DATA:
    {
      vPrintf("Net Data in\n");
      switch (AppData.eAppState)
      {
        case APP_BOUCLE_PRINCIPALE:
        {
          CLAV_NetMsgInput(psData);
        }
        break;

        default:
        {
          vPrintf("ERR:E_JENIE_DATA, AppData.eAppState -> %d\n",
              AppData.eAppState);
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
  PRIVATE uint8 it20tick = 0;

  switch (u32DeviceId)
  {
    case E_JPI_DEVICE_TICK_TIMER:
    {
      IHM_ClignoteLed();

#ifndef CLAV_IS_VELLMAN
      if (b_activer_bip == TRUE)
      {
        temps_bip++;
        if (temps_bip >= C_TEMPS_BIP)
        {
          b_activer_bip = FALSE;
          vAHI_DioSetOutput(0, C_CLAV_BUZER);
        }
      }
#endif
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

#ifndef CLAV_IS_VELLMAN
  // Sortie du buzzer
  vAHI_DioSetDirection(0, C_CLAV_BUZER);
  // Activation pull up
  vAHI_DioSetPullup(0, C_CLAV_BUZER);
  // Mettre a un on pour eteindre le buzzer
  vAHI_DioSetOutput(0, C_CLAV_BUZER);
#endif

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

PRIVATE void PBAR_ISR_Clavier_c3(uint32 u32Device, uint32 u32ItemBitmap)
{
  uint32 tickTimerValue;
  uint32 debounceDelay_down = 100;
  uint32 debounceDelay_up = 0x100;
  static uint32 previous = 0;
  static uint32 lastTick = 0;
  static bool_t last_val = 1;
  bool_t cur_val = 0;
  static uint8 debounce_step = 0;

  if (u32Device == E_AHI_DEVICE_SYSCTRL)
  {
    tickTimerValue = u32AHI_TickTimerRead();

    switch (u32ItemBitmap)
    {
#ifdef CLAV_IS_VELLMAN
      case E_JPI_DIO20_INT: // Touche '#'
      {
        cur_val = (u32AHI_DioReadInput() >> 20) & 0x1;
        if (b_NEW_start_press_count == FALSE)
        {
          //vPrintf("1:# it (%d)/(%d)\n", cur_val, last_val);
          if (cur_val != last_val)
          {
            last_val = cur_val;
          }

          if (last_val == 0)
          {
            //vPrintf("1:# start\n");
            NEW_timer_appuie_touche = 0;
            b_NEW_start_press_count = TRUE;
            memo_its_down = u32ItemBitmap;
            vAHI_DioInterruptEdge(E_JPI_DIO20_INT, 0);
          }
#if 0
          previous = tickTimerValue;
          vAHI_DioInterruptEnable(0, E_JPI_DIO20_INT);
          // Bloquer les its
          while((u32AHI_TickTimerRead()-previous)<debounceDelay_down)
          {
            ; // attendre
          }
          vAHI_DioInterruptEnable(E_JPI_DIO20_INT,0);
#endif
        }
        else
        {
          if (NEW_timer_appuie_touche)
          {
            //vPrintf("2:# it (%d)/(%d)\n", cur_val, last_val);
            if (cur_val != last_val)
            {
              last_val = cur_val;
            }

            if (last_val == 1)
            {
              NEW_memo_delay_touche = NEW_timer_appuie_touche;
              vAHI_DioInterruptEdge(0, E_JPI_DIO20_INT);
              b_NEW_start_press_count = FALSE;
              vPrintf("2:# it:%d\n", NEW_memo_delay_touche);
              // Informer a gerer
              AppData.eClavState = E_KS_TRAITER_IT;

            }

#if 0
            vAHI_DioInterruptEnable(0, E_JPI_DIO20_INT);

            previous = tickTimerValue;
            // Bloquer les its
            while ((u32AHI_TickTimerRead() - previous) < debounceDelay_up)
            {
              ; // attendre
            }
            vAHI_DioInterruptEnable(E_JPI_DIO20_INT, 0);
#endif
          }
        }
      }
      break;
      case E_JPI_DIO11_INT: //touche 1
      case E_JPI_DIO12_INT:
      case E_JPI_DIO13_INT:
      case E_JPI_DIO14_INT:
      case E_JPI_DIO15_INT:
      case E_JPI_DIO16_INT:
      case E_JPI_DIO17_INT:
      case E_JPI_DIO18_INT: // touche 8
      case E_JPI_DIO19_INT: // touche '*'
#ifdef TST_BOUNCE
      {
        // Antirebonds
        vAHI_DioInterruptEnable(0, PBAR_CFG_NUMPAD_IN);////bloquer les its
        while ((u32AHI_TickTimerRead()) - tickTimerValue < debounceDelay_down)
        {
          ; //attendre
        }
        vAHI_DioInterruptEnable(PBAR_CFG_NUMPAD_IN, 0); //reactiver Its
        vPrintf("It a debouncer:%x\n", u32ItemBitmap);
        tickTimerValue= u32AHI_TickTimerRead();
        vPrintf("Tick Value:%x\n\n",tickTimerValue);
      }
#else
      {
        if (b_NEW_start_press_count == FALSE)
        {
          NEW_timer_appuie_touche = 0;
          b_NEW_start_press_count = TRUE;

          // Detection passage down -> up (front montant)
          vAHI_DioInterruptEdge(PBAR_CFG_NUMPAD_IN, 0);
          memo_its_down = u32ItemBitmap;
          //remettre les its
          //bloquer les its
          //vAHI_DioInterruptEnable(PBAR_CFG_NUMPAD_IN, 0);
        }
        else
        {
          //bloquer les its
          //vAHI_DioInterruptEnable(0, PBAR_CFG_NUMPAD_IN);

          NEW_memo_delay_touche = NEW_timer_appuie_touche;

          b_NEW_start_press_count = FALSE;

          // Informer a gerer
          AppData.eClavState = E_KS_TRAITER_IT;
        }
        break;
      }
#endif
#endif

      default:
        vPrintf("It non prevue:%x\n", u32ItemBitmap);
      break;
    }
  }
}
#if 0
// Routine de lecture du clavier matriciel sous IT
PRIVATE void PBAR_ISR_Clavier_c3(uint32 u32Device, uint32 u32ItemBitmap)
{
  int i;
  uint32 tickTimerValue;
  uint32 debounceDelay = 50;
  static uint32 last_time_19;
  static uint32 last_time_20;
  bool_t read_19;
  bool_t read_20;
  static bool_t prev_19 = TRUE;
  static bool_t prev_20 = TRUE;

  if (u32Device == E_AHI_DEVICE_SYSCTRL)
  {
    tickTimerValue= u32AHI_TickTimerRead();

    switch (u32ItemBitmap)
    {
#ifdef CLAV_IS_VELLMAN

      case E_JPI_DIO19_INT: // touche '*'
      read_19 = IsBitSet(u32ItemBitmap,E_JPI_DIO19_INT);
      if(read_19 != prev_19)
      {
        last_time_19 = tickTimerValue;
      }
      if(tickTimerValue-last_time_19>debounceDelay)
      {
        if(read_19 != prev_19)
        {
          prev_19 = read_19;
        }
      }
      case E_JPI_DIO20_INT: // Touche '#'
      read_20 = IsBitSet(u32ItemBitmap,E_JPI_DIO20_INT);
      // Mettre code anti rebond
      // https://www.arduino.cc/en/Tutorial/Debounce
      {
        vPrintf("It a debouncer:%x\n", u32ItemBitmap);
        tickTimerValue= u32AHI_TickTimerRead();
        vPrintf("Tick Value:%x\n\n",tickTimerValue);
      }
      break;
      case E_JPI_DIO11_INT: //touche 1
      case E_JPI_DIO12_INT:
      case E_JPI_DIO13_INT:
      case E_JPI_DIO14_INT:
      case E_JPI_DIO15_INT:
      case E_JPI_DIO16_INT:
      case E_JPI_DIO17_INT:
      case E_JPI_DIO18_INT:// touche 8
#else
      case E_AHI_DIO12_INT:
      //if(!it_en_cours)it_en_cours=1;
      case E_AHI_DIO13_INT:
      //if(!it_en_cours)it_en_cours=2;
      case E_AHI_DIO14_INT:
      //if(!it_en_cours)it_en_cours=3;
      case E_AHI_DIO15_INT:
      //if(!it_en_cours)it_en_cours=4;
#endif
      {
        if (b_NEW_start_press_count == FALSE)
        {
          // Antirebonds
          //bloquer les its
          vAHI_DioInterruptEnable(0,PBAR_CFG_NUMPAD_IN);
          for(i=0;i<10000;i++);

          NEW_timer_appuie_touche = 0;
          b_NEW_start_press_count = TRUE;

          // Detection passage down -> up (front montant)
          vAHI_DioInterruptEdge(PBAR_CFG_NUMPAD_IN, 0);
          memo_its_down = u32ItemBitmap;
          //remettre les its
          //bloquer les its
          vAHI_DioInterruptEnable(PBAR_CFG_NUMPAD_IN,0);
        }
        else
        {
          //bloquer les its
          vAHI_DioInterruptEnable(0,PBAR_CFG_NUMPAD_IN);

          NEW_memo_delay_touche = NEW_timer_appuie_touche;

          b_NEW_start_press_count = FALSE;

          // Informer a gerer
          AppData.eClavState = E_KS_TRAITER_IT;
        }
      }
      break;

      default:
      vPrintf("It non prevue:%x\n", u32ItemBitmap);
      break;
    }
  }
}
#endif

PRIVATE void NEW_CLAV_GestionIts(void)
{
  etCLAV_keys la_touche = E_KEY_NON_DEFINI;
  uint8 uId = 0;

  if (NEW_traiter_It == FALSE)
    return;

  AppData.eClavState = E_KS_TRAITER_IT;

#if 0
  // desactiver les its clavier
  vAHI_DioInterruptEnable(0,PBAR_CFG_NUMPAD_IN);

  la_touche = CLAV_AnalyseIts(&uId);
  if (la_touche != E_KEY_NON_DEFINI )
  {
    vPrintf("Touche '%c' pendant '%d' ms, code dans pgm:%d\n\n",
        code_ascii[uId], NEW_memo_delay_touche, la_touche);

    // Une touche est reconnue on peut demander a la traiter
    AppData.eClavState = E_KS_TRAITER_TOUCHE;
    AppData.eKeyPressed = la_touche;
    AppData.ukey = uId;
    timer_touche[uId] = NEW_memo_delay_touche;

  }
  // Rearmer detection It
  NEW_traiter_It = FALSE;
  // Preparation Detection Front descendant sur entrees
  vAHI_DioInterruptEdge(0, PBAR_CFG_NUMPAD_IN);

  // Autoriser its clavier
  vAHI_DioInterruptEnable(PBAR_CFG_NUMPAD_IN, 0);
#endif
}

PRIVATE void CLAV_GestionIts(void)
{
  etCLAV_keys la_touche = E_KEY_NON_DEFINI;
  uint8 posCodeAscii = 0;

  if (b_it_detect_front_descendant == TRUE)
  {
    la_touche = CLAV_AnalyseIts(&posCodeAscii);
    if (la_touche != E_KEY_NON_DEFINI)
    {
      vPrintf("Touche '%c' trouvee, code dans pgm:%d\n",
          code_ascii[posCodeAscii], la_touche);
      AppData.eKeyPressed = la_touche;
      AppData.ukey = posCodeAscii;

///------------- Gestion temps d'appui
      if ((la_touche == E_KEY_NUM_ETOILE) || (la_touche == E_KEY_NUM_DIESE))
      {
        vPrintf("Debut mesure du temps d'appuie\n");
        b_start_press_count = TRUE;

        AppData.eClavState = E_KS_TRAITER_IT;

        // Detection passage down -> up (front montant)
        vAHI_DioInterruptEdge(PBAR_CFG_NUMPAD_IN, 0);
      }
      else
      {
        b_it_detect_front_descendant = FALSE;
        AppData.eClavState = E_KS_TRAITER_TOUCHE;
      }
    }
    else
    {
      vPrintf("la_touche == E_NO_KEYS\n");
      b_it_detect_front_descendant = FALSE;
      AppData.eClavState = E_KS_ATTENTE_TOUCHE;
    }
  }
  else
  {
    b_start_press_count = FALSE;
    // a 't on un temps minimum d'appui
    if (timer_appuie_touche >= C_MIN_DURE_PRESSION)
    {
      // Calcul du temps d'appuie de la touche + bip + traitement
      timer_touche[AppData.ukey] = timer_appuie_touche;
      vPrintf(" Duree maintient de touche '%c'= %d\n\n",
          code_ascii[AppData.ukey], timer_touche[AppData.ukey]);

      // Memorisation de l'etat avant traitement de la touche
      //AppData.ePrevClav = AppData.eClavState;

      // Un appui puis un relachement on ete detectee et minutee
      //on effectue une action
      AppData.eClavState = E_KS_TRAITER_TOUCHE;
    }
    else
    {
      vPrintf("Temps Pression insuffisant:%d\n", timer_appuie_touche);
      vPrintf("Retour en attente pression touche\n");
      // Ignorer la touche
      AppData.eKeyPressed = E_KEY_NON_DEFINI;
      AppData.ukey = 0;
      AppData.eClavState = E_KS_ATTENTE_TOUCHE;

    }
    timer_appuie_touche = 0;
    // Detection passage up -> down (front descendant)
    vAHI_DioInterruptEdge(0, PBAR_CFG_NUMPAD_IN);
  }
}

#ifdef CLAV_IS_VELLMAN
PUBLIC etCLAV_keys CLAV_AnalyseIts(uint8 *position)
{
  etCLAV_keys ret_val = E_KEY_NON_DEFINI;
  uint32 val3 = 0UL;
  uint16 byte2 = NEW_memo_delay_touche;
  int i = 0;

  // Pression suffisante ?
  if (byte2 < C_MIN_KEY_PRESS_TIME)
  {
    vPrintf("Appuie trop court:'%d' ms\n", byte2);
    return ret_val;
  }

  // Lecture resultat
  val3 = memo_its_down & 0x001FF800;
  byte2 = (uint16) (val3 >> 11);
  //vPrintf("memo_its_down=%x, Val3=%x, info2;%x\n", memo_its_down, val3, byte2);

  for (i = 0; i < CLAV_NB_KEYS; i++)
  {
    if (ligne_colonne[i] == byte2)
    {
      ret_val = key_code[i];
      *position = i;
    }

  }

  return ret_val;
}
#else
PUBLIC etCLAV_keys CLAV_AnalyseIts(uint8 *position)
{
  uint8 byte = 0;
  uint8 tentative = 0;
  uint8 i = 0;
  uint8 depart = 0;
  uint32 val = 0;
  bool_t trouve = FALSE;
  etCLAV_keys ret_val = E_KEY_NON_DEFINI;

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
          ret_val = key_code[i];
          vPrintf("\n  Touche trouvee\n");
          *position = i;

          trouve = TRUE;

          break;
        }
        // Remettre a 1 la ligne
        vAHI_DioSetOutput((1 << pioClavOut[i - depart]), 0);

      }
      tentative++;
    }while ((!trouve) && (tentative < C_MAX_TENTATIVES));

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
#endif

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
  AppData.eKeyPressed = E_KEY_NON_DEFINI;
  AppData.eClavState = E_KS_ATTENTE_TOUCHE;

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

PUBLIC int8 NEW_TrouvePositionTouche(etCLAV_keys laTouche)
{
  int8 val = -1;
  uint8 pos = 0;

  int len = sizeof(key_code) / sizeof(etCLAV_keys);

  for (pos = 0; pos < len; pos++)
  {
    if (key_code[pos] == laTouche)
      return pos;
  }

  return val;
}
