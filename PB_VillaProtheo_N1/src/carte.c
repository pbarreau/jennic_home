/*
 * carte.c
 *
 *  Created on: 15 févr. 2015
 *      Author: Pascal
 */
#include <JPI.h>
#include <Printf.h>

#include "led.h"
#include "bit.h"

#include "c_config.h"
#include "i2c_9555.h"
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void PBAR_LireBtnPgm_TstOutput(void);
PRIVATE void PBAR_LireBtnPgm_NormalUsage(void);
PRIVATE bool_t PBAR_DecodeBtnPgm_TstOutput(uint8 *box_cnf);
PRIVATE bool_t PBAR_DecodeBtnPgm_NormalUsage(uint8 *box_cnf);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
PUBLIC tsAppData sAppData;

PUBLIC uint16 TimePgmPressed = 0;


PUBLIC uint8 uThisBox_Id = 0; /// Reference de la boite
PUBLIC bool_t bStartPgmTimer = FALSE;
PUBLIC PBAR_E_KeyMode ePgmMode = E_CLAV_MODE_NOT_SET;
PUBLIC bool_t cbStartTempoRechercheClavier = FALSE;
PUBLIC bool_t cbUnClavierActif = FALSE;

PUBLIC uint8 	bufEmission[3] ={0,0,0};

PUBLIC uint64           LaBasId	 = 0;
PUBLIC PBAR_KIT_8046	LabasKbd = E_KPD_NONE;
PUBLIC PBAR_E_KeyMode	LabasMod = E_CLAV_MODE_NOT_SET;

PUBLIC uint8 ledId = 0;
PUBLIC uint8 config = 0;


PUBLIC void PBAR_LireBtnPgm(void)
{
	if(uThisBox_Id)
	{
		PBAR_LireBtnPgm_NormalUsage();
	}
	else
	{
		PBAR_LireBtnPgm_TstOutput();
	}
}

PRIVATE void PBAR_LireBtnPgm_TstOutput(void)
{
	if ((u8JPI_PowerStatus() & 0x10) == 0 )
	{
		bStartPgmTimer = TRUE; // Bouton appuyé
	}
	else
	{
		bStartPgmTimer = FALSE;

	}
}

PRIVATE void PBAR_LireBtnPgm_NormalUsage(void)
{
	uint8 boxConf=0;
	static bool showNet = TRUE;

	// Bouton Pgm appuye ??
	if ((u8JPI_PowerStatus() & 0x10) == 0 && ePgmMode == E_CLAV_MODE_NOT_SET)
	{
		bStartPgmTimer = TRUE;
	}
	else
	{
		switch(ePgmMode)
		{
		case E_CLAV_MODE_NOT_SET:
		{
			bStartPgmTimer = FALSE;
			if(TimePgmPressed)
			{
				if (TimePgmPressed<30)
				{
					// On demarre une compteur de temps
					cbStartTempoRechercheClavier = TRUE;
					sAppData.eAppState = APP_STATE_RECHERCHE_CLAVIER;
				}
				else if (TimePgmPressed < 100)
				{
					// eteindre ou alummer led conection ok
					if(showNet)
					{
						vPrintf("Cacher Net Ok\n");
						mNetOkTypeFlash = E_FLASH_FIN;
						au8Led[0].mode = E_FLASH_FIN;
					}
					else
					{
						vPrintf("Montrer Net Ok\n");
						mNetOkTypeFlash = E_FLASH_RESEAU_ACTIF;
						au8Led[0].mode = E_FLASH_RESEAU_ACTIF;
					}
					showNet = !showNet;
					sAppData.eAppState = APP_STATE_RUNNING;
				}

				else {
					if(LaBasId != 0){
						cbUnClavierActif = FALSE;
						sAppData.eAppState = APP_STATE_FIN_CFG_BOX;
					}
					else
					{
						; // Rien on reste comme on est
					}
				}
				//vPrintf(" Time:%d\n",TimePgmPressed);
				TimePgmPressed = 0;
			}
		}
		break;

		case E_CLAV_MODE_1:
		{
			if(PBAR_DecodeBtnPgm(&boxConf))
			{
				if(sAppData.eAppState == APP_STATE_ATTENDRE_FIN_CFG_LOCAL)
				{
					vPrintf("> Envoyer config:%x\n",boxConf);
					bufEmission[0]=E_MSG_CFG_LIENS;
					bufEmission[1]=LabasMod<<4 |LabasKbd;
					bufEmission[2]=boxConf;

					// Emettre
					eJenie_SendData(LaBasId,
							bufEmission, 3,TXOPTION_ACKREQ);

					// Montrer emission
					au8Led[0].mode= E_FLASH_RECHERCHE_RESEAU;
				}
			}

		}
		break;

		default:
		{
			vPrintf("Mode inconnu\n");
		}
		break;
		}

	}
}

PUBLIC bool_t PBAR_DecodeBtnPgm(uint8 *box_cnf)
{
	bool_t bValue = FALSE;

	if(uThisBox_Id)
	{
		bValue=PBAR_DecodeBtnPgm_NormalUsage(box_cnf);
	}
	else
	{
		bValue=PBAR_DecodeBtnPgm_TstOutput(box_cnf);
	}
	return bValue;
}

PRIVATE bool_t PBAR_DecodeBtnPgm_TstOutput(uint8 *box_cnf)
{
	static uint8 io = 0;
	static bool_t pass = FALSE;
	static uint32 conf = 0;

	if ((u8JPI_PowerStatus() & 0x10) == 0)
	{
		bStartPgmTimer = TRUE;
	}
	else
	{
		bStartPgmTimer = FALSE;

		if(TimePgmPressed)
		{
			// Analyse duree appui
			if (TimePgmPressed<30)
			{
				vPrintf("\nTst(%d):conf=%x;\n",io,(uint32)conf);

				BitNset(conf,PBAR_DEBUT_IO+io);
				vPrintf("\nTst(%d):conf=%x;\n",io,(uint32)conf);

				if(io)
					BitNclr(conf,PBAR_DEBUT_IO+io-1);

				// Mettre la config
				vPRT_DioSetOutput(~conf,conf);

				if(io == CARD_NB_LIGHT -1)
					BitNclr(conf,PBAR_DEBUT_IO+io);

				// Passer au bit suivant
				io = (io + 1 )%CARD_NB_LIGHT;
			}
			else {
				if(!pass){
					vPrintf("\nMode ON\n");
					vPRT_DioSetOutput((conf & (0x0<<PBAR_DEBUT_IO)),(conf | (0xFF<<PBAR_DEBUT_IO)));
				}
				else
				{
					vPrintf("\nMode OFF\n");
					vPRT_DioSetOutput((conf | (0xFF<<PBAR_DEBUT_IO)),(conf & (0x0<<PBAR_DEBUT_IO)));
				}
				pass = !pass;

			}
		}
		TimePgmPressed = 0;
	}

	return TRUE;
}


PRIVATE bool_t PBAR_DecodeBtnPgm_NormalUsage(uint8 *box_cnf)
{
	PRIVATE uint8 passage = 0;
	bool_t bReturnConfig = FALSE;
	uint8 saveLed = 0;
	uint32 Maconf = 0;
	static int sel_led = 0;

	// Bouton Pgm appuye ??
	if ((u8JPI_PowerStatus() & 0x10) == 0)
	{
		bStartPgmTimer = TRUE;
	}
	else
	{
		bStartPgmTimer = FALSE;

		if(TimePgmPressed){
			// TBD: ne pas activer les lumieres
			// mais uniquement les leds

			// Analyse duree appui
			if(TimePgmPressed)
			{
				// Analyse duree appui
				if (TimePgmPressed<30)
				{
					if(config)
						sel_led = sel_led | config;

					// Allumer la led
					if(IsBitSet(config,ledId))
					{
						BitNclr(sel_led,ledId);
					}
					else
					{
						BitNset(sel_led,ledId);
					}


					if(ledId){
						if(IsBitSet(config,(ledId-1)))
						{
							BitNset(sel_led,ledId-1);
						}
						else
						{
							BitNclr(sel_led,ledId-1);
						}
					}


					// Mettre la config
					Maconf = sel_led<< PBAR_DEBUT_IO;
					vPRT_DioSetOutput(~Maconf,Maconf);

					if(ledId == CARD_NB_LIGHT -1){
						if(IsBitSet(config,ledId))
						{
							BitNset(sel_led,ledId);
						}
						else
						{
							BitNclr(sel_led,ledId);
						}
					}

					// Passer au bit suivant
					ledId = (ledId + 1 )%CARD_NB_LIGHT;
				}
				else if(TimePgmPressed<80)
				{
					// Dans quel etat config ou test
					if(sAppData.eAppState == APP_STATE_TST_START_LUMIERES){
						sAppData.eAppState = APP_STATE_TST_STOP_LUMIERES;
						bReturnConfig = FALSE;
					}
					else
					{
						// demande de memorisation de config de led
						saveLed = ledId;
						if (!saveLed){
							saveLed = CARD_NB_LIGHT -1;
						}
						else{
							saveLed--;
						}
						vPrintf("  Memorisation de la led id:%d\n",saveLed);
						config = config ^ (1<<saveLed);
						passage++;
					}
				}
				else{
					if(sAppData.eAppState == APP_STATE_TST_START_LUMIERES){
						sAppData.eAppState = APP_STATE_TST_STOP_LUMIERES;
						bReturnConfig = FALSE;
					}
					else
					{

						// Sauvegarde pour envoi a la boite
						// On montre la config a envoyer
						// Configuer les sorties
						vPRT_DioSetOutput(config<<PBAR_DEBUT_IO,(~config)<<PBAR_DEBUT_IO);
						bReturnConfig=TRUE;
					}
				}
			}
			TimePgmPressed=0;
		}
	}

	if(bReturnConfig){
		*box_cnf=config;
	}

	if(sAppData.eAppState == APP_STATE_TST_STOP_LUMIERES){
		config = 0;
		// On quitte le mode test: eteindre les lumieres
		vPRT_DioSetOutput(config<<PBAR_DEBUT_IO,(~config)<<PBAR_DEBUT_IO);
	}

	return(bReturnConfig);
}
