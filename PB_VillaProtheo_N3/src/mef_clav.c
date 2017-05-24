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
PUBLIC bool_t b_DebutIt = FALSE;
PUBLIC uint32 timer_antirebond_dow = 0;

PUBLIC bool_t b_compter_pression = FALSE;
PUBLIC uint32 timer_duree_pression = 0;

PUBLIC bool_t b_FinIt = FALSE;
PUBLIC uint32 timer_antirebond_up = 0;
//---------------------------------------------------

//---------------------------------------------------

PUBLIC bool_t b_activer_bip = FALSE;

//---------------------------
PUBLIC void CLAV_AnalyserEtat(etRunningStp mef_clavier)
{
  uint16 max_time = 0;
  static bool_t oneshot = FALSE;
  static etInUsingkey la_touche = E_KEY_NON_DEFINI;
  uint8 uId = 0;


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

  switch (mef_clavier)
  {
    case E_KS_STP_NON_DEFINI:
      vPrintf("Passage clavier en attente touche\n");
      AppData.stp = E_KS_STP_ATTENTE_TOUCHE;
      AppData.kbd = E_KS_KBD_VIRTUAL_1;
      AppData.rol = E_KS_ROL_UTILISATEUR;
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
      au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_RECHERCHE_RESEAU;
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
      //la_touche = CLAV_AnalyseIts(&uId);
      if (la_touche != E_KEY_NON_DEFINI)
      {
#if 0
        timer_touche[la_touche - 1] = (uint16) NEW_memo_delay_touche;
        vPrintf("Touche '%c' pendant '%d' ms, code dans pgm:%d\n\n",
            code_ascii[uId], timer_touche[la_touche - 1], la_touche);

        AppData.key = la_touche;
#endif
        // Une touche est reconnue on peut demander a la traiter
        AppData.stp = E_KS_STP_TRAITER_TOUCHE;
      }
      else
      {
        AppData.stp = E_KS_STP_ARMER_IT;
      }
    }
    break;


#if 0
    case E_KS_STP_TRAITER_IT:
      if (AppData.key == E_KEY_NUM_DIESE || AppData.key == E_KEY_NUM_ETOILE)
      {
        max_time = C_TIME_ULTRA;
      }
      else
      {
        max_time = C_MAX_DURE_PRESSION;
      }
      // Verif que l'appuie sur le clavier est franc
      if (timer_appuie_touche >= max_time)
      {
        CLAV_ResetLecture();
      }
    break;
#endif
    case E_KS_STP_TRAITER_TOUCHE:
    {

      AppData.eClavState = CLAV_GererTouche(AppData.key);
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
#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_GererMode", (void *) (AppData.pgl),
      E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  switch (mode)
  {
    case E_KEY_NUM_MOD_1:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_OFF;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_OFF;
      modif_mode = E_KS_KBD_VIRTUAL_1;
    break;

    case E_KEY_NUM_MOD_2:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_ERREUR_DECTECTEE;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_OFF;
      modif_mode = E_KS_KBD_VIRTUAL_2;
    break;

    case E_KEY_NUM_MOD_3:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_OFF;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_ERREUR_DECTECTEE;
      modif_mode = E_KS_KBD_VIRTUAL_3;
    break;

    case E_KEY_NUM_MOD_4:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_ALWAYS;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_ALWAYS;
      modif_mode = E_KS_KBD_VIRTUAL_4;
    break;

    default:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_ERREUR_DECTECTEE;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_ERREUR_DECTECTEE;
      modif_mode = E_KS_KBD_NON_DEFINI;
    break;
  }

  if (modif_mode != E_KS_KBD_VIRTUAL_1)
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
  vPrintf("%sNb boite pour touche %s -> %d\n", gch_spaces,
      dbg_etCLAV_keys[touche->la_touche], nbBox);

  vPrintf("%sRecherche dans la liste chainee des boites\n", gch_spaces);
  for (i = 0; (i <= nbBox) && (i < C_MAX_BOXES); i++)
  {
    useBox = eeprom.netConf.boxList[key_mode][key_code][i];

    if (useBox == 0x00)
    {
      // On a parcouru toute la liste
      // sans trouver de correspondance
      vPrintf("%s Sauvegarde necessaire  position %d !\n", gch_spaces, i);
      eReturn = FALSE;
      break;
    }
    else
    {
      vPrintf("%sPtr_box:%d, cur_boxid : %d\n", gch_spaces, i, useBox);

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
