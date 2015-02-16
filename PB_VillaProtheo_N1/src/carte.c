/*
 * carte.c
 *
 *  Created on: 15 f�vr. 2015
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

PUBLIC uint64 LaBasId	 = 0;
PUBLIC PBAR_KIT_8046		LabasKbd = E_KPD_NONE;
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
		bStartPgmTimer = TRUE; // Bouton appuy�
	}
	else
	{
		bStartPgmTimer = FALSE;

	}
}

PRIVATE void PBAR_LireBtnPgm_NormalUsage(void)
{
	uint8 boxConf=0;

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
	static uint8 rf = 0;
	static bool_t pass = FALSE;

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
				if(rf != 0xFF)
				{
					//vPrintf("rf=%x;io=%x\n",rf,io);
					if(io >=1 && io <CARD_NB_LIGHT)
					{
						if((IsBitSet(rf,(io-1))))
						{
							vPRT_DioSetOutput(0,(1 << (PBAR_DEBUT_IO + (io-1)))); //off
							rf = rf ^ (1<<(io-1));
						}
						vPRT_DioSetOutput((1 << (PBAR_DEBUT_IO + (io))),0); //on

					}
					else
					{
						if((IsBitSet(rf,(CARD_NB_LIGHT-1))))
						{
							vPRT_DioSetOutput(0,(1 << (PBAR_DEBUT_IO + (CARD_NB_LIGHT-1)))); //off
							rf = rf ^ (1<<(CARD_NB_LIGHT-1));
						}
						vPRT_DioSetOutput((1 << (PBAR_DEBUT_IO + (0))),0); //on

					}

					rf = rf ^ (1<<io);
					io++;
					io%=CARD_NB_LIGHT;
				}
			}
			else {
				rf=0xFF;
				if(!pass){
					vPRT_DioSetOutput(rf<<11,(~rf)<<11); // On
				}
				else
				{
					vPRT_DioSetOutput(~rf<<11,rf<<11);
					rf = 0;
				}
				pass = !pass;
			}
			TimePgmPressed = 0;
		}
	}
	return TRUE;
}

PRIVATE bool_t PBAR_DecodeBtnPgm_NormalUsage(uint8 *box_cnf)
{
	PRIVATE uint8 passage = 0;
	bool_t bReturnConfig = FALSE;
	uint8 saveLed = 0;

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
			if (TimePgmPressed<30){
				// Appui cour
				// on montre la sortie a configurer
				//vPrintf("Id:%d\tCnf:%x, led:%d\n",passage,config,ledId);
				if(ledId)
				{
					//eteindre la precedente si elle n'est pas a memoriser
					if(!(IsBitSet(config,(ledId-1))))
					{
						vPRT_DioSetOutput(0,(1 << (PBAR_DEBUT_IO + (ledId-1))));
					}
					else
					{
						// sinon la ralummer
						vPRT_DioSetOutput((1 << (PBAR_DEBUT_IO + (ledId-1))),0);
					}
				}
				else{
					if(!(IsBitSet(config,(ledId+(CARD_NB_LIGHT-1)))))
					{
						vPRT_DioSetOutput(0,(1 << (PBAR_DEBUT_IO + (ledId+(CARD_NB_LIGHT-1)))));
					}
					else
					{
						// sinon la ralumer
						vPRT_DioSetOutput((1 << (PBAR_DEBUT_IO + (ledId+(CARD_NB_LIGHT-1)))),0);
					}
				}
				// si La led n'est pas deja alummee on l'allume
				//sinon on l'eteint
				if((IsBitSet(config,ledId)))
				{
					vPRT_DioSetOutput(0,(1 << (PBAR_DEBUT_IO + (ledId))));

				}
				else
				{
					vPRT_DioSetOutput((1 << (PBAR_DEBUT_IO + (ledId))),0);
				}
				ledId++;
				ledId%=CARD_NB_LIGHT;
			}
			else if(TimePgmPressed<80){
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
					vPRT_DioSetOutput(config<<11,(~config)<<11);
					bReturnConfig=TRUE;
				}
			}
			// Mettre a 1 SIG_LE 573 pour BLOQUER bus
			//vPrintf("Time pressed : %d\n", TimePgmPressed);
			TimePgmPressed=0;
		}
	}

	if(bReturnConfig){
		*box_cnf=config;
	}

	if(sAppData.eAppState == APP_STATE_TST_STOP_LUMIERES){
		config = 0;
		// On quitte le mode test: eteidre les lumieres
		vPRT_DioSetOutput(config<<11,(~config)<<11);
	}

	return(bReturnConfig);
}

