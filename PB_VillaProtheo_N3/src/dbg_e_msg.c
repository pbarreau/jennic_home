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
PUBLIC char const *dbg_teRunningNet[] = { "E_KS_NET_NON_DEFINI",
    "E_KS_NET_CONF_START", "E_KS_NET_WAIT_CLIENT", "E_KS_NET_CLIENT_IN",
    "E_KS_NET_CONF_EN_COURS", "E_KS_NET_CONF_END", "E_KS_NET_CONF_BRK",
    "E_KS_NET_CLAV_ON", "E_KS_NET_MSG_IN", "E_KS_NET_ERROR", "E_KS_NET_END" };

PUBLIC char const *dbg_teClavState[] = { "E_KS_STP_NON_DEFINI", "E_TOUCHE",
    "E_KS_STP_TRAITER_IT", "E_KS_STP_TRAITER_TOUCHE", "E_ON", "E_OFF",
    "E_BOITE", "E_FIN_CONFIG_BOITE", "E_KS_STP_EN_PROGR_AVEC_BOITE",
    "E_KS_STP_ULTRA_MODE", "E_KS_STP_EN" };

PUBLIC char const *dbg_etCLAV_role[] = { "E_KS_ROL_NON_DEFINI",
    "E_KS_ROL_UTILISATEUR", "E_KS_ROL_TECHNICIEN", "E_KS_ROL_CHOISIR",
    "E_KS_ROL_END" };

PUBLIC char const *dbg_etCLAV_mod[] = { "E_KS_KBD_NON_DEFINI",
    "E_KS_KBD_VIRTUAL_1", "E_KS_KBD_VIRTUAL_2", "E_KS_KBD_VIRTUAL_3",
    "E_KS_KBD_VIRTUAL_4", "E_KS_KBD_END" };

PUBLIC char const * dbg_etCLAV_keys[] = { "E_KEY_NON_DEFINI", "E_KEY_NUM_1",
    "E_KEY_NUM_2", "E_KEY_NUM_3", "E_KEY_NUM_4", "E_KEY_NUM_5", "E_KEY_NUM_6",
    "E_KEY_NUM_7", "E_KEY_NUM_8", "E_KEY_NUM_9", "E_KEY_NUM_0",
    "E_KEY_NUM_MOD_1", "E_KEY_NUM_MOD_2", "E_KEY_NUM_MOD_3", "E_KEY_NUM_MOD_4",
    "E_KEY_NUM_MOD_5", "E_KEY_NUM_MOD_6", "E_KEY_NUM_DIESE", "E_KEY_NUM_ETOILE",
    "E_KEYS_NUM_END" };

PUBLIC void PBAR_DbgInside(int level, char * pSpaces, teDbgTrace eSens,
    tsClavData val)
{

  int i = 0;
#if 0
  static etInUsingkey my_key = E_KEYS_NUM_END;
  static etRunningKbd my_mod = E_KS_KBD_END;
  static etRunningRol my_role = E_KS_ROL_END;
  static etRunningPgl my_state = E_KS_STP_END;
  char const *sens[] =
  { "IN:", "OUT:"};
#endif

  memset(&gch_spaces, 0x00, sizeof(gch_spaces));
  for (i = 0; (i <= level) && (i < 20); i++)
  {
    strcat(gch_spaces, " ");
  }
  strcpy(pSpaces, gch_spaces);

#if 0
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
      vPrintf("%s%s%s\n", gch_spaces, sens[eSens], dbg_etCLAV_mod[val.kbd]);
      my_mod = val.kbd;
    }

    if (my_key != val.key)
    {
      vPrintf("%s%s%s\n", gch_spaces, sens[eSens], dbg_etCLAV_keys[val.key]);
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
#endif
}

#endif // #if !NO_DEBUG_ON
