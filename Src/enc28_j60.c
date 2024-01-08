/* Includes ------------------------------------------------------------------*/
#include "enc28_j60.h"


/* Private variables ---------------------------------------------------------*/
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



/**
 * @brief  Transmit and receive a single byte with the ENC28J60 using SPI communication.
 *
 * This function transmits a single byte to the ENC28J60 using SPI communication and receives
 * the response byte. It uses the specified SPI interface (e.g., hspi1) for the communication.
 *
 * @param  data  The byte to be transmitted to the ENC28J60.
 *
 * @note   This function assumes the existence of helper functions like HAL_SPI_TransmitReceive for
 *         handling SPI communication.
 *
 * @retval uint8_t  The byte received from the ENC28J60 in response to the transmitted byte.
 */
uint8_t enc28J60_TransceiveByte(uint8_t data) {
	uint8_t received;
	if (HAL_SPI_TransmitReceive(&hspi1, &data, &received, 1, 10) == HAL_OK) {
		return received;
	}
	return 0;
}

/**
 * @brief  Enable the ENC28J60 chip by setting the Chip Select pin low.
 *
 * This function enables communication with the ENC28J60 chip by setting the Chip Select (CS) pin low.
 * It is typically used before performing read or write operations on the ENC28J60 to initiate communication.
 *
 * @note   This function assumes the existence of helper functions like HAL_GPIO_WritePin for
 *         controlling the Chip Select pin.
 *
 * @retval None
 */
void enc28J60_EnableChip(void) {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
}


/**
 * @brief  Disable the ENC28J60 chip by setting the Chip Select pin high.
 *
 * This function disables communication with the ENC28J60 chip by setting the Chip Select (CS) pin high.
 * It is typically used after performing read or write operations on the ENC28J60 to release the chip.
 *
 * @note   This function assumes the existence of helper functions like HAL_GPIO_WritePin for
 *         controlling the Chip Select pin.
 *
 * @retval None
 */
void enc28J60_DisableChip(void) {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
}


/**
 * @brief  Perform a read operation on a register of the ENC28J60 Ethernet module.
 *
 * This function performs a read operation on a specified register of the ENC28J60 Ethernet module.
 * It enables the ENC28J60 chip, sends the operation code combined with the register address,
 * reads the data from the specified register, and then disables the ENC28J60 chip.
 *
 * @param  oper  The operation code to perform (e.g., ENC28J60_READ_CTRL_REG).
 * @param  addr  The address of the register to read.
 *
 * @note   This function assumes the existence of helper functions like enc28J60_EnableChip,
 *         enc28J60_DisableChip, and enc28J60_TransceiveByte for interfacing with the ENC28J60.
 *
 * @retval uint8_t  The data read from the specified register.
 */
uint8_t enc28_readOp(uint8_t oper, uint8_t addr) {
	uint8_t temp;
	enc28J60_EnableChip();

	enc28J60_TransceiveByte(oper | (addr & ADDR_MASK));

	temp = enc28J60_TransceiveByte(0xFF);
	if (addr & 0x80)
		temp = enc28J60_TransceiveByte(0xFF);
	enc28J60_DisableChip();
	return temp;
}



/**
 * @brief  Perform a write operation on a register of the ENC28J60 Ethernet module.
 *
 * This function performs a write operation on a specified register of the ENC28J60 Ethernet module.
 * It enables the ENC28J60 chip, sends the operation code combined with the register address,
 * writes the provided data, and then disables the ENC28J60 chip.
 *
 * @param  oper  The operation code to perform (e.g., ENC28J60_WRITE_CTRL_REG).
 * @param  addr  The address of the register to write.
 * @param  data  The data to write to the specified register.
 *
 * @note   This function assumes the existence of helper functions like enc28J60_EnableChip,
 *         enc28J60_DisableChip, and enc28J60_TransceiveByte for interfacing with the ENC28J60.
 *
 * @retval None
 */
void enc28_writeOp(uint8_t oper, uint8_t addr, uint8_t data) {
	enc28J60_EnableChip();
	
	enc28J60_TransceiveByte(oper | (addr & ADDR_MASK));
	enc28J60_TransceiveByte(data);
	enc28J60_DisableChip();
	
}


/**
 * @brief  Read an 8-bit value from a register of the ENC28J60 Ethernet module.
 *
 * This function reads an 8-bit value from a specified register of the ENC28J60 Ethernet module.
 * It sets the ENC28J60 bank based on the specified address and reads the data from the register.
 *
 * @param  addr  The address of the register to read.
 *
 * @note   This function assumes the existence of helper functions like enc28_setBank and enc28_readOp
 *         for interfacing with the ENC28J60.
 *
 * @retval uint8_t  The 8-bit value read from the specified register.
 */
uint8_t enc28_readReg8(uint8_t addr) {
	enc28_setBank(addr);
	return enc28_readOp(ENC28J60_READ_CTRL_REG, addr);
}



/**
 * @brief  Write an 8-bit value to a register of the ENC28J60 Ethernet module.
 *
 * This function writes an 8-bit value to a specified register of the ENC28J60 Ethernet module.
 * It sets the ENC28J60 bank based on the specified address, then writes the data to the register.
 *
 * @param  addr  The address of the register to write.
 * @param  data  The 8-bit value to write to the specified register.
 *
 * @note   This function assumes the existence of helper functions like enc28_setBank and enc28_writeOp
 *         for interfacing with the ENC28J60.
 *
 * @retval None
 */
void enc28_writeReg8(uint8_t addr, uint8_t data) {
	enc28_setBank(addr);
	enc28_writeOp(ENC28J60_WRITE_CTRL_REG, addr, data);
}



/**
 * @brief  Read a 16-bit value from two consecutive registers of the ENC28J60 Ethernet module.
 *
 * This function reads a 16-bit value from two consecutive registers of the ENC28J60 Ethernet module.
 * It reads the lower byte from the specified register address and the upper byte from the consecutive
 * register address, then combines them into a 16-bit value.
 *
 * @param  addr  The address of the lower byte register.
 *
 * @note   This function assumes the existence of helper functions like enc28_readReg8 for interfacing
 *         with the ENC28J60.
 *
 * @retval uint16_t  The 16-bit value read from the specified registers.
 */
uint16_t enc28_readReg16( uint8_t addr) {
	return enc28_readReg8(addr) + (enc28_readReg8(addr+1) << 8);
}


/**
 * @brief  Write a 16-bit value to two consecutive registers of the ENC28J60 Ethernet module.
 *
 * This function writes a 16-bit value to two consecutive registers of the ENC28J60 Ethernet module.
 * It writes the lower byte of the data to the specified register address and the upper byte to the
 * consecutive register address.
 *
 * @param  addrL  The address of the lower byte register.
 * @param  data   The 16-bit value to write to the specified registers.
 *
 * @note   This function assumes the existence of helper functions like enc28_writeReg8 for interfacing
 *         with the ENC28J60.
 *
 * @retval None
 */
void enc28_writeReg16(uint8_t addrL, uint16_t data) {
	enc28_writeReg8(addrL, data & 0xFF);
	enc28_writeReg8(addrL+1, data >> 8);
}



/**
 * @brief  Set the ENC28J60 bank for register access.
 *
 * This function sets the ENC28J60 bank for register access based on the specified address. If the bank
 * needs to be changed, it clears the current bank bits in ECON1, updates the bank variable, and sets the
 * new bank bits in ECON1.
 *
 * @param  addr  The address used to determine the new bank.
 *
 * @note   This function assumes the existence of helper functions like enc28_writeOp and enc28_readOp
 *         for interfacing with the ENC28J60.
 *
 * @retval None
 */
void enc28_setBank(uint8_t addr) {
	if ((addr & BANK_MASK) != enc28_bank) 
	{
		enc28_writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_BSEL1|ECON1_BSEL0);
		enc28_bank = addr & BANK_MASK;
    enc28_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, enc28_bank>>5);
	}
}

// 3.3.2 WRITING PHY REGISTERS


/**
 * @brief  Write a value to a PHY register of the ENC28J60 Ethernet module.
 *
 * This function writes a 16-bit value to a specified PHY register of the ENC28J60 Ethernet module.
 * It sets the PHY register address, writes the lower and upper bytes of the data to the register,
 * and waits until the write operation is completed.
 *
 * @param  addr  The address of the PHY register to write.
 * @param  data  The 16-bit value to write to the specified PHY register.
 *
 * @note   This function assumes the existence of helper functions like enc28_writeReg8 and
 *         enc28_readReg8 for interfacing with the ENC28J60.
 *
 * @retval None
 */
void enc28_writephy(uint8_t addr, uint16_t data) {
	enc28_writeReg8(MIREGADR, addr);
	enc28_writeReg8(MIWR, data);
	enc28_writeReg8(MIWR+1 , data >> 8);  //?
	while(enc28_readReg8(MISTAT) & MISTAT_BUSY);
}




//3.3.1 READING PHY REGISTERS


/**
 * @brief  Read a value from a PHY register of the ENC28J60 Ethernet module.
 *
 * This function reads a 16-bit value from a specified PHY register of the ENC28J60 Ethernet module.
 * It sets the PHY register address, initiates a read operation, and waits until the operation is completed.
 * After reading the value, it clears the read command and returns the result.
 *
 * @param  addr  The address of the PHY register to read.
 *
 * @note   This function assumes the existence of helper functions like enc28_writeReg8, enc28_readReg8,
 *         enc28_writeReg16, and enc28_readReg16 for interfacing with the ENC28J60.
 *
 * @retval uint16_t  The 16-bit value read from the specified PHY register.
 */
uint16_t enc28_readphy(uint8_t addr) {
	enc28_writeReg8(MIREGADR, addr);
	enc28_writeReg8(MICMD, MICMD_MIIRD);
	while(enc28_readReg8(MISTAT) & MISTAT_BUSY);
	enc28_writeReg8(MICMD, 0x00);
	return enc28_readReg8(MIRD) + (enc28_readReg8(MIRD+1) << 8);
}




/**
 * @brief  Write data to the ENC28J60 buffer.
 *
 * This function writes a specified number of bytes to the ENC28J60 buffer. It enables the ENC28J60 chip,
 * sends the write buffer memory opcode, and writes the specified number of bytes to the buffer. The data
 * is taken from the provided buffer. Finally, it disables the ENC28J60 chip.
 *
 * @param  len   The number of bytes to write to the ENC28J60 buffer.
 * @param  data  A pointer to the buffer containing the data to be written.
 *
 * @note   This function assumes the existence of helper functions like enc28J60_EnableChip,
 *         enc28J60_DisableChip, and enc28J60_TransceiveByte for interfacing with the ENC28J60.
 *
 * @retval None
 */
void enc28_writeBuf(uint16_t len, uint8_t* data) {
	enc28J60_EnableChip();
	enc28J60_TransceiveByte(ENC28_WRITE_BUF_MEM);
	while (len--)
		enc28J60_TransceiveByte(*data++);
	enc28J60_DisableChip();
}


/**
 * @brief  Read data from the ENC28J60 buffer.
 *
 * This function reads a specified number of bytes from the ENC28J60 buffer. It enables the ENC28J60 chip,
 * sends the read buffer memory opcode, and reads the specified number of bytes from the buffer. The data
 * is stored in the provided buffer. Finally, it disables the ENC28J60 chip.
 *
 * @param  len   The number of bytes to read from the ENC28J60 buffer.
 * @param  data  A pointer to the buffer to store the read data.
 *
 * @note   This function assumes the existence of helper functions like enc28J60_EnableChip,
 *         enc28J60_DisableChip, and enc28J60_TransceiveByte for interfacing with the ENC28J60.
 *
 * @retval None
 */
void enc28_readBuf(uint16_t len, uint8_t *data) {
	enc28J60_EnableChip();
	enc28J60_TransceiveByte(ENC28_READ_BUF_MEM);
	while (len--) {
		*data++ = enc28J60_TransceiveByte(0x00);
	}
	enc28J60_DisableChip();
}


/**
 * @brief  Read a 16-bit value from the ENC28J60 buffer.
 *
 * This function reads a 16-bit value from the ENC28J60 buffer. It internally calls the enc28_readBuf
 * function to read the specified number of bytes (2 bytes in this case) and interprets them as a 16-bit
 * value. The result is then returned.
 *
 * @note   This function assumes the existence of the enc28_readBuf function for reading from the buffer.
 *
 * @retval uint16_t  The 16-bit value read from the ENC28J60 buffer.
 */
uint16_t enc28_readBuf16() {
	uint16_t result;
	enc28_readBuf(2, (uint8_t*) &result);
	return result;
}


/**
 * @brief  Initialize the ENC28J60 Ethernet module.
 *
 * This function performs the initialization sequence for the ENC28J60 Ethernet module. It includes
 * a soft reset, configuration of RX and TX buffers, setting MAC and PHY registers, enabling necessary
 * interrupt lines, and configuring filter controls. The function assumes the existence of helper
 * functions like enc28_writeOp, enc28_writeReg8, enc28_writeReg16, enc28_writephy, enc28_readOp, and
 * enc28_readReg8.
 *
 * @param  mac  The MAC address to be set for the ENC28J60 module.
 *
 * @note   This function uses HAL_Delay for timing delays.
 *
 * @retval None
 */
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
	while(!(enc28_readOp(ENC28J60_READ_CTRL_REG, ESTAT) & ESTAT_CLKRDY));
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


/**
 * @brief  Send a packet using the ENC28J60 Ethernet module.
 *
 * This function sends a packet using the ENC28J60 Ethernet module. It checks whether there is no
 * transmission in progress and resets the transmit logic problem if needed. Then, it sets the write
 * pointer to the start of the transmit buffer area, updates the TXND pointer to correspond to the
 * given packet size, writes the per-packet control byte, copies the packet into the transmit buffer,
 * and finally sends the contents of the transmit buffer onto the network.
 *
 * @param  len      The length of the packet to be sent.
 * @param  dataBuf  A pointer to the buffer containing the packet data.
 *
 * @note   This function assumes the existence of helper functions like enc28_readOp, enc28_writeOp,
 *         enc28_writeReg16, enc28_writeBuf, and enc28_writeReg8 for interfacing with the ENC28J60.
 *
 * @retval None
 */
void enc28_packetSend(uint16_t len, uint8_t* dataBuf) {
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


/**
 * @brief  Receive a packet using the ENC28J60 Ethernet module.
 *
 * This function receives a packet using the ENC28J60 Ethernet module. It checks if there is any
 * pending packet in the receive buffer. If a packet is available, it retrieves the packet length,
 * the receive status, and the packet data. The function then copies the packet data into the provided
 * buffer, updates the receive buffer pointers, and decrements the packet counter. If the received
 * packet is invalid or exceeds the specified maximum length, it discards the packet.
 *
 * @param  maxlen    The maximum length of the buffer to store the received packet.
 * @param  dataBuf   A pointer to the buffer to store the received packet data.
 *
 * @note   This function assumes the existence of helper functions like enc28_readReg8, enc28_readBuf16,
 *         enc28_readBuf, enc28_writeReg16, and enc28_writeOp for interfacing with the ENC28J60.
 *
 * @retval uint16_t  The actual length of the received packet (0 if no packet is available or an error occurs).
 */
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
