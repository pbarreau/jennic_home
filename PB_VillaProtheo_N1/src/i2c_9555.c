/*
 * 9555.c
 *
 *  Created on: 27 juin 2013
 *      Author: RapidOs
 */

#include <jendefs.h>
#include <AppHardwareApi.h>

#include <Printf.h>

#include "config.h"
#include "i2c_9555.h"
#include "bit.h"

PUBLIC uint16 prevConfInputs = 0xFFFF;
PUBLIC uint16 prevConfOutputs = 0xFFFF;
PUBLIC bool_t bUneIt = FALSE;

PRIVATE uint8 i2cAddr[] = {ADDR_IO_1,ADDR_IO_1,ADDR_IO_2,ADDR_IO_2};

PRIVATE void u8_I2CWrite_9555(uint8 u8SlaveAddr,uint8 u8RegAddr, uint8 u8Value);
PRIVATE uint16 u16_I2CRead_9555(uint8 u8SlaveAddr);
PRIVATE void vPRT_It_9555 (uint32 u32Device,uint32 u32ItemBitmap);

PRIVATE void vPRT_PrepareJennic(sBusSpeed speed);
PRIVATE void vPRT_PrepareSlio(void);

#if !NO_DEBUG_ON

#if SHOW_TEST_1
PUBLIC bool_t PCA_9555_test_01 (void)
{
	bool_t mes_tests = FALSE;

	vPrintf("\nTest1:(0x0000,0x0000)\n");
	vPRT_Dio16SetDirection(0x0000,0x0000);

	vPrintf("\nTest2:(0x0001,0x0001)\n");
	vPRT_Dio16SetDirection(0x0001,0x0001);

	vPrintf("\nTest3:(0x0000,0x0001)\n");
	vPRT_Dio16SetDirection(0x0000,0x0001);

	vPrintf("\nTest4:(0x0001,0x0000)\n");
	vPRT_Dio16SetDirection(0x0001,0x0000);

	vPrintf("\nTest5:(0x0080,0x0080)\n");
	vPRT_Dio16SetDirection(0x0080,0x0080);

	vPrintf("\nTest6:(0x0080,0x0000)\n");
	vPRT_Dio16SetDirection(0x0080,0x0000);

	vPrintf("\nTest7:(0x0000,0x0080)\n");
	vPRT_Dio16SetDirection(0x0000,0x0080);

	vPrintf("\nTest8:(0x0100,0x0100)\n");
	vPRT_Dio16SetDirection(0x0100,0x0100);

	vPrintf("\nTest9:(0x0000,0x0100)\n");
	vPRT_Dio16SetDirection(0x0000,0x0100);

	vPrintf("\nTestA:(0x0100,0x0000)\n");
	vPRT_Dio16SetDirection(0x0100,0x0000);

	vPrintf("\nTestB:(0x8000,0x8000)\n");
	vPRT_Dio16SetDirection(0x8000,0x8000);

	vPrintf("\nTestC:(0x0000,0x8000)\n");
	vPRT_Dio16SetDirection(0x0000,0x8000);

	vPrintf("\nTestD:(0x8000,0x0000)\n");
	vPRT_Dio16SetDirection(0x8000,0x0000);


	return mes_tests;
}
#endif

#if SHOW_TEST_2
PUBLIC bool_t PCA_9555_test_02 (void)
{

	vPrintf("\nTest2-1:(0x00000000,bit31|bit0)\n");
	vPRT_DioSetDirection(0x00000000,bit31|bit0);

	vPrintf("\nTest2-2:(0x00000000,bit16|bit7)\n");
	vPRT_DioSetDirection(0x00000000,bit16|bit7);

	return TRUE;
}
#endif // SHOW_TEST_2

#endif // !NO_DEBUG_ON

#if SHOW_TEST_3
PUBLIC bool_t PCA_9555_test_03 (void)
{

	vPRT_DioSetDirection(0x00000000,bit7|bit0);

	vPRT_DioSetOutput(0x00000000,bit7|bit0);

	return TRUE;
}
#endif //SHOW_TEST_3

#if SHOW_TEST_1
PUBLIC void vPRT_Dio16SetDirection(uint16 cnf_in, uint16 cnf_out)
{
	PRIVATE uint8 i2cReg[] = {CMD_CFG_0,CMD_CFG_1};

	static uint16 cnf_ref = 0xFFFF;
	uint8 bit = 0;
	uint8 octet = 0;
	uint8 do_in = 0;
	uint8 do_out = 0;
	uint8 resu = 0;

	for(octet = 0; octet <2; octet++)
	{
		resu = (cnf_ref>>(octet*8)) & 0xFF;
		vPrintf("Octet n°:%d->%x\n",octet,resu);

		// On regarde chaque octet des bitmaps
		do_in = (cnf_in>>(octet*8)) & 0xFF;
		do_out = (cnf_out>>(octet*8)) & 0xFF;

		vPrintf("I:%x, O:%x\n",do_in,do_out);

		// analyse de la demande
		if(do_in && !do_out)
		{
			vPrintf("Config In seule\n");
			for(bit=0;bit<8;bit++)
			{
				// test du bit config input
				if(IsBitSet(do_in,bit))
				{
					BitNset(resu,bit);
				}
			}

		}
		else if(do_out && !do_in)
		{
			vPrintf("Config Out seule\n");
			for(bit=0;bit<8;bit++)
			{
				// test du bit config output
				if(IsBitSet(do_out,bit))
				{
					BitNclr(resu,bit);
				}
			}
		}
		else if(do_out && do_in)
		{
			vPrintf("In et Out set\n");
			for(bit=0;bit<8;bit++)
			{
				// test du bit config output
				if(IsBitSet(do_out,bit))
				{
					// test du bit config input
					if(IsBitSet(do_in,bit))
					{
						vPrintf("bit:b%d en entree\n", bit);
						// Mettre en entree (par defaut)
						BitNset(resu,bit);
					}
					else
					{
						vPrintf("bit:b%d en sortie\n", bit);
						// Config ok c'est une sortie
						BitNclr(resu,bit);
					}
				}
			}

		}
		else if(!do_out && !do_in)
		{
			vPrintf("In = Out = 0\n");
		}

		// Ecrire config
		if(resu != (uint8)(cnf_ref >> (octet*8)))
		{
			vPrintf("Resu_calc:%x\n", resu);
			// recup config actuelle
			cnf_ref = ( cnf_ref & (0xFFFF & ~(0xFF<<octet*8)))|(resu << octet*8);

			resu = cnf_ref >> (octet*8);
			vPrintf("Config a i2c:%x, reg:%d, val:%x\n", i2cAddr[octet],i2cReg[octet%2],resu);
			// Port 0 b[0..7], Port 1 b[8..15]
			// Ecriture dans le composant de la config
			u8_I2CWrite_9555(i2cAddr[octet],i2cReg[octet%2],resu);
		}
		else
		{
			vPrintf("Resu = cnf ref\n");
		}



	} // Octet suivant
}
#endif //SHOW_TEST_1

PUBLIC void vPRT_Init_9555(sBusSpeed speed)
{
	vPRT_PrepareJennic(speed);
	vPRT_PrepareSlio();
}

PRIVATE void vPRT_PrepareJennic(sBusSpeed speed)
{
	// Configuration du bus et activation du SI dans le jennic
	vAHI_SiConfigure(TRUE,FALSE,speed);   /* enabled, no interrupt, 400kHz */


	// Pio en entree
	vAHI_DioSetDirection(E_AHI_DIO11_INT|E_AHI_DIO12_INT, 0);

	// pull up activee sur les entrees
	vAHI_DioSetPullup(E_AHI_DIO11_INT|E_AHI_DIO12_INT, 0);

	// sens de la transition pour it (falling ie doc 9555)
	vAHI_DioInterruptEdge(0, E_AHI_DIO11_INT|E_AHI_DIO12_INT);

	// Enregistrer la call back de gestions des its
	vAHI_SysCtrlRegisterCallback(vPRT_It_9555);

	// Autoriser les Its
	vAHI_DioInterruptEnable(E_AHI_DIO11_INT|E_AHI_DIO12_INT, 0);

}

PRIVATE void vPRT_PrepareSlio(void)
{
	uint32 conf = 0;

	// Configuration en sortie
	vPRT_DioSetDirection(0x00000000,bit0|\
									bit1|\
									bit2|\
									bit3|\
									bit4|\
									bit5|\
									bit6|\
									bit7|\
									bit16|\
									bit17|\
									bit18|\
									bit19|\
									bit20|\
									bit21|\
									bit22|\
									bit23);

	// Test mise a 0v entraine led on
	vPRT_DioSetOutput(0x00000000,bit7|bit0);

	// Lecture de la config au demarrage input et ouput des slios
	conf = vPRT_DioReadInput();
	prevConfInputs = (conf>>16 & 0xFF00) | (((uint16)conf>>8) & 0x00FF);
	prevConfOutputs = (conf>>8 & 0xFF00) | (((uint16)conf) & 0x00FF);
}

/**
 *
 */
PUBLIC void vPRT_DioSetDirection(uint32 cnf_in, uint32 cnf_out)
{
	uint8 i2cReg[] = {CMD_CFG_0,CMD_CFG_1};

	static uint32 cnf_ref = 0xFFFFFFFF;
	uint8 bit = 0;
	uint8 octet = 0;
	uint8 do_in = 0;
	uint8 do_out = 0;
	uint8 resu = 0;

	vPrintf("\nProcedure config direction SLIO\n");
	vPrintf(" Ref CFG in = %x\n",cnf_ref);
	vPrintf(" Request -> In:%x, Out:%x\n",cnf_in,cnf_out);

	for(octet = 0; octet <4; octet++)
	{
		resu = (cnf_ref>>(octet*8)) & 0xFF;

		vPrintf("\n Ref octet[%d]=%x\n",octet,resu);
		// On regarde chaque octet des bitmaps
		do_in = (cnf_in>>(octet*8)) & 0xFF;
		do_out = (cnf_out>>(octet*8)) & 0xFF;

		vPrintf(" Demande octet[%d] -> Cfg_in:%x, Cfg_out:%x\n",octet, do_in,do_out);
		// analyse de la demande
		if(do_in && !do_out)
		{
			vPrintf("  Config In seul\n");
			for(bit=0;bit<8;bit++)
			{
				// test du bit config input
				if(IsBitSet(do_in,bit))
				{
					BitNset(resu,bit);
				}
			}

		}
		else if(do_out && !do_in)
		{
			vPrintf("  Config Out seul\n");
			for(bit=0;bit<8;bit++)
			{
				// test du bit config output
				if(IsBitSet(do_out,bit))
				{
					BitNclr(resu,bit);
				}
			}
		}
		else if(do_out && do_in)
		{
			vPrintf("  In et Out set\n");
			for(bit=0;bit<8;bit++)
			{
				// test du bit config output
				if(IsBitSet(do_out,bit))
				{
					// test du bit config input
					if(IsBitSet(do_in,bit))
					{
						vPrintf("   bit:b%d en entree\n", bit);
						// Mettre en entree (par defaut)
						BitNset(resu,bit);
					}
					else
					{
						vPrintf("   bit:b%d en sortie\n", bit);
						// Config ok c'est une sortie
						BitNclr(resu,bit);
					}
				}
			}

		}
		else if(!do_out && !do_in)
		{
			vPrintf("  In = Out = 0\n");
		}

		// Ecrire config
		if(resu != (uint8)(cnf_ref >> (octet*8)))
		{
			vPrintf("  Resu_calc:%x\n", resu);
			// recup config actuelle
			cnf_ref = ( cnf_ref & (0xFFFFFFFF & ~(0xFF<<octet*8)))|(resu << octet*8);
			vPrintf("  New CFG = %x\n",cnf_ref);

			resu = cnf_ref >> (octet*8);
			vPrintf("   Config a i2c:%x, reg:%d, val:%x\n", i2cAddr[octet],i2cReg[octet%2],resu);
			// Port 0 b[0..7], Port 1 b[8..15]
			// Ecriture dans le composant de la config
			u8_I2CWrite_9555(i2cAddr[octet],i2cReg[octet%2],resu);
		}
		else
		{
			vPrintf("  Resu = cnf ref\n");
		}



	} // Octet suivant

	vPrintf("Ref CFG out = %x\n\n",cnf_ref);
}


PUBLIC void vPRT_DioSetOutput(uint32 cnf_on, uint32 cnf_off)
{
	PRIVATE uint8 i2cReg[] = {CMD_OUT_0, CMD_OUT_1};

	static uint32 cnf_ref = 0xFFFFFFFF;
	uint8 bit = 0;
	uint8 octet = 0;
	uint8 do_on = 0;
	uint8 do_off = 0;
	uint8 resu = 0;

	vPrintf("\nProcedure eciture output val SLIO\n");
	vPrintf("Ref OUT in =%x\n",cnf_ref);
	vPrintf(" Request -> In:%x, Out:%x\n",cnf_on,cnf_off);

	for(octet = 0; octet <4; octet++)
	{
		resu = (cnf_ref>>(octet*8)) & 0xFF;

		vPrintf("\n Ref octet[%d]=%x\n",octet,resu);
		// On regarde chaque octet des bitmaps
		do_on = (cnf_on>>(octet*8)) & 0xFF;
		do_off = (cnf_off>>(octet*8)) & 0xFF;

		vPrintf( " Demande octet[%d] -> Cfg_on:%x, Cfg_off:%x\n",octet, do_on,do_off);
		// analyse de la demande
		if(do_on && !do_off)
		{
			vPrintf("  Config On seul\n");
			for(bit=0;bit<8;bit++)
			{
				// test du bit config on
				if(IsBitSet(do_on,bit))
				{
					BitNset(resu,bit);
				}
			}

		}
		else if(do_off && !do_on)
		{
			vPrintf("  Config Out seul\n");
			for(bit=0;bit<8;bit++)
			{
				// test du bit config output
				if(IsBitSet(do_off,bit))
				{
					BitNclr(resu,bit);
				}
			}
		}
		else if(do_off && do_on)
		{
			vPrintf("  On et Off set\n");
			for(bit=0;bit<8;bit++)
			{
				// test du bit config on
				if(IsBitSet(do_on,bit))
				{
					vPrintf("   bit:b%d prepare mode on\n", bit);
					// Config ok c'est une sortie
					BitNset(resu,bit);
				}

				// test du bit
				if(IsBitSet(do_off,bit))
				{
					vPrintf("   bit:b%d prepare mode off\n", bit);
					BitNclr(resu,bit);
				}

			}

		}
		else if(!do_off && !do_on)
		{
			vPrintf("  on = off = 0\n");
		}

		// Ecrire config
		if(resu != (uint8)(cnf_ref >> (octet*8)))
		{
			vPrintf("  Resu_calc:%x\n", resu);
			// recup config actuelle
			cnf_ref = ( cnf_ref & (0xFFFFFFFF & ~(0xFF<<octet*8)))|(resu << octet*8);
			vPrintf("  New OUT = %x\n",cnf_ref);

			resu = cnf_ref >> (octet*8);

			vPrintf("   Config a i2c:%x, reg:%d, val:%x\n", i2cAddr[octet],i2cReg[octet%2],resu);
			// Port 0 b[0..7], Port 1 b[8..15]
			// Ecriture dans le composant de la config
			u8_I2CWrite_9555(i2cAddr[octet],i2cReg[octet%2],resu);
		}
		else
		{
			vPrintf("  Resu = cnf ref\n");
		}



	} // Octet suivant

	vPrintf("Ref OUT out =%x\n\n",cnf_ref);
}

PUBLIC uint32 vPRT_DioReadInput (void)
{
	uint8 i2c_device = 0;
	uint16 tmp = 0;
	uint32 val=0UL;

	vPrintf("\n\nDemande de lecture des SLIOS\n");
	for(i2c_device=0;i2c_device<4;i2c_device = i2c_device +2)
	{
		tmp = u16_I2CRead_9555(i2cAddr[i2c_device]);
		val = val | (tmp<<i2c_device*8);
	}

	vPrintf("Fin lecture des SLIOS val=%x\n",val);
	return val;
}

///http://subversion.assembla.com/svn/Langaton/Acceleration/Source/EndDevice.c
///http://www.automatepc.fr/?page=I2cPcf8574
PRIVATE void u8_I2CWrite_9555(uint8 u8SlaveAddr,uint8 u8RegAddr, uint8 u8Value)
{
	vPrintf("\n   Procedure ecriture sur i2c\n");

	// Configuration du bus et activation du SI dans le jennic
	//vAHI_SiConfigure(TRUE,FALSE,E_BUS_400_KH);   /* enabled, no interrupt, 400kHz */

	// Recherche composant sur bus i2c
	vAHI_SiWriteSlaveAddr(u8SlaveAddr,FALSE);
	vAHI_SiSetCmdReg(E_AHI_SI_START_BIT,
			E_AHI_SI_NO_STOP_BIT,
			E_AHI_SI_NO_SLAVE_READ,
			E_AHI_SI_SLAVE_WRITE,
			E_AHI_SI_SEND_ACK,
			E_AHI_SI_NO_IRQ_ACK);


	while(bAHI_SiPollTransferInProgress()); // wait while busy
	//check to see if we get an ACK back
#if (JENNIC_CHIP_FAMILY == JN513x)
	if(bAHI_SiPollRxNack())
#elif (JENNIC_CHIP_FAMILY == JN514x)
	if(bAHI_SiMasterCheckRxNack())
#endif
	{
		vPrintf("    ERR:Cible i2c pas de reponse\n");
		return;
	}
	vPrintf("    Decouverte cible i2c ok\n");

	//Arbitration bus !!
	if(bAHI_SiPollArbitrationLost()){
		vPrintf("    ERR:Arbitration bus perdu\n");
		return;
	}

	//Ecriture dans registre
	vAHI_SiWriteData8(u8RegAddr);
	vAHI_SiSetCmdReg(E_AHI_SI_NO_START_BIT,
			E_AHI_SI_NO_STOP_BIT,
			E_AHI_SI_NO_SLAVE_READ,
			E_AHI_SI_SLAVE_WRITE,
			E_AHI_SI_SEND_ACK,
			E_AHI_SI_NO_IRQ_ACK);
	while(bAHI_SiPollTransferInProgress()); // wait while busy

#if (JENNIC_CHIP_FAMILY == JN513x)
	if(bAHI_SiPollRxNack())
#elif (JENNIC_CHIP_FAMILY == JN514x)
	if(bAHI_SiMasterCheckRxNack())
#endif
	{
		vPrintf("    ERR:Reponse cmp i2c\n");
		return;
	}

	vPrintf("    Ecriture i2c dans registre commande ok\n");

	//Ecriture de donnee de registre
	vAHI_SiWriteData8(u8Value);
	vAHI_SiSetCmdReg(E_AHI_SI_NO_START_BIT,
			E_AHI_SI_STOP_BIT,
			E_AHI_SI_NO_SLAVE_READ,
			E_AHI_SI_SLAVE_WRITE,
			E_AHI_SI_SEND_ACK,
			E_AHI_SI_NO_IRQ_ACK);
	while(bAHI_SiPollTransferInProgress()); // wait while busy

#if (JENNIC_CHIP_FAMILY == JN513x)
	if(bAHI_SiPollRxNack())
#elif (JENNIC_CHIP_FAMILY == JN514x)
	if(bAHI_SiMasterCheckRxNack())
#endif
	{
		vPrintf("    ERR:Ecriture valeur sur i2c\n");
		return;
	}
	vPrintf("   Configuration Ok\n");

}

PRIVATE uint16 u16_I2CRead_9555(uint8 u8SlaveAddr)
{

	uint8 u8Res = 0;
	uint16 u16Res = 0;
	uint8 u8RegAddr = CMD_INP_0;

	// Configuration du bus et activation du SI dans le jennic
	//vAHI_SiConfigure(TRUE,FALSE,E_BUS_400_KH);   /* enabled, no interrupt, 400kHz */
vPrintf("\n Lecture i2c(%x)\n",u8SlaveAddr);
	// Recherche composant sur bus i2c
	vAHI_SiWriteSlaveAddr(u8SlaveAddr,FALSE);
	vAHI_SiSetCmdReg(E_AHI_SI_START_BIT,
			E_AHI_SI_NO_STOP_BIT,
			E_AHI_SI_NO_SLAVE_READ,
			E_AHI_SI_SLAVE_WRITE,
			E_AHI_SI_SEND_ACK,
			E_AHI_SI_NO_IRQ_ACK);


	while(bAHI_SiPollTransferInProgress()); // wait while busy
	//check to see if we get an ACK back
#if (JENNIC_CHIP_FAMILY == JN513x)
	if(bAHI_SiPollRxNack())
#elif (JENNIC_CHIP_FAMILY == JN514x)
	if(bAHI_SiMasterCheckRxNack())
#endif
	{
		vPrintf("  ERR:Cible i2c(%x) pas de reponse\n",u8SlaveAddr);
		return u16Res;
	}
	vPrintf("  Decouverte cible i2c(%x) ok\n",u8SlaveAddr);

	//Arbitration bus !!
	if(bAHI_SiPollArbitrationLost()){
		vPrintf("  ERR:Arbitration bus perdu\n");
		return u16Res;
	}

	//Ecriture dans registre
	vAHI_SiWriteData8(u8RegAddr);
	vAHI_SiSetCmdReg(E_AHI_SI_NO_START_BIT,
			E_AHI_SI_NO_STOP_BIT,
			E_AHI_SI_NO_SLAVE_READ,
			E_AHI_SI_SLAVE_WRITE,
			E_AHI_SI_SEND_ACK,
			E_AHI_SI_NO_IRQ_ACK);
	while(bAHI_SiPollTransferInProgress()); // wait while busy

#if (JENNIC_CHIP_FAMILY == JN513x)
	if(bAHI_SiPollRxNack())
#elif (JENNIC_CHIP_FAMILY == JN514x)
	if(bAHI_SiMasterCheckRxNack())
#endif
	{
		vPrintf("  ERR:Ecriture dans registre cible i2c\n");
		return u16Res;
	}

	vPrintf("  Ecriture dans registre commande cible i2c ok\n");

	// Changement du sens de transfert
	vAHI_SiWriteSlaveAddr(u8SlaveAddr,TRUE);
	vAHI_SiSetCmdReg(E_AHI_SI_START_BIT,
			E_AHI_SI_NO_STOP_BIT,
			E_AHI_SI_NO_SLAVE_READ,
			E_AHI_SI_SLAVE_WRITE,
			E_AHI_SI_SEND_ACK,
			E_AHI_SI_NO_IRQ_ACK);
	while(bAHI_SiPollTransferInProgress()); // wait while busy

#if (JENNIC_CHIP_FAMILY == JN513x)
	if(bAHI_SiPollRxNack())
#elif (JENNIC_CHIP_FAMILY == JN514x)
	if(bAHI_SiMasterCheckRxNack())
#endif
	{
		vPrintf("  ERR:Pas de msg du composant i2c\n");
		return u16Res;
	}

	vPrintf(" Changement du sens de transfert ok\n");

	//Lecture port 0
	vAHI_SiSetCmdReg(E_AHI_SI_NO_START_BIT,
			E_AHI_SI_NO_STOP_BIT,
			E_AHI_SI_SLAVE_READ,
			E_AHI_SI_NO_SLAVE_WRITE,
			E_AHI_SI_SEND_ACK,
			E_AHI_SI_NO_IRQ_ACK);
	while(bAHI_SiPollTransferInProgress()); // wait while busy

#if (JENNIC_CHIP_FAMILY == JN513x)
	u8Res = u8AHI_SiReadData8();
#elif (JENNIC_CHIP_FAMILY == JN514x)
		u8Res = u8AHI_SiMasterReadData8();
#endif
	vPrintf("  Valeur recu port 0:%x\n", u8Res);
	u16Res = u8Res;

	//Lecture port 1
	vAHI_SiSetCmdReg(E_AHI_SI_NO_START_BIT,
			E_AHI_SI_STOP_BIT,
			E_AHI_SI_SLAVE_READ,
			E_AHI_SI_NO_SLAVE_WRITE,
			E_AHI_SI_SEND_NACK,
			E_AHI_SI_NO_IRQ_ACK);
	while(bAHI_SiPollTransferInProgress()); // wait while busy

#if (JENNIC_CHIP_FAMILY == JN513x)
	u8Res = u8AHI_SiReadData8();
#elif (JENNIC_CHIP_FAMILY == JN514x)
		u8Res = u8AHI_SiMasterReadData8();
#endif
	vPrintf("  Valeur recu port 1:%x\n", u8Res);
	u16Res = u16Res | (u8Res<<8);

	vPrintf(" Valeur retounee:%x\n",u16Res);

	return u16Res;
}

PRIVATE void vPRT_It_9555 (uint32 u32Device,uint32 u32ItemBitmap)
{
	switch (u32Device)
	{
		case E_AHI_DEVICE_SYSCTRL:
		{
			switch (u32ItemBitmap)
			{
				case E_AHI_DIO11_INT:
				case E_AHI_DIO12_INT:
				{
					if(bUneIt==FALSE){
						bUneIt = TRUE;
						// declanchement d'un timer
					}
					// 9555 n1 b[0..15]
					//val = vPRT_DioReadInput();
					//vPrintf("It read = %x\n",val);
				}
				break;


				default:
					vPrintf("Gestion It Perso Dio: %x\n", u32ItemBitmap);
					break;
			}

		}
		break;

		default:
			vPrintf("Gestion It Perso undef: %x\n", u32Device);
			break;
	}

}
