/*
 * interrupteur.c
 *
 *  Created on: 3 juil. 2013
 *      Author: RapidOs
 */

#include <Printf.h>

#include "m_config.h"
#include "c_config.h"
#include "led.h"
#include "interrupteurs.h"


PUBLIC void MEF1_Turing(stMEF2 action)
{
 switch(action.etape)
 {
	 case E_MEF1_START:
	 {
		 // Montrer que le clavier est attente de configuration
		 au8Led[CST_LED_INFO_2].mode = E_LED_MSG_1;

		 // Etape suivante emettre le message de service clavier en attente
	 }
	 break;

	 default:
	 {
		 vPrintf("Erreur dans les etapes de MEF1\n");
	 }
	 break;
 }
}
