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

#define PBAR_CFG_INPUT	(E_JPI_DIO11_INT|\
		E_JPI_DIO12_INT |\
		E_JPI_DIO13_INT |\
		E_JPI_DIO17_INT |\
		E_JPI_DIO18_INT |\
		E_JPI_DIO19_INT |\
		E_JPI_DIO20_INT)

#define C_PIO_LED_1	E_JPI_DIO9_INT
#define	C_LED_PGM	E_JPI_DIO10_INT

#define	PBAR_CFG_OUTPUT	(C_PIO_LED_1|C_LED_PGM)

#endif /* C_CONFIG_H_ */
