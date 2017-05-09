/*
 * 9555.c
 *
 *  Created on: 27 juin 2013
 *      Author: RapidOs
 */

#include <jendefs.h>
#include <JPI.h>
#include <AppHardwareApi.h>

#include <Printf.h>

#include "c_config.h"
#include "i2c_9555.h"
#include "bit.h"
#include "led.h"

PUBLIC uint16 prvCnf_O_9555 = 0xFFFF; /// Configuration des sorties (ports 0) des 9555
PUBLIC uint16 prvCnf_I_9555 = 0xFFFF; /// Configuration des entrees (ports 1) des 9555
PUBLIC bool_t bIt_DIO11 = FALSE;	/// Flag declanchement d'une It sur DIO11

PUBLIC bool_t bIt_DIO12 = FALSE; 	/// Flag declanchement d'une It sur DIO12
PUBLIC bool_t bMessureDureePressionDio12 = FALSE; /// Mesure temps d'appui sur bouton
PUBLIC uint16 timer_pression_DIO12 = 0;

PRIVATE uint8 i2cAddr[] = { ADDR_IO_1, ADDR_IO_1, ADDR_IO_2, ADDR_IO_2 };

PRIVATE void u8_I2CWrite_9555(uint8 u8SlaveAddr, uint8 u8RegAddr, uint8 u8Value);
PRIVATE uint16 u16_I2CRead_9555(uint8 u8SlaveAddr);
PRIVATE void vPRT_It_9555(uint32 u32Device, uint32 u32ItemBitmap);

PRIVATE void vPRT_PrepareJennic(sBusSpeed speed);
PRIVATE void vPRT_PrepareSlio(void);

PRIVATE uint32 vPRT_GererDios(uint32 cnf_a, uint32 cnf_b, uint32 cnf_ref,
    uint8 *ptr_reg);

PUBLIC void vPRT_Init_IosOfCard(sBusSpeed speed)
{
  vPRT_PrepareJennic(speed);
  vPRT_PrepareSlio();
}

PRIVATE void vPRT_PrepareJennic(sBusSpeed speed)
{
  // Pio en entree
  vAHI_DioSetDirection(PBAR_CFG_INPUT, 0);

  // pull up activee sur les entrees
  vAHI_DioSetPullup(PBAR_CFG_INPUT, 0);

  // Pio en sortie
  vAHI_DioSetDirection(0, PBAR_CFG_OUTPUT);

#ifdef CARD_USE_LED_PGM
  vAHI_DioSetOutput(C_LPIO_2, 0); //Eteindre la led mode pgm
  au8Led[C_LID_2].actif = TRUE;
  au8Led[C_LID_2].pio = C_LPID_2;
  au8Led[C_LID_2].mode = E_LED_OFF;
#endif

}

PRIVATE void vPRT_PrepareSlio(void)
{
  // Mettre a 0 SIG_LE 573 pour charger bus
  vAHI_DioSetOutput(0, C_SEL_573);
  // Mettre les sorties a 0
  vAHI_DioSetOutput(0xFF << PBAR_DEBUT_IO, 0);
  // Mettre a 1 SIG_LE 573 pour maintenir bus
  vAHI_DioSetOutput(C_SEL_573, 0);
}

/**
 *
 */
PUBLIC void vPRT_DioSetDirection(uint32 cnf_in, uint32 cnf_out)
{
  uint8 i2cReg[] = { CMD_CFG_0, CMD_CFG_1 };
  static uint32 cnf_ref = 0xFFFFFFFF; /// Par defaut io definies en entrees

  vPrintf("Config direction Ios\n");
  cnf_ref = vPRT_GererDios(cnf_in, cnf_out, cnf_ref, i2cReg);
}

PUBLIC void vPRT_DioSetOutput(uint32 cnf_on, uint32 cnf_off)
{
  vPrintf("Affecter valeur output\n");
  // Mettre a 1 SIG_LE 573 pour charger bus
  vAHI_DioSetOutput(C_SEL_573, 0);

  cnf_on = ~cnf_on;
  cnf_off = ~cnf_off;

  // Configuer les sorties
  vAHI_DioSetOutput(cnf_on, cnf_off);

  // Mettre a 0 SIG_LE 573 pour maintenir bus
  vAHI_DioSetOutput(0, C_SEL_573);
}

PRIVATE uint32 vPRT_GererDios(uint32 cnf_a, uint32 cnf_b, uint32 cnf_ref,
    uint8 *ptr_reg)
{
  uint32 cnf_new = 0; /// Valeur a envoyer sur la carte
  uint32 cnf_tmp = 0;
  uint8 resu = 0;
  int i = 0;

  vPrintf(" Request -> cnf_a:%x, cnf_b:%x, cnf_ref:%x\n", (uint32) cnf_a,
      (uint32) cnf_b, (uint32) cnf_ref);

  // Verifier si un bit dans on et dans off
  for (i = 0; i <= 31; i++)
  {
    if (IsBitSet(cnf_a, i))
    {
      // Verifier si bit A 1 dans In
      if (IsBitSet(cnf_b, i))
      {
        vPrintf("  !!! bit:%d identique !!!\n", i);
        // le bit est defini dans in et out => je force a off
        BitNclr(cnf_a, i);
      }
    }
  }

  // Je memorise mes entrees
  cnf_tmp = cnf_a | cnf_ref;
  vPrintf(" cnf_tmp:%x\n", (uint32) cnf_tmp);

  // Je memorise mes sorties*
  cnf_new = ~cnf_b & cnf_tmp;
  vPrintf(" Msg:%x", (uint32) cnf_new);

  // Envoyer aux composant i2c 9555
  for (i = 0; i < 4; i++)
  {
    resu = (uint8) (cnf_new >> (i * 8));
    if ((i % 2) == 0)
      vPrintf("\n\n");

    vPrintf(" I2c:%x, reg:%d, val:%x", i2cAddr[i], ptr_reg[i % 2], resu);

    if (resu == (uint8) (cnf_ref >> (i * 8)))
    {
      vPrintf("\n   Config actuelle idem precedent => Noi2cWrite !!\n");
    }
    else
    {
      // Port 0 b[0..7], Port 1 b[8..15]
      // Ecriture dans le composant de la config
      u8_I2CWrite_9555(i2cAddr[i], ptr_reg[i % 2], resu);
    }
  }

  // Memoriser les valeurs
  return (cnf_new);
}

PUBLIC uint32 vPRT_DioReadInput(void)
{
  uint8 i2c_device = 0;
  uint16 tmp = 0;
  uint32 val = 0UL;

  vPrintf("\n\nDemande de lecture des SLIOS\n");
  val = u32AHI_DioReadInput();
  //Prendre les bits du composant
  val = val & 0x00004F80;
  //val  = val >> 3;

  vPrintf("Fin lecture des SLIOS val=%x\n\n", val);
  return val;
}

///http://subversion.assembla.com/svn/Langaton/Acceleration/Source/EndDevice.c
///http://www.automatepc.fr/?page=I2cPcf8574
PRIVATE void u8_I2CWrite_9555(uint8 u8SlaveAddr, uint8 u8RegAddr, uint8 u8Value)
{
}

PRIVATE uint16 u16_I2CRead_9555(uint8 u8SlaveAddr)
{
  uint16 tmp = 0;
  return tmp;
}

PRIVATE void vPRT_It_9555(uint32 u32Device, uint32 u32ItemBitmap)
{
}
