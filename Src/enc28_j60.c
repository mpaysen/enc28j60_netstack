/* Includes ------------------------------------------------------------------*/
#include "enc28_j60.h"

extern SPI_HandleTypeDef hspi1;
static uint8_t enc28_bank;
static uint16_t nextPacketPtr;
static uint8_t erxfcon;
uint8_t dataWatch8;

/* private functions prototypes ---------------------------------------------*/

uint8_t enc28J60_TransceiveByte(uint8_t data);

void enc28_enableChip();
void enc28_disableChip();

uint8_t enc28_readOp(uint8_t oper, uint8_t addr);
void enc28_writeOp(uint8_t oper, uint8_t addr, uint8_t data);

uint8_t  enc28_readReg8(uint8_t addr);
void enc28_writeReg8(uint8_t addr, uint8_t data);

uint16_t enc28_readReg16(uint8_t addr);
void enc28_writeReg16(uint8_t addr, uint16_t data);

void enc28_setBank(uint8_t addr);

void enc28_writePhy(uint8_t addr, uint16_t data);
uint16_t enc28_readPhy(uint8_t addr);

void enc28_writeBuf(uint16_t len, uint8_t* data);
void enc28_readBuf(uint16_t len, uint8_t *data);

uint16_t enc28_readBuf16();

/* functions ---------------------------------------------------------------*/

uint8_t enc28J60_TransceiveByte(uint8_t data) {
	uint8_t received;
	if (HAL_SPI_TransmitReceive(&hspi1, &data, &received, 1, 1000) == HAL_OK) {
		return received;
	}
	return 0;
}

void enc28J60_EnableChip(void) {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
}

void enc28J60_DisableChip(void) {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
}


uint8_t enc28_readOp(uint8_t oper, uint8_t addr)
{
	uint8_t temp;
	enc28J60_EnableChip();

	enc28J60_TransceiveByte(oper | (addr & ADDR_MASK));

	temp = enc28J60_TransceiveByte(0xFF);
	if (addr & 0x80)
		temp = enc28J60_TransceiveByte(0xFF);
	enc28J60_DisableChip();
	return temp;
}
void enc28_writeOp(uint8_t oper, uint8_t addr, uint8_t data)
{
	enc28J60_EnableChip();
	
	enc28J60_TransceiveByte(oper | (addr & ADDR_MASK));
	enc28J60_TransceiveByte(data);
	enc28J60_DisableChip();
	
}

uint8_t enc28_readReg8(uint8_t addr)
{
	enc28_setBank(addr);
	return enc28_readOp(ENC28J60_READ_CTRL_REG, addr);
}

void enc28_writeReg8(uint8_t addr, uint8_t data)
{
	enc28_setBank(addr);
	enc28_writeOp(ENC28J60_WRITE_CTRL_REG, addr, data);
}

uint16_t enc28_readReg16( uint8_t addr)
{
	return enc28_readReg8(addr) + (enc28_readReg8(addr+1) << 8);
}

void enc28_writeReg16(uint8_t addrL, uint16_t data)
{
	enc28_writeReg8(addrL, data & 0xFF);
	enc28_writeReg8(addrL+1, data >> 8);
}

void enc28_setBank(uint8_t addr)
{
	if ((addr & BANK_MASK) != enc28_bank) 
	{
		enc28_writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_BSEL1|ECON1_BSEL0);
		enc28_bank = addr & BANK_MASK;
    enc28_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, enc28_bank>>5);
	}
}

// 3.3.2 WRITING PHY REGISTERS
void enc28_writephy(uint8_t addr, uint16_t data){
	enc28_writeReg8(MIREGADR, addr);
	enc28_writeReg8(MIWR, data);
	enc28_writeReg8(MIWR+1 , data >> 8);  //?
	while(enc28_readReg8(MISTAT) & MISTAT_BUSY);
}
//3.3.1 READING PHY REGISTERS
uint16_t enc28_readphy(uint8_t addr){
	enc28_writeReg8(MIREGADR, addr);
	enc28_writeReg8(MICMD, MICMD_MIIRD);
	while(enc28_readReg8(MISTAT) & MISTAT_BUSY);
	enc28_writeReg8(MICMD, 0x00);
	return enc28_readReg8(MIRD) + (enc28_readReg8(MIRD+1) << 8);
}

void enc28_writeBuf(uint16_t len, uint8_t* data){
	enc28J60_EnableChip();
	enc28J60_TransceiveByte(ENC28_WRITE_BUF_MEM);
	while (len--)
		enc28J60_TransceiveByte(*data++);
	enc28J60_DisableChip();
}

void enc28_readBuf(uint16_t len, uint8_t *data) {
	enc28J60_EnableChip();
	enc28J60_TransceiveByte(ENC28_READ_BUF_MEM);
	while (len--) {
		*data++ = enc28J60_TransceiveByte(0x00);
	}
	enc28J60_DisableChip();
}

uint16_t enc28_readBuf16() {
	uint16_t result;
	enc28_readBuf(2, (uint8_t*) &result);
	return result;
}

void enc28_init(mac_address mac) {
	// Disable the Chip Select pin
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
	// delay 1ms
	HAL_Delay(1);
	// Perform soft reset to the ENC28J60 module
	// TABLE 4-1: SPI INSTRUCTION SET FOR THE ENC28J60
	enc28_writeOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	// delay 2ms
	HAL_Delay(2);
	// wait untill Clock is ready
	while(!enc28_readOp(ENC28J60_READ_CTRL_REG, ESTAT) & ESTAT_CLKRDY);
	// Initialise RX and TX buffer size
	
	nextPacketPtr = RXSTART_INIT;
	
	enc28_writeReg16(ERXST, RXSTART_INIT);
	enc28_writeReg16(ERXRDPT, RXSTART_INIT);
	
	enc28_writeReg16(ERXND, RXSTOP_INIT);
	
	
	enc28_writeReg16(ETXST, TXSTART_INIT);
	enc28_writeReg16(ETXND, TXSTOP_INIT);
	
	enc28_writeReg16(ERXWRPT, RXSTART_INIT);
	
	
	// Recive buffer filters
	// REGISTER 8-1: ERXFCON: ETHERNET RECEIVE FILTER CONTROL REGISTER
	enc28_writeReg8(ERXFCON, ERXFCON_UCEN | ERXFCON_BCEN | ERXFCON_CRCEN);
	
	// MAC Control Register 1
	// REGISTER 6-1: MACON1: MAC CONTROL REGISTER 1
	enc28_writeReg8(MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS | MACON1_PASSALL);
	
	// MAC Control Register 3
	// REGISTER 6-2: MACON3: MAC CONTROL REGISTER 3
	enc28_writeOp(ENC28J60_BIT_FIELD_SET, MACON3,  MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);
	
	// NON/Back to back gap
	enc28_writeReg16(MAIPG, 0x0C12); //NonBackToBack gap
	enc28_writeReg8(MABBIPG, 0x12); // BackToBack gap
	
	//set Maximum framelenght
	enc28_writeReg16(MAMXFL, MAX_FRAMELEN); // Set Maximum fram length (any bigger will discarded)
	
	// Set the MAC address of device
	enc28_writeReg8(MAADR5, mac.octet[0]);
	enc28_writeReg8(MAADR4, mac.octet[1]);
	enc28_writeReg8(MAADR3, mac.octet[2]);
	enc28_writeReg8(MAADR2, mac.octet[3]);
	enc28_writeReg8(MAADR1, mac.octet[4]);
	enc28_writeReg8(MAADR0, mac.octet[5]);
	
	dataWatch8 = enc28_readReg8(MAADR0);
	dataWatch8 = enc28_readReg8(MAADR1);
	dataWatch8 = enc28_readReg8(MAADR2);
	dataWatch8 = enc28_readReg8(MAADR3);
	dataWatch8 = enc28_readReg8(MAADR4);
	dataWatch8 = enc28_readReg8(MAADR5);
	
	// Initialise PHY layer registers
	enc28_writephy(PHLCON,PHLCON_LED);
	enc28_writephy(PHCON2,PHCON2_HDLDIS);
	
	// Enable Rx interrupt line
	enc28_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
	enc28_writeOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);
	enc28_writeOp(ENC28J60_BIT_FIELD_SET, EIR, EIR_PKTIF);
}

// FIGURE 7-2: SAMPLE TRANSMIT PACKET LAYOUT
void enc28_packetSend(uint16_t len, uint8_t* dataBuf){
	// Check no transmit in progress
	while (enc28_readOp(ENC28J60_READ_CTRL_REG, ECON1) & ECON1_TXRTS) {
		// Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
		if ((enc28_readReg8(EIR) & EIR_TXERIF)) {
			enc28_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
			enc28_writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
		}
	}
		
	// Set the write pointer to start of transmit buffer area
	enc28_writeReg16(EWRPT, TXSTART_INIT);
	// Set the TXND pointer to correspond to the packet size given
	enc28_writeReg16(ETXND, (TXSTART_INIT + len));
	// FIGURE 7-1: FORMAT FOR PER PACKET CONTROL BYTES
	// write per-packet control byte (0xFF)
	enc28_writeOp(ENC28_WRITE_BUF_MEM, 0, 0xFF);
	// copy the packet into the transmit buffer
	enc28_writeBuf(len, dataBuf);
	// send the contents of the transmit buffer onto the network
	enc28_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
	// Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
}



uint16_t enc28_packetReceive(uint16_t maxlen, uint8_t* dataBuf) {
	uint16_t rxstat;
	uint16_t len;

		if (enc28_readReg8(EPKTCNT) == 0) {
		return 0;
	}
		
	enc28_writeReg16(ERDPT, nextPacketPtr);
	nextPacketPtr = enc28_readBuf16();
	
	len = enc28_readBuf16() - 4;
	
	rxstat = enc28_readBuf16();
	
	if (len > maxlen - 1) {
		len = maxlen - 1;
	}
	
	if ((rxstat & 0x80) == 0) {
		// invalid
		len = 0;
	} else {
		// copy the packet from the receive buffer
		enc28_readBuf(len, dataBuf);
	}
	
	enc28_writeReg16(ERXRDPT, nextPacketPtr);
	
	if ((nextPacketPtr - 1 < RXSTART_INIT)|| (nextPacketPtr - 1 > RXSTOP_INIT)) {
		enc28_writeReg16(ERXRDPT, RXSTOP_INIT);
		//ENC28J60_Write(ERXRDPTL, (RXSTOP_INIT)&0xFF);
		//ENC28J60_Write(ERXRDPTH, (RXSTOP_INIT)>>8);
	} else {
		enc28_writeReg16(ERXRDPT, (nextPacketPtr - 1));
		//ENC28J60_Write(ERXRDPTL, (gNextPacketPtr-1)&0xFF);
		//ENC28J60_Write(ERXRDPTH, (gNextPacketPtr-1)>>8);
	}
	
		// decrement the packet counter indicate we are done with this packet
	enc28_writeOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
	return len;
}
