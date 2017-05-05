// -----------------------------------
// Projet	: PBPJ1
//	Fic		: e_config.h
//  Cree	: 30 juil. 2012
//	Par		: Administrateur
// -----------------------------------

// Noeud End Device (feuille)

#ifndef _PBPJ1_ED_E_CONFIG_H_
#define _PBPJ1_ED_E_CONFIG_H_

#if defined __cplusplus
extern "C"
{
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include <JPI.h>

#include "m_config.h"
#include "clav2pc.h"
#include "e_led.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define TEMPS_MODE			6000 	// en 10 * ms
#define C_TEMPS_ANTI_REBOND	2		//10*ms

#define C_MIN_DURE_PRESSION	4
#define C_MAX_DURE_PRESSION	200
#define C_TIME_ULTRA		200

#define C_PRESSION_T1		50	/// Limite rotation mode
#define C_PRESSION_T2		85	/// Limite Bcast net
#define C_PRESSION_T3		100	/// Limite passage configuration
#define C_PRESSION_T4		100	/// Limite Save
#define C_PRESSION_T5		150	/// Limite Ultracare


// --------------------------
#ifdef CLAV_IS_VELLMAN
#define CLAV_NB_KEYS  10
#define C_MIN_KEY_PRESS_TIME  10  // en ms

#define PBAR_CFG_NUMPAD_IN  (\
    E_JPI_DIO11_INT |\
    E_JPI_DIO12_INT |\
    E_JPI_DIO13_INT |\
    E_JPI_DIO14_INT |\
    E_JPI_DIO15_INT |\
    E_JPI_DIO16_INT |\
    E_JPI_DIO17_INT |\
    E_JPI_DIO18_INT |\
    E_JPI_DIO19_INT |\
    E_JPI_DIO20_INT )

#define PBAR_CFG_NUMPAD_OUT 0

#define TEMPS_IT_19 50
//---------END OF VELLMAN DEF -------------
#else
#define	PBAR_CFG_NUMPAD_IN	(\
		E_AHI_DIO12_INT |\
		E_AHI_DIO13_INT |\
		E_AHI_DIO14_INT |\
		E_AHI_DIO15_INT )

#define	PBAR_CFG_NUMPAD_OUT	(\
		E_JPI_DIO8_INT|\
		E_JPI_DIO9_INT|\
		E_JPI_DIO10_INT|\
		E_JPI_DIO11_INT)

#define C_CLAV_BUZER    E_AHI_DIO16_INT

#define C_CLAV_LGN_OUT    4 /// Nombre de ligne en sortie pour le clavier
#define C_CLAV_PIO_OUT_1  8 /// val L1
#define C_CLAV_PIO_OUT_2  9
#define C_CLAV_PIO_OUT_3  10
#define C_CLAV_PIO_OUT_4  11
#endif

#define	C_TEMPS_BIP			5

#define C_MAX_KEYS  10
#define C_KEY_MEM_ALL C_MAX_KEYS

#define C_MAX_TENTATIVES	4	/// Nb d'essai decodage clavier avant echec
#define	C_MAX_MODES	4
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum _keys {
  E_KEY_NON_DEFINI,
  E_KEY_NUM_1,
  E_KEY_NUM_2,
  E_KEY_NUM_3,
  E_KEY_NUM_4,
  E_KEY_NUM_5,
  E_KEY_NUM_6,
  E_KEY_NUM_7,
  E_KEY_NUM_8,
  E_KEY_NUM_9,
  E_KEY_NUM_0,
  E_KEY_NUM_MOD_1,
  E_KEY_NUM_MOD_2,
  E_KEY_NUM_MOD_3,
  E_KEY_NUM_MOD_4,
  E_KEY_NUM_MOD_5,
  E_KEY_NUM_MOD_6,
  E_KEY_NUM_DIESE,
  E_KEY_NUM_ETOILE,
  E_KEYS_NUM_END
} etCLAV_keys;
extern PUBLIC char const * dbg_etCLAV_keys[];

typedef enum _clav_mod {
  E_KM_NON_DEFINI,    // Keyboard mode non definit
  E_KM_1,
  E_KM_2,
  E_KM_3,
  E_KM_4,
  E_KM_END
} etCLAV_mod; // keyboard modes
extern PUBLIC char const *dbg_etCLAV_mod[];

typedef enum {
  E_KR_NON_DEFINI,
  E_KR_UTILISATEUR,
  E_KR_TECHNICIEN,
  E_KR_CHOISIR_ROLE,
  E_KR_END
} etCLAV_role; /// Keyboard role
extern PUBLIC char const *dbg_etCLAV_role[];

typedef enum _clav_state {
  E_KS_NON_DEFINI,      /// Mode de traitement du clavier non definit
  E_KS_ATTENTE_TOUCHE,  /// Attente que l'utilisateur appuie sur une touche du clavier
  E_KS_TRAITER_IT,      /// Determiner qu'elle touche appuyée
  E_KS_TRAITER_TOUCHE,  /// Effectuer traitement de la touche
  E_KS_SERVICE_ON,
  E_KS_SERVICE_OFF,
  E_KS_ATTENDRE_BOITE,
  E_KS_ATTENDRE_FIN_CONFIG_BOITE,
  E_KS_EN_PROGR_AVEC_BOITE,
  E_KS_ULTRA_MODE,
  E_KS_END
} teClavState; /// Keyboard states
extern PUBLIC char const *dbg_teClavState[];

typedef struct {
  teNetState eAppState;
  teClavState eClavState;
  tePcState ePcState;
  teClavState ePrevClav;
  etCLAV_keys eKeyPressed;
  etCLAV_role usage;
  etCLAV_mod eClavmod;
  uint8 ukey;
  uint8 u8BoxId;
  uint64 u64ServiceAddress;
} tsClavData;
#if !NO_DEBUG_ON
extern PUBLIC void PBAR_DbgInside(int level, char * pSpaces, teDbgTrace eSens,
    tsClavData val);
#endif

typedef struct _touche {
  etCLAV_keys la_touche;
  etCLAV_mod le_mode;
} stToucheDef;

typedef struct {
  // Tableau de Config des Box
  // selon touche et mode
  uint8 boxData[C_MAX_MODES][C_MAX_KEYS + 1][C_MAX_BOXES];

  // Tableau Pointeur liste
  // box configuree pour une touche selon mode
  uint8 boxList[C_MAX_MODES][C_MAX_KEYS + 1][C_MAX_BOXES + 1];

  // Tableau indiquant
  // le premier emplacement disponible
  // dans ptr_boxes pour une touche selon un mode
  uint8 ptr_boxList[C_MAX_MODES][C_MAX_KEYS + 1];

} bpsConfReseau;

// Init
// En mode 0 on configure la touche 2 donc ptr_boxList[0][2] = 0
// donc en indice 0 de ptr_boxList on va mettre la premiere des boites a configurer
// ie: boxList [0][2][0] = 8 // La boite 8 est la premiere associee a la touche 2
// dans box data on va mettre la configuration de chacune des 8 sorties
// ie : boxData[0][2][8]=0x01 // seul la sortie 1 est utilisee
//
// lors du changement de config boite pour cette meme touche ptr_boxList[0][2] vaudra 1

typedef struct {
  uint8 nbBoite;
  uint64 BoxAddr[C_MAX_BOXES];
  bpsConfReseau netConf;
} bpsFlash; // Info a mettre sur la flash

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
extern PUBLIC int8 NEW_TrouvePositionTouche(etCLAV_keys laTouche);
// ----------------
// mef_clav.c
// ----------------
extern PUBLIC void CLAV_AnalyserEtat(teClavState mef_clavier);
extern PUBLIC void CLAV_ResetLecture(void);
extern PUBLIC void CLAV_GererMode(etCLAV_keys mode);

#if fn1
extern PUBLIC bool_t CLAV_TrouverAssociationToucheBoite(etCLAV_mod eMode,
    etCLAV_keys eKey,uint8 BoxId);
#else
extern bool_t CLAV_TrouverAssociationToucheBoite(stToucheDef *touche,
    uint8 BoxId, uint8 *position);
#endif

// ----------------
// mef_pc.c
// ----------------
extern PUBLIC void CLAV_AnalyserPc(tePcState mef_pc);

// ----------------
// clav_usage.c
// ----------------
extern PUBLIC teClavState CLAV_BoutonDeConfiguration(bool_t * bip_on);
extern PUBLIC void CLAV_NetMsgInput(tsData *psData);
extern PUBLIC teClavState CLAV_GererTouche(etCLAV_keys keys);

// -----------------
// pgm_clav.c
// -----------------
extern PUBLIC teClavState CLAV_PgmNetMontrerClavier(void);
extern PUBLIC teClavState CLAV_PgmNetRetirerClavier(void);
extern PUBLIC teClavState CLAV_PgmNetMsgInput(tsData *psData);
extern PUBLIC teClavState CLAV_PgmActionTouche(etCLAV_keys keys);

// -----------------
// usr_clav.c
// -----------------
extern PUBLIC teClavState CLAV_UsrActionTouche(etCLAV_keys keys);
extern PUBLIC teClavState CLAV_UsrNetMsgInput(tsData *psData);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
#ifdef CLAV_IS_VELLMAN
#define CST_NB_MODES  4
extern const PUBLIC etCLAV_keys R_Key_modes[CST_NB_MODES];
extern PUBLIC bool_t bStartTimerIo_19;
extern PUBLIC bool_t OneIt20;

extern PUBLIC uint16 TimingIo_19;
#endif

extern PUBLIC tsClavData AppData;
extern PUBLIC bpsFlash eeprom;

extern PUBLIC bool_t start_timer_of_mode;
extern PUBLIC uint16 compter_duree_mode;

extern PUBLIC uint16 timer_appuie_touche;
extern PUBLIC bool_t b_activer_bip;
extern PUBLIC uint16 timer_touche[];
extern PUBLIC uint8 bufEmission[3];

extern PUBLIC char gch_spaces[20];
extern PUBLIC eLedInfo mNetOkTypeFlash;
extern PUBLIC etCLAV_keys key_code[];

#if defined __cplusplus
}
#endif

#endif // _PBPJ1_ED_E_CONFIG_H_
/****************************************************************************/
/***        End of File                                                   ***/
/****************************************************************************/
