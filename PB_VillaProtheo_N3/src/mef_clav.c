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

//---------------------------------------------------

PUBLIC bool_t b_activer_bip = FALSE;

//---------------------------
PUBLIC void CLAV_AnalyserEtat(teClavState mef_clavier)
{
  etCLAV_keys la_touche = E_KEY_NON_DEFINI;
  uint8 uId = 0;
  static bool_t oneshot = FALSE;
  etCLAV_keys toucheAction = AppData.eKeyPressed;

#ifdef WATCHDOG_ENABLED
  vAHI_WatchdogRestart();
#endif

  if (b_NEW_start_press_count)
  {
    NEW_timer_appuie_touche++;
  }

  switch (mef_clavier)
  {
    case E_KS_NON_DEFINI:
      vPrintf("Passage clavier en attente touche\n");
      AppData.eClavState = E_KS_ATTENTE_TOUCHE;
      AppData.eClavmod = E_KM_1;
      AppData.usage = E_KR_UTILISATEUR;
    break;

    case E_KS_ATTENTE_TOUCHE:
    break;

    case E_KS_SERVICE_ON:
    break;

    case E_KS_SERVICE_OFF:
    break;

    case E_KS_ULTRA_MODE:
    break;

    case E_KS_ATTENDRE_BOITE:
      au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_RECHERCHE_RESEAU;
    break;

    case E_KS_EN_PROGR_AVEC_BOITE:
    break;

    case E_KS_ATTENDRE_FIN_CONFIG_BOITE:
    break;

    case E_KS_ARMER_IT:
    {
      // Rearmer detection It
      // Preparation Detection Front descendant sur entrees
      vAHI_DioInterruptEdge(0, PBAR_CFG_NUMPAD_IN);

      // Autoriser its clavier
      vAHI_DioInterruptEnable(PBAR_CFG_NUMPAD_IN, 0);
      AppData.eClavState = E_KS_ATTENTE_TOUCHE;
    }
    break;

    case E_KS_TRAITER_IT:
    {

      la_touche = CLAV_AnalyseIts(&uId);
      if (la_touche != E_KEY_NON_DEFINI)
      {
        vPrintf("Touche '%c' pendant '%d' ms, code dans pgm:%d\n\n",
            code_ascii[uId], NEW_memo_delay_touche / 100, la_touche);

        AppData.eKeyPressed = la_touche;
        AppData.ukey = uId;
        timer_touche[uId] = NEW_memo_delay_touche / 100;

        // Une touche est reconnue on peut demander a la traiter
        AppData.eClavState = E_KS_TRAITER_TOUCHE;
      }
      else
      {
        AppData.eClavState = E_KS_ARMER_IT;
      }
    }
    break;

    case E_KS_TRAITER_TOUCHE:
    {
      CLAV_GererTouche(toucheAction);
      AppData.eClavState = E_KS_ARMER_IT;
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

PUBLIC void CLAV_GererMode(etCLAV_keys mode)
{
  //teClavState mef_clav = AppData.eClavState;
  etCLAV_mod modif_mode = AppData.eClavmod;
#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_GererMode",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  switch (mode)
  {
    case E_KEY_NUM_MOD_1:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = ~E_FLASH_OFF;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = ~E_FLASH_OFF;
      modif_mode = E_KM_1;
    break;

    case E_KEY_NUM_MOD_2:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_ERREUR_DECTECTEE;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = ~E_FLASH_OFF;
      modif_mode = E_KM_2;
    break;

    case E_KEY_NUM_MOD_3:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = ~E_FLASH_OFF;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_ERREUR_DECTECTEE;
      modif_mode = E_KM_3;
    break;

    case E_KEY_NUM_MOD_4:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = ~E_FLASH_ALWAYS;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = ~E_FLASH_ALWAYS;
      modif_mode = E_KM_4;
    break;

    case E_KEY_NUM_MOD_5:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_ERREUR_DECTECTEE;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = ~E_FLASH_ALWAYS;
      modif_mode = E_KM_1;
    break;

    case E_KEY_NUM_MOD_6:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = ~E_FLASH_ALWAYS;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_ERREUR_DECTECTEE;
      modif_mode = E_KS_ULTRA_MODE;
    break;

    default:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_ERREUR_DECTECTEE;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_ERREUR_DECTECTEE;
      modif_mode = E_KM_NON_DEFINI;
    break;
  }

  if ((modif_mode != E_KM_1) && (AppData.eClavState != E_KS_ULTRA_MODE))
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
  uint8 key_code = (
      (touche->la_touche == E_KEY_NUM_ETOILE) ?
      C_KEY_MEM_ALL :
                                                touche->la_touche - E_KEY_NUM_1);
  ;
  uint8 key_mode = touche->le_mode - E_KM_1;
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
