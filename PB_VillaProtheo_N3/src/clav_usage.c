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
PRIVATE const char *debug[] = { "TBD", "ON", "OFF" };

PRIVATE char const *menu_pgm[] = { "Net Led", "Bip clavier", "Instalation",
    "Sauvegarde" };
PRIVATE uint8 nbItemMenu = sizeof(menu_pgm) / sizeof(char**);
PRIVATE etRunningStp pMenuLed(etInUsingkey touche);
PRIVATE etRunningStp pMenuBip(etInUsingkey touche);
PRIVATE etRunningStp pMenuCnf(etInUsingkey touche);
PRIVATE etRunningStp pMenuSav(etInUsingkey touche);
PRIVATE etRunningStp (*MenuPossible[])(etInUsingkey touche) =
{ pMenuLed, pMenuBip, pMenuCnf, pMenuSav };

PRIVATE etRunningStp clav_BtnPgmL1(etRunningStp mef_clav, uint8 *care);
PRIVATE etRunningStp clav_BtnPgmL2(etRunningStp mef_clav, uint8 *care);
//PRIVATE void clav_EraseOrReset(etInUsingkey keys);
PRIVATE etRunningRol SelectionRole(void);
PRIVATE etRunningStp GererMenuTechnicien(etInUsingkey keys);

PUBLIC etRunningStp CLAV_GererTouche(etInUsingkey keys)
{
  etRunningStp stpVal = AppData.eClavState;
  etRunningRol rolVal = AppData.rol;

  // rolval doit changer ?
  if (keys == E_KEY_NUM_DIESE)
  {
    rolVal = SelectionRole();
    AppData.rol = rolVal;

    return E_KS_STP_ATTENTE_TOUCHE;
  }

  // Cette touche est dans quel contexte ?
  switch (rolVal)
  {
    case E_KS_ROL_TECHNICIEN:
    {
      stpVal = GererMenuTechnicien(keys);
      //stpVal = CLAV_PgmActionTouche(keys);
      vPrintf("Running values TECH\n");
      MyStepDebug();

    }
    break;

    case E_KS_ROL_UTILISATEUR:
    {
      stpVal = CLAV_UsrActionTouche(keys);
      vPrintf("Running values USR\n");
      MyStepDebug();
    }
    break;

    default:
    {
      vPrintf("Non Prevu\n");
      vPrintf("Key context error!!\n");
      AppData.rol = E_KS_ROL_UTILISATEUR;
    }

  }
  return E_KS_STP_ATTENTE_TOUCHE;
}

PRIVATE etRunningRol SelectionRole(void)
{
  static etRunningRol tmp = E_KS_ROL_UTILISATEUR;
  static bool_t b_use_bip = FALSE;

  if (timer_touche[E_KEY_NUM_DIESE - 1] < C_TIME_TO_CHANGE_ROLE)
  {
    if (tmp == E_KS_ROL_UTILISATEUR)
    {
      // bip On/OFF
      // Faire un Bip pour signal touche est detectee
      vPrintf("%sEmettre un BIP ? -> ", gch_spaces);

      if (b_use_bip == TRUE)
      {
        vPrintf("Use bip\n");
        b_MasterBip = TRUE;
        b_activer_bip = TRUE;
        //vAHI_DioSetOutput(C_CLAV_BUZER, 0);
      }
      else
      {
        vPrintf("Use NO bip\n");
        b_MasterBip = FALSE;
        //b_activer_bip = FALSE;
        //vAHI_DioSetOutput(0, C_CLAV_BUZER);
      }
      b_use_bip = !b_use_bip;
    }
    else
    {
      // changement de menu
      GererMenuTechnicien(E_KEY_NUM_DIESE);
    }
  }
  else
  {
    if (tmp == E_KS_ROL_UTILISATEUR)
    {
      tmp = E_KS_ROL_TECHNICIEN;
      // Mettre le menu de base du technicien.
      GererMenuTechnicien(E_KEY_NON_DEFINI);
    }
    else
    {
      tmp = E_KS_ROL_UTILISATEUR;
    }
  }
  return tmp;
}

PRIVATE etRunningStp GererMenuTechnicien(etInUsingkey keys)
{
  static uint8 menuId = 0;
  etRunningStp tmpStep = E_KS_STP_NON_DEFINI;

  switch (keys)
  {
    case E_KEY_NON_DEFINI:
    {
      menuId = 0;
    }
    break;

    case E_KEY_NUM_DIESE:
    {
      menuId++;
      menuId = menuId % nbItemMenu;
    }
    break;

    default:
    {
      // Executer le menu
      tmpStep = MenuPossible[menuId](keys);
    }
  }
  vPrintf("Menu \"%s\" touche:'%s'\n", menu_pgm[menuId], dbg_etCLAV_keys[keys]);

  return E_KS_STP_ATTENTE_TOUCHE;
}

#if 0
PUBLIC etRunningStp CLAV_GererTouche(etInUsingkey keys)
{
  static bool_t b_use_bip = FALSE;
  etRunningStp mef_clav = AppData.eClavState;

#if !NO_DEBUG_ON
  int stepper = 0;
  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_GererTouche", (void *) (AppData.pgl),
      E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  // Faire un Bip pour signal touche est detectee
  vPrintf("%sEmettre un BIP ? -> ", gch_spaces);

  if (b_use_bip == TRUE)
  {
    vPrintf("Use bip\n");
    b_activer_bip = TRUE;
    vAHI_DioSetOutput(C_CLAV_BUZER, 0);
  }
  else
  {
    vPrintf("Use NO bip\n");
    b_activer_bip = FALSE;
    vAHI_DioSetOutput(0, C_CLAV_BUZER);
  }

  if ((mef_clav == E_KS_STP_ULTRA_MODE) || (AppData.stp == E_KS_STP_ULTRA_MODE))
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
      case E_KEY_NUM_MOD_2:
      case E_KEY_NUM_MOD_3:
      case E_KEY_NUM_MOD_4:
      CLAV_GererMode(keys);
      break;

      case E_KEY_NUM_DIESE:
      mef_clav = CLAV_BoutonDeConfiguration(&b_use_bip);
      break;

      default:
      {
        if (AppData.rol == E_KS_ROL_TECHNICIEN)
        {
          mef_clav = CLAV_PgmActionTouche(keys);
        }
        else if (AppData.rol == E_KS_ROL_UTILISATEUR)
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
  PBAR_DbgTrace(E_FN_OUT, "CLAV_GererTouche", (void *) AppData.pgl,
      E_DBG_TYPE_NET_STATE);
#endif

  return mef_clav;
}
#endif

PUBLIC etRunningStp CLAV_BoutonDeConfiguration(bool_t * bip_on)
{
  static bool_t bUltraCareMode = FALSE;
  etRunningStp mef_clav = E_KS_STP_NON_DEFINI;

#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_BoutonDeConfiguration",
      (void *) (AppData.pgl), E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  // Activation ou pas du Bip Clavier ?
  if (timer_touche[AppData.key - 1] <= C_PRESSION_T1)
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
  else if (timer_touche[AppData.key - 1] <= C_PRESSION_T2)
  {
    vPrintf("Broadcast Net Off\n");
    mNetOkTypeFlash = E_FLASH_OFF;
    au8Led_clav[C_CLAV_LED_INFO_1].mode = mNetOkTypeFlash;

    bufEmission[0] = E_MSG_NET_LED_OFF;
    bufEmission[1] = 0;
    bufEmission[2] = 0;

    eJenie_SendData(0, bufEmission, 3,
    TXOPTION_SILENT | TXOPTION_BDCAST);
  }
  else if (timer_touche[AppData.key - 1] <= C_PRESSION_T3)
  {
    vPrintf("Broadcast Net On\n");
    mNetOkTypeFlash = E_FLASH_RESEAU_ACTIF;
    au8Led_clav[C_CLAV_LED_INFO_1].mode = mNetOkTypeFlash;

    bufEmission[0] = E_MSG_NET_LED_ON;
    bufEmission[1] = 0;
    bufEmission[2] = 0;

    eJenie_SendData(0, bufEmission, 3,
    TXOPTION_SILENT | TXOPTION_BDCAST);

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
  PBAR_DbgTrace(E_FN_OUT, "CLAV_BoutonDeConfiguration", (void *) (AppData.pgl),
      E_DBG_TYPE_NET_STATE);
#endif
  return mef_clav;
}

PUBLIC void CLAV_NetMsgInput(tsData *psData)
{
  etRunningStp mef_clav = AppData.eClavState;
  int msgLen = psData->u16Length;
  int i = 0;

#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_NetMsgInput", (void *) (AppData.pgl),
      E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  vPrintf("Msg in:\"");
  for (i = 0; i < (msgLen - 1); i++)
  {
    vPrintf("%x ", psData->pau8Data[i]);
  }
  vPrintf("%x\"\n", psData->pau8Data[i]);

  if (AppData.rol == E_KS_ROL_TECHNICIEN)
  {
    mef_clav = CLAV_PgmNetMsgInput(psData);
  }
  else if (AppData.rol == E_KS_ROL_UTILISATEUR)
  {
    mef_clav = CLAV_UsrNetMsgInput(psData);
  }
  else
  {
    vPrintf("ERR:CLAV_GererMsgDeBoite cas non prevu !\n");
  }

#if !NO_DEBUG_ON
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
  PBAR_DbgTrace(E_FN_OUT, "CLAV_NetMsgInput", (void *) (AppData.pgl),
      E_DBG_TYPE_NET_STATE);
#endif

  AppData.eClavState = mef_clav;
}

PRIVATE etRunningStp clav_BtnPgmL1(etRunningStp mef_clav, uint8 *care)
{
  static bool_t usr_or_tec = FALSE;
#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "clav_BtnPgmL1", (void *) (AppData.pgl),
      E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  if (timer_touche[AppData.key - 1] <= C_PRESSION_T4)
  {
    // On se met en mode defaut que ce soit en usr ou en pgm
    CLAV_GererMode(E_KEY_NUM_MOD_1);
    if (usr_or_tec == FALSE)
    {
      vPrintf("%sPassage en usage : configuration systeme\n", gch_spaces);
      AppData.rol = E_KS_ROL_TECHNICIEN;

      //au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_RECHERCHE_RESEAU;
      mef_clav = CLAV_PgmNetMontrerClavier();
    }
    else
    {
      vPrintf("%sPassage en usage : utilisation courante\n", gch_spaces);
      AppData.rol = E_KS_ROL_UTILISATEUR;

      au8Led_clav[C_CLAV_LED_INFO_1].mode = mNetOkTypeFlash;
      AppData.pgl = E_PGL_BOUCLE_PRINCIPALE;
      mef_clav = CLAV_PgmNetRetirerClavier();
    }
    usr_or_tec = !usr_or_tec;
  }
  else
  {
    vPrintf("%sUltra mode On\n", gch_spaces);

    mef_clav = E_KS_STP_ULTRA_MODE;
    // Memorisation de l'etat avant traitement de la touche
    AppData.stp = E_KS_STP_ULTRA_MODE;

    *care = TRUE;
    au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_RESET_POSSIBLE;
    au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_OFF;
    au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_OFF;
  }

#if !NO_DEBUG_ON
  PBAR_DbgInside(stepper, gch_spaces, E_FN_OUT, AppData);
  PBAR_DbgTrace(E_FN_OUT, "clav_BtnPgmL1", (void *) (AppData.pgl),
      E_DBG_TYPE_NET_STATE);
#endif

  return (mef_clav);
}
PRIVATE etRunningStp clav_BtnPgmL2(etRunningStp mef_clav, uint8 *care)
{
  etRunningStp mef_ici = mef_clav;
#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "clav_BtnPgmL2", (void *) (AppData.pgl),
      E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  if (timer_touche[AppData.key - 1] <= C_PRESSION_T5)
  {
    vPrintf("%sUltra mode Off\n", gch_spaces);
    *care = FALSE;
    au8Led_clav[C_CLAV_LED_INFO_1].mode = mNetOkTypeFlash;
    au8Led_clav[C_CLAV_LED_INFO_2].mode = E_FLASH_OFF;
    au8Led_clav[C_CLAV_LED_INFO_3].mode = E_FLASH_OFF;
    mef_ici = E_KS_STP_NON_DEFINI;
    AppData.stp = E_KS_STP_NON_DEFINI;
  }

#if !NO_DEBUG_ON
  PBAR_DbgInside(stepper, gch_spaces, E_FN_OUT, AppData);
  PBAR_DbgTrace(E_FN_OUT, "clav_BtnPgmL2", (void *) (AppData.pgl),
      E_DBG_TYPE_NET_STATE);
#endif
  return (mef_ici);
}

#if 0
PRIVATE etRunningStp clav_BtnPgmL2(etRunningStp mef_clav, uint8 *care)
{
  static bool_t bReqEraseFlash = FALSE;

  if(timer_touche[AppData.key-1] <= C_PRESSION_T3)
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

#if 0
PRIVATE void clav_EraseOrReset(etInUsingkey keys)
{
#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "clav_EraseOrReset", (void *) (AppData.pgl),
      E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif
  if (keys == E_KEY_NUM_ETOILE)
  {
    if (timer_touche[AppData.key - 1] >= C_PRESSION_T3)
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
  PBAR_DbgTrace(E_FN_OUT, "clav_EraseOrReset", (void *) (AppData.pgl),
      E_DBG_TYPE_NET_STATE);
#endif

}
#endif

PRIVATE etRunningStp pMenuLed(etInUsingkey touche)
{
  bool_t bExecMenu = FALSE;
  uint8 msg = 0;

  switch (touche)
  {
    case E_KEY_NUM_1:
    {
      msg = 1;
      bExecMenu = TRUE;
      mNetOkTypeFlash = E_FLASH_RESEAU_ACTIF;
      bufEmission[0] = E_MSG_NET_LED_ON;
    }
    break;

    case E_KEY_NUM_3:
    {
      msg = 2;
      bExecMenu = TRUE;
      mNetOkTypeFlash = E_FLASH_OFF;
      bufEmission[0] = E_MSG_NET_LED_OFF;
    }
    break;

    default:
    {
      vPrintf("pMenuLed touche(%d) sans effet\n", touche);
    }
  }

  if (bExecMenu == TRUE)
  {
    vPrintf("Broadcast Net \"%s\"\n", debug[msg]);

    au8Led_clav[C_CLAV_LED_INFO_1].mode = mNetOkTypeFlash;
    bufEmission[1] = 0;
    bufEmission[2] = 0;

    eJenie_SendData(0, bufEmission, 3,
    TXOPTION_SILENT | TXOPTION_BDCAST);
  }

  return E_KS_STP_ATTENTE_TOUCHE;
}

PRIVATE etRunningStp pMenuBip(etInUsingkey touche)
{
  bool_t bExecMenu = FALSE;
  uint8 msg = 0;

  switch (touche)
  {
    case E_KEY_NUM_1:
    {
      msg = 1;
      bExecMenu = TRUE;
      b_MasterBip = TRUE;
      b_activer_bip = TRUE;
      //vAHI_DioSetOutput(C_CLAV_BUZER, 0);
    }
    break;

    case E_KEY_NUM_3:
    {
      msg = 2;
      bExecMenu = TRUE;
      b_MasterBip = FALSE;
      //b_activer_bip = FALSE
      //vAHI_DioSetOutput(0, C_CLAV_BUZER);
    }
    break;

    default:
    {
      vPrintf("pMenuBip touche(%d) sans effet\n", touche);
    }
  }

  if (bExecMenu == TRUE)
  {
    vPrintf("BIP \"%s\"\n", debug[msg]);
  }

  return E_KS_STP_ATTENTE_TOUCHE;
}

PRIVATE etRunningStp pMenuCnf(etInUsingkey touche)
{
  switch (touche)
  {
    case E_KEY_NUM_1:
    {

    }
    break;

    case E_KEY_NUM_3:
    {

    }
    break;

    default:
    {
      vPrintf("pMenuCnf touche(%d) sans effet\n", touche);
    }
  }

  return E_KS_STP_ATTENTE_TOUCHE;
}

PRIVATE etRunningStp pMenuSav(etInUsingkey touche)
{
  switch (touche)
  {
    case E_KEY_NUM_1:
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
    break;

    case E_KEY_NUM_3:
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

    }
    break;

    default:
    {
      vPrintf("pMenuSav touche(%d) sans effet\n", touche);
    }
  }

  return E_KS_STP_ATTENTE_TOUCHE;
}

