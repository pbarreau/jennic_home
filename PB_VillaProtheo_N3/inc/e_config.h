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
#define C_TIME_REBOND_DOWN  300
#define C_TIME_REBOND_UP    300

#define C_MIN_DURE_PRESSION	4
#define C_MAX_DURE_PRESSION	200
#define C_TIME_ULTRA		200

#define C_PRESSION_T1		30	/// Limite activation bip
#define C_PRESSION_T2		60	/// Limite Bcast Off
#define C_PRESSION_T3		90	/// Limite Bcast On
#define C_PRESSION_T4		120	/// Limite Save
#define C_PRESSION_T5		150	/// Limite Ultracare

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

#define	C_TEMPS_BIP			5
#define	C_CLAV_BUZER		E_AHI_DIO16_INT

#define C_CLAV_LGN_OUT		4	/// Nombre de ligne en sortie pour le clavier
#define C_CLAV_PIO_OUT_1	8	/// val L1
#define C_CLAV_PIO_OUT_2	9
#define C_CLAV_PIO_OUT_3	10
#define C_CLAV_PIO_OUT_4	11

#define C_MAX_TENTATIVES	4	/// Nb d'essai decodage clavier avant echec
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
} etInUsingkey;
extern PUBLIC char const * dbg_etCLAV_keys[];
#define C_MAX_KEYS  (E_KEYS_NUM_END -E_KEY_NUM_1)

typedef enum _clav_mod {
  E_KS_KBD_NON_DEFINI,
  E_KS_KBD_VIRTUAL_1,
  E_KS_KBD_VIRTUAL_2,
  E_KS_KBD_VIRTUAL_3,
  E_KS_KBD_VIRTUAL_4,
  E_KS_KBD_END
} etRunningKbd;
extern PUBLIC char const *dbg_etCLAV_mod[];
#define C_MAX_MODES (E_KS_KBD_END - E_KS_KBD_VIRTUAL_1)
//#define C_KEY_MEM_ALL C_MAX_KEYS

typedef enum {
  E_KS_ROL_NON_DEFINI,
  E_KS_ROL_UTILISATEUR,
  E_KS_ROL_TECHNICIEN,
  E_KS_ROL_CHOISIR,
  E_KS_ROL_END
} etRunningRol;
extern PUBLIC char const *dbg_etCLAV_role[];

typedef enum _clav_state {
  E_KS_STP_NON_DEFINI,      /// Mode de traitement du clavier non definit
  E_KS_STP_ATTENTE_TOUCHE, /// Attente que l'utilisateur appuie sur une touche du clavier
  E_KS_STP_ARMER_IT,        /// Preparation lecture clavier
  E_KS_STP_TRAITER_IT,      /// Determiner qu'elle touche appuyée
  E_KS_STP_TRAITER_TOUCHE,  /// Effectuer traitement de la touche
  E_KS_STP_SERVICE_ON,
  E_KS_STP_SERVICE_OFF,
  E_KS_STP_ATTENDRE_BOITE,
  E_KS_STP_ATTENDRE_FIN_CONFIG_BOITE,
  E_KS_STP_EN_PROGR_AVEC_BOITE,
  E_KS_STP_ULTRA_MODE,
  E_KS_STP_DEBUT_IT,
  E_KS_STP_REBOND_BAS_FINI,
  E_KS_STP_COMPTER_DUREE_PRESSION,
  E_KS_STP_REBOND_HAUT_COMMENCE,
  E_KS_STP_END
} etRunningStp;
extern PUBLIC char const *dbg_teClavState[];

typedef enum _clav_NetState {
  E_KS_NET_NON_DEFINI,
  E_KS_NET_CONF_START,
  E_KS_NET_WAIT_CLIENT,
  E_KS_NET_CLIENT_IN,
  E_KS_NET_CONF_EN_COURS,
  E_KS_NET_CONF_END, // Config correctement terminee
  E_KS_NET_CONF_BRK, // Configuration reseau interrompu
  E_KS_NET_CLAV_ON,
  E_KS_NET_MSG_IN,
  E_KS_NET_ERROR,
  E_KS_NET_END
} etRunningNet;
extern PUBLIC char const *dbg_teRunningNet[];

typedef struct {
  etDefWifiMsg eWifi;
  etInUsingkey key;
  etRunningKbd kbd;
  etRunningNet net;
  etRunningPgl pgl;
  etRunningRol rol;
  etRunningStp eClavState;
  etRunningStp stp;
  tePcState ePcState;
  uint64 u64ServiceAddress;
  uint8 u8BoxId;
  uint8 ukey;
} tsClavData;
#if !NO_DEBUG_ON
extern PUBLIC void PBAR_DbgInside(int level, char * pSpaces, teDbgTrace eSens,
    tsClavData val);
#endif

typedef struct _touche {
  etInUsingkey la_touche;
  etRunningKbd le_clavier;
} stToucheDef;

typedef struct {
  // Tableau de Config des Box
  // selon touche et mode
  uint8 boxData[C_MAX_MODES][C_MAX_KEYS][C_MAX_BOXES+1];

  // Tableau Pointeur liste
  // box configuree pour une touche selon mode
  uint8 boxList[C_MAX_MODES][C_MAX_KEYS][C_MAX_BOXES+1];

  // Tableau indiquant
  // le premier emplacement disponible
  // dans ptr_boxes pour une touche selon un mode
  uint8 ptr_boxList[C_MAX_MODES][C_MAX_KEYS];

} bpsConfReseau;

// Pour certaine flash a l'init tout est 0xff pour d'autre c'est 0x00
// comme la boite me sert de clef de recherche je n'utilise pas la valeur box_id = 0
// donc je pers en memoire...raison du C_MAX_BOXES+1

// Pour un clavier virtuel donne une des touches de ce clavier peut agir
// sur un certain nombre de boites (ptr_boxList)

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
  uint64 BoxAddr[C_MAX_BOXES+1];
  bpsConfReseau netConf;
} bpsFlash; // Info a mettre sur la flash

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
// ----------------
// mef_clav.c
// ----------------
extern PUBLIC void CLAV_AnalyserEtat(etRunningStp mef_clavier);
extern PUBLIC void CLAV_ResetLecture(void);
extern PUBLIC void CLAV_GererMode(etInUsingkey mode);

#if fn1
extern PUBLIC bool_t CLAV_TrouverAssociationToucheBoite(etRunningKbd eMode,
    etInUsingkey eKey,uint8 BoxId);
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
extern PUBLIC etRunningStp CLAV_BoutonDeConfiguration(bool_t * bip_on);
extern PUBLIC void CLAV_NetMsgInput(tsData *psData);
extern PUBLIC etRunningStp CLAV_GererTouche(etInUsingkey keys);
extern PUBLIC etInUsingkey CLAV_AnalyseIts(uint8 *position);

// -----------------
// pgm_clav.c
// -----------------
extern PUBLIC etRunningStp CLAV_PgmNetMontrerClavier(void);
extern PUBLIC etRunningStp CLAV_PgmNetRetirerClavier(void);
extern PUBLIC etRunningStp CLAV_PgmNetMsgInput(tsData *psData);
extern PUBLIC etRunningStp CLAV_PgmActionTouche(etInUsingkey keys);

// -----------------
// usr_clav.c
// -----------------
extern PUBLIC etRunningStp CLAV_UsrActionTouche(etInUsingkey keys);
extern PUBLIC etRunningStp CLAV_UsrNetMsgInput(tsData *psData);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern PUBLIC tsClavData AppData;
extern PUBLIC bpsFlash eeprom;

extern PUBLIC bool_t start_timer_of_mode;
extern PUBLIC uint16 compter_duree_mode;

extern PUBLIC uint16 timer_appuie_touche;
extern PUBLIC bool_t b_activer_bip;
extern PUBLIC uint16 timer_touche[E_KEYS_NUM_END];
extern PUBLIC uint8 bufEmission[3];

extern PUBLIC char gch_spaces[20];
extern PUBLIC eLedInfo mNetOkTypeFlash;
extern PUBLIC const uint8 code_ascii[];
extern PUBLIC uint32 NEW_memo_delay_touche;

//--------------
extern PUBLIC bool_t b_DebutIt;
extern PUBLIC uint32 timer_antirebond_dow;
extern PUBLIC bool_t b_compter_pression;
extern PUBLIC uint32 timer_duree_pression;
extern PUBLIC bool_t b_FinIt;
extern PUBLIC uint32 timer_antirebond_up;

#if defined __cplusplus
}
#endif

#endif // _PBPJ1_ED_E_CONFIG_H_
/****************************************************************************/
/***        End of File                                                   ***/
/****************************************************************************/
