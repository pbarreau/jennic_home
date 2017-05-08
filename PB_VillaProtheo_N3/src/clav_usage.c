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

PUBLIC char const *menu_pgm[] = { "Net Led", "Bip clavier", "Liaison", "Admin" };
PRIVATE etFlashMsg MenuFlashValue[] =
    { E_FLASH_MENU_LED_NET, E_FLASH_MENU_BIP_CLAVIER, E_FLASH_MENU_LIAISON,
        E_FLASH_ERASE_RESET_POSSIBLE };

PRIVATE uint8 idMenu = 0;

PUBLIC uint8 bufEmission[3] = { 0, 0, 0 };
PUBLIC bool_t b_EteindreNet = FALSE;

PRIVATE etRunningStp NEW_CLAV_GererToucheModeSuperUser(etInUsingkey laTouche);
PRIVATE etRunningStp NEW_CLAV_GererToucheModeSimpleUser(etInUsingkey laTouche);
PRIVATE etRunningStp NEW_CLAV_ChoisirNouveauRole(etInUsingkey laTouche);
PRIVATE void NEW_ManageNetworkLed(etInUsingkey laTouche);
PRIVATE etRunningStp NEW_CLAV_MenuSuperUSer(etFlashMsg flashingType,
    etInUsingkey laTouche);

PRIVATE etRunningKbd tabVisibleMode[] = { E_KS_KBD_VIRTUAL_1,
    E_KS_KBD_VIRTUAL_2, E_KS_KBD_VIRTUAL_3, E_KS_KBD_VIRTUAL_4 };
PRIVATE etInUsingkey tabModeKeys[] = { E_KEY_NUM_MOD_1, E_KEY_NUM_MOD_2,
    E_KEY_NUM_MOD_3, E_KEY_NUM_MOD_4 };

PUBLIC etRunningStp CLAV_GererTouche(etInUsingkey keys)
{
  etRunningStp stpVal = AppData.stp;
  etRunningRol rolVal = AppData.rol;

  vPrintf("CLAV_GererTouche\n");
  MyStepDebug();

  // Cette touche est dans quel contexte ?
  switch (rolVal)
  {
    case E_KS_ROL_TECHNICIEN:
    {
      // Remettre au menu de depart
      stpVal = NEW_CLAV_GererToucheModeSuperUser(keys);

      vPrintf("Teck apres\n");
      MyStepDebug();

    }
    break;

    case E_KS_ROL_UTILISATEUR:
    {
      stpVal = NEW_CLAV_GererToucheModeSimpleUser(keys);
      vPrintf("usr apres\n");
      MyStepDebug();
    }
    break;

    case E_KS_ROL_CHOISIR:
    {
      stpVal = NEW_CLAV_ChoisirNouveauRole(keys);
      vPrintf("Choix apres\n");
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

PRIVATE etRunningStp NEW_CLAV_ChoisirNouveauRole(etInUsingkey laTouche)
{
  etRunningRol cur_role = AppData.rol;

  vPrintf("Usage actuel:%s\n", dbg_etCLAV_role[cur_role]);

  if (laTouche == E_KEY_NUM_0)
  {
    cur_role = E_KS_ROL_UTILISATEUR;
    AppData.net = E_KS_NET_NON_DEFINI;
    AppData.eWifi = E_MSG_NOT_SET;
    au8Led_clav[C_CLAV_LED_INFO_1].mode = mNetOkTypeFlash;

    // Retirer clavier service
    CLAV_PgmNetRetirerClavier();
  }
  else if (laTouche == E_KEY_NUM_1)
  {
    cur_role = E_KS_ROL_TECHNICIEN;
    idMenu = 0;
    au8Led_clav[C_CLAV_LED_INFO_1].mode = E_FLASH_EN_ATTENTE_TOUCHE_BC;
  }

  if (cur_role != E_KS_ROL_CHOISIR)
  {
    AppData.rol = cur_role;
    vPrintf("Usage nouveau:%s\n", dbg_etCLAV_role[cur_role]);
    CLAV_GererMode(E_KEY_NUM_MOD_1); // Bug
  }

  return E_KS_STP_ATTENTE_TOUCHE;
}

PRIVATE etRunningStp NEW_CLAV_GererToucheModeSuperUser(etInUsingkey laTouche)
{
  etRunningStp mef_clav = AppData.stp;
  static etFlashMsg flashingType = E_FLASH_MENU_LED_NET;

  uint8 val_flash = (uint8) MenuFlashValue[idMenu];
  int len = (sizeof(menu_pgm) / sizeof(char **));
  static uint8 idMode = 1;

  if ((laTouche == E_KEY_NUM_DIESE))
  {
    if (timer_touche[E_KEY_NUM_DIESE - 1] <= C_PRESSION_T1)

    {
      if (AppData.net == E_KS_NET_WAIT_CLIENT)
      {
        vPrintf("Test Choix clavier virtuel\n");
        CLAV_GererMode(tabModeKeys[idMode]);
        idMode++;
        idMode = idMode % (sizeof(tabVisibleMode) / sizeof(etRunningKbd));
      }
      else
      {
        vPrintf("Menu '%s' flash:'%x'\n", menu_pgm[idMenu], val_flash);
        au8Led_clav[C_CLAV_LED_INFO_2].mode = val_flash;
        au8Led_clav[C_CLAV_LED_INFO_3].mode = ~E_FLASH_OFF;
        flashingType = MenuFlashValue[idMenu];
        idMenu++;
        idMenu = idMenu % len;
      }
    }

    if (timer_touche[E_KEY_NUM_DIESE - 1] > C_PRESSION_T1)
    {
      AppData.rol = E_KS_ROL_CHOISIR;
      CLAV_GererMode(E_KEY_NUM_MOD_5);
    }

  }

  switch (AppData.net)
  {
    case E_KS_NET_NON_DEFINI:
    {
      mef_clav = NEW_CLAV_MenuSuperUSer(flashingType, laTouche);
    }
    break;

    case E_KS_NET_CLIENT_IN:
    {
      vPrintf("cas 2\n");
      mef_clav = CLAV_PgmActionTouche(laTouche);
    }
    break;

    case E_KS_NET_WAIT_CLIENT:
    {
      vPrintf("cas 3\n");
      mef_clav = CLAV_PgmActionTouche(laTouche);
    }
    break;

    case E_KS_NET_CONF_END:
    {
      vPrintf("cas 4\n");
    }
    break;

    case E_KS_NET_CONF_EN_COURS:
    {
      vPrintf("cas 5\n");
      mef_clav = CLAV_PgmActionTouche(laTouche);
    }
    break;

    default:
    {
      vPrintf("Erreur sur net state:%d\n", AppData.net);
    }
  }

  return mef_clav;
}

PRIVATE etRunningStp NEW_CLAV_MenuSuperUSer(etFlashMsg flashingType,
    etInUsingkey laTouche)
{
  etRunningStp mef_clav = AppData.stp;

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
        AppData.net = E_KS_NET_CONF_START;
        mef_clav = CLAV_PgmNetMontrerClavier();
      }
    }
    break;

    case E_FLASH_ERASE_RESET_POSSIBLE:
    {
      if (laTouche == E_KEY_NUM_0)
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

      if (laTouche == E_KEY_NUM_1)
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

        // Se mettre comme a un nouveau depart
        AppData.net = E_KS_NET_NON_DEFINI;
        AppData.eWifi = E_MSG_NOT_SET;
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
  static uint8 idMode = 1; // enrelation avec le mode affiche par les leds

  if (laTouche == E_KEY_NUM_DIESE)
  {
    // Pression courte
    if (timer_touche[E_KEY_NUM_DIESE - 1] <= C_PRESSION_T1)
    {
      vPrintf("Clavier virtuel '%d'\n", tabVisibleMode[idMode]);
      CLAV_GererMode(tabModeKeys[idMode]);
      idMode++;
      idMode = idMode % (sizeof(tabVisibleMode) / sizeof(etRunningKbd));
    }

    //Pression plus longue
    if (timer_touche[E_KEY_NUM_DIESE - 1] > C_PRESSION_T1)
    {
      vPrintf("Selection role clavier\n", tabVisibleMode[idMode]);
      AppData.rol = E_KS_ROL_CHOISIR;
      CLAV_GererMode(E_KEY_NUM_MOD_5);
    }
  }
  else
  {
    CLAV_UsrActionTouche(laTouche);
  }
  return E_KS_STP_ATTENTE_TOUCHE;
}
#if 0
PUBLIC etRunningStp CLAV_BoutonDeConfiguration(bool_t * bip_on)
{
  etRunningStp mef_clav = E_KS_STP_NON_DEFINI;
  return mef_clav;
}
#endif

PUBLIC void CLAV_NetMsgInput(tsData *psData)
{
  etRunningStp mef_clav = AppData.stp;
  etRunningNet mef_net = E_KS_NET_NON_DEFINI;

#if !NO_DEBUG_ON
  int stepper = 0;

  stepper = PBAR_DbgTrace(E_FN_IN, "CLAV_NetMsgInput", (void *) (AppData.pgl),
      E_DBG_TYPE_NET_STATE);
  PBAR_DbgInside(stepper, gch_spaces, E_FN_IN, AppData);
#endif

  if (AppData.rol == E_KS_ROL_TECHNICIEN)
  {
    mef_net = CLAV_PgmNetMsgInput(psData);
  }
  else if (AppData.rol == E_KS_ROL_UTILISATEUR)
  {
    mef_net = CLAV_UsrNetMsgInput(psData);
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

  AppData.net = mef_net;
  maConfig.net = mef_net;
  AppData.stp = mef_clav;
}

PUBLIC etRunningStp pFn1_1(stParam *param)
{
  etRunningStp retVal = E_KS_STP_ATTENTE_TOUCHE;
  bool_t onShot = TRUE;

  if (onShot)
  {
    onShot = FALSE;
    vPrintf("pFn1_1\n");
  }

  return retVal; //retVal;
}
PUBLIC etRunningStp pFn1_2(stParam *param)
{
  etRunningStp retVal = E_KS_STP_ATTENTE_TOUCHE;
  bool_t onShot = TRUE;

  if (onShot)
  {
    onShot = FALSE;
    vPrintf("pFn1_2\n");
  }

  return retVal; //retVal;
}
PUBLIC etRunningStp pFn1_3(stParam *param)
{
  etRunningStp retVal = E_KS_STP_ATTENTE_TOUCHE;
  bool_t onShot = TRUE;

  if (onShot)
  {
    onShot = FALSE;
    vPrintf("pFn1_3\n");
    //param->netVal = E_KS_NET_CLIENT_IN;
  }

  return retVal; //retVal;
}

PUBLIC etRunningStp pFn2_1(stParam *param)
{
  etRunningStp retVal = E_KS_STP_ATTENTE_TOUCHE;
  bool_t onShot = TRUE;

  if (onShot)
  {
    onShot = FALSE;
    vPrintf("pFn2_1\n");
  }

  return retVal; //retVal;
}
PUBLIC etRunningStp pFn2_2(stParam *param)
{
  etRunningStp retVal = E_KS_STP_ATTENTE_TOUCHE;
  bool_t onShot = TRUE;

  if (onShot)
  {
    onShot = FALSE;
    vPrintf("pFn2_2\n");
  }

  return retVal; //retVal;
}
PUBLIC etRunningStp pFn2_3(stParam *param)
{
  etRunningStp retVal = E_KS_STP_ATTENTE_TOUCHE;
  bool_t onShot = TRUE;

  if (onShot)
  {
    onShot = FALSE;
    vPrintf("pFn2_3\n");
  }

  return retVal; //retVal;
}
