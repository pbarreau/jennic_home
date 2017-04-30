/*
 * clav2pc.h
 *
 *  Created on: 19 août 2013
 *      Author: RapidOs
 */

#ifndef CLAV2PC_H_
#define CLAV2PC_H_

typedef enum _StatusClavier {
  E_AUTONOMOUS, 		  /// Clavier autonome
  E_CONNECTED_TO_PC, 	/// Clavier sous controle du pc
  E_STATUS_CLAV_END		/// Flag de fin de la liste des enums
} eStatusClavier;

typedef enum _MsgPC {
  E_CHERCHE_SVC_LM, 	/// Recherche des boites ayant le service lumiere actif
  E_CHERCHE_SVC_VR,   /// Recherche des boites ayant le service volet roulant actif
  E_PGM_CLAVIER,		  /// Programme ce clavier
  E_TST_COMMANDE,		  /// Test une commande
  E_WR_COMMANDE,		  /// Ecrit une commande dans le clavier
  E_RD_COMMANDE,		  /// Lit une Commande incrite dans le clavier
  E_ER_COMMANDE,		  /// Efface une commande du clavier
  E_MSG_PC_END		    /// Flag de fin de la liste des enums
} eMsgPC;

typedef enum _pc_state {
  E_PC_STATE_END
} tePcState;
extern PUBLIC char const *dbg_tePcState[];

// Attention garder l'ordre pour transfert sur PC
typedef struct _sIhmPc {
  eStatusClavier etat;
  eMsgPC msg;
  unsigned long long adr;
  unsigned char val;
} s_IhmPc;

//|Q_WS_X11|Q_WS_QWS|Q_WS_MAC)
#if defined(Q_WS_WIN)
Q_DECLARE_METATYPE(s_IhmPc);
#endif

#endif /* CLAV2PC_H_ */
