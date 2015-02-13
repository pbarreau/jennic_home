/*
 * dbg_e_msg.c
 *
 *  Created on: 18 juil. 2013
 *      Author: RapidOs
 */


#if !NO_DEBUG_ON
#include <jendefs.h>

PUBLIC char const *dbg_teClavState[]={
		"E_CLAV_ETAT_EN_INITIALISATION",
		"E_CLAV_ETAT_EN_ATTENTE",
		"E_CLAV_ETAT_TRAITER_IT",
		"E_CLAV_ETAT_ANALYSER_TOUCHE",
		"E_CLAV_SERVICE_ON",
		"E_CLAV_SERVICE_OFF",
		"E_CLAV_ATTENDRE_BOITE",
		"E_CLAV_ATTENDRE_FIN_CONFIG_BOITE",
		"E_CLAV_EN_PROGR_AVEC_BOITE",
		"E_CLAV_ETAT_UNDEF",
		"E_CLAV_ULTRA_MODE",
		"E_CLAV_ETAT_END"
};

PUBLIC char const *dbg_etCLAV_role[]={
		  "E_CLAV_USAGE_NON_DEFINI",
		  "E_CLAV_USAGE_NORMAL",
		  "E_CLAV_USAGE_CONFIG",
		  "E_CLAV_USAGE_END"
};

PUBLIC char const *dbg_etCLAV_mod[]={
		"E_CLAV_MODE_NOT_SET",
		"E_CLAV_MODE_DEFAULT",
		"E_CLAV_MODE_2",
		"E_CLAV_MODE_3",
		"E_CLAV_MODE_4",
		"E_CLAV_MODE_END"

};

PUBLIC char const * dbg_etCLAV_keys[]={
		"E_NO_KEYS",
		"E_KEY_NUM_1",
		"E_KEY_NUM_2",
		"E_KEY_NUM_3",
		"E_KEY_NUM_4",
		"E_KEY_NUM_5",
		"E_KEY_NUM_6",
		"E_KEY_NUM_7",
		"E_KEY_NUM_8",
		"E_KEY_NUM_9",
		"E_KEY_NUM_0",
		"E_KEY_MOD_1",
		"E_KEY_MOD_2",
		"E_KEY_MOD_3",
		"E_KEY_MOD_4",
		"E_KEY_ETOILE",
		"E_KEY_DIESE",
		"E_KEYS_END"
};
#endif
