/* Includes ------------------------------------------------------------------*/
#include "eth.h"

/* Private variables ---------------------------------------------------------*/
static ether_types* types;

/* Functions -----------------------------------------------------------------*/

/**
 * Initialisiert die Ethernet-Schicht, indem der Pointer auf die Struktur der Layer-2-Protokolltypen
 * gesetzt wird und der Index auf Null gesetzt wird, um anzuzeigen, dass noch keine Layer-2-Protokolle hinzugefügt wurden.
 *
 * @param types_addr Ein Pointer auf die Struktur der Layer-2-Protokolltypen.
 */
void eth_init(ether_types* types_addr) {
	// Überprüft, ob der bereitgestellte Pointer nicht NULL ist
	if (types_addr != NULL) {
		// Setzt den Pointer auf die Struktur der Layer-2-Protokolltypen
		types = types_addr;
		
		types->idx = 0;
	}
}

/**
 * Fügt einen neuen Layer-2-Protokolltyp zur Ethernet-Schicht hinzu.
 * Setzt die ether_type- und func-Felder der Struktur der Layer-2-Protokolltypen
 * an der aktuellen Indexposition und erhöht den Index, wenn im Array noch Platz vorhanden ist.
 *
 * @param type Der EtherType des hinzuzufügenden Protokolls.
 * @param func Ein Pointer auf die Funktion, die für das hinzugefügte Protokoll aufgerufen werden soll.
 */
void eth_add_type(uint16_t type, void* func){
	 // Setzt die ether_type- und func-Felder der Struktur der Layer-2-Protokolltypen an der aktuellen Indexposition
	types->types[types->idx].ether_type = type;
	types->types[types->idx].func = func;
	
	 // Erhöht den Index, wenn im Typenarray noch Platz vorhanden ist
	if (types->idx + 1 < ETHER_TYPE_SIZE){
		types->idx = types->idx + 1;
	}
}

/**
 * Verarbeitet Ethernet-Pakete, indem der EtherType aus dem Ethernet-Paketheader extrahiert wird
 * und dann die registrierten Protokolltypen durchgegangen werden, um die entsprechende Verarbeitungsfunktion aufzurufen.
 *
 * @param buf Ein Pointer auf den Puffer, der das empfangene Ethernet-Paket enthält.
 * @param length Die Länge des empfangenen Ethernet-Pakets.
 * @return 0, wenn die Verarbeitung erfolgreich war; 1, wenn kein passender Protokolltyp und Handler gefunden wurde.
 */
int eth_handler(const uint8_t* buf, uint16_t length) {
	// Extrahiert den EtherType aus dem Ethernet-Paketheader
	uint16_t typ = (buf[12]  + (buf[13] << 8));
	
	// Durchläuft die registrierten Protokolltypen in der ether_types-Struktur
	for(uint8_t i = 0; i < ETHER_TYPE_SIZE; i++) { 
		// Überprüft, ob der EtherType des Pakets mit einem registrierten Protokolltyp übereinstimmt
		if (typ == types->types[i].ether_type){
			// Ruft die entsprechende Verarbeitungsfunktion für den übereinstimmenden EtherType auf
			return types->types[i].func(buf, length);
		}
	}
	return 1;
}

/**
 * Überprüft, ob die gegebene Ziel-IP-Adresse im selben Netzwerk wie die lokale IP-Adresse liegt.
 *
 * @param my_ip Ein Pointer auf die lokale IP-Adresse.
 * @param dst_ip Ein Pointer auf die Ziel-IP-Adresse, die überprüft werden soll.
 * @param sub_netmask Ein Pointer auf die Subnetzmaske des Netzwerks.
 * @return 1, wenn die Ziel-IP-Adresse im selben Netzwerk liegt; 0, wenn nicht.
 */
int isInSameNetwork(ip_address* my_ip, ip_address* dst_ip, ip_address* sub_netmask) {
    // Überprüfen, ob die IP-Adresse im selben Netzwerk liegt
    for (int i = 0; i < 4; i++) {
        if ((my_ip->octet[i] & sub_netmask->octet[i]) != (dst_ip->octet[i] & sub_netmask->octet[i])) {
            return 0; // Nicht im selben Netzwerk
        }
    }
    return 1; // Im selben Netzwerk
}

/**
 * Vertauscht die Byte-Reihenfolge eines 32-Bit-Werts (Endian-Swap).
 *
 * @param value Der 32-Bit-Wert, dessen Byte-Reihenfolge getauscht werden soll.
 * @return Der 32-Bit-Wert mit vertauschter Byte-Reihenfolge.
 */
uint32_t swapEndian32(uint32_t value) {
    return ((value & 0xFF000000) >> 24) |
           ((value & 0x00FF0000) >> 8) |
           ((value & 0x0000FF00) << 8) |
           ((value & 0x000000FF) << 24);
}

/**
 * Vertauscht die Byte-Reihenfolge eines 16-Bit-Werts (Endian-Swap).
 *
 * @param value Der 16-Bit-Wert, dessen Byte-Reihenfolge getauscht werden soll.
 * @return Der 16-Bit-Wert mit vertauschter Byte-Reihenfolge.
 */
uint16_t swapEndian16(uint16_t value) {
    return ((value & 0xFF00) >> 8) |
           ((value & 0x00FF) << 8);
}