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

PUBLIC uint8 bufEmission[3] = { 0, 0, 0 };

PRIVATE teClavState clav_BtnPgmL1(teClavState mef_clav, uint8 *care);
PRIVATE teClavState clav_BtnPgmL2(teClavState mef_clav, uint8 *care);
PRIVATE void clav_EraseOrReset(etCLAV_keys keys);

PUBLIC teClavState CLAV_GererTouche(etCLAV_keys keys)
{
  static bool_t b_use_bip = FALSE;
  teClavState mef_clav = AppData.eClavState;

#if !NO_DEBUG_ON
  int stepper = 0;
  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_GererTouche",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  // Faire un Bip pour signal touche est detectee
  vPrintf("%sEmettre un BIP ?\n", gch_spaces);

#ifndef CLAV_IS_VELLMAN
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

  if ((mef_clav == E_CLAV_ULTRA_MODE)
      || (AppData.ePrevClav == E_CLAV_ULTRA_MODE))
  {
    clav_EraseOrReset(keys);
    mef_clav = CLAV_BoutonDeConfiguration(&b_use_bip);
  }
  else
  {
    mef_clav = E_CLAV_ETAT_EN_ATTENTE;
    switch (keys)
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
        if (AppData.usage == E_CLAV_USAGE_CONFIG)
        {
          mef_clav = CLAV_PgmActionTouche(keys);
        }
        else if (AppData.usage == E_CLAV_USAGE_NORMAL)
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

PUBLIC teClavState CLAV_BoutonDeConfiguration(bool_t * bip_on)
{
  static bool_t bUltraCareMode = FALSE;
  teClavState mef_clav = E_CLAV_ETAT_UNDEF;

  static uint8 mode_clavier = 1;
  //etCLAV_mod cur_mod = E_CLAV_MODE_NOT_SET;
  etCLAV_keys choix_mode;

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
    mef_clav = E_CLAV_ETAT_EN_ATTENTE;
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
    mef_clav = E_CLAV_ETAT_EN_ATTENTE;
  }
#endif
  else if (timer_touche[AppData.ukey] <= C_PRESSION_T2)
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
    vPrintf("Passage en mode programmation clavier\n");
  }
  else if (bUltraCareMode == FALSE)
  {
    // Programmation de niveau 1
    mef_clav = clav_BtnPgmL1(mef_clav, &bUltraCareMode);
  }
  else
  {
    // Programmation niveau 2
    mef_clav = clav_BtnPgmL2(mef_clav, &bUltraCareMode);
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
  teClavState mef_clav = AppData.eClavState;

#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_NetMsgInput",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  if (AppData.usage == E_CLAV_USAGE_CONFIG)
  {
    mef_clav = CLAV_PgmNetMsgInput(psData);
  }
  else if (AppData.usage == E_CLAV_USAGE_NORMAL)
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

PRIVATE teClavState clav_BtnPgmL1(teClavState mef_clav, uint8 *care)
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
    CLAV_GererMode(E_KEY_MOD_1);
    if (usr_or_tec == FALSE)
    {
      vPrintf("%sPassage en usage : configuration systeme\n", gch_spaces);
      AppData.usage = E_CLAV_USAGE_CONFIG;

      //au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_RECHERCHE_RESEAU;
      mef_clav = CLAV_PgmNetMontrerClavier();
    }
    else
    {
      vPrintf("%sPassage en usage : utilisation courante\n", gch_spaces);
      AppData.usage = E_CLAV_USAGE_NORMAL;

      au8Led_clav[C_CLAV_LED_INFO_1].mode = mNetOkTypeFlash;
      AppData.eAppState = APP_BOUCLE_PRINCIPALE;
      mef_clav = CLAV_PgmNetRetirerClavier();
    }
    usr_or_tec = !usr_or_tec;
  }
  else
  {
    vPrintf("%sUltra mode On\n", gch_spaces);

    mef_clav = E_CLAV_ULTRA_MODE;
    // Memorisation de l'etat avant traitement de la touche
    AppData.ePrevClav = E_CLAV_ULTRA_MODE;

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
PRIVATE teClavState clav_BtnPgmL2(teClavState mef_clav, uint8 *care)
{
  teClavState mef_ici = mef_clav;
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
    mef_ici = E_CLAV_ETAT_EN_INITIALISATION;
    AppData.ePrevClav = E_CLAV_ETAT_UNDEF;
  }

#if !NO_DEBUG_ON
  PBAR_DbgInside(stepper, gch_spaces, E_FN_OUT, AppData);
  PBAR_DbgTrace(E_FN_OUT, "clav_BtnPgmL2", (void *) (AppData.eAppState),
      E_DBG_TYPE_NET_STATE);
#endif
  return (mef_ici);
}

#if 0
PRIVATE teClavState clav_BtnPgmL2(teClavState mef_clav, uint8 *care)
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
      mef_clav = E_CLAV_ULTRA_MODE;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_OFF;

    }
    else
    {
      vPrintf("Ultra mode Off\n");
      *care = FALSE;
      au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_RESEAU_ACTIF;
      au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_OFF;
      au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_OFF;
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
#endif

PRIVATE void clav_EraseOrReset(etCLAV_keys keys)
{
#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "clav_EraseOrReset",
      (void *) (AppData.eAppState), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif
  if (keys == E_KEY_ETOILE)
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