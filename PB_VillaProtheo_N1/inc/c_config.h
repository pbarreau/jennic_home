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


#define	C_LID_1		0
#define C_LPIO_1	E_JPI_DIO9_INT	// Reseau
#define C_LPID_1	9

#define	C_LID_2		1
#define	C_LPIO_2	E_JPI_DIO10_INT	// Pgm
#define	C_LPID_2	10

#define	PBAR_CFG_OUTPUT	(C_LPIO_1|C_LPIO_2)

#endif /* C_CONFIG_H_ */
