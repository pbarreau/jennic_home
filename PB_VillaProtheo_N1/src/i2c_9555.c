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

PRIVATE uint8 i2cAddr[] = {ADDR_IO_1,ADDR_IO_1,ADDR_IO_2,ADDR_IO_2};

PRIVATE void u8_I2CWrite_9555(uint8 u8SlaveAddr,uint8 u8RegAddr, uint8 u8Value);
PRIVATE uint16 u16_I2CRead_9555(uint8 u8SlaveAddr);
PRIVATE void vPRT_It_9555 (uint32 u32Device,uint32 u32ItemBitmap);

PRIVATE void vPRT_PrepareJennic(sBusSpeed speed);
PRIVATE void vPRT_PrepareSlio(void);

PRIVATE uint32  vPRT_GererDios(uint32 cnf_a, uint32 cnf_b, uint32 cnf_ref, uint8 *ptr_reg);

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


	// Pio en sortie
	vAHI_DioSetDirection(0,PBAR_CFG_OUTPUT);
	vAHI_DioSetOutput(C_LPIO_2,0); //Eteindre la led mode pgm
	au8Led[C_LID_2].actif = TRUE;
	au8Led[C_LID_2].pio = C_LPID_2;
	au8Led[C_LID_2].mode = E_LED_OFF;

	// Pio en entree
	vAHI_DioSetDirection(PBAR_CFG_INPUT, 0);

	// pull up activee sur les entrees
	vAHI_DioSetPullup(PBAR_CFG_INPUT, 0);

	// sens de la transition pour it (falling ie doc 9555)
	vAHI_DioInterruptEdge(0, E_AHI_DIO11_INT);

	// Enregistrer la call back de gestions des its
	vAHI_SysCtrlRegisterCallback(vPRT_It_9555);

	// Autoriser les Its
	vAHI_DioInterruptEnable(E_AHI_DIO11_INT, 0);

}

PRIVATE void vPRT_PrepareSlio(void)
{
	uint32 conf = 0;

	// Configuration en sortie
	vPRT_DioSetDirection(0x00000000,
			bit0|\
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

	// Lecture de la config au demarrage input et ouput des slios
	conf = vPRT_DioReadInput();
	prvCnf_I_9555 = (conf>>16 & 0xFF00) | (((uint16)conf>>8) & 0x00FF);
	prvCnf_O_9555 = (conf>>8 & 0xFF00) | (((uint16)conf) & 0x00FF);
}

/**
 *
 */
PUBLIC void vPRT_DioSetDirection(uint32 cnf_in, uint32 cnf_out)
{
	uint8 i2cReg[] = {CMD_CFG_0,CMD_CFG_1};
	static uint32 cnf_ref = 0xFFFFFFFF; /// Par defaut io definies en entrees

	vPrintf("Config direction Ios\n");
	cnf_ref=vPRT_GererDios(cnf_in, cnf_out, cnf_ref, i2cReg);
}

PUBLIC void vPRT_DioSetOutput(uint32 cnf_on, uint32 cnf_off)
{
	PRIVATE uint8 i2cReg[] = {CMD_OUT_0, CMD_OUT_1};
	static uint32 cnf_ref = 0xFFFFFFFF;

	vPrintf("Affecter valeur output\n");
	cnf_ref=vPRT_GererDios(cnf_on, cnf_off, cnf_ref, i2cReg);

	// La config des output devrait etre ok
	// On prend le risque de ne pas relire l'etat des ios.
	prvCnf_O_9555 = (cnf_ref>>8 & 0xFF00) | (((uint16)cnf_ref) & 0x00FF);
}

PRIVATE uint32 vPRT_GererDios(uint32 cnf_a, uint32 cnf_b, uint32 cnf_ref, uint8 *ptr_reg)
{
	uint32 cnf_new = 0; /// Valeur a envoyer sur la carte
	uint32 cnf_tmp = 0;
	uint8 resu = 0;
	int i=0;

	vPrintf(" Request -> cnf_a:%x, cnf_b:%x, cnf_ref:%x\n",(uint32)cnf_a,(uint32)cnf_b,(uint32)cnf_ref);

	// Verifier si un bit dans on et dans off
	for(i=0;i<=31;i++)
	{
		if(IsBitSet(cnf_a,i))
		{
			// Verifier si bit A 1 dans In
			if(IsBitSet(cnf_b,i))
			{
				vPrintf("  !!! bit:%d identique !!!\n",i);
				// le bit est defini dans in et out => je force a off
				BitNclr(cnf_a,i);
			}
		}
	}

	// Je memorise mes entrees
	cnf_tmp = cnf_a | cnf_ref;
	vPrintf(" cnf_tmp:%x\n",(uint32)cnf_tmp);


	// Je memorise mes sorties*
	cnf_new = ~cnf_b & cnf_tmp;
#if 0
	if(~cnf_b){
	cnf_new = ~cnf_b & cnf_tmp;
	}
	else
	{
		cnf_new = cnf_tmp;
	}
#endif
	vPrintf(" Msg:%x",(uint32)cnf_new);

	// Envoyer aux composant i2c 9555
	for(i=0;i<4;i++)
	{
		resu = (uint8)(cnf_new >> (i*8));
		if((i%2)==0)
			vPrintf("\n\n");

		vPrintf(" I2c:%x, reg:%d, val:%x", i2cAddr[i],ptr_reg[i%2],resu);

		if(resu == (uint8)(cnf_ref >> (i*8)))
		{
			vPrintf("\n   Config actuelle idem precedent => Noi2cWrite !!\n");
		}
		else
		{
			// Port 0 b[0..7], Port 1 b[8..15]
			// Ecriture dans le composant de la config
			u8_I2CWrite_9555(i2cAddr[i],ptr_reg[i%2],resu);
		}
	}

	// Memoriser les valeurs
	return(cnf_new);
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

	vPrintf("Fin lecture des SLIOS val=%x\n\n",val);
	return val;
}

///http://subversion.assembla.com/svn/Langaton/Acceleration/Source/EndDevice.c
///http://www.automatepc.fr/?page=I2cPcf8574
PRIVATE void u8_I2CWrite_9555(uint8 u8SlaveAddr,uint8 u8RegAddr, uint8 u8Value)
{
	vPrintf("\n  Procedure ecriture sur i2c\n");

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
	vPrintf("   Decouverte cible i2c ok\n");

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

	vPrintf("   Ecriture i2c dans registre commande ok\n");

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
	vPrintf("  Configuration Ok\n");

}

PRIVATE uint16 u16_I2CRead_9555(uint8 u8SlaveAddr)
{

	uint8 u8Res = 0;
	uint16 u16Res = 0;
	uint8 u8RegAddr = CMD_INP_0;

	// Configuration du bus et activation du SI dans le jennic
	//vAHI_SiConfigure(TRUE,FALSE,E_BUS_400_KH);   /* enabled, no interrupt, 400kHz */
	vPrintf(" Lecture i2c(%x)\n",u8SlaveAddr);
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

	vPrintf(" Valeur retounee:%x\n\n",u16Res);

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
				{
					if(bIt_DIO11==FALSE){
						bIt_DIO11 = TRUE;
						// declanchement d'un timer
					}
				}
				break;

				case E_AHI_DIO12_INT:
				{

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
