/*
 * m_network_msg.h
 *
 *  Created on: 16 mai 2017
 *      Author: Pascal
 */

#ifndef MODULE_INC_M_NETWORK_MSG_H_
#define MODULE_INC_M_NETWORK_MSG_H_

typedef enum {
  E_MSG_NOT_SET,
  E_MSG_DATA_ALL,
  E_MSG_DATA_SELECT,
  E_MSG_ASK_ID_BOX,
  E_MSG_RSP_ID_BOX,
  E_MSG_ASK_CFG_LIENS,
  E_MSG_RSP_CFG_LIENS,
  E_MSG_CFG_BOX_END,
  E_MSG_NET_LED_OFF,
  E_MSG_NET_LED_ON,
  E_MSG_OUT_STROBE,       /// TODO: Mettre les sorties en mode chenillard
  E_MSG_ASK_RESET_CARD,   /// TODO: Demande de reset
  E_MSG_ASK_ERASE_FLASH,  /// TODO: Effacement de config TBD
  E_MSG_SAVE_CONF,        /// TODO: Sauvegarde par wifi de la config d'un clavier
  E_MSG_DEFINITION_END
} etDefWifiMsg;
extern PUBLIC char const *dbg_etDefWifiMsg[];

#endif /* MODULE_INC_M_NETWORK_MSG_H_ */
