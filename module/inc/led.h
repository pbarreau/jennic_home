/*
 * led.h
 *
 *  Created on: 3 juil. 2013
 *      Author: RapidOs
 */

#ifndef LED_H_
#define LED_H_
#include <jendefs.h>

// Gestion des Leds
// Rpl: void vAHI_DioSetOutput(uint32 u32On, uint32 u32Off);
#define vLedSet(ON,LED) \
		vAHI_DioSetOutput( \
				(ON) ? 0 			: (1<<LED), \
						(ON) ? (1<<LED) 	: 0\
		)

typedef enum
{
	E_FLASH_BP_TEST_SORTIES = 0x00,
	E_FLASH_OFF=0x00,
	E_FLASH_RECHERCHE_RESEAU = 0x01,
	E_FLASH_RECHERCHE_BC = 0x01,
	E_FLASH_ERREUR_DECTECTEE = 0x03,
	E_FLASH_RESET_POSSIBLE = 0x05,
	E_FLASH_EN_ATTENTE_TOUCHE_BC = 0x07,
	E_FLASH_LIAISON_BP_BC_ON = 0x07,
	E_FLASH_ERASE_RESET_POSSIBLE=0x09,
	E_FLASH_RESEAU_ACTIF = 0x10,
	E_FLASH_BP_EN_CONFIGURATION_SORTIES=0x20,
	E_FLASH_ALWAYS=0xFF,
	E_FLASH_FIN = 0xFF
} eLedInfo;

typedef struct
{
	bool_t		actif;	/// Flag indiquant s'il faut utiliser la led
	eLedInfo	mode;	/// Type de Flash, Fixe, Eteinte, ..
	uint8		pio;	/// Pio ou est branché la led
}sLed;


extern PUBLIC void IHM_ClignoteLed(void);

#endif /* LED_H_ */
