#include <jendefs.h>

#if !NO_DEBUG_ON
#include <Printf.h>
#endif

#include "e_config.h"
#include "services.h"

PRIVATE teClavState pgm_GererBoiteEntrante(tsData *psData);
PRIVATE void pgm_CreerConfigAll(uint8 box_id);


// ------------------

PUBLIC teClavState CLAV_PgmNetMontrerClavier(void)
{
	teJenieStatusCode eStatus = E_JENIE_ERR_UNKNOWN;
	teClavState mef_clav = AppData.eClavState;

	vPrintf("  IN:CLAV_PgmNetMontrerClavier MEF:%s\n", dbg_teClavState[mef_clav]);
	// Enregistrer le service clavier pour etre vu des boitiers puissances
	vPrintf("   Transmettre passage en mode conf service:%x\n",SRV_INTER);

	eStatus = eJenie_RegisterServices((uint32)SRV_INTER);

	switch(eStatus)
	{
		case E_JENIE_SUCCESS:
			vPrintf("    Ce module est routeur : OK\n");
			mef_clav = E_CLAV_ATTENDRE_BOITE;
			break;

		case E_JENIE_DEFERRED:
			vPrintf("    Ce module est end device transfert au pere\n");
			mef_clav = E_CLAV_SERVICE_ON;
			break;

		default:
			vPrintf("    !!Activation service clavier a revoir\n");
			mef_clav = E_CLAV_ETAT_UNDEF;
			AppData.eAppState = APP_BOUCLE_PRINCIPALE;
			break;
	}

	vPrintf("  OUT:CLAV_PgmNetMontrerClavier MEF:%s\n", dbg_teClavState[mef_clav]);

	return mef_clav;
}

PUBLIC teClavState CLAV_PgmNetRetirerClavier(void)
{
	teJenieStatusCode eStatus = E_JENIE_ERR_UNKNOWN;
	teClavState mef_clav = AppData.eClavState;

	vPrintf("  IN:CLAV_PgmNetRetirerClavier MEF:%s\n", dbg_teClavState[mef_clav]);
	// Enregistrer le service clavier pour etre vu des boitiers puissances
	vPrintf("   Effacer mode conf service\n");

	eStatus = eJenie_RegisterServices((uint32)0);

	switch(eStatus)
	{
		case E_JENIE_SUCCESS:
			vPrintf("    Ce module est routeur : OK\n");
			mef_clav = E_CLAV_ETAT_EN_ATTENTE;
			break;

		case E_JENIE_DEFERRED:
			vPrintf("    Ce module est end device transfert au pere\n");
			mef_clav = E_CLAV_SERVICE_OFF;
			break;

		default:
			vPrintf("    !!Activation service clavier a revoir\n");
			mef_clav = E_CLAV_ETAT_UNDEF;
			AppData.eAppState = APP_BOUCLE_PRINCIPALE;
			break;
	}

	vPrintf("  OUT:CLAV_PgmNetRetirerClavier MEF:%s\n", dbg_teClavState[mef_clav]);

	return mef_clav;
}

PUBLIC teClavState CLAV_PgmNetMsgInput(tsData *psData)
{
	teClavState mef_clav = E_CLAV_ETAT_UNDEF;
	uint8 mode = 0;
	uint8 clav = 0;
	uint8 conf= 0;

	vPrintf("\n IN:CLAV_PgmNetMsgInput MEF:%s\n", dbg_teClavState[mef_clav]);

	if((psData->u16Length) == 1)
	{
		if(AppData.eClavState == E_CLAV_ATTENDRE_BOITE)
		{
			vPrintf("  une boite se fait connaitre\n");
			mef_clav = pgm_GererBoiteEntrante(psData);
			vPrintf("  une boite a ete geree\n");
		}
		else
		{
			vPrintf("  Message recut hors contexte prevu!\n");
		}
	}
	else
		if(psData->u16Length == 3)
		{
			switch(psData->pau8Data[psData->u16Length-3])
			{
				case E_MSG_CFG_BOX_END:
				{
					vPrintf("  Msg Fin config boite %d\n\n",AppData.u8BoxId);
					pgm_CreerConfigAll(AppData.u8BoxId);
					mef_clav = E_CLAV_ATTENDRE_BOITE;
					AppData.eAppState = APP_BOUCLE_PRINCIPALE;
				}
				break;

				case E_MSG_CFG_LIENS:
				{
					mode = psData->pau8Data[psData->u16Length-2]>>4 &0x0F;
					clav = psData->pau8Data[psData->u16Length-2] & 0x0F;
					conf = psData->pau8Data[psData->u16Length-1];

					vPrintf("  Reception config liens\n");
					vPrintf("   Box:%d, Mode:%d, clav:%d, conf:%x\n\n",
							AppData.u8BoxId,mode,clav,conf);

					eeprom.netConf.boxData[mode][clav][AppData.u8BoxId] = conf;
					mef_clav = E_CLAV_ATTENDRE_FIN_CONFIG_BOITE;
				}
				break;


				default:
				{
					vPrintf("  FIN CFG Msg de la boite non compris\n");
				}
				break;

			}
		}
		else
		{
			vPrintf("  ERR:La taille n'est pas celle prevue\n");
			vPrintf("  Reception d'un message non normalise !!\n");

			AppData.eAppState = APP_BOUCLE_PRINCIPALE;
		}

	vPrintf(" OUT:CLAV_PgmNetMsgInput MEF:%s\n", dbg_teClavState[mef_clav]);
	return mef_clav;
}

PUBLIC teClavState CLAV_PgmActionTouche(etCLAV_keys keys)
{
	teClavState mef_clav = AppData.eClavState;
	stToucheDef touche = {0};
	etCLAV_mod key_mode = AppData.eClavmod-E_CLAV_MODE_DEFAULT;
	etCLAV_keys key_code = keys-E_KEY_NUM_1;
	uint8 box_id = AppData.u8BoxId;
	uint8 position = 0;

	vPrintf("\n IN:CLAV_PgmActionTouche MEF:%s\n", dbg_teClavState[mef_clav]);

	if(keys >= E_KEY_NUM_1 && keys <= E_KEY_NUM_0)
	{
		// On envoie la touche et le mode a la carte puissance
		vPrintf("\n  Programmation touche clavier:%s, mode:%s\n",dbg_etCLAV_keys[keys],dbg_etCLAV_mod[key_mode+E_CLAV_MODE_DEFAULT]);

		// verifier si cette touche connait la boite a configurer
		// si non rajouter cette boite a la touche
#if fn1
		CLAV_TrouverAssociationToucheBoite(AppData.eClavmod,keys,AppData.u8BoxId);
#else
		touche.la_touche = keys;
		touche.le_mode = AppData.eClavmod;
		if (CLAV_TrouverAssociationToucheBoite(&touche,box_id, &position) == FALSE)
		{
			eeprom.netConf.boxList[key_mode][key_code][position]=box_id;
			eeprom.netConf.ptr_boxList[key_mode][key_code]++;
			vPrintf("   La touche a la connaisance de la boite %d!\n",box_id);
		}
#endif
		bufEmission[0]= E_MSG_CFG_LIENS;
		bufEmission[1]= key_mode<<4| key_code;
		bufEmission[2]= eeprom.netConf.boxData[key_mode][key_code][box_id];

		vPrintf("   Msg %x,%x,%x vers [%x;%x]\n",bufEmission[0],
				bufEmission[1],
				bufEmission[2],
				(uint32)(eeprom.BoxAddr[box_id] >> 32),
				(uint32)(eeprom.BoxAddr[box_id] &  0xFFFFFFFF)
		);
		eJenie_SendData(eeprom.BoxAddr[box_id],
				bufEmission, 3,
				TXOPTION_SILENT);
		mef_clav = E_CLAV_ATTENDRE_FIN_CONFIG_BOITE;
	}

	vPrintf(" OUT:CLAV_PgmActionTouche MEF:%s\n", dbg_teClavState[mef_clav]);

	return mef_clav;
}

PRIVATE teClavState pgm_GererBoiteEntrante(tsData *psData)
{
	teClavState mef_clav = E_CLAV_ETAT_UNDEF;
	uint8 box_number = psData->pau8Data[psData->u16Length-1];

	vPrintf("   Boite id [%d] \n",box_number);

	if(box_number < C_MAX_BOXES){
		// verifier que l'on ne connait pas deja cette @
		//if(eeprom.BoxAddr[IncomingBoxId]==0xffffffffffffffffULL){
		if(eeprom.BoxAddr[box_number]==0ULL){
			vPrintf("   Cette boite [%d] est nouvelle\n",box_number);
			vPrintf("  Je la memorise\n");

			eeprom.BoxAddr[box_number]=psData->u64SrcAddress;
			// Une boite de plus dans le clavier
			eeprom.nbBoite++;
		}
		else
		{
			vPrintf("  Je connais a priori cette boite (id:%d)\n",box_number);
			if(eeprom.BoxAddr[box_number]!=psData->u64SrcAddress)
			{
				vPrintf("   La config a changee\n");
				eeprom.BoxAddr[box_number]=psData->u64SrcAddress;
			}
			else
			{
				vPrintf("   C'est bien celle que je connais\n");
			}
		}
		vPrintf("  Selectionner un mode et une touche\n\n");

		au8Led_clav[C_CLAV_LED_INFO_1].mode=E_FLASH_BP_EN_CONFIGURATION_SORTIES;

		mef_clav = E_CLAV_EN_PROGR_AVEC_BOITE;
		AppData.u8BoxId = box_number;

		AppData.eAppState = APP_BOUCLE_PRINCIPALE;
	}
	else
	{
		vPrintf("   ERROR !! Box id %d superieur a %d\n", box_number, C_MAX_BOXES);

		// Retour au mode normal
		vPrintf("   Retour en usage : utilisation courante\n");
		mef_clav = CLAV_GererTouche(AppData.eKeyPressed);

		// TODO : Envoyer msg a la box id (en erreur) pour qu'elle ne reste pas en attente de touche clavier
	}
	return mef_clav;
}

PRIVATE void pgm_CreerConfigAll(uint8 box_id)
{
	//teClavState mef_clav 	= E_CLAV_ETAT_UNDEF;
	etCLAV_keys eLaTouche 	= AppData.eKeyPressed;
	etCLAV_mod 	eLeMode 	= AppData.eClavmod;
	uint8		valThisBox 	= 0;
	uint8 		i 			= 0;
	uint8 		useBox 		= 0;
	uint8 		tmp 		= 0;

	stToucheDef touche 		= {0};
	etCLAV_mod 	key_mode 	= AppData.eClavmod-E_CLAV_MODE_DEFAULT;
	etCLAV_keys key_code 	= eLaTouche - E_KEY_NUM_1;
	uint8 		position 	= 0;

	// rajout de cette boite a la liste de celle du clavier
	vPrintf("Une B:%d de + a  key:%s ?\n",box_id,dbg_etCLAV_keys[eLaTouche]);
	touche.la_touche = eLaTouche;
	touche.le_mode = eLeMode;
	if (CLAV_TrouverAssociationToucheBoite(&touche,box_id, &position) == TRUE)
	{
		vPrintf("km=%d,kc=%d,po=%d,v=%d\n",key_mode,key_code,position,box_id);
		eeprom.netConf.boxList[key_mode][key_code][position]=box_id;
		eeprom.netConf.ptr_boxList[key_mode][key_code]++;
		vPrintf("   Sauvegarde Terminee!\n");
	}

	// on va parcourir toute les boites associees a cette touche
	// et creer une commande ALL
	vPrintf("\nCreation cmd 'ALL' pour touche %s\n", dbg_etCLAV_keys[E_KEY_ETOILE]);
	// Parcourir toute les touches de la boite en config
	valThisBox = 0;
	for(i=0;i<C_MAX_KEYS;i++)
	{
		// recuperer la config du clavier et la stocker
		// dans la touche ALL
		//eeprom.netConf.boxData[mode][clav][AppData.u8BoxId] = conf;
		tmp = eeprom.netConf.boxData[key_mode][i][box_id];
		valThisBox |= eeprom.netConf.boxData[key_mode][i][box_id];
		useBox = eeprom.netConf.boxList[key_mode][key_code][box_id];
		vPrintf(" box_position:%d key:%s, val:%x, all:%x\n",useBox,dbg_etCLAV_keys[i+1],tmp,valThisBox);
	}

	// sauvegarder cette valeur dans la touche ALL de la boite en cours
	// On sauve la touche all
	eeprom.netConf.boxData[key_mode][C_KEY_MEM_ALL][box_id] = valThisBox;

	// Dire que Touche ALL position ptr++ a une Boite configuree
	// La touche ALL est la derniere de toute les touche autorisee
	touche.la_touche = E_KEY_ETOILE;
	touche.le_mode = eLeMode;
	if (CLAV_TrouverAssociationToucheBoite(&touche,box_id, &position) == FALSE)
	{
		vPrintf("   Sauvegarde touche 'ALL' Terminee!\n");
		eeprom.netConf.boxList[key_mode][C_KEY_MEM_ALL][position]=box_id;
		eeprom.netConf.ptr_boxList[key_mode][C_KEY_MEM_ALL]++;
	}

	// sauvegarder la config dans la flash !!!
	if(!bAHI_FlashEraseSector(7)){
		vPrintf("Pb lors effacement Sector 3\n");
	}
	else{
		// Programmation

		if(!bAHI_FullFlashProgram(FLASH_START, sizeof(bpsFlash), (uint8 *) &eeprom)
		)
		{
			vPrintf("Pb Lors de la sauvegarde en Flash\n");
		}
		else
		{
			vPrintf("Sauvegarde sur Flash terminee\n");
		}
	}
}
