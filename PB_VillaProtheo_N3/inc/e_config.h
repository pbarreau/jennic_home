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

#define C_PRESSION_T1		85	/// Limite rotation mode
#define C_PRESSION_T2		85	/// Limite Bcast net
#define C_PRESSION_T3		100	/// Limite passage configuration
#define C_PRESSION_T4		100	/// Limite Save
#define C_PRESSION_T5		150	/// Limite Ultracare

// --------------------------
#ifdef CLAV_IS_VELLMAN
#define CLAV_NB_KEYS  10
#define C_MIN_KEY_PRESS_TIME  5  // en ms

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
} etInUsingkey;
extern PUBLIC char const * dbg_etCLAV_keys[];

typedef enum _clav_mod {
  E_KS_KBD_NON_DEFINI,    // Keyboard mode non definit
  E_KS_KBD_VIRTUAL_1,
  E_KS_KBD_VIRTUAL_2,
  E_KS_KBD_VIRTUAL_3,
  E_KS_KBD_VIRTUAL_4,
  E_KS_KBD_END
} etRunningKbd; // keyboard modes
extern PUBLIC char const *dbg_etCLAV_mod[];

typedef enum {
  E_KS_ROL_NON_DEFINI,
  E_KS_ROL_UTILISATEUR,
  E_KS_ROL_TECHNICIEN,
  E_KS_ROL_CHOISIR,
  E_KS_ROL_END
} etRunningRol; /// Keyboard role
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
  E_KS_STP_END
} etRunningStp; /// Keyboard states
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
  etRunningPgl pgl;
  etInUsingkey key;
  etRunningRol rol;
  etRunningKbd kbd;
  etRunningStp stp;
  etRunningNet net;
  etDefWifiMsg eWifi;
  tePcState ePcState;
  etRunningStp ePrevClav;
  uint8 ukey;
  uint8 u8BoxId;
  uint64 u64ServiceAddress;
} tsClavData;

typedef struct {
  etRunningPgl pgl;
  etRunningStp stp;
  etRunningRol rol;
  etRunningKbd kbd;
  etInUsingkey key;
  etRunningNet net;
  etDefWifiMsg msg;
} stParam;

#if !NO_DEBUG_ON
extern PUBLIC void PBAR_DbgInside(int level, char * pSpaces, teDbgTrace eSens,
    tsClavData val);
#endif

typedef struct _touche {
  etRunningRol le_role;
  etInUsingkey la_touche;
  etRunningKbd le_clavier;
} stToucheDef;

typedef struct {
  // Tableau de Config des Box
  // selon touche et mode
  uint8 boxData[(E_KS_KBD_END - 1)][(E_KEYS_NUM_END - 1)][C_MAX_BOXES];

  // Tableau Pointeur liste
  // box configuree pour une touche selon mode
  uint8 boxList[(E_KS_KBD_END - 1)][(E_KEYS_NUM_END - 1)][C_MAX_BOXES];

  // Tableau indiquant
  // le premier emplacement disponible
  // dans ptr_boxes pour une touche selon un mode
  uint8 ptr_boxList[(E_KS_KBD_END - 1)][(E_KEYS_NUM_END - 1)];

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
extern PUBLIC int8 NEW_TrouvePositionTouche(etInUsingkey laTouche);

extern PUBLIC etRunningPgl NEW_AnalyseBoitierDeCommande(etRunningStp *stpVal,
    etRunningRol*rolVal, etRunningKbd*kbdVal, etInUsingkey *keyVal,
    etRunningNet*netVal);

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
extern PUBLIC void MyStepDebug(void);

// ----------------
// clav_usage.c
// ----------------
extern PUBLIC etRunningStp CLAV_BoutonDeConfiguration(bool_t * bip_on);
extern PUBLIC void CLAV_NetMsgInput(tsData *psData);
extern PUBLIC etRunningStp CLAV_GererTouche(etInUsingkey keys);

// -----------------
// pgm_clav.c
// -----------------
extern PUBLIC etRunningStp CLAV_PgmNetMontrerClavier(void);
extern PUBLIC etRunningNet CLAV_PgmNetRetirerClavier(void);
extern PUBLIC etRunningNet CLAV_PgmNetMsgInput(tsData *psData);
extern PUBLIC etRunningStp CLAV_PgmActionTouche(etInUsingkey keys);

// -----------------
// usr_clav.c
// -----------------
extern PUBLIC etRunningStp CLAV_UsrActionTouche(etInUsingkey keys);
extern PUBLIC etRunningNet CLAV_UsrNetMsgInput(tsData *psData);
extern PUBLIC etInUsingkey CLAV_AnalyseIts(uint8 *position);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
#ifdef CLAV_IS_VELLMAN
#define CST_NB_MODES  4
extern const PUBLIC etInUsingkey R_Key_modes[CST_NB_MODES];
extern PUBLIC bool_t bStartTimerIo_19;
extern PUBLIC bool_t OneIt20;

extern PUBLIC uint16 TimingIo_19;
#endif

extern PUBLIC tsClavData AppData;
extern PUBLIC bpsFlash eeprom;
extern PUBLIC stParam maConfig;

extern PUBLIC bool_t start_timer_of_mode;
extern PUBLIC uint16 compter_duree_mode;

extern PUBLIC uint16 timer_appuie_touche;
extern PUBLIC bool_t b_activer_bip;
extern PUBLIC uint16 timer_touche[E_KEYS_NUM_END];
extern PUBLIC uint8 bufEmission[3];

extern PUBLIC char gch_spaces[20];
extern PUBLIC etFlashMsg mNetOkTypeFlash;
extern PUBLIC etInUsingkey key_code[];
extern PUBLIC bool_t b_EteindreNet;
extern PUBLIC bool_t NEW_traiter_It;
extern PUBLIC const uint8 code_ascii[];
extern PUBLIC bool_t b_NEW_start_press_count;
extern PUBLIC uint16 NEW_memo_delay_touche;
extern PUBLIC uint16 NEW_timer_appuie_touche;

extern PUBLIC etRunningStp (*MenuPossible[2][3])(stParam *param);
extern PUBLIC etRunningStp pFn1_1(stParam *param);
extern PUBLIC etRunningStp pFn1_2(stParam *param);
extern PUBLIC etRunningStp pFn1_3(stParam *param);

extern PUBLIC etRunningStp pFn2_1(stParam *param);
extern PUBLIC etRunningStp pFn2_2(stParam *param);
extern PUBLIC etRunningStp pFn2_3(stParam *param);

#if defined __cplusplus
}
#endif

#endif // _PBPJ1_ED_E_CONFIG_H_
/****************************************************************************/
/***        End of File                                                   ***/
/****************************************************************************/
