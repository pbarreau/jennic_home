/*
 * dbg_msg.c
 *
 *  Created on: 18 juil. 2013
 *      Author: RapidOs
 */

#if !NO_DEBUG_ON
#include <jendefs.h>

PUBLIC char const *dbg_teNetState[]={
		"APP_INITIALISATION",
		"APP_RECHERCHE_RESEAU",
		"APP_RESEAU_ETABLI",
		"APP_CONNECTION_RESEAU",
		"APP_PERTE_RESEAU",
		"APP_ATTENTE_ENREGISTREMENT_AU_PERE",
		"APP_BOUCLE_PRINCIPALE"
};

PUBLIC char const *dbg_PBAR_TypeMsg[]={
		"E_MSG_DATA_ALL",
		"E_MSG_DATA_SELECT",
		"E_MSG_ASK_ID_BOX",
		"E_MSG_RSP_ID_BOX",
		"E_MSG_CFG_LIENS",
		"E_MSG_CFG_BOX_END"
};
#endif
