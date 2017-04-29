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

#define PBAR_CFG_INPUT	(E_JPI_DIO9_INT|\
		E_JPI_DIO10_INT)


// 10 outputs : 8 Relais lumiere + Led et LE 573
// 0000 0000 0001 1111 1111 1000 0000 0000 (OUT)
#define	PBAR_CFG_CMD_RL			(E_JPI_DIO0_INT|\
		E_JPI_DIO1_INT |\
		E_JPI_DIO2_INT |\
		E_JPI_DIO3_INT |\
		E_JPI_DIO13_INT |\
		E_JPI_DIO15_INT |\
		E_JPI_DIO19_INT |\
		E_JPI_DIO20_INT)

#define C_SEL_573	E_JPI_DIO8_INT

#define	C_LID_1		0
#define C_LPIO_1	E_JPI_DIO16_INT	// Reseau
#define C_LPID_1	16

#define CARD_USE_LED_PGM 0

#ifdef CARD_USE_LED_PGM
#define	C_LID_2		1
#define	C_LPIO_2	E_JPI_DIO17_INT	// Pgm (Non Assign)
#define	C_LPID_2	17
#define	PBAR_CFG_OUTPUT	(C_LPIO_1|C_LPIO_2|PBAR_CFG_CMD_RL|C_SEL_573)
#else
#define	PBAR_CFG_OUTPUT	(C_LPIO_1|PBAR_CFG_CMD_RL|C_SEL_573)
#endif




// Public variable
extern PUBLIC tsAppData sAppData;
extern PUBLIC uint8 uThisBox_Id;
extern PUBLIC bool_t bStartPgmTimer;
extern PUBLIC PBAR_E_KeyMode ePgmMode;
extern PUBLIC uint16 TimePgmPressed;
extern PUBLIC bool_t cbStartTempoRechercheClavier;
extern PUBLIC bool_t cbUnClavierActif;
extern PUBLIC uint64 LaBasId;
extern PUBLIC uint8 	bufEmission[3];
extern PUBLIC PBAR_KIT_8046		LabasKbd;
extern PUBLIC PBAR_E_KeyMode	LabasMod;
extern PUBLIC uint8 ledId;
extern PUBLIC uint8 config;
extern PUBLIC uint8 MesIos[8];
extern PUBLIC ebpLedInfo mNetOkTypeFlash;

// Public fonctions
extern PUBLIC void PBAR_LireBtnPgm(void);
extern PUBLIC bool_t PBAR_DecodeBtnPgm(uint8 *box_cnf);
extern PUBLIC void PBAR_ConstruitreTabIOs(void);

#endif /* C_CONFIG_H_ */
