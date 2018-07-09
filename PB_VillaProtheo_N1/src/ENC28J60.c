/****************************************************************************
 * $Rev::                   $: Revision of last commit
 * $Author::                $: Author of last commit
 * $Date::                  $: Date of last commit
 * $HeadURL:                $
 ****************************************************************************
 * This software is owned by Jennic and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on Jennic products. You, and any third parties must reproduce
 * the copyright and warranty notice and any other legend of ownership on each
 * copy or partial copy of the software.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS". JENNIC MAKES NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * ACCURACY OR LACK OF NEGLIGENCE. JENNIC SHALL NOT, IN ANY CIRCUMSTANCES,
 * BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT LIMITED TO, SPECIAL,
 * INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON WHATSOEVER.
 *
 * Copyright Jennic Ltd 2010. All rights reserved
 ****************************************************************************/
/** @file
 *
 * @defgroup
 *
 */
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include <AppHardwareApi.h>

#include "spi.h"
#include "enc28j60.h"
#include "uip-conf.h"

#ifdef SERIAL_DEBUG
    #include <config.h>
    #include <serial.h>
    #include <xsprintf.h>
#endif

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/* Bank 0 Registers */
#define ERDPTL      0x00
#define ERDPTH      0x01
#define EWRPTL      0x02
#define EWRPTH      0x03
#define ETXSTL      0x04
#define ETXSTH      0x05
#define ETXNDL      0x06
#define ETXNDH      0x07
#define ERXSTL      0x08
#define ERXSTH      0x09
#define ERXNDL      0x0a
#define ERXNDH      0x0b
#define ERXRDPTL    0x0c
#define ERXRDPTH    0x0d
#define ERXWRPTL    0x0e
#define ERXWRPTH    0x0f
#define EDMASTL     0x10
#define EDMASTH     0x11
#define EDMANDL     0x12
#define EDMANDH     0x13
#define EDMADSTL    0x14
#define EDMADSTH    0x15
#define EDMACSL     0x16
#define EDMACSH     0x17

/* Bank 1 Registers */
#define EHT0        0x00
#define EHT1        0x01
#define EHT2        0x02
#define EHT3        0x03
#define EHT4        0x04
#define EHT5        0x05
#define EHT6        0x06
#define EHT7        0x07
#define EPMM0       0x08
#define EPMM1       0x09
#define EPMM2       0x0a
#define EPMM3       0x0b
#define EPMM4       0x0c
#define EPMM5       0x0d
#define EPMM6       0x0e
#define EPMM7       0x0f
#define EPMCSL      0x10
#define EPMCSH      0x11
#define EPMOL       0x14
#define EPMOH       0x15
#define ERXFCON     0x18
#define EPKTCNT     0x19

/* Bank 2 Registers */
#define MACON1      0x00
#define MACON3      0x02
#define MACON4      0x03
#define MABBIPG     0x04
#define MAIPGL      0x06
#define MAIPGH      0x07
#define MACLCON1    0x08
#define MACLCON2    0x09
#define MAMXFLL     0x0a
#define MAMXFLH     0x0b
#define MICMD       0x12
#define MIREGADR    0x14
#define MIWRL       0x16
#define MIWRH       0x17
#define MIRDL       0x18
#define MIRDH       0x19

/* Bank 3 Registers */
#define MAADR5      0x00
#define MAADR6      0x01
#define MAADR3      0x02
#define MAADR4      0x03
#define MAADR1      0x04
#define MAADR2      0x05
#define EBSTSD      0x06
#define EBSTCON     0x07
#define EBSTCSL     0x08
#define EBSTCSH     0x09
#define MISTAT      0x0a
#define EREVID      0x12
#define ECOCON      0x15
#define EFLOCON     0x17
#define EPAUSL      0x18
#define EPAUSH      0x19

/* Registers which exist in all banks */
#define EIE         0x1b
#define EIR         0x1c
#define ESTAT       0x1d
#define ECON2       0x1e
#define ECON1       0x1f

/* PHY subregisters */
#define PHCON1      0x00
#define PHSTAT1     0x01
#define PHID1       0x02
#define PHID2       0x03
#define PHCON2      0x10
#define PHSTAT2     0x11
#define PHIE        0x12
#define PHIR        0x13
#define PHLCON      0x14

/* ESTAT - bit fields */
#define CLKRDY      0x01
#define TXABRT      0x02
#define RXBUSY      0x04
#define LATECOL     0x10
#define BUFFER      0x40
#define INT         0x80

/* ECON1 - bit fields */
#define TXRST       0x80
#define RXRST       0x40
#define DMAST       0x20
#define CSUMEN      0x10
#define TXRTS       0x08
#define RXEN        0x04
#define BSEL1       0x02
#define BSEL0       0x01

/* ECON2 - bit fields */
#define AUTOINC     0x80
#define PKTDEC      0x40
#define PWRSV       0x20
#define VRPS        0x08

/* ERXFCON - bit fields */
#define UCEN        0x80
#define ANDOR       0x40
#define CRCEN       0x20
#define PMEN        0x10
#define MPEN        0x08
#define HTEN        0x04
#define MCEN        0x02
#define BCEN        0x01

/* MACON1 - bit fields */
#define TXPAUS      0x08
#define RXPAUS      0x04
#define PASSALL     0x02
#define MARXEN      0x01

/* MACON3 - bit fields */
#define PADCFG2     0x80
#define PADCFG1     0x40
#define PADCFG0     0x20
#define TXCRCEN     0x10
#define PHDREN      0x08
#define HFRMEN      0x04
#define FRMLNEN     0x02
#define FULDPX      0x01

/* MACON4 - bit fields */
#define DEFER       0x40
#define BPEN        0x20
#define NOBKOFF     0x10

/* EIR - bit fields */
#define PKTIF       0x40
#define DMAIF       0x20
#define LINKIF      0x10
#define TXIF        0x08
#define TXERIF      0x02
#define RXERIF      0x01

/* MICMD - bit fields */
#define MIIRD       0x01

/* MSTAT - bit fields */
#define BUSY        0x01

/* PHYSTAT2 - bit fields */
#define LSTAT       0x0400U

/* Commands */
#define RCR         0x00    /* Read control register */
#define RBM         0x3A    /* Read buffer memory */
#define WCR         0x40    /* Write control register */
#define WBM         0x7A    /* Write buffer memory */
#define BFS         0x80    /* Bit field set */
#define BFC         0xA0    /* Bit field clear */
#define SRC         0xFF    /* System reset */

#define CRC_LEN     4       /* Bytes */

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct __attribute__((__packed__))
{
    uint16 u16ByteCount;

    uint8  bfTxDone         : 1;
    uint8  bfLenRngError    : 1;
    uint8  bfLenChkError    : 1;
    uint8  bfCRCError       : 1;
    uint8  bfCollisionCount : 4;

    uint8  bfTxUnderrun     : 1;
    uint8  bfTxGiant        : 1;
    uint8  bfTxLateColl     : 1;
    uint8  bfTxExcessColl   : 1;
    uint8  bfTxExcessDefer  : 1;
    uint8  bfTxPacketDefer  : 1;
    uint8  bfTxBroadcast    : 1;
    uint8  bfTxMulticast    : 1;

    uint16 u16TotalTxOnWire;

    /* Not sure if the following fields are in the correct order */
    uint8  bfTxCtrlFrame    : 1;
    uint8  bfTxPauseFrame   : 1;
    uint8  bfBackPressure   : 1;
    uint8  bfTxVLANFrame    : 1;
    uint8  bfZero           : 4;

}xENC28J60TxSTATUSFIELDS;

typedef union __attribute__((__packed__))
{
     uint8       au8Byte[sizeof(xENC28J60TxSTATUSFIELDS)];
     xENC28J60TxSTATUSFIELDS xField;
}xENC28J60TxSTATUS;

typedef struct __attribute__((__packed__))
{
    uint8 bfPOVERRIDE : 1;
    uint8 bfPCRCEN    : 1;
    uint8 bfPPADEN    : 1;
    uint8 bfPHUGEEN   : 1;
    uint8 bfReserved  : 4;
}xENC28J60TxCTRLFIELDS;

typedef union __attribute__((__packed__))
{
    uint8                   u8Byte;
    xENC28J60TxCTRLFIELDS   xField;
}xENC28J60TxCTRL;

typedef struct __attribute__((__packed__))
{
    uint16 u16ByteCount;
    uint8  bfLongEvent      : 1;
    uint8  bfReserved1      : 1;
    uint8  bfCarrPrevSeen   : 1;
    uint8  bfReserved2      : 1;
    uint8  bfCRCError       : 1;
    uint8  bfLenChkError    : 1;
    uint8  bfLenRngError    : 1;
    uint8  bfRxOK           : 1;
    uint8  bfRxMulticast    : 1;
    uint8  bfRxBroadcast    : 1;
    uint8  bfDribbleNibble  : 1;
    uint8  bfRxCtrlFrame    : 1;
    uint8  bfRxPauseFrame   : 1;
    uint8  bfRxUnkownOpcode : 1;
    uint8  bfRxVLANFrame    : 1;
    uint8  bfZero           : 1;
}xENC28J60RxSTATUSFIELDS;

typedef union __attribute__((__packed__))
{
    uint8       au8Byte[sizeof(xENC28J60RxSTATUSFIELDS)];
    xENC28J60RxSTATUSFIELDS xField;
}xENC28J60RxSTATUS;

typedef struct __attribute__((__packed__))
{
    uint16              u16NextPktPtr;
    xENC28J60RxSTATUS   xStatus;
}xENC28J60RxHDR;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void  vENC28J60_SetBank(uint8 u8Bank);
PRIVATE uint8 u8ENC28J60_ReadRegETH(uint8 u8Addr);
PRIVATE void  vENC28J60_BitSet(uint8 u8Addr, uint8 u8Data);
PRIVATE void  vENC28J60_BitClear(uint8 u8Addr, uint8 u8Data);
PRIVATE void  vENC28J60_WriteRegCTRL(uint8 u8Addr, uint8 u8Data);

#ifdef SERIAL_DEBUG
    PRIVATE uint8 u8ENC28J60_ReadRegMAC(uint8 u8Addr);
    PRIVATE uint16 u16ENC28J60_ReadRegPHY(uint8 u8Addr);
#endif

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
#ifdef SERIAL_DEBUG
    extern uint8 au8DebugMsg[256];
#endif

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC bool_t bENC28J60_Init(uint8 *pu8MAC)
{
    bool_t bResult = FALSE;

    /* Hardware reset of the ENC28J60 */
    vENC28J60_HwReset();

    /* Configure SPI port */
    vSPI_Init(SPI_MODE_0, SPI_CLOCK_8MHz);

    /* Read revision number from ENC28J60 */
    vENC28J60_SetBank(3);
    uint8 u8Rev = u8ENC28J60_ReadRegETH(EREVID);

    #ifdef SERIAL_DEBUG
        xsprintf((char *)au8DebugMsg, "\n\rENC28J60 Silicon Rev - 0x%02X", u8Rev);
        vSerial_TxString(DEBUG_PORT, au8DebugMsg);
    #endif

    /* Attempt to detect if we can communicate with the ENC28J60 */
    if (u8Rev != 0xff)
    {
        #ifdef SERIAL_DEBUG
            /* Read the PHY part number, revision and OUI */
            vENC28J60_SetBank(2);

            uint16 u16PHID1 = u16ENC28J60_ReadRegPHY(PHID1);
            uint16 u16PHID2 = u16ENC28J60_ReadRegPHY(PHID2);
            uint8  u8PhyNbr = (uint8)((u16PHID2 >> 5) & 0x1f);
            uint8  u8PhyRev = (uint8)(u16PHID2 & 0x000f);
            uint32 u32OUI  = ((uint32)u16PHID1) | ((uint32)(u16PHID2 & 0xffe0));

            xsprintf((char *)au8DebugMsg, "\n\rPHY Part Number 0x%02X", u8PhyNbr);
            vSerial_TxString(DEBUG_PORT, au8DebugMsg);
            xsprintf((char *)au8DebugMsg, "\n\rPHY Revision 0x%02X", u8PhyRev);
            vSerial_TxString(DEBUG_PORT, au8DebugMsg);
            xsprintf((char *)au8DebugMsg, "\n\rPHY OUI 0x%08X", u32OUI);
            vSerial_TxString(DEBUG_PORT, au8DebugMsg);
        #endif

        /* The receive buffer constitutes a circular FIFO buffer managed by
           hardware. The register pairs ERXSTH:ERXSTL and ERXNDH:ERXNDL serve
           as pointers to define the buffer’s size and location within the
           memory. The byte pointed to by ERXST and the byte pointed to by
           ERXND are both included in the FIFO buffer. */
        vENC28J60_SetBank(0);
        vENC28J60_WriteRegCTRL(ERXSTL, 0x00);
        vENC28J60_WriteRegCTRL(ERXSTH, 0x00);
        vENC28J60_WriteRegCTRL(ERXNDL, 0xff); /* Rx buffer size is 4096 bytes */
        vENC28J60_WriteRegCTRL(ERXNDH, 0x0f);

        /* Set the RX RD pointer to an odd address to work around B5 errata #11 */
        vENC28J60_WriteRegCTRL(ERXRDPTL, 0xff);
        vENC28J60_WriteRegCTRL(ERXRDPTH, 0x0f);

        /* Set the initial read pointer location */
        vENC28J60_WriteRegCTRL(ERDPTL, 0x00);
        vENC28J60_WriteRegCTRL(ERDPTH, 0x00);

        /* Set the MAC TX start pointer */
        vENC28J60_WriteRegCTRL(ETXSTL, 0x00);
        vENC28J60_WriteRegCTRL(ETXSTH, 0x10);

        /* Set to automatically increment read pointer in circular buffer */
        vENC28J60_BitSet(ECON2, AUTOINC);

        /* Select receive filters for my MAC address, multicast and broadcast */
        vENC28J60_SetBank(1);
        vENC28J60_WriteRegCTRL(ERXFCON, UCEN | CRCEN | BCEN | MCEN);

        /* Wait for oscillator ready, but don't hang here! */
        uint8 u8Status = 0x00;

        while ((u8Status & 0x01) != 0)
        {
            u8Status = u8ENC28J60_ReadRegETH(ESTAT);
        }

        /* Enable MAC to receive frames (MACON1.0) */
        vENC28J60_SetBank(2);
        vENC28J60_WriteRegCTRL(MACON1, TXPAUS | RXPAUS | MARXEN);

        /* Set up padding, CRC, and half duplex */
        vENC28J60_WriteRegCTRL(MACON3, PADCFG0 | TXCRCEN | FRMLNEN);

	    /* Allow infinite deferals if the medium is continuously busy
          (do not time out a transmission if the half duplex medium is
          completely saturated with other people's data) */
    	vENC28J60_WriteRegCTRL(MACON4, DEFER);

        /* Set maximum ethernet frame length */
        vENC28J60_WriteRegCTRL(MAMXFLL, UIP_CONF_BUFFER_SIZE & 0xff);
        vENC28J60_WriteRegCTRL(MAMXFLH, UIP_CONF_BUFFER_SIZE >> 8);

        /* Configure inter-packet gap lengths (MABBIPG, MAIPGL, MAIPGH) */
        vENC28J60_WriteRegCTRL(MABBIPG, 0x12);
        vENC28J60_WriteRegCTRL(MAIPGL, 0x12);
        vENC28J60_WriteRegCTRL(MAIPGH, 0x0c);

        /* Configure MAC address */
        vENC28J60_SetBank(3);
        vENC28J60_WriteRegCTRL(MAADR1, pu8MAC[0]);
        vENC28J60_WriteRegCTRL(MAADR2, pu8MAC[1]);
        vENC28J60_WriteRegCTRL(MAADR3, pu8MAC[2]);
        vENC28J60_WriteRegCTRL(MAADR4, pu8MAC[3]);
        vENC28J60_WriteRegCTRL(MAADR5, pu8MAC[4]);
        vENC28J60_WriteRegCTRL(MAADR6, pu8MAC[5]);

		/* Disable the CLKOUT output to reduce EMI generation */
		vENC28J60_WriteRegCTRL(ECOCON, 0x00);

        /* PHY Init */
        vENC28J60_SetBank(2);

        /* Set PHCON1 subregister for half duplex. */
        vENC28J60_WriteRegCTRL(MIREGADR, PHCON1);
        vENC28J60_WriteRegCTRL(MIWRL, 0x00);
        vENC28J60_WriteRegCTRL(MIWRH, 0x00);

        /* Set PHCON2 subregister for half-duplex loopback inhibit */
        /*  (ie. don't receive copies of packets we transmit)      */
        vENC28J60_WriteRegCTRL(MIREGADR, PHCON2);
        vENC28J60_WriteRegCTRL(MIWRL, 0x00);
        vENC28J60_WriteRegCTRL(MIWRH, 0x01);

        /* Set up LEDs for link status on A and rx/tx on B */
        vENC28J60_WriteRegCTRL(MIREGADR, PHLCON);
        vENC28J60_WriteRegCTRL(MIWRL, 0x72);
        vENC28J60_WriteRegCTRL(MIWRH, 0x34);

        /* Enable writing of received packets into receive buffer */
        vENC28J60_BitSet(ECON1, RXEN);

        bResult = TRUE;
    }
    return bResult;
}

/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC void vENC28J60_Transmit(uint8 *pu8Buffer, uint16 u16Len)
{
    uint16 i;
    xENC28J60TxCTRL xTxCtrl;
    xENC28J60TxSTATUS xTxStatus;

    /* Everything we need is in bank 0 */
    vENC28J60_SetBank(0);

    /* Clear abort and collision flags so that we now what state they start in */
    vENC28J60_BitClear(ESTAT, TXABRT);
    vENC28J60_BitClear(ESTAT, LATECOL);

    vENC28J60_BitClear(EIR, TXIF);

    /* Over come Errata #10 in ENC28J60B */
    vENC28J60_BitSet(ECON1, TXRST);
    vENC28J60_BitClear(ECON1, TXRST);

    /* Set the SPI write buffer pointer location */
    vENC28J60_WriteRegCTRL(EWRPTL, 0x00);
    vENC28J60_WriteRegCTRL(EWRPTH, 0x10);

    vSPI_SelectChip(SPI_CS_1);
    vSPI_Write8(WBM);

    /* TX options control byte header needed by the ENC28J60 */
    xTxCtrl.xField.bfPOVERRIDE = 0;
    xTxCtrl.xField.bfPCRCEN = 0;
    xTxCtrl.xField.bfPPADEN = 0;
    xTxCtrl.xField.bfPHUGEEN = 0;

    vSPI_Write8(xTxCtrl.u8Byte);

    /* Write Ethernet Payload into ENC28J60 RAM */
    for (i = 0; i < u16Len; i++)
    {
        vSPI_Write8(pu8Buffer[i]);
    }

    vSPI_SelectChip(SPI_CS_NONE);

    /* Set the MAC TX end pointer */
    vENC28J60_WriteRegCTRL(ETXNDL, (0x1000 + (u16Len)) & 0x00ff);
    vENC28J60_WriteRegCTRL(ETXNDH, (0x1000 + (u16Len)) >> 8);

    /* Start the transmission */
    vENC28J60_BitSet(ECON1, TXRTS);

    /* Wait for the transmission to complete, TXRTS is set low when tx completes */
    i = 0;
    while (u8ENC28J60_ReadRegETH(ECON1) & TXRTS)
    {
        /* Timeout so we dont get stuck */
        if (++i > 50000)
        {
            break;
        }
    }

    /* Store current value of read ptr */
    uint8 u8ERDPTL = u8ENC28J60_ReadRegETH(ERDPTL);
    uint8 u8ERDPTH = u8ENC28J60_ReadRegETH(ERDPTH);

    /* Read the transmit status */
    vENC28J60_WriteRegCTRL(ERDPTL, u8ENC28J60_ReadRegETH(ETXNDL));
    vENC28J60_WriteRegCTRL(ERDPTH, u8ENC28J60_ReadRegETH(ETXNDH));

    vSPI_SelectChip(SPI_CS_1);
    vSPI_Write8(RBM);

    (void)u8SPI_Read8();
    xTxStatus.au8Byte[1]  = u8SPI_Read8();
    xTxStatus.au8Byte[0]  = u8SPI_Read8();
    xTxStatus.au8Byte[2]  = u8SPI_Read8();
    xTxStatus.au8Byte[3]  = u8SPI_Read8();
    xTxStatus.au8Byte[5]  = u8SPI_Read8();
    xTxStatus.au8Byte[4]  = u8SPI_Read8();
    xTxStatus.au8Byte[6]  = u8SPI_Read8();

    vSPI_SelectChip(SPI_CS_NONE);

    /* Restore the previous value of read ptr */
    vENC28J60_WriteRegCTRL(ERDPTL, u8ERDPTL);
    vENC28J60_WriteRegCTRL(ERDPTH, u8ERDPTH);

    #ifdef SERIAL_DEBUG
        xsprintf((char *)au8DebugMsg, "\n\rBytes Tx - %d", xTxStatus.xField.u16ByteCount);
        vSerial_TxString(DEBUG_PORT, au8DebugMsg);
        xsprintf((char *)au8DebugMsg, "\n\rCRC Error - %d", xTxStatus.xField.bfCRCError);
        vSerial_TxString(DEBUG_PORT, au8DebugMsg);
        xsprintf((char *)au8DebugMsg, "\n\rTx Done - %d", xTxStatus.xField.bfTxDone);
        vSerial_TxString(DEBUG_PORT, au8DebugMsg);
        xsprintf((char *)au8DebugMsg, "\n\rBytes Wire - %d", xTxStatus.xField.u16TotalTxOnWire);
        vSerial_TxString(DEBUG_PORT, au8DebugMsg);
    #endif

    /* Just in case, clear the transmit abort and late collision flags */
    vENC28J60_BitClear(ESTAT, TXABRT);
    vENC28J60_BitClear(ESTAT, LATECOL);
}

/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC uint16 u16ENC28J60_Receive(uint8 *pu8Buffer)
{
    xENC28J60RxHDR xRxHdr;
    uint16 u16RxPktPtr, i;
    uint16 u16PktLen = 0;

    #ifdef SERIAL_DEBUG
        xsprintf((char *)au8DebugMsg, "\n\rENC28J60 Packet Received");
        vSerial_TxString(DEBUG_PORT, au8DebugMsg);
    #endif

    /* Retrieve the receive pointer location */
    vENC28J60_SetBank(0);
    u16RxPktPtr   = u8ENC28J60_ReadRegETH(ERDPTH);
    u16RxPktPtr <<= 8;
    u16RxPktPtr  |= u8ENC28J60_ReadRegETH(ERDPTL);

    /* All received data and status information can be read from buffer memory */
    vSPI_SelectChip(SPI_CS_1);
    vSPI_Write8(RBM);

    /* Assemble the header information for the received frame */
    xRxHdr.u16NextPktPtr  = u8SPI_Read8();
    xRxHdr.u16NextPktPtr |= (((uint16)u8SPI_Read8()) << 8);
    xRxHdr.xStatus.xField.u16ByteCount  = u8SPI_Read8();
    xRxHdr.xStatus.xField.u16ByteCount |= (((uint16)u8SPI_Read8()) << 8);
    xRxHdr.xStatus.au8Byte[3] = u8SPI_Read8();
    xRxHdr.xStatus.au8Byte[2] = u8SPI_Read8();

    #ifdef SERIAL_DEBUG
        xsprintf((char *)au8DebugMsg, "\n\r bfCRCError - %d", xRxHdr.xStatus.xField.bfCRCError);
        vSerial_TxString(DEBUG_PORT, au8DebugMsg);
        xsprintf((char *)au8DebugMsg, "\n\r bfRxOK - %d", xRxHdr.xStatus.xField.bfRxOK);
        vSerial_TxString(DEBUG_PORT, au8DebugMsg);
    #endif

    /* Check if the received CRC was correct */
    if (xRxHdr.xStatus.xField.bfCRCError == 0)
    {
        uint16 u16CalcPktLen = 0;

        #ifdef SERIAL_DEBUG
            xsprintf((char *)au8DebugMsg, "\n\rRx Pkt Ptr - 0x%04X", u16RxPktPtr);
            vSerial_TxString(DEBUG_PORT, au8DebugMsg);
            xsprintf((char *)au8DebugMsg, "\n\rNx Pkt Ptr - 0x%04X", xRxHdr.u16NextPktPtr);
            vSerial_TxString(DEBUG_PORT, au8DebugMsg);
            xsprintf((char *)au8DebugMsg, "\n\rByte Count - %d", xRxHdr.xStatus.xField.u16ByteCount);
            vSerial_TxString(DEBUG_PORT, au8DebugMsg);
        #endif

        /* Check the next packet pointer is not corrupt */
        if (xRxHdr.u16NextPktPtr <= 0x0FFF)
        {
            /* Calculate packet length based on receive pointer and next packet pointer */
            if (xRxHdr.u16NextPktPtr < u16RxPktPtr)
            {
                /* Receive data must have wrapped around the end of the buffer */
               u16CalcPktLen = (0x1000 - u16RxPktPtr) + xRxHdr.u16NextPktPtr;
            }
            else
            {
                /* No wrapping */
                u16CalcPktLen = xRxHdr.u16NextPktPtr - u16RxPktPtr;
            }

            #ifdef SERIAL_DEBUG
                xsprintf((char *)au8DebugMsg, "\n\rCalc Pkt Len - %d", u16CalcPktLen);
                vSerial_TxString(DEBUG_PORT, au8DebugMsg);
            #endif

            /* Check that the received packet lenght does not exceed the maximum we can accept */
            if (u16CalcPktLen <= UIP_CONF_BUFFER_SIZE)
            {
                /* Check packet length calculated based on pointers with packet length contained within Ethernet frame.
                   Could be off by one here due to the word alignment. If so, go ahead and accept. */
                if ((u16CalcPktLen == xRxHdr.xStatus.xField.u16ByteCount + sizeof(xENC28J60RxHDR)) ||
                    (u16CalcPktLen == xRxHdr.xStatus.xField.u16ByteCount + sizeof(xENC28J60RxHDR) + 1))
                {
                    /* Copy Ethernet payload into destination buffer */
                    for (i = 0; i < (u16CalcPktLen - sizeof(xENC28J60RxHDR)); i++)
                    {
                        pu8Buffer[i] = u8SPI_Read8();
                    }
                    u16PktLen = xRxHdr.xStatus.xField.u16ByteCount - CRC_LEN;
                }
            }
        }
    }
    /* Finished buffer memory read */
    vSPI_SelectChip(SPI_CS_NONE);

    /* Errata #11 - Ensure only odd addresses are written to ERXRDPT */
    if ((xRxHdr.u16NextPktPtr & 0x01) == 0)
    {
        xRxHdr.u16NextPktPtr -= 1;
    }

    if (xRxHdr.u16NextPktPtr > 0x0fff)
    {
        /* Wrap around. */
        xRxHdr.u16NextPktPtr = 0x0fff;
    }

    /* Free receive buffer space by moving ERXRDPT forward */
    vENC28J60_WriteRegCTRL(ERXRDPTL, xRxHdr.u16NextPktPtr & 0xff);
    vENC28J60_WriteRegCTRL(ERXRDPTH, xRxHdr.u16NextPktPtr >> 8);

    /* Now we have read out the packet decrement the received packet count */
    vENC28J60_BitSet(ECON2, PKTDEC);

    /* Return length of Ethernet packet */
    return (u16PktLen);
}

/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC uint8 u8ENC28J60_RxPacketCount(void)
{
    vENC28J60_SetBank(1);

    /* Read the received packet counter */
    return (u8ENC28J60_ReadRegETH(EPKTCNT));
}

/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PUBLIC void vENC28J60_HwReset(void)
{
    volatile uint16 i;

    /* Hardware reset of the ENC28J60 - /RST input connected to DIO14 */
    vAHI_DioSetDirection(0, (1UL << 14));

    /* Set /RST low */
    vAHI_DioSetOutput(0, (1UL << 14));

    for (i = 0; i < 1000; i++);

    /* Set /RST high */
    vAHI_DioSetOutput((1UL << 14), 0);

	/* Errata #1: Wait at least 1ms following a reset before talking to the device. */
    for (i = 0; i < 10000; i++);
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PRIVATE void vENC28J60_BitClear(uint8 u8Addr, uint8 u8Data)
{
    vSPI_SelectChip(SPI_CS_1);
    vSPI_Write8(u8Addr | BFC);
    vSPI_Write8(u8Data);
    vSPI_SelectChip(SPI_CS_NONE);
}

/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PRIVATE void vENC28J60_BitSet(uint8 u8Addr, uint8 u8Data)
{
    vSPI_SelectChip(SPI_CS_1);
    vSPI_Write8(u8Addr | BFS);
    vSPI_Write8(u8Data);
    vSPI_SelectChip(SPI_CS_NONE);
}

/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PRIVATE void vENC28J60_SetBank(uint8 u8Bank)
{
    switch(u8Bank)
    {
        case 0:
            vENC28J60_BitClear(ECON1, BSEL1 | BSEL0);
            break;
        case 1:
            vENC28J60_BitSet(ECON1, BSEL0);
            vENC28J60_BitClear(ECON1, BSEL1);
            break;
        case 2:
            vENC28J60_BitClear(ECON1, BSEL0);
            vENC28J60_BitSet(ECON1, BSEL1);
            break;
        case 3:
            vENC28J60_BitSet(ECON1, BSEL0|BSEL1);
            break;
        default:
            /* Invalid request */
            break;
    }
}

/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PRIVATE void vENC28J60_WriteRegCTRL(uint8 u8Addr, uint8 u8Data)
{
    vSPI_SelectChip(SPI_CS_1);
    vSPI_Write8(u8Addr | WCR);
    vSPI_Write8(u8Data);
    vSPI_SelectChip(SPI_CS_NONE);
}

/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
PRIVATE uint8 u8ENC28J60_ReadRegETH(uint8 u8Addr)
{
    uint8 u8Data;

    vSPI_SelectChip(SPI_CS_1);
    vSPI_Write8(u8Addr | RCR);
    u8Data = u8SPI_Read8();
    vSPI_SelectChip(SPI_CS_NONE);

    return u8Data;
}
/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
#ifdef SERIAL_DEBUG
static uint8 u8ENC28J60_ReadRegMAC(uint8 u8Addr)
{
    volatile uint8 u8Data;

    vSPI_SelectChip(SPI_CS_1);

    vSPI_Write8(u8Addr | RCR);
    u8Data = u8SPI_Read8(); /* Dummy read, throw away first byte */
    u8Data = u8SPI_Read8();

    vSPI_SelectChip(SPI_CS_NONE);

    return u8Data;
}
#endif

/****************************************************************************
 *
 * NAME:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:      Name            RW  Usage
 * None.
 *
 * RETURNS:
 * None.
 *
 * NOTES:
 * None.
 ****************************************************************************/
#ifdef SERIAL_DEBUG
PRIVATE uint16 u16ENC28J60_ReadRegPHY(uint8 u8Addr)
{
    volatile uint32 i;
    uint16 u16Result;
    uint8 u8TempL, u8TempH;

    /* Set the address of the regsiter we want ot read */
    vENC28J60_WriteRegCTRL(MIREGADR, u8Addr);

    /* Set the read bit to start the read operation */
    vENC28J60_BitSet(MICMD, MIIRD);

    /* Wait 10us, this is very approximate!! */
    for (i = 0; i < 100; i++);

    /* Poll busy bit until read is complete */
    while (u8ENC28J60_ReadRegMAC(MISTAT) & BUSY);

    /* Clear the read bit */
    vENC28J60_BitClear(MICMD, MIIRD);

    /* Read register data */
    u8TempH = u8ENC28J60_ReadRegMAC(MIRDH);
    u8TempL = u8ENC28J60_ReadRegMAC(MIRDL);

    u16Result   = u8TempH;
    u16Result <<= 8;
    u16Result  |= u8TempL;

    return (u16Result);
}
#endif

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
