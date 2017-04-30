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

#define CARD_NB_LIGHT  8

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

// Public variable
extern PUBLIC tsAppData sAppData;
extern PUBLIC uint8 uThisBox_Id;
extern PUBLIC bool_t bStartPgmTimer;
extern PUBLIC PBAR_E_KeyMode ePgmMode;
extern PUBLIC uint16 TimePgmPressed;
extern PUBLIC bool_t cbStartTempoRechercheClavier;
extern PUBLIC bool_t cbUnClavierActif;
extern PUBLIC uint64 LaBasId;
extern PUBLIC uint8 bufEmission[3];
extern PUBLIC PBAR_KIT_8046 LabasKbd;
extern PUBLIC PBAR_E_KeyMode LabasMod;
extern PUBLIC uint8 ledId;
extern PUBLIC uint8 config;
extern PUBLIC ebpLedInfo mNetOkTypeFlash;

// Public fonctions
extern PUBLIC void PBAR_LireBtnPgm(void);
extern PUBLIC bool_t PBAR_DecodeBtnPgm(uint8 *box_cnf);

#endif /* C_CONFIG_H_ */
