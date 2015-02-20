// -----------------------------------
// Projet	: PBPJ1
//	Fic		: net.c
//  Cree	: 23 août 2012
//	Par		: Administrateur
// -----------------------------------

#include <jendefs.h>
#include <Jenie.h>

#include <Printf.h>

#include "m_config.h"


PUBLIC void bp_CommunStackMgmtEvent(teAppState *eState,
    teEventType eEventType, void *pvEventPrim)
{
  tsNwkStartUp *pNet = (tsNwkStartUp*)pvEventPrim;


  switch(eEventType)
  {
    case E_JENIE_NETWORK_UP:
    {
      /* Indicates stack is up and running */
      vPrintf("\nReseau Ok\n");
      if(*eState == APP_STATE_WAITING_FOR_NETWORK)
      {
        vPrintf(" Canal:%d, Id:%x\n", pNet->u8Channel, pNet->u16PanID);
        vPrintf(" Pere:[%x:%x]\n",
            (uint32) (pNet->u64ParentAddress >> 32),
            (uint32) (pNet->u64ParentAddress & 0xFFFFFFFF)
        );
        vPrintf(" Ici:[%x:%x], Niveau:%d\n\n",
            (uint32) (pNet->u64LocalAddress >> 32),
            (uint32) (pNet->u64LocalAddress & 0xFFFFFFFF),
            pNet->u16Depth
        );
        *eState = APP_STATE_NETWORK_UP;
      }
    }
    break;

    default:
    {
      vPrintf("NotSet\n");
    }
    break;
  }

}
