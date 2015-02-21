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
PUBLIC void CLAV_AnalyserEtat(teClavState mef_clavier)
{
  uint16 max_time =0;
  static bool_t oneshot = FALSE;

#ifdef WATCHDOG_ENABLED
  vAHI_WatchdogRestart();
#endif

  switch(mef_clavier)
  {
    case E_CLAV_ETAT_EN_INITIALISATION:
      vPrintf("Passage clavier en attente touche\n");
      AppData.eClavState = E_CLAV_ETAT_EN_ATTENTE;
      AppData.eClavmod = E_CLAV_MODE_DEFAULT;
      AppData.usage = E_CLAV_USAGE_NORMAL;
      break;

    case E_CLAV_ETAT_EN_ATTENTE:
      break;

    case E_CLAV_SERVICE_ON:
      break;

    case E_CLAV_SERVICE_OFF:
      break;

    case E_CLAV_ULTRA_MODE:
      break;

    case E_CLAV_ATTENDRE_BOITE:
      au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_RECHERCHE_RESEAU;
      break;

    case E_CLAV_EN_PROGR_AVEC_BOITE:
      break;

    case E_CLAV_ATTENDRE_FIN_CONFIG_BOITE:
      break;

    case E_CLAV_ETAT_TRAITER_IT:
      if(AppData.eKeyPressed == E_KEY_DIESE||AppData.eKeyPressed == E_KEY_ETOILE)
      {
        max_time = C_TIME_ULTRA;

      }
      else
      {
        max_time = 	C_MAX_DURE_PRESSION;
      }
      // Verif que l'appuie sur le clavier est franc
      if(timer_appuie_touche >= max_time)
      {
        CLAV_ResetLecture();
      }
      break;

    case E_CLAV_ETAT_ANALYSER_TOUCHE:
    {

      AppData.eClavState = CLAV_GererTouche(AppData.eKeyPressed);

    }
    break;

    default:
      if (oneshot == FALSE){
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

  stepper = PBAR_DbgTrace(E_FN_IN,"CLAV_GererMode",(void *)(AppData.eAppState),E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper,E_FN_IN,AppData);
#endif

  switch(mode)
  {
    case E_KEY_MOD_1:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_OFF;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_OFF;
      modif_mode = E_CLAV_MODE_DEFAULT;
      break;

    case E_KEY_MOD_2:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_ERREUR_DECTECTEE;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_OFF;
      modif_mode = E_CLAV_MODE_2;
      break;

    case E_KEY_MOD_3:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_OFF;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_ERREUR_DECTECTEE;
      modif_mode = E_CLAV_MODE_3;
      break;

    case E_KEY_MOD_4:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_ALWAYS;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_ALWAYS;
      modif_mode = E_CLAV_MODE_4;
      break;

    default:
      au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_ERREUR_DECTECTEE;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_ERREUR_DECTECTEE;
      modif_mode = E_CLAV_MODE_NOT_SET;
      break;
  }

  if(modif_mode != E_CLAV_MODE_DEFAULT)
  {
    compter_duree_mode = 0;
    start_timer_of_mode = TRUE;
  }

  AppData.eClavmod = modif_mode;
#if !NO_DEBUG_ON
  PBAR_DbgInside(stepper,E_FN_OUT,AppData);
  PBAR_DbgTrace(E_FN_OUT,"CLAV_GererMode",(void *)AppData.eAppState,E_DBG_TYPE_NET_STATE);
#endif
}


PUBLIC bool_t CLAV_TrouverAssociationToucheBoite(stToucheDef *touche, uint8 BoxId, uint8 *position)
{
  bool_t eReturn = FALSE;
  uint8 key_code = ((touche->la_touche==E_KEY_ETOILE)?C_KEY_MEM_ALL:touche->la_touche - E_KEY_NUM_1);;
  uint8 key_mode = touche->le_mode - E_CLAV_MODE_DEFAULT;
  uint8 nbBox = eeprom.netConf.ptr_boxList[key_mode][key_code];;
  uint8 i =0;
  uint8 useBox = 0;

  vPrintf("   IN:CLAV_TrouverAssociationToucheBoite\n");
  vPrintf("    Nb boite associee à la touche %s -> %d\n",dbg_etCLAV_keys[touche->la_touche],nbBox);

  vPrintf("    Recherche dans la liste chainee des boites associee à la touche\n");
  for(i=0;(i<=nbBox)&& (i<C_MAX_BOXES);i++)
  {
    useBox = eeprom.netConf.boxList[key_mode][key_code][i];
    vPrintf("     Ptr_box:%d, boxid : %d\n",i,useBox);

    if(useBox == 0x00){
      // On a parcouru toute la liste
      // sans trouver de correspondance
      vPrintf("     Sauvegarde necessaire a position %d !\n",i);
      eReturn = FALSE;
      break;
    }
    else
    {
      if(useBox == BoxId){
        vPrintf("     Trouve a position:%d !\n",i);
        eReturn = TRUE;
        break;
      }
      else{
        vPrintf("     On regarde avec la suivante\n");
        continue;
      }
    }

  }

  // On memorise la position rouvee
  *position = i;
  vPrintf("   OUT:CLAV_TrouverAssociationToucheBoite\n");
  return eReturn;
}
