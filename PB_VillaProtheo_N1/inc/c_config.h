/*
 * c_config.h
 *
 *  Created on: 3 juil. 2013
 *      Author: RapidOs
 */

#ifndef C_CONFIG_H_
#define C_CONFIG_H_

#include "m_config.h"
#include "interrupteurs.h"

typedef enum
{
	E_MEF1_UNDEF,	/// Machine de Turing non initialisee
	E_MEF1_START, 	/// Passage en mode programmation
	E_MEF1_STOP, 	/// Sortie du mode Programmation


	E_MEF1_FIN /// Flag de fin des divers etats


}etMEF1; /// Machine a etat fini Commande

typedef struct
{
	etMEF1	etape;
}stMEF1;

#if 0
typedef struct
{
	stMEF_JenNet net;	/// Etape JenNet

	bool_t	use_pwr;	/// Application utilise module puissance
	stMEF1	pwr;		/// etat courant de MEF pour module puissance

	bool_t	use_clv;	/// Application utilise module interrupteur
	stMEF2	clv;		/// Etat courant de MEF interrupteur
} tsAppData;
#endif

#endif /* C_CONFIG_H_ */
