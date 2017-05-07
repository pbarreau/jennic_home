// -----------------------------------
// Projet	: PBPJ1
//	Fic		: m_config.h
//  Cree	: 30 juil. 2012
//	Par		: Administrateur
// -----------------------------------

// Definitions communes a : Coordonateur, routeur, end device

#ifndef _PBPJ1_MO_M_CONFIG_H_
#define _PBPJ1_MO_M_CONFIG_H_

#if defined __cplusplus
extern "C"
{
#endif
/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include <Jenie.h>
#include "led.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define NO_DEBUG_ON	FALSE

#if NO_DEBUG_ON
#define vPrintf(...)
#endif

// Config du reseau
#define PBAR_CHANNEL			0
#if PBAR_CHANNEL
#define PBAR_SCAN_CHANNELS		(1<<PBAR_CHANNEL)
#else
#define PBAR_SCAN_CHANNELS		0x07FFF800
#endif

#define PBAR_PAN_ID				0x1968
#define PBAR_NID				0xdeaddead

/* Flash addresses */
#define FLASH_SECTOR 			3
#define FLASH_START  			0x70000
#define FLASH_END    			0x80000

#if 0
#if (JENNIC_CHIP_FAMILY == JN514x)
#define FLASH_SECTOR 3
#define FLASH_START  0x30000
#define FLASH_END    0x40000
#else
#define FLASH_SECTOR 3
#define FLASH_START  0x18000
#define FLASH_END    0x20000
#endif
#endif

#define C_MAX_BOXES 20

// Buffer reception de trames entre mes elements 2 * Taille msg trame
#define	PBAR_RBUF_SIZE			6

// Gestion bouton pgm de carte
#define	PBAR_ATR_BTNPGM						30 // Anti rebond btn pgm
#define BUTTON_P_MASK (BUTTON_3_MASK << 1)  /* Mask for program button {v3} */

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

#if !NO_DEBUG_ON
typedef enum {
  E_FN_IN, E_FN_OUT, E_FN_END
} teDbgTrace;

typedef enum {
  E_DBG_TYPE_NET_STATE, E_DBG_TYPE_TYPE_MSG, E_DBG_TYPE_END
} teDbgModulEnum;

extern PUBLIC int PBAR_DbgTrace(teDbgTrace eSens, char *fn, void * val_enum,
    teDbgModulEnum enumType);
#endif

typedef enum {
  E_PGL_INITIALISATION,
  E_PGL_RECHERCHE_RESEAU,
  E_PGL_RESEAU_ETABLI,
  E_PGL_CONNECTION_RESEAU,
  E_PGL_PERTE_RESEAU,
  E_PGL_ATTENTE_ENREGISTREMENT_AU_PERE,
  E_PGL_BOUCLE_PRINCIPALE,
  E_PGL_DEFINITION_END
} teRunningPgl; // Program Level
extern PUBLIC char const *dbg_teRunningPgl[];
extern PUBLIC char const *dbg_teEventType[];

typedef enum {
  E_MSG_NOT_SET,
  E_MSG_DATA_ALL,
  E_MSG_DATA_SELECT,
  E_MSG_ASK_ID_BOX,
  E_MSG_RSP_ID_BOX,
  E_MSG_CFG_LIENS,
  E_MSG_CFG_BOX_END,
  E_MSG_NET_LED_OFF,
  E_MSG_NET_LED_ON,
  E_MSG_DEFINITION_END
} etDefWifiMsg;
extern PUBLIC char const *dbg_PBAR_TypeMsg[];

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
extern PUBLIC void bp_CommunMsgReseau(teRunningPgl *eState,
    teEventType eEventType, void *pvEventPrim);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
#if defined __cplusplus
}
#endif

#endif // _PBPJ1_MO_M_CONFIG_H_
/****************************************************************************/
/***        End of File                                                   ***/
/****************************************************************************/
