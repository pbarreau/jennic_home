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
  teClavNetStates mef_net = AppData.eNetState;
//toto
au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_MENU_LIAISON;

#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_PgmNetMontrerClavier",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif
  // Enregistrer le service clavier pour etre vu des boitiers puissances
  vPrintf("%sTransmettre passage en mode conf service:%x\n", gch_spaces,
      SRV_INTER);

  eStatus = eJenie_RegisterServices((uint32) SRV_INTER);

  switch (eStatus)
  {
    case E_JENIE_SUCCESS:
      vPrintf("%s Ce module est routeur : OK\n", gch_spaces);
      mef_net = E_KS_NET_WAIT_CLIENT;
    break;

    case E_JENIE_DEFERRED:
      vPrintf("%sCe module est end device transfert au pere\n", gch_spaces);
      mef_net = E_KS_NET_CONF_START;
    break;

    default:
      vPrintf("%s!!Activation service clavier a revoir\n", gch_spaces);
      mef_net = E_KS_NET_NON_DEFINI;
      AppData.eAppState = APP_BOUCLE_PRINCIPALE;
    break;
  }

#if !NO_DEBUG_ON
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
  PBAR_DbgTrace(E_FN_OUT, "CLAV_PgmNetMontrerClavier",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
#endif

  AppData.eNetState = mef_net;
  return mef_clav;
}

PUBLIC teClavState CLAV_PgmNetRetirerClavier(void)
{
  teJenieStatusCode eStatus = E_JENIE_ERR_UNKNOWN;
  teClavState mef_clav = AppData.eClavState;

#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_PgmNetRetirerClavier",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif
  // Enregistrer le service clavier pour etre vu des boitiers puissances
  vPrintf("%sEffacer mode conf service\n", gch_spaces);

  eStatus = eJenie_RegisterServices((uint32) 0);

  switch (eStatus)
  {
    case E_JENIE_SUCCESS:
      vPrintf("%sCe module est routeur : OK\n", gch_spaces);
      mef_clav = E_KS_ATTENTE_TOUCHE;
    break;

    case E_JENIE_DEFERRED:
      vPrintf("%sCe module est end device transfert au pere\n", gch_spaces);
      mef_clav = E_KS_SERVICE_OFF;
    break;

    default:
      vPrintf("%s!!Activation service clavier a revoir\n", gch_spaces);
      mef_clav = E_KS_NON_DEFINI;
      AppData.eAppState = APP_BOUCLE_PRINCIPALE;
    break;
  }

#if !NO_DEBUG_ON
  PBAR_DbgTrace(E_FN_OUT, "CLAV_PgmNetRetirerClavier",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  return mef_clav;
}

PUBLIC teClavState CLAV_PgmNetMsgInput(tsData *psData)
{
  teClavState mef_clav = E_KS_NON_DEFINI;
  teClavNetStates mef_net = AppData.eNetState;
  uint8 mode = 0;
  uint8 clav = 0;
  uint8 conf = 0;

#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_PgmNetMsgInput",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  if ((psData->u16Length) == 1)
  {
    if (mef_net == E_KS_NET_WAIT_CLIENT)
    {
          vPrintf("%sune boite se fait connaitre\n", gch_spaces);
          mef_clav = pgm_GererBoiteEntrante(psData);
    }
    else
    {
      vPrintf("%sMessage recut hors contexte prevu!\n", gch_spaces);
    }
  }
  else if (psData->u16Length == 3)
  {
    switch (psData->pau8Data[psData->u16Length - 3])
    {
      case E_MSG_CFG_BOX_END:
      {
        vPrintf("%sMsg Fin config boite %d\n\n", gch_spaces, AppData.u8BoxId);
        pgm_CreerConfigAll(AppData.u8BoxId);
        mef_net = E_KS_NET_WAIT_CLIENT;
        AppData.eAppState = APP_BOUCLE_PRINCIPALE;
      }
      break;

      case E_MSG_CFG_LIENS:
      {
        mode = psData->pau8Data[psData->u16Length - 2] >> 4 & 0x0F;
        clav = psData->pau8Data[psData->u16Length - 2] & 0x0F;
        conf = psData->pau8Data[psData->u16Length - 1];

        vPrintf("%sReception config liens\n", gch_spaces);
        vPrintf("%s Box:%d, Mode:%d, clav:%d, conf:%x\n\n", gch_spaces,
            AppData.u8BoxId, mode, clav, conf);

        eeprom.netConf.boxData[mode][clav][AppData.u8BoxId] = conf;
        mef_net = E_KS_NET_CONF_EN_COURS;
      }
      break;

      case E_MSG_NET_LED_OFF:
      {
        vPrintf("Demande reseau de couper led Net\n");
        mNetOkTypeFlash = E_FLASH_RESEAU_ACTIF;
        au8Led_clav[C_CLAV_LED_INFO_1].mode =mNetOkTypeFlash;
      }
      break;

      case E_MSG_NET_LED_ON:
      {
        vPrintf("Demande reseau de montrer led Net\n");
        mNetOkTypeFlash = E_FLASH_RESEAU_ACTIF;
        au8Led_clav[C_CLAV_LED_INFO_1].mode =mNetOkTypeFlash;
      }
      break;

      default:
      {
        vPrintf("%sFIN CFG Msg de la boite non compris\n", gch_spaces);
      }
      break;

    }
  }
  else
  {
    vPrintf("%sERR:La taille n'est pas celle prevue\n", gch_spaces);
    vPrintf("%sReception d'un message non normalise !!\n", gch_spaces);

    AppData.eAppState = APP_BOUCLE_PRINCIPALE;
  }

#if !NO_DEBUG_ON
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
  PBAR_DbgTrace(E_FN_OUT, "CLAV_PgmNetMsgInput", (void *) (AppData.eAppState),
      E_DBG_TYPE_NET_STATE);
#endif
  return mef_clav;
}

PUBLIC teClavState CLAV_PgmActionTouche(etCLAV_keys keys)
{
  teClavState mef_clav = AppData.eClavState;
  stToucheDef touche = { 0 };
  etCLAV_mod key_mode = AppData.eClavmod - E_KM_1;
  etCLAV_keys key_code = keys - E_KEY_NUM_1;
  uint8 box_id = AppData.u8BoxId;
  uint8 position = 0;

#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_PgmActionTouche",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  if (keys >= E_KEY_NUM_1 && keys <= E_KEY_NUM_0)
  {
    // On envoie la touche et le mode a la carte puissance
    vPrintf("\n%sProgrammation touche clavier:%s, mode:%s\n", gch_spaces,
        dbg_etCLAV_keys[keys], dbg_etCLAV_mod[key_mode + E_KM_1]);

    // verifier si cette touche connait la boite a configurer
    // si non rajouter cette boite a la touche
#if fn1
    CLAV_TrouverAssociationToucheBoite(AppData.eClavmod,keys,AppData.u8BoxId);
#else
    touche.la_touche = keys;
    touche.le_mode = AppData.eClavmod;
    if (CLAV_TrouverAssociationToucheBoite(&touche, box_id, &position) == FALSE)
    {
      eeprom.netConf.boxList[key_mode][key_code][position] = box_id;
      eeprom.netConf.ptr_boxList[key_mode][key_code]++;
      vPrintf("%s La touche a la connaisance de la boite %d!\n", gch_spaces,
          box_id);
    }
#endif
    bufEmission[0] = E_MSG_CFG_LIENS;
    bufEmission[1] = key_mode << 4 | key_code;
    bufEmission[2] = eeprom.netConf.boxData[key_mode][key_code][box_id];

    vPrintf("%s Msg %x,%x,%x vers [%x;%x]\n", gch_spaces, bufEmission[0],
        bufEmission[1], bufEmission[2], (uint32) (eeprom.BoxAddr[box_id] >> 32),
        (uint32) (eeprom.BoxAddr[box_id] & 0xFFFFFFFF));
    eJenie_SendData(eeprom.BoxAddr[box_id], bufEmission, 3,
    TXOPTION_SILENT);
    mef_clav = E_KS_ATTENDRE_FIN_CONFIG_BOITE;
  }

#if !NO_DEBUG_ON
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
  PBAR_DbgTrace(E_FN_OUT, "CLAV_PgmActionTouche", (void *) (AppData.eAppState),
      E_DBG_TYPE_NET_STATE);
#endif

  return mef_clav;
}

PRIVATE teClavState pgm_GererBoiteEntrante(tsData *psData)
{
  teClavState mef_clav = E_KS_NON_DEFINI;
  uint8 box_number = psData->pau8Data[psData->u16Length - 1];

#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "pgm_GererBoiteEntrante",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  vPrintf("%sBoite id [%d] \n", gch_spaces, box_number);

  if (box_number < C_MAX_BOXES)
  {
    // verifier que l'on ne connait pas deja cette @
    //if(eeprom.BoxAddr[IncomingBoxId]==0xffffffffffffffffULL){
    if (eeprom.BoxAddr[box_number] == 0ULL)
    {
      vPrintf("%s Cette boite [%d] est nouvelle\n", gch_spaces, box_number);
      vPrintf("%s  Je la memorise\n", gch_spaces);

      eeprom.BoxAddr[box_number] = psData->u64SrcAddress;
      // Une boite de plus dans le clavier
      eeprom.nbBoite++;
    }
    else
    {
      vPrintf("%s Je connais a priori cette boite (id:%d)\n", gch_spaces,
          box_number);
      if (eeprom.BoxAddr[box_number] != psData->u64SrcAddress)
      {
        vPrintf("%s  La config a changee\n", gch_spaces);
        eeprom.BoxAddr[box_number] = psData->u64SrcAddress;
      }
      else
      {
        vPrintf("%s  C'est bien celle que je connais\n", gch_spaces);
      }
    }
    vPrintf("%s   Selectionner un mode et une touche\n\n", gch_spaces);

    au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_EN_ATTENTE_TOUCHE_BC;

    AppData.eNetState = E_KS_NET_CLIENT_IN;
    AppData.u8BoxId = box_number;

    AppData.eAppState = APP_BOUCLE_PRINCIPALE;
  }
  else
  {
    vPrintf("%sERROR !! Box id %d superieur a %d\n", gch_spaces, box_number,
        C_MAX_BOXES);

    // Retour au mode normal
    vPrintf("%sRetour en usage : utilisation courante\n", gch_spaces);
    mef_clav = CLAV_GererTouche(AppData.eKeyPressed);
    AppData.eNetState = E_KS_NET_CONF_BRK;


    // TODO : Envoyer msg a la box id (en erreur) pour qu'elle ne reste pas en attente de touche clavier
  }
#if !NO_DEBUG_ON
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
  PBAR_DbgTrace(E_FN_OUT, "pgm_GererBoiteEntrante",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
#endif

  return mef_clav;
}

PRIVATE void pgm_CreerConfigAll(uint8 box_id)
{
  //teClavState mef_clav 	= E_CLAV_ETAT_UNDEF;
  etCLAV_keys eLaTouche = AppData.eKeyPressed;
  etCLAV_mod eLeMode = AppData.eClavmod;
  uint8 valThisBox = 0;
  uint8 i = 0;
  uint8 useBox = 0;
  uint8 tmp = 0;

  stToucheDef touche = { 0 };
  etCLAV_mod key_mode = AppData.eClavmod - E_KM_1;
  etCLAV_keys key_code = eLaTouche - E_KEY_NUM_1;
  uint8 position = 0;

  // rajout de cette boite a la liste de celle du clavier
  vPrintf("Une B:%d de + a  key:%s ?\n", box_id, dbg_etCLAV_keys[eLaTouche]);
  touche.la_touche = eLaTouche;
  touche.le_mode = eLeMode;
  if (CLAV_TrouverAssociationToucheBoite(&touche, box_id, &position) == TRUE)
  {
    vPrintf("km=%d,kc=%d,po=%d,v=%d\n", key_mode, key_code, position, box_id);
    eeprom.netConf.boxList[key_mode][key_code][position] = box_id;
    eeprom.netConf.ptr_boxList[key_mode][key_code]++;
    vPrintf("   Sauvegarde Terminee!\n");
  }

  // on va parcourir toute les boites associees a cette touche
  // et creer une commande ALL
  vPrintf("\nCreation cmd 'ALL' pour touche %s\n",
      dbg_etCLAV_keys[E_KEY_NUM_ETOILE]);
  // Parcourir toute les touches de la boite en config
  valThisBox = 0;
  for (i = 0; i < C_MAX_KEYS; i++)
  {
    // recuperer la config du clavier et la stocker
    // dans la touche ALL
    //eeprom.netConf.boxData[mode][clav][AppData.u8BoxId] = conf;
    tmp = eeprom.netConf.boxData[key_mode][i][box_id];
    valThisBox |= eeprom.netConf.boxData[key_mode][i][box_id];
    useBox = eeprom.netConf.boxList[key_mode][key_code][box_id];
    vPrintf(" box_position:%d key:%s, val:%x, all:%x\n", useBox,
        dbg_etCLAV_keys[i + 1], tmp, valThisBox);
  }

  // sauvegarder cette valeur dans la touche ALL de la boite en cours
  // On sauve la touche all
  eeprom.netConf.boxData[key_mode][C_KEY_MEM_ALL][box_id] = valThisBox;

  // Dire que Touche ALL position ptr++ a une Boite configuree
  // La touche ALL est la derniere de toute les touche autorisee
  touche.la_touche = E_KEY_NUM_ETOILE;
  touche.le_mode = eLeMode;
  if (CLAV_TrouverAssociationToucheBoite(&touche, box_id, &position) == FALSE)
  {
    vPrintf("   Sauvegarde touche 'ALL' Terminee!\n");
    eeprom.netConf.boxList[key_mode][C_KEY_MEM_ALL][position] = box_id;
    eeprom.netConf.ptr_boxList[key_mode][C_KEY_MEM_ALL]++;
  }

  // sauvegarder la config dans la flash !!!
  if (!bAHI_FlashEraseSector(7))
  {
    vPrintf("Pb lors effacement Sector 3\n");
  }
  else
  {
    // Programmation

    if (!bAHI_FullFlashProgram(FLASH_START, sizeof(bpsFlash),
        (uint8 *) &eeprom))
    {
      vPrintf("Pb Lors de la sauvegarde en Flash\n");
    }
    else
    {
      vPrintf("Sauvegarde sur Flash terminee\n");
    }
  }
}
