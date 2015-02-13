/*
 * clav_usage.c
 *
 *  Created on: 17 juil. 2013
 *      Author: RapidOs
 */

#include <jendefs.h>
#include <Utilities.h> /// pour memset

#if !NO_DEBUG_ON
#include <Printf.h>
#endif

#include "e_config.h"

PUBLIC uint8 	bufEmission[3] ={0,0,0};


PRIVATE teClavState  clav_BtnPgmL1(teClavState mef_clav, uint8 *care);
PRIVATE teClavState  clav_BtnPgmL2(teClavState mef_clav, uint8 *care);

PUBLIC teClavState CLAV_GererTouche(etCLAV_keys keys)
{
	static bool_t b_use_bip = FALSE;
	teClavState mef_clav = AppData.eClavState;

	vPrintf("IN:CLAV_GererTouche MEF:%s\n", dbg_teClavState[mef_clav]);

	// Faire un Bip pour signal touche est detectee
	vPrintf(" Emettre un BIP ?\n");

	if(b_use_bip == TRUE){
		vPrintf("  Use bip\n");
		b_activer_bip=TRUE;
		vAHI_DioSetOutput(C_CLAV_BUZER,0);
	}
	else
	{
		vPrintf("  Use NO bip\n");
		b_activer_bip=FALSE;
		vAHI_DioSetOutput(0,C_CLAV_BUZER);
	}

	if(mef_clav == E_CLAV_ULTRA_MODE)
	{
		mef_clav = CLAV_BoutonDeConfiguration(&b_use_bip);
	}
	else
	{
		mef_clav = E_CLAV_ETAT_EN_ATTENTE;
		switch(keys)
		{
			case E_KEY_MOD_1:
			case E_KEY_MOD_2:
			case E_KEY_MOD_3:
			case E_KEY_MOD_4:
				CLAV_GererMode(keys);
				break;

			case E_KEY_DIESE:
				mef_clav = CLAV_BoutonDeConfiguration(&b_use_bip);
				break;

			default:
			{
				if(AppData.usage == E_CLAV_USAGE_CONFIG)
				{
					mef_clav = CLAV_PgmActionTouche(keys);
				}
				else
					if(AppData.usage == E_CLAV_USAGE_NORMAL){
						mef_clav = CLAV_UsrActionTouche(keys);
					}
					else
					{
						vPrintf("ERR:CLAV_GererTouche cas non prevu !\n");
					}

			}
			break;
		}
	}

	vPrintf("OUT:CLAV_GererTouche MEF:%s\n", dbg_teClavState[mef_clav]);
	return mef_clav;
}


PUBLIC teClavState CLAV_BoutonDeConfiguration(bool_t * bip_on)
{
	static bool_t bUltraCareMode = FALSE;
	teClavState mef_clav = E_CLAV_ETAT_UNDEF;

	vPrintf(" IN:CLAV_BoutonDeConfiguration MEF:%s\n", dbg_teClavState[mef_clav]);

	if(timer_touche[AppData.ukey] <= C_PRESSION_T1)
	{
		if( (*bip_on) == FALSE){
			vPrintf("  Activation bip clavier\n");
		}
		else
		{
			vPrintf("  Supression bip clavier\n");
		}
		(*bip_on) = !(*bip_on);
		mef_clav = E_CLAV_ETAT_EN_ATTENTE;
	}
	else
		if(bUltraCareMode == FALSE)
		{
			mef_clav = clav_BtnPgmL1(mef_clav,&bUltraCareMode);
		}
		else
		{
			mef_clav = clav_BtnPgmL2(mef_clav,&bUltraCareMode);
		}


	vPrintf(" OUT:CLAV_BoutonDeConfiguration MEF:%s\n", dbg_teClavState[mef_clav]);
	return mef_clav;
}


PUBLIC void CLAV_NetMsgInput(tsData *psData)
{
	teClavState mef_clav = AppData.eClavState;

	vPrintf("IN:CLAV_NetMsgInput MEF:%s\n",dbg_teClavState[mef_clav]);

	if(AppData.usage == E_CLAV_USAGE_CONFIG)
	{
		mef_clav = CLAV_PgmNetMsgInput(psData);
	}
	else
		if(AppData.usage == E_CLAV_USAGE_NORMAL){
			mef_clav = CLAV_UsrNetMsgInput(psData);
		}
		else
		{
			vPrintf("ERR:CLAV_GererMsgDeBoite cas non prevu !\n");
		}

	vPrintf("OUT:CLAV_NetMsgInput MEF:%s\n",dbg_teClavState[mef_clav]);
	vPrintf("NET:CLAV_NetMsgInput MEF:%d\n",AppData.eAppState);

	AppData.eClavState = mef_clav;
}

PRIVATE teClavState  clav_BtnPgmL1(teClavState mef_clav, uint8 *care)
{
	static bool_t usr_or_tec = FALSE;

	if(timer_touche[AppData.ukey] <= C_PRESSION_T3)
	{
		// On se met en mode defaut que ce soit en usr ou en pgm
		CLAV_GererMode(E_KEY_MOD_1);
		if(usr_or_tec == FALSE){
			vPrintf(" Passage en usage : configuration systeme\n");
			AppData.usage = E_CLAV_USAGE_CONFIG;

			//au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_RECHERCHE_RESEAU;
			mef_clav = CLAV_PgmNetMontrerClavier();
		}
		else
		{
			vPrintf(" Passage en usage : utilisation courante\n");
			AppData.usage = E_CLAV_USAGE_NORMAL;

			au8Led_clav[C_CLAV_LED_INFO_1].mode =E_FLASH_RESEAU_ACTIF;
			AppData.eAppState = APP_BOUCLE_PRINCIPALE;
			mef_clav = CLAV_PgmNetRetirerClavier();
		}
		usr_or_tec = !usr_or_tec;
	}
	else
	{
		vPrintf("Ultra mode On\n");
		mef_clav = E_CLAV_ULTRA_MODE;
		*care = TRUE;
		au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_RESET_POSSIBLE;
	}

	return(mef_clav);
}

PRIVATE teClavState  clav_BtnPgmL2(teClavState mef_clav, uint8 *care)
{
	static bool_t bReqEraseFlash = FALSE;

	if(timer_touche[AppData.ukey] <= C_PRESSION_T3)
	{
		if(bReqEraseFlash == FALSE)
		{
			bReqEraseFlash = TRUE;
			if(eeprom.nbBoite == 0xFF || eeprom.nbBoite == 0x00){
				vPrintf(" La Flash n'a aucune config a effacer\n");
			}
			else
			{
				vPrintf(" Effacement de la config en memoire Flash\n");
				if(!bAHI_FlashEraseSector(7)){
					vPrintf("Pb lors effacement Secteur\n");
				}
				// reset de la structure des donnees eprom
				memset(&eeprom,0x00,sizeof(eeprom));
			}
			mef_clav = E_CLAV_ULTRA_MODE;
			au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_RESET_POSSIBLE;

		}
		else
		{
			vPrintf("Ultra mode Off\n");
			*care = FALSE;
			au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_RESEAU_ACTIF;
			au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_OFF;
			mef_clav = E_CLAV_ETAT_EN_INITIALISATION;
		}
	}
	else
	{
		vPrintf("\n\nReset du module demande\n");
		// Vidage buffer UART
		while ((u8JPI_UartReadLineStatus(E_JPI_UART_0) & (E_JPI_UART_LS_THRE | E_JPI_UART_LS_TEMT)) !=
				(E_JPI_UART_LS_THRE | E_JPI_UART_LS_TEMT));
		// effectuer un reset du jennic
		vAHI_SwReset ();
	}

	return(mef_clav);
}
