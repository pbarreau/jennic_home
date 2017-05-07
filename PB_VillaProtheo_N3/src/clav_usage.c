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

PUBLIC char const *menu_pgm[] = { "Net Led", "Bip clavier", "Liaison" };
PRIVATE eLedInfo MenuFlashValue[] = { E_FLASH_MENU_LED_NET,
    E_FLASH_MENU_BIP_CLAVIER, E_FLASH_MENU_LIAISON };

PUBLIC uint8 bufEmission[3] = { 0, 0, 0 };
PUBLIC bool_t b_EteindreNet = FALSE;

PRIVATE etRunningStp NEW_CLAV_GererToucheModeSuperUser(etInUsingkey laTouche);
PRIVATE etRunningStp NEW_CLAV_GererToucheModeSimpleUser(etInUsingkey laTouche);
PRIVATE etRunningStp NEW_CLAV_ChoisirNouveauRole(etInUsingkey laTouche);
PRIVATE void NEW_ManageNetworkLed(etInUsingkey laTouche);
PRIVATE etRunningStp NEW_CLAV_MenuSuperUSer(eLedInfo flashingType,
    etInUsingkey laTouche);

PRIVATE etRunningStp clav_BtnPgmL1(etRunningStp mef_clav, uint8 *care);
PRIVATE etRunningStp clav_BtnPgmL2(etRunningStp mef_clav, uint8 *care);
PRIVATE void clav_EraseOrReset(etInUsingkey keys);
PRIVATE etRunningKbd tabVisibleMode[] = { E_KS_KBD_VIRTUAL_1,
    E_KS_KBD_VIRTUAL_2, E_KS_KBD_VIRTUAL_3, E_KS_KBD_VIRTUAL_4 };
PRIVATE etInUsingkey tabModeKeys[] = { E_KEY_NUM_MOD_1, E_KEY_NUM_MOD_2,
    E_KEY_NUM_MOD_3, E_KEY_NUM_MOD_4 };

PUBLIC etRunningStp CLAV_GererTouche(etInUsingkey keys)
{
  static int menuLevel = 0;

  stParam maConfig;

  maConfig.stpVal = E_KS_STP_NON_DEFINI;
  maConfig.rolVal = E_KS_ROL_NON_DEFINI;
  maConfig.kbdVal = E_KS_KBD_NON_DEFINI;
  maConfig.netVal = E_KS_NET_NON_DEFINI;

  etRunningStp stpVal = maConfig.stpVal;
  etRunningRol rolVal = maConfig.rolVal;
  etRunningKbd kbdVal = maConfig.kbdVal;
  etRunningNet netVal = maConfig.netVal;

  ///etRunningStp (*ptrFnTraitement)(stParam *info);

  vPrintf("Rol:%s, Kbd:%s\n", dbg_etCLAV_role[rolVal], dbg_etCLAV_mod[kbdVal]);
  vPrintf("Stp:%s, Key:%s\n", dbg_teClavState[stpVal], dbg_etCLAV_keys[kbdVal]);
  vPrintf("Net:%s\n", dbg_teRunningNet[netVal]);

  if ((keys == E_KEY_NUM_DIESE)
      && (timer_touche[E_KEY_NUM_DIESE] <= C_PRESSION_T1))
  {
    menuLevel++;
    menuLevel = menuLevel % 3;
  }

  if ((keys == E_KEY_NUM_DIESE)
      && (timer_touche[E_KEY_NUM_DIESE] > C_PRESSION_T1))
  {
    maConfig.rolVal++;
    maConfig.rolVal = maConfig.rolVal % 2;
  }

  stpVal = (*MenuPossible[maConfig.rolVal][menuLevel])(&maConfig);
  return stpVal;
  //return E_KS_STP_ATTENTE_TOUCHE;

}
#if 0
PUBLIC etRunningStp CLAV_GererTouche(etInUsingkey keys)
{
  etRunningStp stpVal = AppData.eClavState;
  etRunningRol rolVal = AppData.usage;
  etRunningKbd kbdVal = AppData.eClavmod;
  etRunningNet netVal = AppData.eNetState;

  vPrintf("Rol:%s, Kbd:%s\n", dbg_etCLAV_role[rolVal], dbg_etCLAV_mod[kbdVal]);
  vPrintf("Stp:%s, Key:%s\n", dbg_teClavState[stpVal], dbg_etCLAV_keys[kbdVal]);
  vPrintf("Net:%s\n", dbg_teRunningNet[netVal]);

  // Cette touche est dans quel contexte ?
  if (rolVal == E_KS_ROL_TECHNICIEN)
  {
    stpVal = NEW_CLAV_GererToucheModeSuperUser(keys);
  }
  else if (rolVal == E_KS_ROL_UTILISATEUR)
  {
    stpVal = NEW_CLAV_GererToucheModeSimpleUser(keys);
  }
  else if (rolVal == E_KS_ROL_CHOISIR)
  {
    vPrintf("Choix 1 ou 2\n");
    stpVal = NEW_CLAV_ChoisirNouveauRole(keys);
  }
  else
  {
    vPrintf("Key context error!!\n");
  }
  return stpVal;
}
#endif

PRIVATE etRunningStp NEW_CLAV_ChoisirNouveauRole(etInUsingkey laTouche)
{
  etRunningRol cur_role = AppData.usage;

  vPrintf("Utilisation courante:%d\n", cur_role);

  if (laTouche == E_KEY_NUM_0)
  {
    cur_role = E_KS_ROL_UTILISATEUR;
    au8Led_clav[C_CLAV_LED_INFO_1].mode = mNetOkTypeFlash;
  }
  else if (laTouche == E_KEY_NUM_1)
  {
    cur_role = E_KS_ROL_TECHNICIEN;
    au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_EN_ATTENTE_TOUCHE_BC;
  }

  if (cur_role != E_KS_ROL_CHOISIR)
  {
    AppData.usage = cur_role;
    CLAV_GererMode(E_KEY_NUM_MOD_1);
  }

  return E_KS_STP_ATTENTE_TOUCHE;
}

PRIVATE etRunningStp NEW_CLAV_GererToucheModeSuperUser(etInUsingkey laTouche)
{
  etRunningStp mef_clav = AppData.eClavState;
  uint8 uKeyPos = AppData.ukey;

  static uint8 idMode = 0;
  static eLedInfo cur_menu = E_FLASH_MENU_LED_NET;
  uint8 val_flash = (uint8) MenuFlashValue[idMode];
  int len = (sizeof(menu_pgm) / sizeof(char **));

  if ((laTouche == E_KEY_NUM_DIESE) && (timer_touche[uKeyPos] <= C_PRESSION_T1))
  {
    vPrintf("Len = %d ", len);
    vPrintf("Menu '%s' flash:'%x'\n", menu_pgm[idMode], val_flash);
    au8Led_clav[C_CLAV_LED_INFO_2].mode = val_flash;
    au8Led_clav[C_CLAV_LED_INFO_3].mode = ~E_FLASH_OFF;

    cur_menu = MenuFlashValue[idMode];
    idMode++;
    idMode = idMode % len;

  }

  if ((laTouche == E_KEY_NUM_DIESE) && (timer_touche[uKeyPos] > C_PRESSION_T1))
  {
    AppData.usage = E_KS_ROL_CHOISIR;
    CLAV_GererMode(E_KEY_NUM_MOD_5);
  }

  mef_clav = NEW_CLAV_MenuSuperUSer(cur_menu, laTouche);
  //      CLAV_PgmActionTouche(laTouche);

  return mef_clav;
}

PRIVATE etRunningStp NEW_CLAV_MenuSuperUSer(eLedInfo flashingType,
    etInUsingkey laTouche)
{
  etRunningStp mef_clav = AppData.eClavState;

  switch (flashingType)
  {
    case E_FLASH_MENU_LED_NET:
    {
      NEW_ManageNetworkLed(laTouche);
    }
    break;

    case E_FLASH_MENU_BIP_CLAVIER:
    {

    }
    break;
    case E_FLASH_MENU_LIAISON:
    {
      if (laTouche == E_KEY_NUM_0)
      {
        au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_MENU_LIAISON;
        au8Led_clav[C_CLAV_LED_INFO_2].mode = ~E_FLASH_OFF;
        au8Led_clav[C_CLAV_LED_INFO_3].mode = ~E_FLASH_OFF;

        mef_clav = CLAV_PgmNetMontrerClavier();
      }
    }
    break;
    default:
    {
      vPrintf("PGM Menu Error!!\n");
    }
    break;
  }
  return mef_clav;
}

PRIVATE void NEW_ManageNetworkLed(etInUsingkey laTouche)
{
  bufEmission[0] = 0;
  bufEmission[1] = 0;
  bufEmission[2] = 0;

  switch (laTouche)
  {
    case E_KEY_NUM_0:
    {
      vPrintf("Broadcast Net Off\n");
#ifdef CLAV_IS_VELLMAN
      mNetOkTypeFlash = ~E_FLASH_OFF;
#else
      mNetOkTypeFlash = E_FLASH_OFF;
#endif

      bufEmission[0] = E_MSG_NET_LED_OFF;
      b_EteindreNet = TRUE; // Pour le faire quand on va quitter le mode programmation
    }
    break;

    case E_KEY_NUM_2:
    {

    }
    break;
    case E_KEY_NUM_1:
    {
      vPrintf("Broadcast Net On\n");
      mNetOkTypeFlash = E_FLASH_RESEAU_ACTIF;

      bufEmission[0] = E_MSG_NET_LED_ON;
      b_EteindreNet = FALSE; // Pour le faire quand on va quitter le mode programmation
    }
    break;
    default:
    {

    }
    break;
  }

  if (bufEmission[0])
  {
    eJenie_SendData(0, bufEmission, 3,
    TXOPTION_SILENT | TXOPTION_BDCAST);
  }
}
PRIVATE etRunningStp NEW_CLAV_GererToucheModeSimpleUser(etInUsingkey laTouche)
{
  uint8 uKeyPos = AppData.ukey;
  static uint8 idMode = 1; // enrelation avec le mode affiche par les leds

#if 0
  if (uKeyPos < 0)
  {
    vPrintf("Erreur Codage a corriger\n");
    return mef_clav;
  }
#endif

  if ((laTouche == E_KEY_NUM_DIESE) && (timer_touche[uKeyPos] <= C_PRESSION_T1))
  {
    vPrintf("Passage en mode:%d", tabVisibleMode[idMode]);
    CLAV_GererMode(tabModeKeys[idMode]);
    idMode++;
    idMode = idMode % (sizeof(tabVisibleMode) / sizeof(etRunningKbd));

  }

  if ((laTouche == E_KEY_NUM_DIESE) && (timer_touche[uKeyPos] > C_PRESSION_T1))
  {
    AppData.usage = E_KS_ROL_CHOISIR;
    CLAV_GererMode(E_KEY_NUM_MOD_5);
  }

  CLAV_UsrActionTouche(laTouche);
  return E_KS_STP_ATTENTE_TOUCHE;
}

#if OLD_CLAV
PUBLIC etRunningStp CLAV_GererTouche(etInUsingkey keys)
{
  static bool_t b_use_bip = FALSE;
  etRunningStp mef_clav = AppData.eClavState;

#if !NO_DEBUG_ON
  int stepper = 0;
  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_GererTouche",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

#ifndef CLAV_IS_VELLMAN
  // Faire un Bip pour signal touche est detectee
  vPrintf("%sEmettre un BIP ?\n", gch_spaces);

  if (b_use_bip == TRUE)
  {
    vPrintf("%s Use bip\n", gch_spaces);
    b_activer_bip = TRUE;
    vAHI_DioSetOutput(C_CLAV_BUZER, 0);
  }
  else
  {
    vPrintf("%s Use NO bip\n", gch_spaces);
    b_activer_bip = FALSE;
    vAHI_DioSetOutput(0, C_CLAV_BUZER);
  }
#endif

  if ((mef_clav == E_KS_STP_ULTRA_MODE)
      || (AppData.ePrevClav == E_KS_STP_ULTRA_MODE))
  {
    clav_EraseOrReset(keys);
    mef_clav = CLAV_BoutonDeConfiguration(&b_use_bip);
  }
  else
  {
    mef_clav = E_KS_STP_ATTENTE_TOUCHE;
    switch (keys)
    {
      case E_KEY_NUM_MOD_1:
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
        if (AppData.usage == E_KS_ROL_TECHNICIEN)
        {
          mef_clav = CLAV_PgmActionTouche(keys);
        }
        else if (AppData.usage == E_KS_ROL_UTILISATEUR)
        {
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

#if !NO_DEBUG_ON
  PBAR_DbgInside(stepper, gch_spaces, E_FN_OUT, AppData);
  PBAR_DbgTrace(E_FN_OUT, "CLAV_GererTouche", (void *) AppData.eAppState,
      E_DBG_TYPE_NET_STATE);
#endif

  return mef_clav;
}
#endif

PUBLIC etRunningStp CLAV_BoutonDeConfiguration(bool_t * bip_on)
{
  static bool_t bUltraCareMode = FALSE;
  etRunningStp mef_clav = E_KS_STP_NON_DEFINI;

  static uint8 mode_clavier = 1;
  static bool_t usr_or_tec = FALSE;
  etInUsingkey choix_mode;

#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_BoutonDeConfiguration",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

#ifdef CLAV_IS_VELLMAN
  if (timer_touche[AppData.ukey] <= C_PRESSION_T1)
  {
    // Selection rotatif du mode
    choix_mode = R_Key_modes[mode_clavier];
    CLAV_GererMode(choix_mode);
    mode_clavier++;
    mode_clavier = (mode_clavier) % (CST_NB_MODES);
    vPrintf("Next Mode:%d\n", mode_clavier);
    mef_clav = E_KS_STP_ATTENTE_TOUCHE;
  }
#else
  // Activation ou pas du Bip Clavier ?
  if (timer_touche[AppData.ukey] <= C_PRESSION_T1)
  {
    if ((*bip_on) == FALSE)
    {
      vPrintf("%sActivation bip clavier\n", gch_spaces);
    }
    else
    {
      vPrintf("%sSupression bip clavier\n", gch_spaces);
    }
    (*bip_on) = !(*bip_on);
    mef_clav = E_KS_STP_ATTENTE_TOUCHE;
  }
#endif
  else if ((timer_touche[AppData.ukey] <= C_PRESSION_T2)
      && (AppData.usage == E_KS_ROL_UTILISATEUR))
  {
    bufEmission[1] = 0;
    bufEmission[2] = 0;

    if (mNetOkTypeFlash == E_FLASH_RESEAU_ACTIF)
    {
      vPrintf("Broadcast Net Off\n");
#ifdef CLAV_IS_VELLMAN
      mNetOkTypeFlash = ~E_FLASH_OFF;
#else
      mNetOkTypeFlash = E_FLASH_OFF;
#endif

      bufEmission[0] = E_MSG_NET_LED_OFF;
    }
    else
    {
      vPrintf("Broadcast Net On\n");
      mNetOkTypeFlash = E_FLASH_RESEAU_ACTIF;

      bufEmission[0] = E_MSG_NET_LED_ON;
    }
    au8Led_clav[C_CLAV_LED_INFO_1].mode = mNetOkTypeFlash;

    eJenie_SendData(0, bufEmission, 3,
    TXOPTION_SILENT | TXOPTION_BDCAST);
  }
  else if (timer_touche[AppData.ukey] <= C_PRESSION_T3)
  {
    vPrintf("Mode installateur ou utilisateur ?\n");

    // On se met en mode defaut que ce soit en usr ou en pgm
    if (usr_or_tec == FALSE)
    {
      CLAV_GererMode(E_KEY_NON_DEFINI);
      vPrintf("%sPassage en usage : configuration systeme\n", gch_spaces);
      AppData.usage = E_KS_ROL_TECHNICIEN;

      mef_clav = CLAV_PgmNetMontrerClavier();
    }
    else
    {
      CLAV_GererMode(E_KEY_NUM_MOD_1);
      vPrintf("%sPassage en usage : utilisation courante\n", gch_spaces);
      AppData.usage = E_KS_ROL_UTILISATEUR;

      au8Led_clav[C_CLAV_LED_INFO_1].mode = mNetOkTypeFlash;
      AppData.eAppState = E_PGL_BOUCLE_PRINCIPALE;
      mef_clav = CLAV_PgmNetRetirerClavier();
    }
    usr_or_tec = !usr_or_tec;

  }
  else if (AppData.eClavState == E_KS_STP_ULTRA_MODE)
  {
    // Revenir en normal
    vPrintf("Sortie en ultra mode\n");
    mef_clav = E_KS_STP_NON_DEFINI;
    CLAV_GererMode(E_KEY_NUM_MOD_5);
  }
  else
  {
    // Allez en ultra care
    vPrintf("Passage en ultra mode\n");
    AppData.eClavState = E_KS_STP_ULTRA_MODE;
    mef_clav = E_KS_STP_ULTRA_MODE;
    CLAV_GererMode(E_KEY_NUM_MOD_6);
  }

#if !NO_DEBUG_ON
  PBAR_DbgInside(stepper, gch_spaces, E_FN_OUT, AppData);
  PBAR_DbgTrace(E_FN_IN, "CLAV_BoutonDeConfiguration",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
#endif
  return mef_clav;
}

PUBLIC void CLAV_NetMsgInput(tsData *psData)
{
  etRunningStp mef_clav = AppData.eClavState;

#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_NetMsgInput",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  if (AppData.usage == E_KS_ROL_TECHNICIEN)
  {
    mef_clav = CLAV_PgmNetMsgInput(psData);
  }
  else if (AppData.usage == E_KS_ROL_UTILISATEUR)
  {
    mef_clav = CLAV_UsrNetMsgInput(psData);
  }
  else
  {
    vPrintf("ERR:CLAV_GererMsgDeBoite cas non prevu !\n");
  }

#if !NO_DEBUG_ON
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
  PBAR_DbgTrace(E_FN_OUT, "CLAV_NetMsgInput", (void *) (AppData.eAppState),
      E_DBG_TYPE_NET_STATE);
#endif

  AppData.eClavState = mef_clav;
}

PRIVATE etRunningStp clav_BtnPgmL1(etRunningStp mef_clav, uint8 *care)
{
  static bool_t usr_or_tec = FALSE;
#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "clav_BtnPgmL1",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  if (timer_touche[AppData.ukey] <= C_PRESSION_T4)
  {
    // On se met en mode defaut que ce soit en usr ou en pgm
    CLAV_GererMode(E_KEY_NUM_MOD_1);
    if (usr_or_tec == FALSE)
    {
      vPrintf("%sPassage en usage : configuration systeme\n", gch_spaces);
      AppData.usage = E_KS_ROL_TECHNICIEN;

      mef_clav = CLAV_PgmNetMontrerClavier();
    }
    else
    {
      vPrintf("%sPassage en usage : utilisation courante\n", gch_spaces);
      AppData.usage = E_KS_ROL_UTILISATEUR;

      au8Led_clav[C_CLAV_LED_INFO_1].mode = mNetOkTypeFlash;
      AppData.eAppState = E_PGL_BOUCLE_PRINCIPALE;
      mef_clav = CLAV_PgmNetRetirerClavier();
    }
    usr_or_tec = !usr_or_tec;
  }
  else
  {
    vPrintf("%sUltra mode On\n", gch_spaces);

    mef_clav = E_KS_STP_ULTRA_MODE;
    // Memorisation de l'etat avant traitement de la touche
    AppData.ePrevClav = E_KS_STP_ULTRA_MODE;

    *care = TRUE;
    au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_RESET_POSSIBLE;
    au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_OFF;
    au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_OFF;
  }

#if !NO_DEBUG_ON
  PBAR_DbgInside(stepper, gch_spaces, E_FN_OUT, AppData);
  PBAR_DbgTrace(E_FN_OUT, "clav_BtnPgmL1", (void *) (AppData.eAppState),
      E_DBG_TYPE_NET_STATE);
#endif

  return (mef_clav);
}
PRIVATE etRunningStp clav_BtnPgmL2(etRunningStp mef_clav, uint8 *care)
{
  etRunningStp mef_ici = mef_clav;
#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "clav_BtnPgmL2",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  if (timer_touche[AppData.ukey] <= C_PRESSION_T5)
  {
    vPrintf("%sUltra mode Off\n", gch_spaces);
    *care = FALSE;
    au8Led_clav[C_CLAV_LED_INFO_1].mode = mNetOkTypeFlash;
    au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_OFF;
    au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_OFF;
    mef_ici = E_KS_STP_NON_DEFINI;
    AppData.ePrevClav = E_KS_STP_NON_DEFINI;
  }

#if !NO_DEBUG_ON
  PBAR_DbgInside(stepper, gch_spaces, E_FN_OUT, AppData);
  PBAR_DbgTrace(E_FN_OUT, "clav_BtnPgmL2", (void *) (AppData.eAppState),
      E_DBG_TYPE_NET_STATE);
#endif
  return (mef_ici);
}

#if 0
PRIVATE etRunningStp clav_BtnPgmL2(etRunningStp mef_clav, uint8 *care)
{
  static bool_t bReqEraseFlash = FALSE;

  if(timer_touche[AppData.ukey] <= C_PRESSION_T3)
  {
    if(bReqEraseFlash == FALSE)
    {
      bReqEraseFlash = TRUE;
      if(eeprom.nbBoite == 0xFF || eeprom.nbBoite == 0x00)
      {
        vPrintf(" La Flash n'a aucune config a effacer\n");
      }
      else
      {
        vPrintf(" Effacement de la config en memoire Flash\n");
        if(!bAHI_FlashEraseSector(7))
        {
          vPrintf("Pb lors effacement Secteur\n");
        }
        // reset de la structure des donnees eprom
        memset(&eeprom,0x00,sizeof(eeprom));
      }
      mef_clav = E_KS_STP_ULTRA_MODE;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_OFF;

    }
    else
    {
      vPrintf("Ultra mode Off\n");
      *care = FALSE;
      au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_RESEAU_ACTIF;
      au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_OFF;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_OFF;
      mef_clav = E_KS_STP_NON_DEFINI;
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
#endif

PRIVATE void clav_EraseOrReset(etInUsingkey keys)
{
#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "clav_EraseOrReset",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif
  if (keys == E_KEY_NUM_ETOILE)
  {
    if (timer_touche[AppData.ukey] >= C_PRESSION_T3)
    {
      vPrintf("%sDemande effacement de config clavier !\n", gch_spaces);

      if (eeprom.nbBoite == 0xFF || eeprom.nbBoite == 0x00)
      {
        vPrintf("%s La Flash n'a aucune config a effacer\n", gch_spaces);
      }
      else
      {
        vPrintf("%s Effacement de la config en memoire Flash\n", gch_spaces);
        if (!bAHI_FlashEraseSector(7))
        {
          vPrintf("Pb lors effacement Secteur\n");
        }
        // reset de la structure des donnees eprom
        memset(&eeprom, 0x00, sizeof(eeprom));
      }
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_OFF;
    }
    else
    {
      vPrintf("\n\n%sReset du module demande\n", gch_spaces);
      // Vidage buffer UART
      while ((u8JPI_UartReadLineStatus(E_JPI_UART_0)
          & (E_JPI_UART_LS_THRE | E_JPI_UART_LS_TEMT))
          != (E_JPI_UART_LS_THRE | E_JPI_UART_LS_TEMT))
        ;

      // effectuer un reset du jennic
      vAHI_SwReset();
    }
  }
#if !NO_DEBUG_ON
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
  PBAR_DbgTrace(E_FN_OUT, "clav_EraseOrReset", (void *) (AppData.eAppState),
      E_DBG_TYPE_NET_STATE);
#endif

}

PUBLIC etRunningStp pFn1_1(stParam *param)
{
  etRunningStp retVal = E_KS_STP_NON_DEFINI;
  bool_t onShot = TRUE;

  if (onShot)
  {
    onShot = FALSE;
    vPrintf("pFn1_1\n");
  }

  return E_KS_STP_ATTENTE_TOUCHE; //retVal;
}
PUBLIC etRunningStp pFn1_2(stParam *param)
{
  etRunningStp retVal = E_KS_STP_NON_DEFINI;
  bool_t onShot = TRUE;

  if (onShot)
  {
    onShot = FALSE;
    vPrintf("pFn1_2\n");
  }


  return E_KS_STP_ATTENTE_TOUCHE; //retVal;
}
PUBLIC etRunningStp pFn1_3(stParam *param)
{
  etRunningStp retVal = E_KS_STP_NON_DEFINI;
  bool_t onShot = TRUE;

  if (onShot)
  {
    onShot = FALSE;
    vPrintf("pFn1_3\n");
  }


  return E_KS_STP_ATTENTE_TOUCHE; //retVal;
}

PUBLIC etRunningStp pFn2_1(stParam *param)
{
  etRunningStp retVal = E_KS_STP_NON_DEFINI;
  bool_t onShot = TRUE;

  if (onShot)
  {
    onShot = FALSE;
    vPrintf("pFn2_1\n");
  }


  return E_KS_STP_ATTENTE_TOUCHE; //retVal;
}
PUBLIC etRunningStp pFn2_2(stParam *param)
{
  etRunningStp retVal = E_KS_STP_NON_DEFINI;
  bool_t onShot = TRUE;

  if (onShot)
  {
    onShot = FALSE;
    vPrintf("pFn2_2\n");
  }


  return E_KS_STP_ATTENTE_TOUCHE; //retVal;
}
PUBLIC etRunningStp pFn2_3(stParam *param)
{
  etRunningStp retVal = E_KS_STP_NON_DEFINI;
  bool_t onShot = TRUE;

  if (onShot)
  {
    onShot = FALSE;
    vPrintf("pFn2_3\n");
  }


  return E_KS_STP_ATTENTE_TOUCHE; //retVal;
}
