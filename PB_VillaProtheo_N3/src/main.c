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
PRIVATE uint8 getItFromMask(uint32 mask);

PUBLIC etInUsingkey CLAV_AnalyseIts(uint8 *position);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
PUBLIC tsClavData AppData;
PUBLIC bpsFlash eeprom;
PUBLIC stParam maConfig;

PUBLIC bool_t start_timer_of_mode = FALSE;
PUBLIC uint16 compter_duree_mode = 0;
PUBLIC etFlashMsg mNetOkTypeFlash = E_FLASH_RESEAU_ACTIF;

PUBLIC etRunningStp (*MenuPossible[2][3])(stParam *param) =
{
  { pFn1_1,pFn1_2,pFn1_3},
  { pFn2_1,pFn2_2,pFn2_3} };
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE uint32 memo_its_down = 0UL;
PRIVATE eStatusClavier liaison_clavier = E_AUTONOMOUS;

typedef enum _my_it {
  E_IT_ZN_NOT_SET, E_IT_ZN_1, E_IT_ZN_2, E_IT_ZN_3, E_IT_ZN_4, E_IT_ZN_END
} etItZone;

//PRIVATE bool_t b_It_tempo_enable = FALSE;
PRIVATE bool_t preserve_states = FALSE;
PRIVATE bool_t b_it_detect_front_descendant = FALSE;
//PRIVATE uint16 tempo_rebond = 0;
PUBLIC bool_t NEW_traiter_It = FALSE;

PRIVATE bool_t b_start_press_count = FALSE;
PUBLIC uint16 timer_appuie_touche = 0;
PUBLIC uint32 NEW_timer_appuie_touche = 200;
PUBLIC uint32 NEW_memo_delay_touche = 0;

PRIVATE bool_t b_NEW_it_down = FALSE;
PRIVATE uint32 edge_it_down = 0;

#ifdef CLAV_IS_VELLMAN
PRIVATE const uint16 ligne_colonne[] = { 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40,
    0x80, 0x200, 0x100 };

PUBLIC etInUsingkey key_code[] = { E_KEY_NUM_0, E_KEY_NUM_1, E_KEY_NUM_2,
    E_KEY_NUM_3, E_KEY_NUM_4, E_KEY_NUM_5, E_KEY_NUM_6, E_KEY_NUM_7,
    E_KEY_NUM_DIESE, E_KEY_NUM_ETOILE };

//PUBLIC uint16 timer_touche[sizeof(key_code) / sizeof(etInUsingkey)] = { 0 };
PUBLIC uint16 timer_touche[E_KEYS_NUM_END] = { 0 };

PUBLIC const etInUsingkey R_Key_modes[CST_NB_MODES] = { E_KEY_NUM_MOD_1,
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
PRIVATE const etInUsingkey key_code[] =
{ E_KEY_NUM_1, E_KEY_NUM_4, E_KEY_NUM_7,
  E_KEY_NUM_ETOILE, E_KEY_NUM_2, E_KEY_NUM_5, E_KEY_NUM_8, E_KEY_NUM_0,
  E_KEY_NUM_3, E_KEY_NUM_6, E_KEY_NUM_9, E_KEY_NUM_DIESE, E_KEY_NUM_MOD_1,
  E_KEY_NUM_MOD_2, E_KEY_NUM_MOD_3, E_KEY_NUM_MOD_4};
PUBLIC uint16 timer_touche[sizeof(key_code) / sizeof(etInUsingkey)] =
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

  if ((bWarmStart == FALSE) && (preserve_states == FALSE))
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

  if (b_NEW_it_down)
  {
    edge_it_down++;
  }

  switch (AppData.pgl)
  {
    case E_PGL_RECHERCHE_RESEAU:
      ; // Rien attendre
    break;

    case E_PGL_RESEAU_ETABLI:
      vPrintf("Connection Reseau etablit\n");
      // Montrer par flash que l'on est connecte
      au8Led_clav[C_CLAV_LED_INFO_1].mode = mNetOkTypeFlash;
      //au8Led_clav[C_CLAV_LED_INFO_2].actif = FALSE;
      //au8Led_clav[C_CLAV_LED_INFO_3].actif = FALSE;

      // Autoriser la prise en compte des its clavier
      vAHI_DioInterruptEnable(PBAR_CFG_NUMPAD_IN, 0);

      // etape suivante
      AppData.pgl = E_PGL_BOUCLE_PRINCIPALE;

    break;

    case E_PGL_BOUCLE_PRINCIPALE:
    {
      CLAV_AnalyserEtat(AppData.stp);

#if 0
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
#endif

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
      if (AppData.net == E_KS_NET_CONF_START)
      {
        vPrintf(" Le service est bien enregistre chez le pere\n\n");
        vPrintf("En attente de demande d'une boite\n\n");
        AppData.net = E_KS_NET_WAIT_CLIENT;
      }
      else if (AppData.net == E_KS_NET_CONF_END)
      {
        vPrintf(" Le service est bien Retire chez le pere\n\n");
        vPrintf("En attente de commandes utilisateur\n\n");
        AppData.net = E_KS_NET_CLAV_ON;
      }
      else
      {
        vPrintf("ERREUR RESEAU CLAVIER %d\n", AppData.net);
      }

    }
    break;

    case E_JENIE_STACK_RESET:
    {
      // Preserver l'etat "Programme" du clavier
      vPrintf("Pile Jennic redemarre\n");
      preserve_states = TRUE;
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
      vPrintf("Net Data in\n");
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
  static bool_t last_val = 0;
  static uint32 start_tick = 0;
  bool_t cur_val = 0;
  bool_t cur_it = 0;
  uint8 it_id = 0;
  uint8 looptime = 0;
  static bool_t it_down = FALSE;
  static etItZone it_zone = E_IT_ZN_NOT_SET;

  if (u32Device == E_AHI_DEVICE_SYSCTRL)
  {
    tickTimerValue = u32AHI_TickTimerRead();

    switch (u32ItemBitmap)
    {
#ifdef CLAV_IS_VELLMAN
      case E_JPI_DIO20_INT: // Touche '#'
      case E_JPI_DIO11_INT: //touche 1
      case E_JPI_DIO12_INT:
      case E_JPI_DIO13_INT:
      case E_JPI_DIO14_INT:
      case E_JPI_DIO15_INT:
      case E_JPI_DIO16_INT:
      case E_JPI_DIO17_INT:
      case E_JPI_DIO18_INT: // touche 8
      case E_JPI_DIO19_INT: // touche '*'
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
        it_id = getItFromMask(u32ItemBitmap);

        switch (AppData.stp)
        {
          case E_KS_STP_ATTENTE_TOUCHE:
          {
            vPrintf("1:Down(%d)\n", it_id);
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
            vPrintf("2:UP(%d)->time used '%d'\n", it_id, NEW_memo_delay_touche);
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

#ifdef VERSION_2
switch (it_zone)
{
  case E_IT_ZN_NOT_SET:
  {
    if(NEW_timer_appuie_touche<100)
    return;
    // une it
    it_zone = E_IT_ZN_1;
    b_NEW_it_down = TRUE;
    edge_it_down = 0;
    vPrintf("DOWN start\n");
    memo_its_down = u32ItemBitmap;
    vPrintf("Round Trip:%d\n",NEW_timer_appuie_touche);
    b_NEW_start_press_count = TRUE;
    NEW_timer_appuie_touche = 0;
    vAHI_DioInterruptEdge(u32ItemBitmap, 0);
  }
  break;

  case E_IT_ZN_1:
  {
    NEW_memo_delay_touche = edge_it_down/100;
    vPrintf("Value:%d\n",NEW_memo_delay_touche);

    b_NEW_start_press_count = TRUE;
    NEW_timer_appuie_touche = 0;
    if (NEW_memo_delay_touche > C_MIN_KEY_PRESS_TIME)
    {
      vPrintf("Traitement\n");
      AppData.stp = E_KS_STP_TRAITER_IT;
    }
    else
    {
      AppData.stp = E_KS_STP_ARMER_IT;
      vPrintf("!!Error\n");
    }
    vPrintf("\n");
    it_zone = E_IT_ZN_NOT_SET;
  }
  break;
#if 0
  case E_IT_ZN_2:
  {
    vPrintf("UP\n");
    it_zone = E_IT_ZN_NOT_SET;
  }
  break;

  case E_IT_ZN_3:
  {
    vPrintf("Value 2:%d\n",edge_it_down);

    if (edge_it_down < 15)
    {
      // une it
      it_zone = E_IT_ZN_3;
    }
    else
    {
      //b_NEW_start_press_count = FALSE;
      b_NEW_it_down = FALSE;
      NEW_memo_delay_touche = edge_it_down/100;
      edge_it_down = 0;
      // Informer a gerer
      vPrintf("3:(%d)->time used '%d'\n\n", it_id, NEW_memo_delay_touche);

      if (NEW_memo_delay_touche > C_MIN_KEY_PRESS_TIME)
      {
        vPrintf("Traitement\n");
        AppData.stp = E_KS_STP_TRAITER_IT;
      }
      else
      {
        AppData.stp = E_KS_STP_ARMER_IT;
        vPrintf("!!Error\n");
      }

      // une it
      it_zone = E_IT_ZN_NOT_SET;
    }
  }
  break;
#endif
  default:
  {
    vPrintf("!!Erreur zone analyse it\n");
  }
}

#endif

#if 0
//desactiver it
vAHI_DioInterruptEnable(0, u32ItemBitmap);
it_id = getItFromMask(u32ItemBitmap);

// lancer timer
if(b_NEW_it_down == FALSE)
{
  b_NEW_it_down = TRUE;
}
else
{
  if(edge_it_down < 150)
  {
    return;
  }
  else
  {
    /// Niveau bas OK
    NEW_timer_appuie_touche = 0;
    b_NEW_start_press_count = TRUE;

    // Armer remonte it
  }
}
NEW_timer_appuie_touche = 0;
b_NEW_start_press_count = TRUE;
// Attendre fin rebond
do
{
  // verifier niveau
  cur_val = (u32AHI_DioReadInput() >> it_id) & 0x1;

  // Nouvelle it survenue ?
  cur_it = (u32AHI_DioInterruptStatus() >> it_id) & 0x1;
}while ((NEW_timer_appuie_touche < 150) && (cur_val != last_val));

if (cur_val == 0)
{
  //Le bouton est presse
  //vPrintf("\n\n");
  //vPrintf("1:(%d)->start %d\n", it_id, tickTimerValue);
  vPrintf("Down\n");
  start_tick = tickTimerValue;

  // Lancer le timer perso
  NEW_timer_appuie_touche = 0;
  //b_NEW_start_press_count = TRUE;
  memo_its_down = u32ItemBitmap;
  // changer sens leture it
  vAHI_DioInterruptEdge(u32ItemBitmap, 0);
}
else
{
  // Arreter timer
  //vPrintf("2:(%d)->stop %d\n", it_id, tickTimerValue);
  vPrintf("UP\n");
  b_NEW_start_press_count = FALSE;
  NEW_memo_delay_touche = NEW_timer_appuie_touche / 100;
  //NEW_memo_delay_touche = (start_tick - tickTimerValue)/100;

  // changer sens leture it
  vAHI_DioInterruptEdge(0, u32ItemBitmap);

  // Informer a gerer
  vPrintf("3:(%d)->time used '%d'\n\n", it_id, NEW_memo_delay_touche);

  if (NEW_memo_delay_touche > C_MIN_KEY_PRESS_TIME)
  {
    AppData.stp = E_KS_STP_TRAITER_IT;
  }
  else
  {
    AppData.stp = E_KS_STP_ARMER_IT;
    vPrintf("!!Error\n");
  }
}
// reactiver it
vAHI_DioInterruptEnable(u32ItemBitmap, 0);
last_val = !last_val;

#endif

PRIVATE uint8 getItFromMask(uint32 mask)
{
  int i = 0;

  for (i = 0; i < (8 * sizeof(uint32)); i++)
  {
    if (IsBitSet(mask, i))
      break;
  }

  return i;
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
          AppData.stp = E_KS_STP_TRAITER_IT;
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

#ifdef CLAV_IS_VELLMAN
PUBLIC etInUsingkey CLAV_AnalyseIts(uint8 *position)
{
  etInUsingkey ret_val = E_KEY_NON_DEFINI;
  uint32 val3 = 0UL;
  uint32 byte2 = NEW_memo_delay_touche;
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
  AppData.key = E_KEY_NON_DEFINI;
  AppData.stp = E_KS_STP_ATTENTE_TOUCHE;

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

// Test Parametres
  memset(&maConfig, 0, sizeof(stParam));

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

PUBLIC int8 NEW_TrouvePositionTouche(etInUsingkey laTouche)
{
  int8 val = -1;
  uint8 pos = 0;

  int len = sizeof(key_code) / sizeof(etInUsingkey);

  for (pos = 0; pos < len; pos++)
  {
    if (key_code[pos] == laTouche)
      return pos;
  }

  return val;
}
