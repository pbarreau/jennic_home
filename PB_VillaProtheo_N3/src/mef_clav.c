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

PUBLIC bool_t b_DebutIt = FALSE;
PUBLIC uint32 timer_antirebond_dow = 0;

PUBLIC bool_t b_compter_pression = FALSE;
PUBLIC uint32 timer_duree_pression = 0;

PUBLIC bool_t b_FinIt = FALSE;
PUBLIC uint32 timer_antirebond_up = 0;
//---------------------------------------------------

PUBLIC bool_t b_activer_bip = FALSE;

//---------------------------
#if 0
PUBLIC etRunningPgl NEW_AnalyseBoitierDeCommande(etRunningStp *stpVal,
    etRunningRol *rolVal, etRunningKbd *kbdVal, etInUsingkey *keyVal,
    etRunningNet *netVal)
{
  static etRunningPgl pglVal = E_PGL_BOUCLE_PRINCIPALE;
  static bool_t oneshot = FALSE;
  static bool_t oneshot_2 = FALSE;

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
#endif

PUBLIC void CLAV_AnalyserEtat(etRunningStp mef_clavier)
{
  etInUsingkey la_touche = E_KEY_NON_DEFINI;
  uint8 uId = 0;
  static bool_t oneshot = FALSE;
  etInUsingkey toucheAction = AppData.key;
  etDefWifiMsg wifi_msg = E_MSG_NOT_SET;

#ifdef WATCHDOG_ENABLED
  vAHI_WatchdogRestart();
#endif

  if (b_DebutIt)
  {
    timer_antirebond_dow++;
  }

  if (b_compter_pression)
  {
    timer_duree_pression++;
  }

  if (b_FinIt)
  {
    timer_antirebond_up++;
  }

  wifi_msg = AppData.eWifi;

  switch (mef_clavier)
  {
    case E_KS_STP_NON_DEFINI:
    {
      vPrintf("Passage clavier en attente touche\n");
      AppData.pgl = E_PGL_BOUCLE_PRINCIPALE;
      AppData.stp = E_KS_STP_ATTENTE_TOUCHE;
      AppData.key = E_KEY_NON_DEFINI;

      if (AppData.rol == E_KS_ROL_NON_DEFINI)
        AppData.rol = E_KS_ROL_UTILISATEUR;

      if (AppData.kbd == E_KS_KBD_NON_DEFINI)
        AppData.kbd = E_KS_KBD_VIRTUAL_1;

      if (AppData.net == E_KS_NET_NON_DEFINI)
        AppData.net = E_KS_NET_NON_DEFINI;

      if (AppData.eWifi == E_MSG_NOT_SET)
        AppData.eWifi = E_MSG_NOT_SET;

#if 0
      maConfig.pgl = E_PGL_BOUCLE_PRINCIPALE;
      maConfig.key = E_KEY_NON_DEFINI;
      maConfig.rol = E_KS_ROL_UTILISATEUR;
      maConfig.kbd = E_KS_KBD_VIRTUAL_1;
      maConfig.stp = E_KS_STP_ATTENTE_TOUCHE;
      maConfig.net = E_KS_NET_NON_DEFINI;
      maConfig.msg = E_MSG_NOT_SET;
#endif
    }
    break;

    case E_KS_STP_ATTENTE_TOUCHE:
      // On regarde les fronts descendants
      vAHI_DioInterruptEdge(0, PBAR_CFG_NUMPAD_IN);
    break;

    case E_KS_STP_DEBUT_IT:
    {
      if (timer_antirebond_dow < C_TIME_REBOND_DOWN)
      {
        AppData.stp = E_KS_STP_DEBUT_IT;
      }
      else
      {
        b_DebutIt = FALSE;
        AppData.stp = E_KS_STP_COMPTER_DUREE_PRESSION;
        timer_duree_pression = 0;
        b_compter_pression = TRUE;
      }
    }
    break;

    case E_KS_STP_COMPTER_DUREE_PRESSION:
    {
      // On regarde les fronts Montant
      vAHI_DioInterruptEdge(PBAR_CFG_NUMPAD_IN, 0);
    }
    break;

    case E_KS_STP_REBOND_HAUT_COMMENCE:
    {
      if (timer_antirebond_up < C_TIME_REBOND_UP)
      {
        AppData.stp = E_KS_STP_REBOND_HAUT_COMMENCE;
      }
      else
      {
        b_FinIt = FALSE;
        vPrintf("Traitement It\n\n");
        AppData.stp = E_KS_STP_TRAITER_IT;
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
      //au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_RECHERCHE_RESEAU;
    break;

    case E_KS_STP_EN_PROGR_AVEC_BOITE:
    break;

    case E_KS_STP_ATTENDRE_FIN_CONFIG_BOITE:
    break;

    case E_KS_STP_ARMER_IT:
    {

      // Bloquer Its
      //vAHI_DioInterruptEnable(0, PBAR_CFG_NUMPAD_IN);
      //flush its
      //u32AHI_DioInterruptStatus();
      // Preparation Detection Front descendant sur entrees
      vAHI_DioInterruptEdge(0, PBAR_CFG_NUMPAD_IN);
      // deBloquer Its
      //vAHI_DioInterruptEnable(PBAR_CFG_NUMPAD_IN, 0);
      AppData.stp = E_KS_STP_ATTENTE_TOUCHE;

    }
    break;

    case E_KS_STP_TRAITER_IT:
    {
      la_touche = CLAV_AnalyseIts(&uId);
      if (la_touche != E_KEY_NON_DEFINI)
      {
        timer_touche[la_touche - 1] = (uint16) NEW_memo_delay_touche;
        vPrintf("Touche '%c' pendant '%d' ms, code dans pgm:%d\n\n",
            code_ascii[uId], timer_touche[la_touche - 1], la_touche);

        AppData.key = la_touche;

        // Une touche est reconnue on peut demander a la traiter
        AppData.stp = E_KS_STP_TRAITER_TOUCHE;
      }
      else
      {
        AppData.stp = E_KS_STP_ARMER_IT;
      }
    }
    break;

    case E_KS_STP_TRAITER_TOUCHE:
    {
      CLAV_GererTouche(toucheAction);
      AppData.stp = E_KS_STP_ARMER_IT;
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
  etRunningKbd modif_mode = AppData.kbd;
  etFlashMsg flash_val;

#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_GererMode", (void *) (AppData.pgl),
      E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  switch (AppData.rol)
  {
    case E_KS_ROL_UTILISATEUR:
      flash_val = ~E_FLASH_OFF;
    break;
    case E_KS_ROL_TECHNICIEN:
      flash_val = ~E_FLASH_ALWAYS;
    break;
    case E_KS_ROL_CHOISIR:
      au8Led_clav[C_CLAV_LED_INFO_3].mode = 0x10;
      flash_val = 0x10;
    break;
    default:
      flash_val = E_FLASH_RECHERCHE_RESEAU;
    break;
  }
  au8Led_clav[C_CLAV_LED_INFO_2].mode = flash_val;

  // montrer seulement le changement de role
  if (AppData.rol == E_KS_ROL_CHOISIR)
    return;

  switch (mode)
  {
    case E_KEY_NUM_MOD_1:
      au8Led_clav[C_CLAV_LED_INFO_3].mode = ~E_FLASH_OFF;
      modif_mode = E_KS_KBD_VIRTUAL_1;
    break;

    case E_KEY_NUM_MOD_2:
      au8Led_clav[C_CLAV_LED_INFO_3].mode = 0x2; //E_FLASH_MENU_LED_NET;
      modif_mode = E_KS_KBD_VIRTUAL_2;
    break;

    case E_KEY_NUM_MOD_3:
      au8Led_clav[C_CLAV_LED_INFO_3].mode = 0x05; //E_FLASH_MENU_BIP_CLAVIER;
      modif_mode = E_KS_KBD_VIRTUAL_3;
    break;

    case E_KEY_NUM_MOD_4:
      au8Led_clav[C_CLAV_LED_INFO_3].mode = 0x08; //E_FLASH_MENU_LIAISON;
      modif_mode = E_KS_KBD_VIRTUAL_4;
    break;

    case E_KEY_NUM_MOD_5:
      au8Led_clav[C_CLAV_LED_INFO_3].mode = 0x0B; //E_FLASH_ERREUR_DECTECTEE;
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
      && (AppData.stp != E_KS_STP_ULTRA_MODE))
  {
    compter_duree_mode = 0;
    start_timer_of_mode = TRUE;
  }

  AppData.kbd = modif_mode;
#if !NO_DEBUG_ON
  PBAR_DbgInside(stepper, gch_spaces, E_FN_OUT, AppData);
  PBAR_DbgTrace(E_FN_OUT, "CLAV_GererMode", (void *) AppData.pgl,
      E_DBG_TYPE_NET_STATE);
#endif
}

PUBLIC bool_t CLAV_TrouverAssociationToucheBoite(stToucheDef *touche,
    uint8 BoxId, uint8 *position)
{
  bool_t eReturn = FALSE;
  uint8 key_code = touche->la_touche - E_KEY_NUM_1;
  uint8 key_mode = touche->le_clavier - E_KS_KBD_VIRTUAL_1;
  uint8 nbBox = eeprom.netConf.ptr_boxList[key_mode][key_code];
  ;
  uint8 i = 0;
  uint8 useBox = 0;

#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_TrouverAssociationToucheBoite",
      (void *) (AppData.pgl), E_DBG_TYPE_NET_STATE);
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
      (void *) (AppData.pgl), E_DBG_TYPE_NET_STATE);
#endif
  return eReturn;
}
