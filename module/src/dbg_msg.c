/*
 * dbg_msg.c
 *
 *  Created on: 18 juil. 2013
 *      Author: RapidOs
 */

#include <jendefs.h>

#include <string.h>
#include <Printf.h>

#include "Utils.h"
#include "m_config.h"

#if !NO_DEBUG_ON

PUBLIC char const *dbg_etFlashMsg[] = { "E_FLASH_BP_TEST_SORTIES", "E_FLASH_OFF",
    "E_FLASH_RECHERCHE_RESEAU", "E_FLASH_RECHERCHE_BC",
    "E_FLASH_ERREUR_DECTECTEE", "E_FLASH_RESET_POSSIBLE",
    "E_FLASH_EN_ATTENTE_TOUCHE_BC", "E_FLASH_LIAISON_BP_BC_ON",
    "E_FLASH_ERASE_RESET_POSSIBLE", "E_FLASH_MENU_LED_NET",
    "E_FLASH_MENU_BIP_CLAVIER", "E_FLASH_MENU_LIAISON", "E_FLASH_RESEAU_ACTIF",
    "E_FLASH_RSP_BC", "E_FLASH_BP_EN_CONFIGURATION_SORTIES", "E_FLASH_ALWAYS",
    "E_FLASH_FIN" };

PUBLIC char const *dbg_teRunningPgl[] = { "E_PGL_INITIALISATION",
    "E_PGL_RECHERCHE_RESEAU", "E_PGL_RESEAU_ETABLI", "E_PGL_CONNECTION_RESEAU",
    "E_PGL_PERTE_RESEAU", "E_PGL_ATTENTE_ENREGISTREMENT_AU_PERE",
    "E_PGL_BOUCLE_PRINCIPALE", "E_PGL_DEFINITION_END" };

PUBLIC char const *dbg_etDefWifiMsg[] = { "E_MSG_NOT_SET", "E_MSG_DATA_ALL",
    "E_MSG_DATA_SELECT", "E_MSG_ASK_ID_BOX", "E_MSG_RSP_ID_BOX",
    "E_MSG_CFG_LIENS", "E_MSG_CFG_BOX_END", "E_MSG_NET_LED_OFF",
    "E_MSG_NET_LED_ON", "E_MSG_DEFINITION_END" };

/// Specific API Jeni
PUBLIC char const *dbg_teEventType[] = { "E_JENIE_REG_SVC_RSP",
    "E_JENIE_SVC_REQ_RSP", "E_JENIE_POLL_CMPLT", "E_JENIE_PACKET_SENT",
    "E_JENIE_PACKET_FAILED", "E_JENIE_NETWORK_UP", "E_JENIE_CHILD_JOINED",
    "E_JENIE_DATA", "E_JENIE_DATA_TO_SERVICE", "E_JENIE_DATA_ACK",
    "E_JENIE_DATA_TO_SERVICE_ACK", "E_JENIE_STACK_RESET", "E_JENIE_CHILD_LEAVE",
    "E_JENIE_CHILD_REJECTED" };

PUBLIC int PBAR_DbgTrace(teDbgTrace eSens, char *fn, void *val_enum,
    teDbgModulEnum enumType)
{
  static uint8 level = 0;

#if 0
  char const *sens[] =
  { "->", "<-"};
  char spaces[21] =
  { 0};
  char enumMsg[40] =
  { 0};
  int i = 0;

  if (eSens == E_FN_OUT)
  {
    if (level > 0)
    {
      level--;
    }
    else
    {
      level = 0;
    }
  }

  memset(&spaces, 0x00, sizeof(spaces));
  for (i = 0; (i < level) && (i < 20); i++)
  {
    strcat(spaces, " ");
  }

  vPrintf("%s%d%s%s(", spaces, level, sens[eSens], fn);

  if (eSens == E_FN_IN)
  {
    if (level < 20)
    {
      level++;
    }
    else
    {
      level = 20;
    }
  }

  switch (enumType)
  {
    case E_DBG_TYPE_NET_STATE:
    strcpy(enumMsg, dbg_teRunningPgl[(teNetState) val_enum]);
    break;

    case E_DBG_TYPE_TYPE_MSG:
    strcpy(enumMsg, dbg_PBAR_TypeMsg[(PBAR_TypeMsg) val_enum]);
    break;

    default:
    strcpy(enumMsg, "DBG enum ERROR !!\n");
  }
  vPrintf("%s)\n", enumMsg);
#endif
  return (level);
}
#endif // #if !NO_DEBUG_ON
