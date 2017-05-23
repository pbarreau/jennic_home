/*
 * dbg_e_msg.c
 *
 *  Created on: 18 juil. 2013
 *      Author: RapidOs
 */

#include <jendefs.h>
#include <string.h>
#include <Printf.h>

#include "Utils.h"
#include "e_config.h"

#if !NO_DEBUG_ON
PUBLIC char gch_spaces[20] = { 0 }; /// Global buffer definition for debug alignement

PUBLIC char const *dbg_teClavState[] = { "E_CLAV_ETAT_EN_INITIALISATION",
    "E_CLAV_ETAT_EN_ATTENTE", "E_CLAV_ETAT_TRAITER_IT",
    "E_CLAV_ETAT_ANALYSER_TOUCHE", "E_CLAV_SERVICE_ON", "E_CLAV_SERVICE_OFF",
    "E_CLAV_ATTENDRE_BOITE", "E_CLAV_ATTENDRE_FIN_CONFIG_BOITE",
    "E_CLAV_EN_PROGR_AVEC_BOITE", "E_CLAV_ETAT_UNDEF", "E_CLAV_ULTRA_MODE",
    "E_CLAV_ETAT_END" };

PUBLIC char const *dbg_etCLAV_role[] = { "E_CLAV_USAGE_NON_DEFINI",
    "E_CLAV_USAGE_NORMAL", "E_CLAV_USAGE_CONFIG", "E_CLAV_USAGE_END" };

PUBLIC char const *dbg_etCLAV_mod[] = { "E_CLAV_MODE_NOT_SET",
    "E_CLAV_MODE_DEFAULT", "E_CLAV_MODE_2", "E_CLAV_MODE_3", "E_CLAV_MODE_4",
    "E_CLAV_MODE_END"

};

PUBLIC char const * dbg_etCLAV_keys[] = { "E_NO_KEYS", "E_KEY_NUM_1",
    "E_KEY_NUM_2", "E_KEY_NUM_3", "E_KEY_NUM_4", "E_KEY_NUM_5", "E_KEY_NUM_6",
    "E_KEY_NUM_7", "E_KEY_NUM_8", "E_KEY_NUM_9", "E_KEY_NUM_0", "E_KEY_MOD_1",
    "E_KEY_MOD_2", "E_KEY_MOD_3", "E_KEY_MOD_4", "E_KEY_ETOILE", "E_KEY_DIESE",
    "E_KEYS_END" };

PUBLIC void PBAR_DbgInside(int level, char * pSpaces, teDbgTrace eSens,
    tsClavData val)
{
  char const *sens[] = { "IN:", "OUT:" };
  int i = 0;

  static etInUsingkey my_key = E_KEYS_NUM_END;
  static etRunningKbd my_mod = E_KS_KBD_END;
  static etRunningRol my_role = E_KS_ROL_END;
  static etRunningPgl my_state = E_KS_STP_END;

  memset(&gch_spaces, 0x00, sizeof(gch_spaces));
  for (i = 0; (i <= level) && (i < 20); i++)
  {
    strcat(gch_spaces, " ");
  }
  strcpy(pSpaces, gch_spaces);

  if (level > 0)
  {
    if (my_role != val.rol)
    {
      vPrintf("%s%s%s\n", gch_spaces, sens[eSens], dbg_etCLAV_role[val.rol]);
      my_role = val.rol;
    }

    if (my_state != val.eClavState)
    {
      vPrintf("%s%s%s\n", gch_spaces, sens[eSens],
          dbg_teClavState[val.eClavState]);
      my_state = val.eClavState;
    }

    if (my_mod != val.kbd)
    {
      vPrintf("%s%s%s\n", gch_spaces, sens[eSens],
          dbg_etCLAV_mod[val.kbd]);
      my_mod = val.kbd;
    }

    if (my_key != val.key)
    {
      vPrintf("%s%s%s\n", gch_spaces, sens[eSens],
          dbg_etCLAV_keys[val.key]);
      my_key = val.key;
    }
  }

  if ((level == 1) && (eSens == E_FN_OUT))
  {
    my_key = E_KEYS_NUM_END;
    my_mod = E_KS_KBD_END;
    my_role = E_KS_ROL_END;
    my_state = E_KS_STP_END;
  }

  if (eSens == E_FN_IN)
    vPrintf("\n");
}

#endif // #if !NO_DEBUG_ON
