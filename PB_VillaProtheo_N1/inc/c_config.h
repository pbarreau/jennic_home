// -----------------------------------
// Projet	: PBPJ1
//	Fic		: c_config.h
//  Cree	: 30 juil. 2012
//	Par		: Administrateur
// -----------------------------------



// Noeud Coordonateur

#ifndef _PBPJ1_CO_C_CONFIG_H_
#define _PBPJ1_CO_C_CONFIG_H_

#if defined __cplusplus
extern "C" {
#endif


/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

// 10 outputs : 8 Relais lumiere + Led et LE 573
// 0000 0000 0001 1111 1111 1000 0000 0000 (OUT)
#define	PBAR_CFG_CMD_RL			(E_JPI_DIO11_INT|\
		E_JPI_DIO12_INT |\
		E_JPI_DIO13_INT |\
		E_JPI_DIO14_INT |\
		E_JPI_DIO15_INT |\
		E_JPI_DIO16_INT |\
		E_JPI_DIO17_INT |\
		E_JPI_DIO18_INT |\
		E_JPI_DIO19_INT |\
		E_JPI_DIO20_INT)

// 0000 0000 0000 0000 0000 0011 0000 0011 (IN)
#define PBAR_CFG_INPUT			0x00000303

#define CARD_NB_LIGHT  8
#define C_LED_NETWORK				E_JPI_DIO19_INT
#define C_SEL_573						E_JPI_DIO20_INT
#define C_PIO_LED_1  19
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif // _PBPJ1_CO_C_CONFIG_H_
/****************************************************************************/
/***        End of File                                                   ***/
/****************************************************************************/
