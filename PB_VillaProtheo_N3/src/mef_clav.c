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
//---------------------------------------------------

PUBLIC bool_t b_activer_bip = FALSE;

//---------------------------
PUBLIC void CLAV_AnalyserEtat(etRunningStp mef_clavier)
{
  uint16 max_time = 0;
  static bool_t oneshot = FALSE;

#ifdef WATCHDOG_ENABLED
  vAHI_WatchdogRestart();
#endif

  switch (mef_clavier)
  {
    case E_KS_STP_NON_DEFINI:
      vPrintf("Passage clavier en attente touche\n");
      AppData.eClavState = E_KS_STP_ATTENTE_TOUCHE;
      AppData.kbd = E_KS_KBD_VIRTUAL_1;
      AppData.rol = E_KS_ROL_UTILISATEUR;
    break;

    case E_KS_STP_ATTENTE_TOUCHE:
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

    case E_KS_STP_TRAITER_IT:
      if (AppData.key == E_KEY_NUM_DIESE
          || AppData.key == E_KEY_NUM_ETOILE)
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

    case E_KS_STP_TRAITER_TOUCHE:
    {

      AppData.eClavState = CLAV_GererTouche(AppData.key);

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

  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_GererMode",
      (void *) (AppData.pgl), E_DBG_TYPE_NET_STATE);
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
  uint8 key_code = (
      (touche->la_touche == E_KEY_NUM_ETOILE) ?
          C_KEY_MEM_ALL : touche->la_touche - E_KEY_NUM_1);
  ;
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
  vPrintf("%sNb boite associee à la touche %s -> %d\n", gch_spaces,
      dbg_etCLAV_keys[touche->la_touche], nbBox);

  vPrintf("%sRecherche dans la liste chainee des boites associee à la touche\n",
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
