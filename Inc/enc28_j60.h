/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ENC28_H
#define __ENC28_H

/* Includes ------------------------------------------------------------------*/

#include "eth.h"

/* Defines ------------------------------------------------------------------*/

// Operations Defines
// TABLE 4-1: SPI INSTRUCTION SET FOR THE ENC28J60
#define ENC28J60_READ_CTRL_REG 	0x00
#define ENC28J60_WRITE_CTRL_REG 0x40
#define ENC28J60_BIT_FIELD_CLR 	0xA0
#define ENC28J60_BIT_FIELD_SET 	0x80
#define ENC28J60_SOFT_RESET 	0xFF

#define ENC28_READ_BUF_MEM		 0x3A //0x01
#define ENC28_WRITE_BUF_MEM		 0x7A //0x03

// Masks and some constants
#define ADDR_MASK	0x1F
#define BANK_MASK 0x60 //0110 0000
// FIGURE 3-2: ETHERNET BUFFER ORGANIZATION
#define RXSTART_INIT 0x0000
#define RXSTOP_INIT 0x0BFF
#define TXSTART_INIT 0x0C00
#define TXSTOP_INIT 0x11FF

#define MAX_FRAMELEN							1500


// TABLE 3-1: ENC28J60 CONTROL REGISTER MAP
// Bank0 - control registers addresses
#define ERDPT 	0x00
#define EWRPT 	0x02
#define ETXST 	0x04
#define ETXND 	0x06
#define ERXST 	0x08
#define ERXND 	0x0A
#define ERXRDPT 0x0C
#define ERXWRPT 0x0E
#define MISTAT_BUSY								0x01

// Bank1 - control registers addresses
#define ERXFCON 	0x18 | 0x20
#define EPMM0 		0x08 | 0x20
#define EPMCS 		0x10 | 0x20
#define EPKTCNT 	0x19 | 0x20

// Bank2 - control registers addresses
#define MACON1	 	0x00 | 0x40
#define MACON2	 	0x01 | 0x40
#define MACON3	 	0x02 | 0x40
#define MAIPG	 	  0x06 | 0x40 | 0x80
#define MABBIPG	 	0x04 | 0x40 | 0x80
#define MAMXFL		0x0A | 0x40 | 0x80
#define MIREGADR  0x14 | 0x40 | 0x80
#define MIWR  		0x16 | 0x40 | 0x80
#define MICMD  		0x12 | 0x40 | 0x80
#define MIRD  		0x18 | 0x40 | 0x80

// Bank3 - control registers addresses
#define MAADR5	 	0x04 | 0x60 | 0x80
#define MAADR4	 	0x05 | 0x60 | 0x80
#define MAADR3	 	0x02 | 0x60 | 0x80
#define MAADR2	 	0x03 | 0x60 | 0x80
#define MAADR1	 	0x00 | 0x60 | 0x80
#define MAADR0		0x01 | 0x60 | 0x80
#define MISTAT    0x0A | 0x60 | 0x80
#define EREVID  	0x12 | 0x60

// Common registers
// TABLE 3-1: ENC28J60 CONTROL REGISTER MAP
#define EIE 	0x1B
#define EIR 	0x1C
#define ESTAT 0x1D
#define ECON1 0x1F
#define ECON2 0x1E

// BitField Defines
// REGISTER 3-1: ECON1: ETHERNET CONTROL REGISTER 1
// BSEL<1:0>: Bank Select bits
#define ECON1_BSEL0 	0x01
#define ECON1_BSEL1 	0x02
#define ESTAT_CLKRDY 	0x01

#define ECON2_PKTDEC							0x40
#define ECON2_AUTOINC							0x80
#define ECON1_RXEN								0x04
#define ECON1_TXRST								0x80
#define ECON1_TXRTS								0x08

#define ERXFCON_UCEN							0x80
#define ERXFCON_CRCEN							0x20
#define ERXFCON_PMEN							0x10
#define ERXFCON_BCEN							0x01 
#define ERXFCON_ANDOR							0x40

#define MACON1_MARXEN							0x01
#define MACON1_TXPAUS							0x08
#define MACON1_RXPAUS							0x04
#define MACON1_PASSALL						0x02

#define MACON3_PADCFG0						0x20
#define MACON3_TXCRCEN						0x10
#define MACON3_FRMLNEN						0x02
#define MACON3_FULDPX							0x01
#define MICMD_MIIRD								0x01

#define EIE_INTIE									0x80 
#define EIE_PKTIE									0x40
#define EIR_TXERIF								0x02
#define EIR_PKTIF 								0x40
#define EIR_TXIF									0x08
#define MICMD_MIIRD								0x01

//PHY layer
// REGISTER 2-2: PHLCON: PHY MODULE LED CONTROL REGISTER
#define PHLCON										0x14
#define PHCON1										0x00
#define PHCON2										0x10

// bit 1 STRCH: LED Pulse Stretching Enable bit
// 1 = Stretchable LED events will cause lengthened LED pulses based on LFRQ<1:0> configuration
// bit 7-4 LBCFG<3:0>: LEDB Configuration bits
// 0010 = Display receive activity (stretchable)
// bit 11-8 LACFG<3:0>: LEDA Configuration bits
// 0001 = Display transmit activity (stretchable)
#define PHLCON_LED								0x0122
#define PHCON2_HDLDIS							0x0100
#define PHCON1_PDPXMD							0x0100


/* Exported functions prototypes ---------------------------------------------*/
void enc28_init(mac_address mac);

void enc28_packetSend(uint16_t len, uint8_t* dataBuf);

uint16_t enc28_packetReceive(uint16_t maxlen, uint8_t* dataBuf);

#endif /* __ENC28_H */