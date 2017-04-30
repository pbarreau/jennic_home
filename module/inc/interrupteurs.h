/*
 * interrupteurs.h
 *
 *  Created on: 3 juil. 2013
 *      Author: RapidOs
 */

#ifndef INTERRUPTEURS_H_
#define INTERRUPTEURS_H_

typedef enum {
  E_MEF2_UNDEF,	/// Machine de Turing non initialisee
  E_MEF2_START, 	/// Passage en mode programmation
  E_MEF2_STOP, 	/// Sortie du mode Programmation

  E_MEF2_WAITING_FOR_CONF_SERVICE,
  E_MEF2_ATTENDRE_BOITES,
  E_MEF2_ATTENDRE_SEL_MODE_AND_KBD,
  E_MEF2_SEND_MODE_AND_KBD,
  E_MEF2_ATTENDRE_RETOUR_CFG_BOITE,
  E_MEF2_EXECUTER_TOUCHE_KBD,
  E_MEF2_REMOVE_SVC,
  E_MEF2_ATTENDRE_FIN_REMOVE_SVC,
  E_MEF2_SVC_IS_REMOVE,

  E_MEF2_FIN /// Flag de fin des divers etats

} etMEF2; /// Machine a etat fini Commande

typedef struct {
  etMEF2 etape;
} stMEF2;

#endif /* INTERRUPTEURS_H_ */
