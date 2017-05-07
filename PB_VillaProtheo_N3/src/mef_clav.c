/*
 * mef_clav.c
 *
 *  Created on: 12 juil. 2013
 *      Author: RapidOs
 */

// Machine etat fini clavier
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
//#include "services.h"
#include "e_config.h"

//PRIVATE void CLAV_GererMultiple();
PUBLIC bool_t b_NEW_start_press_count = FALSE;
PRIVATE bool_t b_client_in = FALSE;
//---------------------------------------------------

PUBLIC bool_t b_activer_bip = FALSE;

//---------------------------
PUBLIC teRunningPgl NEW_AnalyseBoitierDeCommande(etRunningStp *stpVal,
    etRunningRol *rolVal, etRunningKbd *kbdVal, etInUsingkey *keyVal,
    etRunningNet *netVal)
{
  //etInUsingkey la_touche = E_KEY_NON_DEFINI;
  //uint8 uId = 0;
  static teRunningPgl pglVal = E_PGL_BOUCLE_PRINCIPALE;
  static bool_t oneshot = FALSE;
  static bool_t oneshot_2 = FALSE;
  //teWifiMsg wifi_msg = E_MSG_NOT_SET;

#ifdef WATCHDOG_ENABLED
  vAHI_WatchdogRestart();
#endif

  if (b_NEW_start_press_count)
  {
    NEW_timer_appuie_touche++;
  }

  switch ((*stpVal))
  {
    case E_KS_STP_NON_DEFINI:
      vPrintf("Passage clavier en attente touche\n");
      (*stpVal) = E_KS_STP_ATTENTE_TOUCHE;
      (*kbdVal) = E_KS_KBD_VIRTUAL_1;
      (*rolVal) = E_KS_ROL_UTILISATEUR;
    break;

    case E_KS_STP_ATTENTE_TOUCHE:
    {
      if (oneshot_2 == FALSE)
      {
        oneshot_2 = TRUE;
        vPrintf("En attente de touche clavier !!\n");
      }
    }
    break;

    default:
    {
      if (oneshot == FALSE)
      {
        oneshot = TRUE;
        vPrintf("Cas non prevu %d -> '%s'\n", (*stpVal),
            dbg_teClavState[(*stpVal)]);
      }
      pglVal = E_PGL_DEFINITION_END;
    }
    break;
  }

  return pglVal;
}

PUBLIC void CLAV_AnalyserEtat(etRunningStp mef_clavier)
{
  etInUsingkey la_touche = E_KEY_NON_DEFINI;
  uint8 uId = 0;
  static bool_t oneshot = FALSE;
  etInUsingkey toucheAction = AppData.eKeyPressed;
  etDefWifiMsg wifi_msg = E_MSG_NOT_SET;

#ifdef WATCHDOG_ENABLED
  vAHI_WatchdogRestart();
#endif

  if (b_NEW_start_press_count)
  {
    NEW_timer_appuie_touche++;
  }

  switch (mef_clavier)
  {
    case E_KS_STP_NON_DEFINI:
      vPrintf("Passage clavier en attente touche\n");
      AppData.eClavState = E_KS_STP_ATTENTE_TOUCHE;
      AppData.eClavmod = E_KS_KBD_VIRTUAL_1;
      AppData.usage = E_KS_ROL_UTILISATEUR;
    break;

    case E_KS_STP_ATTENTE_TOUCHE:
    {

      switch (AppData.eNetState)
      {
        case E_KS_NET_CONF_START:
        break;
        case E_KS_NET_WAIT_CLIENT:
          au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_RECHERCHE_RESEAU;
        break;
        case E_KS_NET_CONF_END:
        break;

        case E_KS_NET_CLIENT_IN:
        {
          if (b_client_in == FALSE)
          {
            b_client_in = TRUE;
            vPrintf("Lecture clavier Mode et Touche puis envoie a la boite\n");
          }
        }
        break;

        case E_KS_NET_CONF_EN_COURS:
        {
          wifi_msg = AppData.eWifiMsg;
          switch (wifi_msg)
          {
            case E_MSG_CFG_LIENS:
            {
              vPrintf("Reception d'une config de liens\n");
            }
            break;
            default:
            {
              vPrintf("Wifi Message error :'%d'", wifi_msg);
            }
          }
        }
        break;

        case E_KS_NET_NON_DEFINI:
        case E_KS_NET_CONF_BRK:
        case E_KS_NET_CLAV_ON:
        case E_KS_NET_END:
        break;

        default:
          vPrintf("CASE NET NON PREVU\n");
        break;

      }
    }
    break;

    case E_KS_STP_SERVICE_ON:
    break;

    case E_KS_STP_SERVICE_OFF:
    break;

    case E_KS_STP_ULTRA_MODE:
    break;

    case E_KS_STP_ATTENDRE_BOITE:
      au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_RECHERCHE_RESEAU;
    break;

    case E_KS_STP_EN_PROGR_AVEC_BOITE:
    break;

    case E_KS_STP_ATTENDRE_FIN_CONFIG_BOITE:
    break;

    case E_KS_STP_ARMER_IT:
    {
      // Rearmer detection It
      // Preparation Detection Front descendant sur entrees
      vAHI_DioInterruptEdge(0, PBAR_CFG_NUMPAD_IN);

      // Autoriser its clavier
      //vAHI_DioInterruptEnable(PBAR_CFG_NUMPAD_IN, 0);
      AppData.eClavState = E_KS_STP_ATTENTE_TOUCHE;
    }
    break;

    case E_KS_STP_TRAITER_IT:
    {

      la_touche = CLAV_AnalyseIts(&uId);
      if (la_touche != E_KEY_NON_DEFINI)
      {
        vPrintf("Touche '%c' pendant '%d' ms, code dans pgm:%d\n\n",
            code_ascii[uId], NEW_memo_delay_touche, la_touche);

        AppData.eKeyPressed = la_touche;
        AppData.ukey = uId;
        timer_touche[uId] = NEW_memo_delay_touche / 100;

        // Une touche est reconnue on peut demander a la traiter
        AppData.eClavState = E_KS_STP_TRAITER_TOUCHE;
      }
      else
      {
        AppData.eClavState = E_KS_STP_ARMER_IT;
      }
    }
    break;

    case E_KS_STP_TRAITER_TOUCHE:
    {
      CLAV_GererTouche(toucheAction);
      AppData.eClavState = E_KS_STP_ARMER_IT;
    }
    break;

    default:
      if (oneshot == FALSE)
      {
        oneshot = TRUE;
        vPrintf("Etat clavier non prevu :%s\n", dbg_teClavState[mef_clavier]);
      }
    break;
  }
}

PUBLIC void CLAV_GererMode(etInUsingkey mode)
{
  //teClavState mef_clav = AppData.eClavState;
  etRunningKbd modif_mode = AppData.eClavmod;
  eLedInfo flash_val;

#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_GererMode",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  switch (AppData.usage)
  {
    case E_KS_ROL_UTILISATEUR:
      flash_val = ~E_FLASH_OFF;
    break;
    case E_KS_ROL_TECHNICIEN:
      flash_val = ~E_FLASH_ALWAYS;
    break;
    case E_KS_ROL_CHOISIR:
      flash_val = E_FLASH_ERREUR_DECTECTEE;
    break;
    default:
      flash_val = E_FLASH_RECHERCHE_RESEAU;
    break;
  }
  au8Led_clav[C_CLAV_LED_INFO_2].mode = flash_val;

  switch (mode)
  {
    case E_KEY_NUM_MOD_1:
      au8Led_clav[C_CLAV_LED_INFO_3].mode = ~E_FLASH_OFF;
      modif_mode = E_KS_KBD_VIRTUAL_1;
    break;

    case E_KEY_NUM_MOD_2:
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_MENU_LED_NET;
      modif_mode = E_KS_KBD_VIRTUAL_2;
    break;

    case E_KEY_NUM_MOD_3:
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_MENU_BIP_CLAVIER;
      modif_mode = E_KS_KBD_VIRTUAL_3;
    break;

    case E_KEY_NUM_MOD_4:
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_MENU_LIAISON;
      modif_mode = E_KS_KBD_VIRTUAL_4;
    break;

    case E_KEY_NUM_MOD_5:
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_ERREUR_DECTECTEE;
      modif_mode = E_KS_KBD_VIRTUAL_1;
    break;

    case E_KEY_NUM_MOD_6:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = ~E_FLASH_ALWAYS;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_RESET_POSSIBLE;
      modif_mode = E_KS_STP_ULTRA_MODE;
    break;

    default:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_ERREUR_DECTECTEE;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_ERREUR_DECTECTEE;
      modif_mode = E_KS_KBD_NON_DEFINI;
    break;
  }

  if ((modif_mode != E_KS_KBD_VIRTUAL_1)
      && (AppData.eClavState != E_KS_STP_ULTRA_MODE))
  {
    compter_duree_mode = 0;
    start_timer_of_mode = TRUE;
  }

  AppData.eClavmod = modif_mode;
#if !NO_DEBUG_ON
  PBAR_DbgInside(stepper, gch_spaces, E_FN_OUT, AppData);
  PBAR_DbgTrace(E_FN_OUT, "CLAV_GererMode", (void *) AppData.eAppState,
      E_DBG_TYPE_NET_STATE);
#endif
}

PUBLIC bool_t CLAV_TrouverAssociationToucheBoite(stToucheDef *touche,
    uint8 BoxId, uint8 *position)
{
  bool_t eReturn = FALSE;
#if 0
  uint8 key_code = (
      (touche->la_touche == E_KEY_NUM_ETOILE) ?
      C_KEY_MEM_ALL :
      touche->la_touche - E_KEY_NUM_1);
  ;
#endif
  uint8 key_code = touche->la_touche - E_KEY_NUM_1;
  uint8 key_mode = touche->le_mode - E_KS_KBD_VIRTUAL_1;
  uint8 nbBox = eeprom.netConf.ptr_boxList[key_mode][key_code];
  ;
  uint8 i = 0;
  uint8 useBox = 0;

#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_TrouverAssociationToucheBoite",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif
  vPrintf("%sNb boite associee a la touche %s -> %d\n", gch_spaces,
      dbg_etCLAV_keys[touche->la_touche], nbBox);

  vPrintf("%sRecherche dans la liste chainee des boites associee a la touche\n",
      gch_spaces);
  for (i = 0; (i <= nbBox) && (i < C_MAX_BOXES); i++)
  {
    useBox = eeprom.netConf.boxList[key_mode][key_code][i];
    vPrintf("%sPtr_box:%d, boxid : %d\n", gch_spaces, i, useBox);

    if (useBox == 0x00)
    {
      // On a parcouru toute la liste
      // sans trouver de correspondance
      vPrintf("%s Sauvegarde necessaire a position %d !\n", gch_spaces, i);
      eReturn = FALSE;
      break;
    }
    else
    {
      if (useBox == BoxId)
      {
        vPrintf("%sTrouve a position:%d !\n", gch_spaces, i);
        eReturn = TRUE;
        break;
      }
      else
      {
        vPrintf("%s On regarde avec la suivante\n", gch_spaces);
        continue;
      }
    }

  }

  // On memorise la position trouvee
  *position = i;
#if !NO_DEBUG_ON
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
  PBAR_DbgTrace(E_FN_OUT, "CLAV_TrouverAssociationToucheBoite",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
#endif
  return eReturn;
}
