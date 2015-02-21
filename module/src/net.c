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


PUBLIC void bp_CommunMsgReseau(teNetState *eState,
		teEventType eEventType, void *pvEventPrim)
{
	tsNwkStartUp *pNet = (tsNwkStartUp*)pvEventPrim;
	tsData *psData = (tsData *) pvEventPrim;

	switch(eEventType)
	{
		case E_JENIE_REG_SVC_RSP:
		break;

		case E_JENIE_NETWORK_UP:
		{
			/* Indicates stack is up and running */
			vPrintf("\nReseau Ok\n");
			if(*eState == APP_RECHERCHE_RESEAU)
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
				//*eState = APP_RESEAU_ETABLI;
			}
		}
		break;

		case E_JENIE_DATA:
		{
			vPrintf("\nMsg de[%x:%x] sur %d octets\n",
					(uint32)(psData->u64SrcAddress >> 32),
					(uint32)(psData->u64SrcAddress &  0xFFFFFFFF),
					psData->u16Length
			);
		}
		break;


		case E_JENIE_STACK_RESET:
			break;

		case E_JENIE_POLL_CMPLT:
			break;

		default:
		{
			vPrintf("ERR:bp_CommunMsgReseau ->%s\n",dbg_teEventType[eEventType]);
		}
		break;
	}
}
