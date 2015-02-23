/*
 * dbg_msg.c
 *
 *  Created on: 18 juil. 2013
 *      Author: RapidOs
 */

#if !NO_DEBUG_ON
#include <jendefs.h>

#include <string.h>
#include <Printf.h>

#include "Utils.h"
#include "m_config.h"

PUBLIC char const *dbg_teNetState[]={
    "APP_INITIALISATION",
    "APP_RECHERCHE_RESEAU",
    "APP_RESEAU_ETABLI",
    "APP_CONNECTION_RESEAU",
    "APP_PERTE_RESEAU",
    "APP_ATTENTE_ENREGISTREMENT_AU_PERE",
    "APP_BOUCLE_PRINCIPALE",
    "APP_DEFINITION_END"
};

PUBLIC char const *dbg_PBAR_TypeMsg[]={
    "E_MSG_DATA_ALL",
    "E_MSG_DATA_SELECT",
    "E_MSG_ASK_ID_BOX",
    "E_MSG_RSP_ID_BOX",
    "E_MSG_CFG_LIENS",
    "E_MSG_CFG_BOX_END",
    "E_MSG_DEFINITION_END"
};

/// Specific API Jeni
PUBLIC char const *dbg_teEventType[]={
    "E_JENIE_REG_SVC_RSP",
    "E_JENIE_SVC_REQ_RSP",
    "E_JENIE_POLL_CMPLT",
    "E_JENIE_PACKET_SENT",
    "E_JENIE_PACKET_FAILED",
    "E_JENIE_NETWORK_UP",
    "E_JENIE_CHILD_JOINED",
    "E_JENIE_DATA",
    "E_JENIE_DATA_TO_SERVICE",
    "E_JENIE_DATA_ACK",
    "E_JENIE_DATA_TO_SERVICE_ACK",
    "E_JENIE_STACK_RESET",
    "E_JENIE_CHILD_LEAVE",
    "E_JENIE_CHILD_REJECTED"
};

PUBLIC int PBAR_DbgTrace(teDbgTrace eSens, char *fn, void *val_enum, teDbgModulEnum enumType)
{
  static uint8 level = 0;
  char const *sens[]={"->","<-"};
  char spaces[21]={0};
  char enumMsg[40]={0};
  int i=0;


  if(eSens == E_FN_OUT)
  {
    if(level > 0){
      level--;
    }
    else
    {
      level = 0;
    }
  }

  memset(&spaces,0x00,sizeof(spaces));
  for(i=0;(i<level) && (i<20);i++)
  {
    strcat(spaces," ");
  }

  vPrintf("%s%d%s%s(",spaces,level,sens[eSens],fn);

  if(eSens == E_FN_IN)
  {
    if(level <20){
      level++;
    }
    else
    {
      level =20;
    }
  }



  switch(enumType)
  {
    case E_DBG_TYPE_NET_STATE:
      strcpy(enumMsg,dbg_teNetState[(teNetState)val_enum]);
      break;

    case E_DBG_TYPE_TYPE_MSG:
      strcpy(enumMsg,dbg_PBAR_TypeMsg[(PBAR_TypeMsg)val_enum]);
      break;

    default:
      strcpy(enumMsg,"DBG enum ERROR !!\n");
  }
  vPrintf("%s)\n",enumMsg);

  return(level);
}
#endif
