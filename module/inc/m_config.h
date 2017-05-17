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
#include <Button.h>
#include <Jenie.h>
#include "services.h"
#include "m_network_msg.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define NO_DEBUG_ON	FALSE

#if NO_DEBUG_ON
#define vPrintf(...)
#endif

#define	C_IMPULSION_COURTE	60
#define	C_IMPULSION_LONGUE	200
#define	C_IMPULSION_RESET_IN	500
#define	C_IMPULSION_RESET_OUT	1000
#define	C_IMPULSION_ERASE_RST	1000

// Config du reseau
#define	PBAR_E_POLL_PER		10
#define PBAR_E_PING_PER		10

#define PBAR_RTBL_SIZE		100

#define PBAR_CHANNEL		0
#if PBAR_CHANNEL
#define PBAR_SCAN_CHANNELS	(1<<PBAR_CHANNEL)
#else
#define PBAR_SCAN_CHANNELS	0x07FFF800
#endif

#define PBAR_PAN_ID             0x1968
#define PBAR_NID		0xdeaddead

/* Flash addresses */
#define FLASH_SECTOR            7
#define FLASH_START             0x70000
#define FLASH_END               0x80000

#if 0
#if (JENNIC_CHIP_FAMILY == JN514x)
#define FLASH_SECTOR            3
#define FLASH_START             0x30000
#define FLASH_END               0x40000
#else
#define FLASH_SECTOR            3
#define FLASH_START             0x18000
#define FLASH_END               0x20000
#endif
#endif

#if 0
#define PBAR_DEBUT_LED_CO       19
#define PBAR_DEBUT_LED_ED       10

#define vLedControl(LED,ON) \
    vAHI_DioSetOutput((ON) ? 0 : (1 << (PBAR_DEBUT_LED_CO + LED)), \
        (ON) ? (1 << (PBAR_DEBUT_LED_CO + LED)) : 0)
#endif

#define C_CLEF_VIDE             0x00
#define C_MAX_BOXES             20

// Buffer reception de trames entre mes elements 2 * Taille msg trame
#define	PBAR_RBUF_SIZE		6

// Gestion bouton pgm de carte
#define	PBAR_ATR_BTNPGM 	30 // Anti rebond btn pgm
#define BUTTON_P_MASK           (BUTTON_3_MASK << 1)  /* Mask for program button {v3} */

#if 0
// Gestion des Leds
#define vLedCtrlKpd(ON,LED) \
    vAHI_DioSetOutput( \
        (ON) ? 0             : (1<<LED), \
            (ON) ? (1<<LED) : 0\
    )

vAHI_DioSetOutput(
    (ON) ? (C_BAR_LED_1) : 0,
    (ON) ? 0 : (C_BAR_LED_1)\
)
#endif

#define PBAR_DEBUT_IO	        0

typedef enum {
  E_FLASH_BP_TEST_SORTIES = 0x00,
  E_FLASH_OFF = 0x00,
  E_FLASH_RECHERCHE_RESEAU = 0x01,
  E_FLASH_RECHERCHE_BC = 0x01,
  E_FLASH_ERREUR_DECTECTEE = 0x03,
  E_FLASH_RESET_POSSIBLE = 0x05,
  E_FLASH_EN_ATTENTE_TOUCHE_BC = 0x07,
  E_FLASH_LIAISON_BP_BC_ON = 0x07,
  E_FLASH_ERASE_RESET_POSSIBLE = 0x09,
  E_FLASH_RESEAU_ACTIF = 0x10,
  E_FLASH_BP_EN_CONFIGURATION_SORTIES = 0x20,
  E_FLASH_ALWAYS = 0xFF,
  E_FLASH_FIN = 0xFF
} ebpLedInfo;

typedef struct {
  uint8 pio;
  ebpLedInfo mode;
  bool_t actif;
} sbpLed;

typedef enum {
  APP_STATE_WAITING_FOR_NETWORK,
  APP_STATE_NETWORK_UP,
  APP_STATE_REGISTERING_SERVICE,
  APP_STATE_WAITING_FOR_REGISTER_SERVICE,
  APP_STATE_SERVICE_REGISTERED,
  APP_STATE_REQUEST_SERVICES,
  APP_STATE_WAITING_FOR_REQUEST_SERVICE,
  APP_STATE_SERVICE_REQUEST_RETURNED,

  // Clavier
  APP_STATE_CONF_NEEDED,
  APP_STATE_WAITING_FOR_CONF_SERVICE,
  APP_STATE_CONF_START,
  APP_STATE_ATTENDRE_BOITES,
  APP_STATE_ATTENDRE_SEL_MODE_AND_KBD,
  APP_STATE_SEND_MODE_AND_KBD,
  APP_STATE_ATTENDRE_RETOUR_CFG_BOITE,
  APP_STATE_EXECUTER_TOUCHE_KBD,
  APP_STATE_REMOVE_SVC,
  APP_STATE_ATTENDRE_FIN_REMOVE_SVC,
  APP_STATE_SVC_IS_REMOVE,

  // coordonateur
  APP_STATE_TST_START_LUMIERES,
  APP_STATE_TST_STOP_LUMIERES,
  APP_STATE_RECHERCHE_CLAVIER,
  APP_STATE_ATTENTE_CLAV_RSP,
  APP_STATE_CLAV_READY,
  APP_STATE_SET_MY_OUTPUT,
  APP_STATE_ATTENDRE_FIN_CFG_LOCAL,
  APP_STATE_FIN_CFG_BOX,
  APP_STATE_MK_KBD_ALL,
  APP_STATE_REPONSE_CLAVIER_TROP_LONG,

  // Commun
  APP_STATE_CHANGE_MOD,
  APP_STATE_TRAITER_INPUT_MESSAGE,
  APP_STATE_RUNNING
} teAppState;

typedef enum {
  E_CLAV_EN_NON_DEFINI, E_CLAV_EN_USAGE, E_CLAV_EN_CONFIG
} bpeClav;

typedef enum {
  E_CLAV_MODE_NOT_SET,
  E_CLAV_MODE_DEFAULT,
  E_CLAV_MODE_1,
  E_CLAV_MODE_2,
  E_CLAV_MODE_3,
  E_CLAV_MODE_END
} PBAR_E_KeyMode;

typedef enum {
  E_KPD_A,
  E_KPD_B,
  E_KPD_C,
  E_KPD_D,
  E_KPD_0,
  E_KPD_1,
  E_KPD_2,
  E_KPD_3,
  E_KPD_4,
  E_KPD_5,
  E_KPD_6,
  E_KPD_7,
  E_KPD_8,
  E_KPD_9,
  E_KPD_ALL, /// touche *
  E_KPD_NONE,
  E_KPD_MODE, /// touche #
  E_KPD_END_DEF
} PBAR_KIT_8046;

typedef struct {
  teAppState eAppState;
  bpeClav eClavState;
  uint8 u8BoxId;
  uint64 u64ServiceAddress;
} tsAppData;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
extern PUBLIC void PBAR_ClignoterLedNFois(uint32 gpio, uint8 n);
extern PUBLIC void bp_CommunStackMgmtEvent(teAppState *eState,
    teEventType eEventType, void *pvEventPrim);
extern PUBLIC void PBAR_ClignoteLed_1(void);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern PUBLIC bool_t bStartFlashing;
extern PUBLIC uint32 mLedId;
//extern PUBLIC sbpLed au8Led[2];

#if defined __cplusplus
}
#endif

#endif // _PBPJ1_MO_M_CONFIG_H_
/****************************************************************************/
/***        End of File                                                   ***/
/****************************************************************************/
