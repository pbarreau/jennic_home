/*
 * usr_clav.c
 *
 *  Created on: 17 juil. 2013
 *      Author: RapidOs
 */

#include <jendefs.h>

#if !NO_DEBUG_ON
#include <Printf.h>
#endif

#include "e_config.h"

PUBLIC bool_t bStartTimerIo_19 = FALSE;
PUBLIC bool_t OneIt20 = FALSE;

PUBLIC uint16 TimingIo_19 = 0;


PUBLIC teClavState CLAV_UsrNetMsgInput(tsData *psData)
{
  teClavState mef_clav = E_KS_NON_DEFINI;
  PBAR_TypeMsg le_message = E_MSG_NOT_SET;
  vPrintf("Message Reseau a destination de clavier recut\n");

  if (psData->u16Length == 3)
  {
    le_message = (PBAR_TypeMsg) (psData->pau8Data[psData->u16Length - 3]);

    switch (le_message)
    {
      case E_MSG_NET_LED_OFF:
      {
        vPrintf("Demande reseau de couper led Net\n");
        mNetOkTypeFlash = E_FLASH_OFF;
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
        vPrintf("Msg reseau entrant en mode normal non prevu : %d\n",
            gch_spaces);
      }
      break;
    }
  }

  return mef_clav;
}

PUBLIC teClavState CLAV_UsrActionTouche(etCLAV_keys keys)
{
  static bool_t SetAllOff = FALSE;
  teClavState mef_clav = E_KS_NON_DEFINI;

  etCLAV_mod eKeyMode = AppData.eClavmod;
  uint8 key_mode = eKeyMode - E_KM_1;
  uint8 key_code = (keys == E_KEY_NUM_ETOILE ? C_KEY_MEM_ALL : keys - E_KEY_NUM_1); /// a corriger

  uint8 box = 0;
  uint8 useBox = 0;

  vPrintf("\nIN:CLAV_UsrActionTouche MEF:%s\n", dbg_teClavState[mef_clav]);
  vPrintf(" Traiter touche:%s, mode:%s\n", dbg_etCLAV_keys[keys],
      dbg_etCLAV_mod[eKeyMode]);

  // A ton une demande valide
  if ((eKeyMode > E_KM_NON_DEFINI) && (eKeyMode < E_KM_END)
      && (keys < E_KEYS_NUM_END))
  {
    // sur clavier 4x4 regarder 1-9,0 et *
    // Regarder le tableau ptr_destination
    // Max boucle == C_MAX_BOXES
    for (box = 0; box < C_MAX_BOXES + 1; box++)
    {
      useBox = eeprom.netConf.boxList[key_mode][key_code][box];
      vPrintf(" keymode=%d,keycode=%d,boxuse=%d,v=%d\n", key_mode, key_code,
          box, useBox);
      if (eeprom.netConf.boxList[key_mode][key_code][box] == 0x00)
      {
        //suivant de box = box
        // sortir de la boucle
        vPrintf("  Arret recherche id box niveau:%d\n\n", box);
        break;
      }
      else
      {
        // il y a une boite configuree
        vPrintf("  Use box %d (m:%s, k:%s, ptr:%d)\n", useBox,
            dbg_etCLAV_mod[eKeyMode], dbg_etCLAV_keys[keys], box);

        vPrintf("  bit:%x\n",
            eeprom.netConf.boxData[key_mode][key_code][useBox]);

        if (eeprom.netConf.boxData[key_mode][key_code][useBox])
        {
          vPrintf("  Io [%x] avec boite %d\n",
              eeprom.netConf.boxData[key_mode][key_code][useBox], useBox);
          // Oui alors a t on en memoire l'@ de cette boite
          if (eeprom.BoxAddr[useBox])
          {
            // Oui on a une addresse connue
            AppData.u64ServiceAddress = eeprom.BoxAddr[useBox];

            // Le temps d'appui sur la touche determine le on ou le off
            // ON (court), OFF (Long)
            if (keys != E_KEY_NUM_ETOILE)
            {
              //Demande sous forme de bascule
              bufEmission[0] = E_MSG_DATA_SELECT;

              if (timer_touche[AppData.ukey] <= C_PRESSION_T1)
              {
                bufEmission[2] = 0xFF;
              }
              else
              {
                bufEmission[2] = 0x00;
              }
            }
            else
            {
              // Demande Globale imposee
              bufEmission[0] = E_MSG_DATA_ALL;

              if (SetAllOff == TRUE)
              {
                // On doit tout eteindre
                bufEmission[2] = 0x00;
              }
              else
              {
                // On doit tout allumer
                bufEmission[2] = 0xFF;
              }
            }
#if 0
            // envoyer le message a cette boite
            if(keys != E_KEY_NUM_ETOILE)
            {
              //Demande sous forme de bascule
              bufEmission[0]=E_MSG_DATA_SELECT;
              bufEmission[2]=0x00;
            }
            else
            {
              // Demande Globale imposee
              bufEmission[0]=E_MSG_DATA_ALL;

              if(SetAllOff == TRUE)
              {
                // On doit tout eteindre
                bufEmission[2]=0x00;
              }
              else
              {
                // On doit tout allumer
                bufEmission[2]=0xFF;
              }
            }
#endif
            bufEmission[1] = eeprom.netConf.boxData[key_mode][key_code][useBox];

            vPrintf("  --> MSG (%x,%x,%x) a [%x:%x]\n", bufEmission[0],
                bufEmission[1], bufEmission[2],
                (uint32) (AppData.u64ServiceAddress >> 32),
                (uint32) (AppData.u64ServiceAddress & 0xFFFFFFFF));

            eJenie_SendData(AppData.u64ServiceAddress, bufEmission, 3,
            TXOPTION_SILENT);
          }
          else
          {
            vPrintf("Erreur dans la memorisation des @ boites\n");
          }

        }
      }
    }
  }

  if (keys == E_KEY_NUM_ETOILE)
  {
    SetAllOff = !SetAllOff; // Global
  }

  mef_clav = E_KS_ATTENTE_TOUCHE;
  return mef_clav;
}

#if 0
PRIVATE keyball(void)
{

  // rajout de cette boite a la liste de celle du clavier
  vPrintf("Une B:%d de + a  key:%d ?\n",IncomingBoxId,eKeyTouched);
  VerifierExistanceBoite(eModeOnConf,eKeyTouched,IncomingBoxId);

  // on va parcourir toute les boites de cette touche
  // et creer une commande ALL
  vPrintf("\nCreation cmd pour kbd ALL\n");
  // Parcourir toute les touche de la boite en config
  valThisBox = 0;
  for(i=0;i<C_MAX_KEYS;i++)
  {
    // recuperer la config du clavier et la stocker
    // dans la touche ALL
    valThisBox |= eeprom.netConf.boxData[eModeOnConf-E_KM_1][i][IncomingBoxId];
    vPrintf(" box:%d key:%d, val:%x\n",useBox,i,valThisBox);
  }

  // sauvegarder cette valeur dans la touche ALL de la boite en cours
  // On sauve la touche all
  eeprom.netConf.boxData[eModeOnConf-E_KM_1][C_MAX_KEYS][IncomingBoxId] = valThisBox;

  // Dire que Touche ALL position ptr++ a une Boite configuree
  VerifierExistanceBoite(eModeOnConf,E_KPD_ALL,IncomingBoxId);

  // sauvegarder la config dans la flash !!!
#if 0
  bAHI_FlashEraseSector(FLASH_SECTOR);
  bAHI_FullFlashProgram(FLASH_START, sizeof(bpsFlash), (uint8 *) &eeprom);

  // Test LEcture
  bAHI_FullFlashRead(FLASH_START, sizeof(bpsFlash), (uint8 *) &eeprom);
  vPrintf("Relecture boite=%d\n",eeprom.nbBoite);
#endif

  if(!bAHI_FlashEraseSector(FLASH_SECTOR))
  {
    vPrintf("Pb lors effacement Sector 3\n");
  }
  else
  {
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
  // montrer mode pgm next box ..?
  au8Led[0].mode=E_FLASH_EN_ATTENTE_TOUCHE_BC;

  bConfStart = FALSE;
  sAppData.eAppState = APP_STATE_ATTENDRE_BOITES;
}
#endif
