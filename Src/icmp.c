/* Includes ------------------------------------------------------------------*/
#include "icmp.h"

static ip_address *my_ip_addr;
static ip_address *my_subnet_addr;
static ip_address *my_gateway_addr;
static mac_address my_mac;

/* Private functions prototypes ---------------------------------------------*/
int handle_icmp(const uint8_t* buf, uint16_t length);
static uint16_t calculate_checksum(const void* data, size_t length);
void send_icmp_rep(ip_address target_ip, uint16_t ident, uint16_t seq, uint8_t ttl);
void get_icmp_req(const uint8_t* buf);

/* Functions -----------------------------------------------------------------*/

/**
 * Initialisiert das Internet Control Message Protocol (ICMP) für die Verarbeitung von IPv4-Paketen.
 *
 * @param src_ip Die lokale IP-Adresse des Geräts.
 * @param my_subnet Die Subnetzmaske des Geräts.
 * @param my_gateway Die Gateway-IP-Adresse des Geräts.
 * @param src_mac Die MAC-Adresse des Geräts.
 */
void icmp_init(ip_address* src_ip, ip_address *my_subnet,ip_address *my_gateway, mac_address src_mac) {
	// Fügt ICMP als unterstütztes Layer-3-Protokoll hinzu und verknüpft es mit der Handler-Funktion
	ipv4_add_type(ICMP_TYPE, &handle_icmp);
	
	// Setzt die lokale IP-Adresse und MAC-Adresse für die ICMP-Paketverarbeitung
	my_ip_addr = src_ip;
	my_subnet_addr = my_subnet;
	my_gateway_addr = my_gateway;
	my_mac = src_mac;
}


/**
 * Sendet ein ICMP (Internet Control Message Protocol) Anfragepaket an die angegebene Ziel-IP-Adresse.
 *
 * @param target_ip Die IP-Adresse des Zielgeräts, an das die ICMP-Anfrage gesendet werden soll.
 *
 * @note Wird in diesem System nicht benötigt.
 */
void send_icmp_req(ip_address target_ip){
	// Struktur erstellen, um die Schichten des ICMP-Anfragepakets zu halten
	struct package {
		mac_header mac_header;
		ipv4_header ipv4_header;
		icmp_package icmp_package;
	};
	
	struct package req;
		
	ip_address ip_dst = target_ip;
	
	// Überprüfen, ob die Ziel-IP im gleichen Netzwerk ist; andernfalls Gateway verwenden
	if(!isInSameNetwork(my_ip_addr, &ip_dst, my_subnet_addr)){ip_dst = *my_gateway_addr;};
	
	// MAC-Adresse des Ziel-IP abrufen
	mac_address dest_mac;
	if(get_mac(ip_dst, &dest_mac) != 1){
		// MAC-Adresse konnte nicht abgerufen werden; ohne Senden der ICMP-Antwort zurückkehren
		return;
	}
	
	
	// Layer 2
	req.mac_header.dest_mac = dest_mac;
	req.mac_header.src_mac = my_mac;
	req.mac_header.ether_type = IPV4_TYPE;
	
	// Layer 3 (IPv4)
	req.ipv4_header.version_length = IPV4_VERSION;
	req.ipv4_header.service_field = 0x00;
	req.ipv4_header.total_length = swapEndian16(sizeof(req) - sizeof(req.mac_header));
	req.ipv4_header.ident = calculate_next_id();
	req.ipv4_header.flags = 0x00;
	req.ipv4_header.ttl = 0xff;
	req.ipv4_header.prtcl = 0x01; // ICMP-Protokoll
	req.ipv4_header.src = *my_ip_addr;
	req.ipv4_header.dst = target_ip;
	req.ipv4_header.header_checksum = calculate_checksum(&req.ipv4_header, sizeof(req.ipv4_header));
	
	// ICMP-Paket
	req.icmp_package.type = ICMP_REQ;
	req.icmp_package.code = ICMP_CODE;
	req.icmp_package.ident = ICMP_IDENT;
	req.icmp_package.seq = ICMP_SEQ;
	req.icmp_package.data = (payload) {0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69};
	req.icmp_package.checksum = calculate_checksum(&req.icmp_package, sizeof(req.icmp_package));
		
	// ICMP-Anfrage senden
	enc28_packetSend(sizeof(req), (uint8_t*)&req);
}


/**
 * Sendet ein ICMP (Internet Control Message Protocol) Antwortpaket als Reaktion auf eine ICMP-Anfrage an die angegebene Ziel-IP-Adresse.
 *
 * @param target_ip Die IP-Adresse des Zielgeräts, an das die ICMP-Antwort gesendet werden soll.
 * @param ident Der Identifikator, der von der empfangenen ICMP-Echo-Anfrage übernommen wird.
 * @param seq Die Sequenznummer, die von der empfangenen ICMP-Echo-Anfrage übernommen wird.
 * @param ttl Die Time-to-Live (TTL)-Wert, der für das ICMP-Antwortpaket festgelegt werden soll.
 */
void send_icmp_rep(ip_address target_ip, uint16_t ident, uint16_t seq, uint8_t ttl){
	// Struktur erstellen, um die Schichten des ICMP-Antwortpakets zu halten
	struct package {
		mac_header mac_header;
		ipv4_header ipv4_header;
		icmp_package icmp_package;
	};
	
	struct package rep;
	
	ip_address ip_dst = target_ip;
	
	// Überprüfen, ob die Ziel-IP im gleichen Netzwerk ist; andernfalls Gateway verwenden
	if(!isInSameNetwork(my_ip_addr, &ip_dst, my_subnet_addr)){ip_dst = *my_gateway_addr;};
	
	// MAC-Adresse des Ziel-IP abrufen
	mac_address dest_mac;
	if(get_mac(ip_dst, &dest_mac) != 1){
		// MAC-Adresse konnte nicht abgerufen werden; ohne Senden der ICMP-Antwort zurückkehren
		return;
	}
	// Layer 2 - MAC-Header
	rep.mac_header.dest_mac = dest_mac;
	rep.mac_header.src_mac = my_mac;
	rep.mac_header.ether_type = IPV4_TYPE;
	// Layer 3 (IPv4)
	rep.ipv4_header.version_length = IPV4_VERSION;
	rep.ipv4_header.service_field = 0x00;
	rep.ipv4_header.total_length = swapEndian16(sizeof(rep) - sizeof(rep.mac_header));
	rep.ipv4_header.ident = calculate_next_id();
	rep.ipv4_header.flags = 0x00;
	rep.ipv4_header.ttl = ttl /2;
	rep.ipv4_header.prtcl = 0x01;
	rep.ipv4_header.src = *my_ip_addr;
	rep.ipv4_header.dst = target_ip;
	rep.ipv4_header.header_checksum = calculate_checksum(&rep.ipv4_header, sizeof(rep.ipv4_header));
	// Layer 4 (ICMP)
	rep.icmp_package.type = ICMP_REPLY;
	rep.icmp_package.code = ICMP_CODE;
	rep.icmp_package.ident = ident; // Den Identifikator von der empfangenen ICMP-Echo-Anfrage übernehmen
	rep.icmp_package.seq = seq; // Die Sequenznummer von der empfangenen ICMP-Echo-Anfrage übernehmen
	rep.icmp_package.data = (payload) {0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69};
	rep.icmp_package.checksum = calculate_checksum(&rep.icmp_package, sizeof(rep.icmp_package));
	// ICMP-Antwort senden
	enc28_packetSend(sizeof(rep), (uint8_t*)&rep);
}


/**
 * Verarbeitet eine eingehende ICMP (Internet Control Message Protocol) Echo-Anforderung und sendet eine ICMP Echo-Antwort.
 *
 * @param buf Der Puffer, der die empfangenen ICMP-Anforderungsdaten enthält.
 */
void get_icmp_req(const uint8_t* buf){
			// Ziel-IP-Adresse aus dem empfangenen Paket extrahiere
			ip_address dest_ip;
			dest_ip.octet[0] = buf[26];
			dest_ip.octet[1] = buf[27];
			dest_ip.octet[2] = buf[28];
			dest_ip.octet[3] = buf[29];
	
			 // TTL (Time-to-Live) aus dem empfangenen Paket extrahieren
			uint8_t ttl = buf[22];
	
			// Identifikator und Sequenznummer aus dem empfangenen Paket extrahieren
			icmp_package pkg;
			pkg.ident = (buf[38]  + (buf[39] << 8));
			pkg.seq = (buf[40]  + (buf[41] << 8));
			 // ICMP-Antwort senden
			send_icmp_rep(dest_ip, pkg.ident, pkg.seq, ttl);
			return;
}

/**
 * Verarbeitet eingehende ICMP (Internet Control Message Protocol) Pakete basierend auf dem Typ des Pakets.
 *
 * @param buf Pointer auf den empfangenen Netzwerkpaket-Puffer
 * @param length Die Länge der empfangenen Daten im Puffer.
 * @return Gibt 0 zurück.
 */
int handle_icmp(const uint8_t* buf, uint16_t length){
		// Überprüfen den Typ des ICMP-Pakets
		if (buf[34] == ICMP_REQ){get_icmp_req(buf);}
		if (buf[34] == ICMP_REPLY){return 0;} //(Nicht Implementiert)
		return 0;
}

/**
 * Berechnet die 16-Bit-Prüfsumme (Checksum) für die übergebenen Daten.
 *
 * @param data Ein Pointer auf die Daten, für die die Prüfsumme berechnet werden soll.
 * @param length Die Länge der Daten in Bytes.
 * @return Die berechnete 16-Bit-Prüfsumme.
 */
static uint16_t calculate_checksum(const void* data, size_t length) {
    const uint16_t* p = data;
    uint32_t sum = 0;

    // Summation der 16-Bit-Werte im Datenbereich
    while (length > 1) {
        sum += *p++;
        length -= 2;
    }

    // Falls die Länge ungerade ist, füge das letzte Byte hinzu
    if (length > 0) {
        sum += *(uint8_t*)p;
    }

    // Füge die Überträge hinzu
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    // Invertiere die Bits, um die Prüfsumme zu erhalten
    return (uint16_t)~sum;
}