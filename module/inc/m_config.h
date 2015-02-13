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
extern "C" {
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

#define C_MAX_BOXES 12

// Buffer reception de trames entre mes elements 2 * Taille msg trame
#define	PBAR_RBUF_SIZE			6

// Gestion bouton pgm de carte
#define	PBAR_ATR_BTNPGM						30 // Anti rebond btn pgm
#define BUTTON_P_MASK (BUTTON_3_MASK << 1)  /* Mask for program button {v3} */


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
	APP_INITIALISATION,
	APP_RECHERCHE_RESEAU,
	APP_RESEAU_ETABLI,
	APP_CONNECTION_RESEAU,
	APP_PERTE_RESEAU,
	APP_ATTENTE_ENREGISTREMENT_AU_PERE,
	APP_BOUCLE_PRINCIPALE
} teNetState;


 typedef enum{
  E_MSG_DATA_ALL,
  E_MSG_DATA_SELECT,
  E_MSG_ASK_ID_BOX,
  E_MSG_RSP_ID_BOX,
  E_MSG_CFG_LIENS,
  E_MSG_CFG_BOX_END
 }PBAR_TypeMsg;


/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
extern PUBLIC void bp_CommunMsgReseau(teNetState *eState,
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
