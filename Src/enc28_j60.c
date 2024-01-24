/* Includes ------------------------------------------------------------------*/
#include "enc28_j60.h"


/* Private variables ---------------------------------------------------------*/
extern SPI_HandleTypeDef hspi1;
static uint8_t enc28_bank;
static uint16_t nextPacketPtr;

/* Private functions prototypes ---------------------------------------------*/
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

/* Functions -----------------------------------------------------------------*/

/**
 * Überträgt ein Byte über SPI an den ENC28J60 Ethernet-Controller und empfängt gleichzeitig ein Byte.
 *
 * @param data Das zu übertragende Byte.
 * @return Das empfangene Byte, wenn die Übertragung erfolgreich war; andernfalls 0.
 */
uint8_t enc28J60_TransceiveByte(uint8_t data) {
	uint8_t received;
	// Überträgt ein Byte über SPI und empfängt gleichzeitig ein Byte vom ENC28J60 Ethernet-Controller
	if (HAL_SPI_TransmitReceive(&hspi1, &data, &received, 1, 10) == HAL_OK) {
		return received;
	}
	return 0;
}

/**
 * Aktiviert den ENC28J60 Ethernet-Controller, indem der Chip-Auswahl-Pin (CS) auf LOW gesetzt wird.
 */
void enc28J60_EnableChip(void) {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
}

/**
 * Deaktiviert den ENC28J60 Ethernet-Controller, indem der Chip-Auswahl-Pin (CS) auf HIGH gesetzt wird.
 */
static void enc28J60_DisableChip(void) {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
}

/**
 * Führt eine Leseoperation auf dem ENC28J60 Ethernet-Controller durch.
 *
 * @param oper Die Art der Operation (z.B., Read Control Register).
 * @param addr Die Adresse des Registers, das gelesen werden soll.
 * @return Der gelesene Wert aus dem angegebenen Register.
 */
uint8_t enc28_readOp(uint8_t oper, uint8_t addr) {
	uint8_t temp;
	enc28J60_EnableChip();
	// Überträgt die Operation und die Adresse an den ENC28J60 Ethernet-Controller
	enc28J60_TransceiveByte(oper | (addr & ADDR_MASK));
	
	// Liest den Wert aus dem angegebenen Register
	temp = enc28J60_TransceiveByte(0xFF);
	
	// Falls das Register-Flag gesetzt ist, liest einen weiteren Wert aus dem Register
	if (addr & 0x80) {
		temp = enc28J60_TransceiveByte(0xFF);
	}
	enc28J60_DisableChip();
	return temp;
}


/**
 * Führt eine Schreiboperation auf dem ENC28J60 Ethernet-Controller durch.
 *
 * @param oper Die Art der Operation.
 * @param addr Die Adresse des Registers, das geschrieben werden soll.
 * @param data Der zu schreibende Datenwert.
 */
void enc28_writeOp(uint8_t oper, uint8_t addr, uint8_t data) {
	enc28J60_EnableChip();
	// Überträgt die Operation und die Adresse an den ENC28J60 Ethernet-Controller
	enc28J60_TransceiveByte(oper | (addr & ADDR_MASK));
	
	// Überträgt den zu schreibenden Datenwert an den ENC28J60 Ethernet-Controller
	enc28J60_TransceiveByte(data);
	enc28J60_DisableChip();
	
}


/**
 * Liest einen 8-Bit-Wert aus einem Register des ENC28J60 Ethernet-Controllers.
 *
 * @param addr Die Adresse des Registers, das gelesen werden soll.
 * @return Der gelesene 8-Bit-Wert aus dem angegebenen Register.
 */
uint8_t enc28_readReg8(uint8_t addr) {
	// Setzt die Bank des ENC28J60 Ethernet-Controllers entsprechend der Adresse
	enc28_setBank(addr);
	return enc28_readOp(ENC28J60_READ_CTRL_REG, addr);
}


/**
 * Schreibt einen 8-Bit-Wert in ein Register des ENC28J60 Ethernet-Controllers.
 *
 * @param addr Die Adresse des Registers, das beschrieben werden soll.
 * @param data Der zu schreibende 8-Bit-Wert.
 */
void enc28_writeReg8(uint8_t addr, uint8_t data) {
	// Setzt die Bank des ENC28J60 Ethernet-Controllers entsprechend der Adresse
	enc28_setBank(addr);
	enc28_writeOp(ENC28J60_WRITE_CTRL_REG, addr, data);
}

/**
 * Liest einen 16-Bit-Wert aus einem Registerpaar des ENC28J60 Ethernet-Controllers.
 *
 * @param addr Die Adresse des ersten Registers im Registerpaar.
 * @return Der gelesene 16-Bit-Wert aus dem angegebenen Registerpaar.
 */
uint16_t enc28_readReg16( uint8_t addr) {
	// Liest die beiden 8-Bit-Werte aus dem Registerpaar und kombiniert sie zu einem 16-Bit-Wert
	return enc28_readReg8(addr) + (enc28_readReg8(addr+1) << 8);
}


/**
 * Schreibt einen 16-Bit-Wert in ein Registerpaar des ENC28J60 Ethernet-Controllers.
 *
 * @param addrL Die Adresse des ersten Registers im Registerpaar.
 * @param data Der zu schreibende 16-Bit-Wert.
 */
void enc28_writeReg16(uint8_t addrL, uint16_t data) {
	// Schreibt die beiden 8-Bit-Werte des 16-Bit-Werts in das Registerpaar
	enc28_writeReg8(addrL, data & 0xFF);
	enc28_writeReg8(addrL+1, data >> 8);
}


//ECON1: ETHERNET CONTROL REGISTER 1
//FIGURE 3-1: ENC28J60 MEMORY ORGANIZATION

/**
 * Setzt die Bank des ENC28J60 Ethernet-Controllers entsprechend der Adresse des Registers.
 *
 * @param addr Die Adresse des Registers, für das die Bank gesetzt werden soll.
 */
void enc28_setBank(uint8_t addr) {
	// Überprüft, ob die aktuelle Bank nicht mit der Zielbank übereinstimmt
	if ((addr & BANK_MASK) != enc28_bank) 
	{
		// Löscht die BSEL1- und BSEL0-Bits in ECON1, um die aktuelle Bank zu deaktivieren
		enc28_writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_BSEL1|ECON1_BSEL0);
		// Aktualisiert die Variable für die aktuelle Bank
		enc28_bank = addr & BANK_MASK;
		// Setzt die BSEL1- und BSEL0-Bits in ECON1 entsprechend der neuen Bank
    enc28_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, enc28_bank>>5);
	}
}

// 3.3.2 WRITING PHY REGISTERS

/**
 * Schreibt Daten in ein PHY-Register des ENC28J60 Ethernet-Controllers.
 *
 * @param addr Die Adresse des PHY-Registers, in das die Daten geschrieben werden sollen.
 * @param data Die zu schreibenden Daten.
 */
void enc28_writephy(uint8_t addr, uint16_t data) {
	// Setzt die Adresse des zu schreibenden PHY-Registers
	enc28_writeReg8(MIREGADR, addr);
	// Schreibt die unteren 8 Bits der Daten in das MIWR-Register
	enc28_writeReg8(MIWR, data);
	// Schreibt die oberen 8 Bits der Daten in das MIWR+1-Register
	enc28_writeReg8(MIWR+1 , data >> 8);
	// Wartet, bis der MISTAT_BUSY-Bit nicht mehr gesetzt ist, um den Abschluss der Schreiboperation zu überwachen
	while(enc28_readReg8(MISTAT) & MISTAT_BUSY);
}




//3.3.1 READING PHY REGISTERS

/**
 * Liest Daten aus einem PHY-Register des ENC28J60 Ethernet-Controllers.
 *
 * @param addr Die Adresse des PHY-Registers, aus dem die Daten gelesen werden sollen.
 * @return Die gelesenen Daten aus dem angegebenen PHY-Register.
 */
uint16_t enc28_readphy(uint8_t addr) {
	// Setzt die Adresse des zu lesenden PHY-Registers
	enc28_writeReg8(MIREGADR, addr);
	// Startet den PHY-Lesevorgang (MIIRD-Bit setzen)
	enc28_writeReg8(MICMD, MICMD_MIIRD);
	// Wartet, bis der MISTAT_BUSY-Bit nicht mehr gesetzt ist, um den Abschluss des Lesevorgangs zu überwachen
	while(enc28_readReg8(MISTAT) & MISTAT_BUSY);
	// Beendet den PHY-Lesevorgang (MIIRD-Bit zurücksetzen)
	enc28_writeReg8(MICMD, 0x00);
	// Liest die unteren 8 Bits und oberen 8 Bits der Daten aus dem MIRD-Register und kombiniert sie zu einem 16-Bit-Wert
	return enc28_readReg8(MIRD) + (enc28_readReg8(MIRD+1) << 8);
}


/**
 * Schreibt Daten in den Puffer des ENC28J60 Ethernet-Controllers.
 *
 * @param len Die Länge der zu schreibenden Daten.
 * @param data Ein Pointer auf den Puffer mit den zu schreibenden Daten.
 */
void enc28_writeBuf(uint16_t len, uint8_t* data) {
	enc28J60_EnableChip();
	// Überträgt das Schreibkommando (ENC28_WRITE_BUF_MEM) an den ENC28J60 Ethernet-Controller
	enc28J60_TransceiveByte(ENC28_WRITE_BUF_MEM);
	// Überträgt die zu schreibenden Daten an den ENC28J60 Ethernet-Controller
	while (len--) {
		enc28J60_TransceiveByte(*data++);
	}
	enc28J60_DisableChip();
}

/**
 * Liest Daten aus dem Puffer des ENC28J60 Ethernet-Controllers.
 *
 * @param len Die Länge der zu lesenden Daten.
 * @param data Ein Pointer auf den Puffer, in den die gelesenen Daten geschrieben werden sollen.
 */
void enc28_readBuf(uint16_t len, uint8_t *data) {
	enc28J60_EnableChip();
	// Überträgt das Lese-Kommando (ENC28_READ_BUF_MEM) an den ENC28J60 Ethernet-Controller
	enc28J60_TransceiveByte(ENC28_READ_BUF_MEM);
	// Liest die Daten aus dem Puffer und speichert sie im angegebenen Puffer
	while (len--) {
		*data++ = enc28J60_TransceiveByte(0x00);
	}
	enc28J60_DisableChip();
}

/**
 * Liest einen 16-Bit-Wert aus dem Puffer des ENC28J60 Ethernet-Controllers.
 *
 * @return Der gelesene 16-Bit-Wert aus dem Puffer.
 */
uint16_t enc28_readBuf16() {
	uint16_t result;
	// Liest einen 16-Bit-Wert aus dem Puffer und speichert ihn in der result-Variablen
	enc28_readBuf(2, (uint8_t*) &result);
	return result;
}

/**
 * Initialisiert den ENC28J60 Ethernet-Controller mit den angegebenen Konfigurationen.
 *
 * @param mac Die MAC-Adresse des Geräts.
 */
void enc28_init(mac_address mac) {
	
	enc28J60_DisableChip();
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);

	HAL_Delay(1);
	// TABLE 4-1: SPI INSTRUCTION SET FOR THE ENC28J60
	// Führt einen Soft-Reset des ENC28J60-Moduls durch
	enc28_writeOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	// delay 2ms
	HAL_Delay(2);
	// Wartet, bis die Clock bereit ist
	while(!(enc28_readOp(ENC28J60_READ_CTRL_REG, ESTAT) & ESTAT_CLKRDY));
	
	 // Initialisiert die Größe der RX- und TX-Puffer
	nextPacketPtr = RXSTART_INIT;
	
	enc28_writeReg16(ERXST, RXSTART_INIT);
	enc28_writeReg16(ERXRDPT, RXSTART_INIT);
	
	enc28_writeReg16(ERXND, RXSTOP_INIT);
	
	
	enc28_writeReg16(ETXST, TXSTART_INIT);
	enc28_writeReg16(ETXND, TXSTOP_INIT);
	
	enc28_writeReg16(ERXWRPT, RXSTART_INIT);
	
	//6.5 MAC Initialization Settings
	
	// Empfangs-Puffer-Filter
	// REGISTER 8-1: ERXFCON: ETHERNET RECEIVE FILTER CONTROL REGISTER
	enc28_writeReg8(ERXFCON, ERXFCON_UCEN | ERXFCON_BCEN | ERXFCON_CRCEN);
	// ERXFCON_UCEN, Pakete, deren Zieladresse nicht mit der lokalen MAC-Adresse übereinstimmt, werden verworfen.
  // ERXFCON_BCEN, Pakete mit der Zieladresse Broadcast-MAC-Adresse werden akzeptiert.
  // ERXFCON_CRCEN, Alle Pakete mit ungültiger CRC werden verworfen.
	
	// MAC Control Register 1
	// REGISTER 6-1: MACON1: MAC CONTROL REGISTER 1
	enc28_writeReg8(MACON1, MACON1_MARXEN | MACON1_RXPAUS | MACON1_PASSALL);
	
	// MAC Control Register 3
	// REGISTER 6-2: MACON3: MAC CONTROL REGISTER 3
	enc28_writeOp(ENC28J60_BIT_FIELD_SET, MACON3,  MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);
	
	// Non-Back-to-Back Gap
	enc28_writeReg16(MAIPG, 0x0C12); // Non-Back-to-Back Gap
	enc28_writeReg8(MABBIPG, 0x12); // Non-Back-to-Back Gap
	
	// Setzt die maximale Rahmengröße
	enc28_writeReg16(MAMXFL, MAX_FRAMELEN);
	
	// Setzt die MAC-Adresse des Geräts
	enc28_writeReg8(MAADR5, mac.octet[0]);
	enc28_writeReg8(MAADR4, mac.octet[1]);
	enc28_writeReg8(MAADR3, mac.octet[2]);
	enc28_writeReg8(MAADR2, mac.octet[3]);
	enc28_writeReg8(MAADR1, mac.octet[4]);
	enc28_writeReg8(MAADR0, mac.octet[5]);
	
	// Initialisiert die PHY-Layer-Register
	enc28_writephy(PHLCON,PHLCON_LED);
	enc28_writephy(PHCON2,PHCON2_HDLDIS);
	
	// Aktiviert die Rx-Interrupt-Leitung
	enc28_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
	enc28_writeOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);
	enc28_writeOp(ENC28J60_BIT_FIELD_SET, EIR, EIR_PKTIF);
}




// FIGURE 7-2: SAMPLE TRANSMIT PACKET LAYOUT

/**
 * Sendet ein Paket über den ENC28J60 Ethernet-Controller.
 *
 * @param len Die Länge des zu sendenden Pakets.
 * @param dataBuf Ein Pointer auf den Puffer mit den zu sendenden Daten.
 */
void enc28_packetSend(uint16_t len, uint8_t* dataBuf) {

	while (enc28_readOp(ENC28J60_READ_CTRL_REG, ECON1) & ECON1_TXRTS) {
		// Setzt das Übertragungs-Logic-Problem zurück
		if ((enc28_readReg8(EIR) & EIR_TXERIF)) {
			enc28_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
			enc28_writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
		}
	}
		
	// Setzt den Pointer auf den Anfang des Übertragungspufferbereichs
	enc28_writeReg16(EWRPT, TXSTART_INIT);
	 // Setzt den TXND-Pointer so, dass er der gegebenen Paketgröße entspricht
	enc28_writeReg16(ETXND, (TXSTART_INIT + len));
	
	// FIGURE 7-1: FORMAT FOR PER PACKET CONTROL BYTES
	// Schreibt das per-Paket-Kontrollbyte (0xFF)
	enc28_writeOp(ENC28_WRITE_BUF_MEM, 0, 0xFF);
	// Kopiert das Paket in den Übertragungspuffer
	enc28_writeBuf(len, dataBuf);
	// Sendet den Inhalt des Übertragungspuffers ins Netzwerk
	enc28_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}

/**
 * Empfängt ein Paket über den ENC28J60 Ethernet-Controller und speichert es im angegebenen Puffer.
 *
 * @param maxlen Die maximale Länge des zu empfangenden Pakets.
 * @param dataBuf Ein Pointer auf den Puffer, in dem das empfangene Paket gespeichert wird.
 * @return Die tatsächliche Länge des empfangenen Pakets.
 */
uint16_t enc28_packetReceive(uint16_t maxlen, uint8_t* dataBuf) {
	uint16_t rxstat;
	uint16_t len;
	// Überprüft, ob keine Pakete im Puffer vorhanden sind
	if (enc28_readReg8(EPKTCNT) == 0) {
		return 0;
	}
	// Setzt den Lesepointer für den nächsten Puffer
	enc28_writeReg16(ERDPT, nextPacketPtr);
	nextPacketPtr = enc28_readBuf16();
	
	// Liest die Länge des empfangenen Pakets und subtrahiert 4 Bytes (CRC)
	len = enc28_readBuf16() - 4;
	
	// Liest den Status des empfangenen Pakets
	rxstat = enc28_readBuf16();
	
	// Begrenzt die Länge auf die maximale Länge minus 1 (für Nullterminierung)
	if (len > maxlen - 1) {
		len = maxlen - 1;
	}
	
	// Überprüft, ob das Paket ungültig ist
	if ((rxstat & 0x80) == 0) {
		// Ungültig
		len = 0;
	} else {
		// Kopiert das Paket aus dem Empfangspuffer in den angegebenen Puffer
		enc28_readBuf(len, dataBuf);
	}
	// Setzt den Lesepointer für den Empfangspuffer zurück
	enc28_writeReg16(ERXRDPT, nextPacketPtr);
	
	// Überprüft, ob der nächste Punkt außerhalb des gültigen Bereichs liegt
	if ((nextPacketPtr - 1 < RXSTART_INIT)|| (nextPacketPtr - 1 > RXSTOP_INIT)) {
		enc28_writeReg16(ERXRDPT, RXSTOP_INIT);
	} else {
		enc28_writeReg16(ERXRDPT, (nextPacketPtr - 1));
	}
	
	// Dekrementiert den Paketzähler, um anzuzeigen, dass das Paket verarbeitet wurde
	enc28_writeOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
	// Gibt die Länge des empfangenen Pakets zurück
	return len;
}
