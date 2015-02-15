/*
 * led.h
 *
 *  Created on: 3 juil. 2013
 *      Author: RapidOs
 */

#ifndef LED_H_
#define LED_H_

#define	CST_NB_IHM_LED	2

#define	CST_LED_PERIOD	8

// Gestion des Leds
// Rpl: void vAHI_DioSetOutput(uint32 u32On, uint32 u32Off);
#define vLedSet(ON,LED) \
		vAHI_DioSetOutput( \
				(ON) ? 0 			: (1<<LED), \
						(ON) ? (1<<LED) 	: 0\
		)

typedef enum
{
	E_LED_OFF 		= 0x00,	/// Etteindre la Led
	E_LED_MSG_1 	= 0x01,
	E_LED_MSG_2 	= 0x02,
	E_LED_MSG_3 	= 0x04,
	E_LED_MSG_4 	= 0x06,
	E_LED_MSG_5 	= 0x08,
	E_LED_MSG_6 	= 0x0C,
	E_LED_MSG_7 	= 0x10,
	E_LED_MSG_8 	= 0x14,
	E_LED_ON 		= 0xFF	/// Allumer la led
} eLedInfo;

typedef struct
{
	bool_t		actif;	/// Flag indiquant s'il faut utiliser la led
	eLedInfo	mode;	/// Type de Flash, Fixe, Eteinte, ..
	uint8		pio;	/// Pio ou est branché la led
}sLed;


extern PUBLIC sLed au8Led[CST_NB_IHM_LED];
extern PUBLIC  void IHM_ClignoteLed(void);

#endif /* LED_H_ */
