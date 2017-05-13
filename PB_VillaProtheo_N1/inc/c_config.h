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

#define PBAR_JEN_BOX_ID  17
#define CARD_HAVE_HPM   FALSE  // HIGH POWER MODUL


#define V1_USE_TRIAC  1
#define V1_USE_RELAY  2
#define V2_USE_TRIAC  3
#define V2_USE_TRIAC  3
#define V0_USE_RELAY  4

#define PBAR_POWER_CARD V0_USE_RELAY

#if CARD_HAVE_HPM
#define CARD_NB_LIGHT  6
#define PBAR_CFG_CMD_RL     (E_JPI_DIO0_INT|\
    E_JPI_DIO1_INT |\
    E_JPI_DIO11_INT |\
    E_JPI_DIO18_INT |\
    E_JPI_DIO19_INT |\
    E_JPI_DIO20_INT)
#else
#define CARD_NB_LIGHT  8
#define PBAR_CFG_CMD_RL     (E_JPI_DIO0_INT|\
    E_JPI_DIO1_INT |\
    E_JPI_DIO2_INT |\
    E_JPI_DIO3_INT |\
    E_JPI_DIO11_INT |\
    E_JPI_DIO18_INT |\
    E_JPI_DIO19_INT |\
    E_JPI_DIO20_INT)
#endif

#define PBAR_CFG_INPUT	(E_JPI_DIO9_INT|\
		E_JPI_DIO10_INT)

// 10 outputs : 8 Relais lumiere + Led et LE 573
// 0000 0000 0001 1111 1111 1000 0000 0000 (OUT)

#define C_SEL_573	E_JPI_DIO8_INT

#define	C_LID_1		0
#define C_LPIO_1	E_JPI_DIO16_INT	// Reseau
#define C_LPID_1	16

#define CARD_USE_LED_PGM FALSE

#if CARD_USE_LED_PGM
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
extern PUBLIC uint8 bufEmission[3];
extern PUBLIC PBAR_KIT_8046 LabasKbd;
extern PUBLIC PBAR_E_KeyMode LabasMod;
extern PUBLIC uint8 ledId;
extern PUBLIC uint8 config;
extern PUBLIC uint8 MesIos[8];
extern PUBLIC ebpLedInfo mNetOkTypeFlash;

// Public fonctions
extern PUBLIC void PBAR_LireBtnPgm(void);
extern PUBLIC bool_t PBAR_DecodeBtnPgm(uint8 *box_cnf);
extern PUBLIC void PBAR_MemoriserPioIdDesRelais(void);
extern PUBLIC void PBAR_MontrerPioIdDesEntrees(void);

#endif /* C_CONFIG_H_ */
