/* Includes ------------------------------------------------------------------*/
#include "udp.h"

/* Private variables ---------------------------------------------------------*/
static ip_address my_ip;
static mac_address my_mac;
static udp_serivces* serivces;

/* Private functions prototypes ---------------------------------------------*/
int handle_udp(const uint8_t* buf, uint16_t length);

/* Functions -----------------------------------------------------------------*/

/**
 * Initialisiert den UDP-Layer mit den angegebenen Parametern.
 *
 * @param services_addr Ein Pointer auf die Struktur, die Informationen über die UDP-Dienste enthält.
 * @param src_ip Die Quell-IP-Adresse für den UDP-Layer.
 * @param src_mac Die Quell-MAC-Adresse für den UDP-Layer.
 */
void udp_init(udp_serivces* serivces_addr, ip_address src_ip, mac_address src_mac) {
	// Überprüfe, ob der Pointer auf die UDP-Services-Struktur nicht NULL ist
	if (serivces_addr != NULL) {
		// Füge UDP zu den Protokolltypen der IPv4-Layer hinzu und verknüpfe es mit der handle_udp-Funktion
		ipv4_add_type(UDP_TYPE, &handle_udp); 
		// Set the pointer to the UDP services structure
		serivces = serivces_addr;
		// Set the index to zero, assuming no protocols have been added yet
		serivces->idx = 0;
	}
	// Setze die Quell-IP- und MAC-Adressen für den UDP-Layer
	my_ip = src_ip;
	my_mac = src_mac;
}

/**
 * Fügt einen UDP-Protokolltyp mit einem lokalen Port und einer zugehörigen Funktion hinzu.
 *
 * @param lport Der lokale Port, der dem UDP-Protokolltyp zugeordnet ist.
 * @param func Ein Pointer auf die Funktion, die mit dem UDP-Protokolltyp verknüpft ist.
 */
void udp_add_type(uint16_t lport, void* func){
	// Setze den lokalen UDP-Port und die zugehörige Funktion in der UDP-Services-Struktur
	serivces->serivces[serivces->idx].lport = lport;
	serivces->serivces[serivces->idx].func = func;
	
	if (serivces->idx + 1 < UDP_SERVICES_SIZE){
		serivces->idx = serivces->idx + 1;
	}
}



/**
 * Verarbeitet ein eingehendes UDP-Paket und ruft die entsprechende registrierte Funktion für den lokalen Port auf.
 *
 * @param buf Ein Pointer auf den UDP-Paketdatenbereich.
 * @param length Die Länge des UDP-Pakets.
 * @return 0, wenn die Verarbeitung erfolgreich war; andernfalls 1.
 */
int handle_udp(const uint8_t* buf, uint16_t length) {
	// Extrahiert den UDP-Zielport aus dem Paket
	uint16_t lport = (buf[36]  + (buf[37] << 8));
	
	// Durchläuft die UDP-Services, um eine Übereinstimmung für den lokalen Port zu finden
	for(uint8_t i = 0; i < UDP_SERVICES_SIZE; i++) { 
		if (lport == serivces->serivces[i].lport){
			
			// Ruft die Handler-Funktion für den identifizierten lokalen Port auf
			return serivces->serivces[i].func(buf, length);
		}
	}
	// Kein passender Service für den UDP-Zielport gefunden
	return 1;
}

/**
 * Berechnet die UDP-Prüfsumme unter Verwendung des Pseudo-Headers, des UDP-Headers und der Payload.
 *
 * @param ip_header Ein Pointer auf den IPv4-Header.
 * @param udp_header Ein Pointer auf den UDP-Header.
 * @param payload Ein Pointer auf die Payload.
 * @param payload_size Die Größe der Payload in Bytes.
 * @return Die berechnete UDP-Prüfsumme.
 */
uint16_t udp_checksum(ipv4_header *ip_header, udp_header *udp_header, uint8_t *payload, size_t payload_size) {
    // Pseudoheader für die Checksummenberechnung
    struct {
        uint32_t src;
        uint32_t dst;
        uint8_t reserved;
        uint8_t protocol;
        uint16_t udp_length;
    } pseudoheader;

		// Fülle den Pseudo-Header mit Informationen (SRC & DST IPv4)
    pseudoheader.src = (uint32_t)(ip_header->src.octet[0]) << 24 |
                      (uint32_t)(ip_header->src.octet[1]) << 16 |
                      (uint32_t)(ip_header->src.octet[2]) << 8 |
                      (uint32_t)(ip_header->src.octet[3]);

    pseudoheader.dst = (uint32_t)(ip_header->dst.octet[0]) << 24 |
                      (uint32_t)(ip_header->dst.octet[1]) << 16 |
                      (uint32_t)(ip_header->dst.octet[2]) << 8 |
                      (uint32_t)(ip_header->dst.octet[3]);

    pseudoheader.reserved = 0;
    pseudoheader.protocol = ip_header->prtcl;
    pseudoheader.udp_length = (sizeof(udp_header) + payload_size);

    // Checksummenberechnung
    uint32_t sum = 0;

    // Pseudoheader
    sum += (pseudoheader.src >> 16) + (pseudoheader.src & 0xFFFF);
    sum += (pseudoheader.dst >> 16) + (pseudoheader.dst & 0xFFFF);
    sum += pseudoheader.reserved;
    sum += pseudoheader.protocol;
    sum += pseudoheader.udp_length;

    // UDP-Header
    sum += (udp_header->src >> 8) + ((udp_header->src & 0xFF) << 8);
    sum += (udp_header->dest >> 8) + ((udp_header->dest & 0xFF) << 8);
    sum += (udp_header->length >> 8) + ((udp_header->length & 0xFF) << 8);
    sum += udp_header->checksum;

    // Payload
    for (size_t i = 0; i < payload_size / 2; i++) {
        sum += (payload[i * 2] << 8) + payload[i * 2 + 1];
        while (sum >> 16) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }
    }

    // Abschließende Bitumkehr und Rückgabe der Checksumme
    return (uint16_t)~sum - 4; // (CRC)
}

