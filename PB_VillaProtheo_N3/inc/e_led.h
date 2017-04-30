/*
 * e_led.h
 *
 *  Created on: 11 juil. 2013
 *      Author: RapidOs
 */

#ifndef E_LED_H_
#define E_LED_H_
#include "led.h"

#define	C_CLAV_NB_IHM_LED	3
#define	CST_LED_PERIOD		8

#define	C_CLAV_LED_INFO_1	0
#define	C_CLAV_LED_INFO_2	1
#define	C_CLAV_LED_INFO_3	2

#define	C_CLAV_MSK_INFO_1	E_AHI_DIO19_INT
#define	C_CLAV_MSK_INFO_2	E_AHI_DIO18_INT
#define	C_CLAV_MSK_INFO_3	E_AHI_DIO17_INT

#define	C_CLAV_PIO_INFO_1	19
#define	C_CLAV_PIO_INFO_2	18
#define	C_CLAV_PIO_INFO_3	17

#define PBAR_CFG_LED		(\
		C_CLAV_MSK_INFO_1 |\
		C_CLAV_MSK_INFO_2 |\
		C_CLAV_MSK_INFO_3)

extern PUBLIC sLed au8Led_clav[C_CLAV_NB_IHM_LED];

#endif /* E_LED_H_ */
