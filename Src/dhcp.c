/* Includes ------------------------------------------------------------------*/
#include "dhcp.h"

/* Private variables ---------------------------------------------------------*/
static ip_address *my_ip_addr;
static ip_address *my_subnet_addr;
static ip_address *my_gateway_addr;
static ip_address *my_dhcp_server_addr;
static uint8_t *dhcp_rdy_addr;
static mac_address my_mac;

/* Private functions prototypes ---------------------------------------------*/
int handle_dhcp(const uint8_t* buf, uint16_t length);
static uint16_t calculate_checksum(const void* data, size_t length);
uint32_t rand(uint32_t* seed);
uint32_t generateID();
void extract_option_1(const uint8_t *buffer, uint16_t length, option_1 *result);
void extract_option_3(const uint8_t *buffer, uint16_t length, option_3 *result);
void extract_option_53(const uint8_t *buffer, uint16_t length, option_53 *result);
void extract_option_54(const uint8_t *buffer, uint16_t length, option_54 *result);
void send_dhcp_req();
void get_dhcp_offer(const uint8_t* buf, uint16_t length);
void get_dhcp_ack(const uint8_t* buf, uint16_t length, uint8_t* dhcp_rdy);

/* Functions -----------------------------------------------------------------*/

/**
 * Initialisiert das DHCP-Modul mit den erforderlichen Parametern und fügt den UDP-Service hinzu.
 *
 * @param my_ip Ein Pointer auf die lokale IP-Adresse.
 * @param my_subnet Ein Pointer auf die Subnetzmaske.
 * @param my_gateway Ein Pointer auf das Standardgateway.
 * @param my_dhcp_server Ein Pointer auf den DHCP-Server.
 * @param dhcp_rdy Ein Pointer auf den Status des DHCP-Dienstes.
 * @param src_mac Die Quell-MAC-Adresse.
 */
void dhcp_init(ip_address *my_ip, ip_address *my_subnet, ip_address *my_gateway, ip_address *my_dhcp_server, uint8_t *dhcp_rdy, mac_address src_mac) {
	// Fügt DHCP als unterstütztes UDP-Protokoll hinzu und verknüpft es mit der Handler-Funktion
	udp_add_type(DHCP_LPORT,  &handle_dhcp);
	
	// Setze die lokalen IP-Adresse und MAC-Adresse für die DHCP-Paketverarbeitung
	my_ip_addr = my_ip;
	my_subnet_addr = my_subnet;
	my_gateway_addr = my_gateway;
	my_dhcp_server_addr = my_dhcp_server;
	dhcp_rdy_addr = dhcp_rdy;
	my_mac = src_mac;
}


/**
 * Generiert eine Zufallszahl unter Verwendung eines linearen Kongruenzgenerators.
 *
 * @param seed Ein Pointer auf den aktuellen Zustand des Generators.
 * @return Die generierte 32-Bit-Zufallszahl.
 */
uint32_t rand(uint32_t* seed) {
    // Lineare Kongruenz für die Zufallszahlengenerierung
    *seed = (*seed * 1133769420 + 12345) & 0xFFFFFFFF;
    return (*seed >> 16) & 0x7FFF;
}


/**
 * Generiert eine eindeutige 32-Bit-ID unter Verwendung des linearen Kongruenzgenerators.
 *
 * @return Die generierte 32-Bit-ID.
 */
uint32_t generateID() {
    // Seed mit der Adresse einer lokalen Variable initialisieren
    uint32_t seed = (uint32_t)&seed;

    // Zufällige 32-Bit-Zahl generieren
    uint32_t id = rand(&seed);

    return id;
}



/**
 * Extrahiert die Informationen aus Option 1 eines DHCP-Paketes.
 *
 * @param buffer Der Puffer, der das DHCP-Paket enthält.
 * @param length Die Länge des Puffers.
 * @param result Ein Pointer auf eine Option-1-Struktur, in der das Ergebnis gespeichert wird.
 *               Wenn die Option nicht gefunden wird, wird `result->option_type` auf 0 gesetzt.
 */
void extract_option_1(const uint8_t *buffer, uint16_t length, option_1 *result) {
		// Startoffset für die DHCP-Optionen nach dem DHCP-Header
    uint16_t offset = sizeof(mac_header) + sizeof(ipv4_header) + sizeof(udp_header) + sizeof(dhcp_header);
		// Durchlaufe die Optionen im DHCP-Paket
    while (offset < length) {
        uint8_t option_type = buffer[offset];
        uint8_t option_length = buffer[offset + 1];

				// Überprüfe, ob es sich um Option-1 (Subnetzmaske) handelt
        if (option_type == 1 && (option_length + 2) == sizeof(option_1)) {
						// Option-1 gefunden, speichere die Informationen in der Ergebnisstruktur
            result->option_type = option_type;
            result->length = option_length;
            result->subnet_mask = *(ip_address*)(buffer + offset + 2);
            return;
        }

        // Zum nächsten Optionsfeld bewegen
        offset += 2 + option_length;
    }

    // Option nicht gefunden
    result->option_type = 0;
    result->length = 0;
    result->subnet_mask = (ip_address) {0};
}

/**
 * Extrahiert die Informationen aus Option 3 eines DHCP-Paketes.
 *
 * @param buffer Der Puffer, der das DHCP-Paket enthält.
 * @param length Die Länge des Puffers.
 * @param result Ein Pointer auf eine Option-3-Struktur, in der das Ergebnis gespeichert wird.
 *               Wenn die Option nicht gefunden wird, wird `result->option_type` auf 0 gesetzt.
 */
void extract_option_3(const uint8_t *buffer, uint16_t length, option_3 *result) {
	// Startoffset für die DHCP-Optionen nach dem DHCP-Header
    uint16_t offset = sizeof(mac_header) + sizeof(ipv4_header) + sizeof(udp_header) + sizeof(dhcp_header); // DHCP header size + magic cookie size
		// Durchlaufe die Optionen im DHCP-Paket
    while (offset < length) {
        uint8_t option_type = buffer[offset];
        uint8_t option_length = buffer[offset + 1];
				// Überprüfe, ob es sich um Option-3 (Router) handelt
        if (option_type == 3 && (option_length + 2) == sizeof(option_3)) {

            result->option_type = option_type;
            result->length = option_length;

            result->router = *(ip_address*)(buffer + offset + 2);
            return;
        }

        // Zum nächsten Optionsfeld bewegen
        offset += 2 + option_length;
    }

    // Option nicht gefunden
    result->option_type = 0;
    result->length = 0;
    result->router = (ip_address) {0};
}

/**
 * Extrahiert die Informationen aus Option 53 eines DHCP-Paketes.
 *
 * @param buffer Der Puffer, der das DHCP-Paket enthält.
 * @param length Die Länge des Puffers.
 * @param result Ein Pointer auf eine Option-53-Struktur, in der das Ergebnis gespeichert wird.
 *               Wenn die Option nicht gefunden wird, wird `result->option_type` auf 0 gesetzt.
 */
void extract_option_53(const uint8_t *buffer, uint16_t length, option_53 *result) {
		// Startoffset für die DHCP-Optionen nach dem DHCP-Header
    uint16_t offset = sizeof(mac_header) + sizeof(ipv4_header) + sizeof(udp_header) + sizeof(dhcp_header); // DHCP header size + magic cookie size
		// Durchlaufe die Optionen im DHCP-Paket
    while (offset < length) {
        uint8_t option_type = buffer[offset];
        uint8_t option_length = buffer[offset + 1];
				// Überprüfe, ob es sich um Option-53 (DHCP Identifier) handelt
        if (option_type == 53 && (option_length + 2) == sizeof(option_53)) {

            result->option_type = option_type;
            result->length = option_length;

            result->dhcp_option = *(uint8_t*)(buffer + offset + 2);
            return;
        }

        // Zum nächsten Optionsfeld bewegen
        offset += 2 + option_length;
    }

    // Option nicht gefunden
    result->option_type = 0;
    result->length = 0;
    result->dhcp_option = 0;
}


/**
 * Extrahiert die Informationen aus Option 54 eines DHCP-Paketes.
 *
 * @param buffer Der Puffer, der das DHCP-Paket enthält.
 * @param length Die Länge des Puffers.
 * @param result Ein Pointer auf eine Option-54-Struktur, in der das Ergebnis gespeichert wird.
 *               Wenn die Option nicht gefunden wird, wird `result->option_type` auf 0 gesetzt.
 */
void extract_option_54(const uint8_t *buffer, uint16_t length, option_54 *result) {
		// Startoffset für die DHCP-Optionen nach dem DHCP-Header
    uint16_t offset = sizeof(mac_header) + sizeof(ipv4_header) + sizeof(udp_header) + sizeof(dhcp_header); // DHCP header size + magic cookie size
		// Durchlaufe die Optionen im DHCP-Paket
    while (offset < length) {
        uint8_t option_type = buffer[offset];
        uint8_t option_length = buffer[offset + 1];
				// Überprüfe, ob es sich um Option-54 (DHCP Server) handelt
        if (option_type == 54 && (option_length + 2) == sizeof(option_54)) {

            result->option_type = option_type;
            result->length = option_length;

            result->ip_addr = *(ip_address*)(buffer + offset + 2);
            return;
        }

        // Zum nächsten Optionsfeld bewegen
        offset += 2 + option_length;
    }

    // Option nicht gefunden
    result->option_type = 0;
    result->length = 0;
    result->ip_addr = (ip_address) {0};
}

/**
 * Sendet eine DHCP Discover-Nachricht über das Netzwerk.
 */
void send_dhcp_disc(){
	// Struktur für das DHCP Discover-Paket
	struct package {
		mac_header mac_header;
		ipv4_header ipv4_header;
		udp_header udp_header;
		struct payload {
		dhcp_header dhcp_header;
		
		option_53 dhcp_53;
		option_61 dhcp_61;
		option_50 dhcp_50;
		option_55 dhcp_55;
		option_255 dhcp_255;
		uint8_t padding[7];
		} payload;
	};
	// Initialisiere das DHCP Discover-Paket
	struct package disc = {
	.payload.padding = {0x00}
	};

	// Layer 2 (Ethernet)
	disc.mac_header.dest_mac = (mac_address){0xff,0xff,0xff,0xff,0xff,0xff};
	disc.mac_header.src_mac = my_mac;
	disc.mac_header.ether_type = IPV4_TYPE;
	// Layer 3 (IPv4)
	disc.ipv4_header.version_length = IPV4_VERSION;
	disc.ipv4_header.service_field = 0x00;
	disc.ipv4_header.total_length = swapEndian16(sizeof(disc) - sizeof(disc.mac_header));
	disc.ipv4_header.ident = calculate_next_id();
	disc.ipv4_header.flags = 0x00;
	disc.ipv4_header.ttl = 0xff;
	disc.ipv4_header.prtcl = 0x11;
	disc.ipv4_header.src = (ip_address){0x00,0x00,0x00,0x00};
	disc.ipv4_header.dst = (ip_address){0xff,0xff,0xff,0xff};
	
	// Layer 4 (UDP)
	disc.udp_header.src = DHCP_LPORT;
	disc.udp_header.dest = DHCP_RPORT;
	disc.udp_header.length = swapEndian16(sizeof(disc) - sizeof(disc.ipv4_header) - sizeof(disc.mac_header));
	// Layer 7 (DHCP)
	disc.payload.dhcp_header.type = 0x01; 
	disc.payload.dhcp_header.hw_type = 0x01;
	disc.payload.dhcp_header.hw_len = 0x06;
	disc.payload.dhcp_header.hops = 0x00;
	disc.payload.dhcp_header.id = swapEndian32(generateID());
	disc.payload.dhcp_header.secs = 0x0000;
	disc.payload.dhcp_header.flags = 0x0000;
	disc.payload.dhcp_header.ip_client = (ip_address){0x00,0x00,0x00,0x00};
	disc.payload.dhcp_header.ip_your = (ip_address){0x00,0x00,0x00,0x00};
	disc.payload.dhcp_header.ip_server = (ip_address){0x00,0x00,0x00,0x00};
	disc.payload.dhcp_header.ip_relay = (ip_address){0x00,0x00,0x00,0x00};
	disc.payload.dhcp_header.mac_addr = my_mac;
	disc.payload.dhcp_header.addr_padding = (padding){0x00};
	disc.payload.dhcp_header.name = (server){0x00};
	disc.payload.dhcp_header.boot = (file){0x00};
	disc.payload.dhcp_header.cookie = 0x63538263;
	// DHCP Option 53
	disc.payload.dhcp_53.option_type = 0x35; 
	disc.payload.dhcp_53.length = 0x01;
	disc.payload.dhcp_53.dhcp_option = DHCP_DISCOVER;
	// DHCP Option 61
	disc.payload.dhcp_61.option_type = 0x3d;
	disc.payload.dhcp_61.length = 0x07;
	disc.payload.dhcp_61.hw_type = 0x01;
	disc.payload.dhcp_61.mac_addr = my_mac;
	// DHCP Option 50
	disc.payload.dhcp_50.option_type = 0x32;
	disc.payload.dhcp_50.length = 0x04;
	disc.payload.dhcp_50.ip_addr = (ip_address){0x00,0x00,0x00,0x00};
	// DHCP Option 55
	disc.payload.dhcp_55.option_type = 0x37;
	disc.payload.dhcp_55.length = 0x04;
	disc.payload.dhcp_55.sub_mask = 0x01;
	disc.payload.dhcp_55.router = 0x03;
	disc.payload.dhcp_55.dns = 0x06;
	disc.payload.dhcp_55.ntps = 0x2a;
	// DHCP Option 255
	disc.payload.dhcp_255.option_type = 0xff;
	
	// Berechne die Prüfsummen
	disc.ipv4_header.header_checksum = calculate_checksum(&disc.ipv4_header, sizeof(disc.ipv4_header));
	//disc.udp_header.checksum = 0x0000;
	disc.udp_header.checksum = swapEndian16(udp_checksum(&disc.ipv4_header, &disc.udp_header, (uint8_t*) &disc.payload, sizeof(disc.payload))); //(pseudoheader + udp data)
	// Sende das DHCP Discover-Paket
	enc28_packetSend(sizeof(disc), (uint8_t*)&disc);
}

void send_dhcp_req(){
	// Struktur für das DHCP Request-Paket
	struct package {
		mac_header mac_header;
		ipv4_header ipv4_header;
		udp_header udp_header;
		struct payload {
		dhcp_header dhcp_header;
		
		option_53 dhcp_53;
		option_61 dhcp_61;
		option_50 dhcp_50;
		option_54 dhcp_54;
		option_55 dhcp_55;
		option_255 dhcp_255;
		uint8_t padding[1];
		} payload;
	};
	// Initialisiere das DHCP Request-Paket
	struct package req = {
	.payload.padding = {0x00}
	};

	// Layer 2 (Ethernet)
	req.mac_header.dest_mac = (mac_address){0xff,0xff,0xff,0xff,0xff,0xff};
	req.mac_header.src_mac = my_mac;
	req.mac_header.ether_type = IPV4_TYPE;
	// Layer 3 (IPv4)
	req.ipv4_header.version_length = IPV4_VERSION;
	req.ipv4_header.service_field = 0x00;
	req.ipv4_header.total_length = swapEndian16(sizeof(req) - sizeof(req.mac_header));
	req.ipv4_header.ident = calculate_next_id();
	req.ipv4_header.flags = 0x00;
	req.ipv4_header.ttl = 0xff;
	req.ipv4_header.prtcl = 0x11;
	req.ipv4_header.src = (ip_address){0x00,0x00,0x00,0x00};
	req.ipv4_header.dst = (ip_address){0xff,0xff,0xff,0xff};
	
	// Layer 4 (UDP)
	req.udp_header.src = DHCP_LPORT;
	req.udp_header.dest = DHCP_RPORT;
	req.udp_header.length = swapEndian16(sizeof(req) - sizeof(req.ipv4_header) - sizeof(req.mac_header));
	// Layer 5 (DHCP)
	req.payload.dhcp_header.type = 0x01;
	req.payload.dhcp_header.hw_type = 0x01;
	req.payload.dhcp_header.hw_len = 0x06;
	req.payload.dhcp_header.hops = 0x00;
	req.payload.dhcp_header.id = swapEndian32(generateID());
	req.payload.dhcp_header.secs = 0x0000;
	req.payload.dhcp_header.flags = 0x0000;
	req.payload.dhcp_header.ip_client = (ip_address){0x00,0x00,0x00,0x00};
	req.payload.dhcp_header.ip_your = (ip_address){0x00,0x00,0x00,0x00};
	req.payload.dhcp_header.ip_server = (ip_address){0x00,0x00,0x00,0x00};
	req.payload.dhcp_header.ip_relay = (ip_address){0x00,0x00,0x00,0x00};
	req.payload.dhcp_header.mac_addr = my_mac;
	req.payload.dhcp_header.addr_padding = (padding){0x00};
	req.payload.dhcp_header.name = (server){0x00};
	req.payload.dhcp_header.boot = (file){0x00};
	req.payload.dhcp_header.cookie = 0x63538263;
	// DHCP Option 53
	req.payload.dhcp_53.option_type = 0x35;
	req.payload.dhcp_53.length = 0x01;
	req.payload.dhcp_53.dhcp_option = DHCP_REQUEST;
	// DHCP Option 61
	req.payload.dhcp_61.option_type = 0x3d;
	req.payload.dhcp_61.length = 0x07;
	req.payload.dhcp_61.hw_type = 0x01;
	req.payload.dhcp_61.mac_addr = my_mac;
	// DHCP Option 50
	req.payload.dhcp_50.option_type = 0x32;
	req.payload.dhcp_50.length = 0x04;
	req.payload.dhcp_50.ip_addr = *my_ip_addr;
	// DHCP Option 54
	req.payload.dhcp_54.option_type = 0x36;
	req.payload.dhcp_54.length = 0x04;
	req.payload.dhcp_54.ip_addr = *my_dhcp_server_addr; //dhcp_server_ip;
	// DHCP Option 55
	req.payload.dhcp_55.option_type = 0x37;
	req.payload.dhcp_55.length = 0x04;
	req.payload.dhcp_55.sub_mask = 0x01;
	req.payload.dhcp_55.router = 0x03;
	req.payload.dhcp_55.dns = 0x06;
	req.payload.dhcp_55.ntps = 0x2a;
	// DHCP Option 255
	req.payload.dhcp_255.option_type = 0xff;
	
	// Berechne die Prüfsummen
	req.ipv4_header.header_checksum = calculate_checksum(&req.ipv4_header, sizeof(req.ipv4_header));
	//req.udp_header.checksum = 0x0000;
	req.udp_header.checksum = swapEndian16(udp_checksum(&req.ipv4_header, &req.udp_header, (uint8_t*) &req.payload, sizeof(req.payload))); //(pseudoheader + udp data)
	// Sende das DHCP Request-Paket
	enc28_packetSend(sizeof(req), (uint8_t*)&req);
}

/**
 * Verarbeitet eine DHCP Offer-Nachricht und führt entsprechende Aktionen aus.
 * 
 * @param buf Pointer auf den empfangenen Netzwerkpaket-Puffer
 * @param length Länge des empfangenen Netzwerkpakets
 */
void get_dhcp_offer(const uint8_t* buf, uint16_t length){
			// Extrahiere die neue IP-Adresse aus dem empfangenen Paket
			my_ip_addr->octet[0] = buf[58];
			my_ip_addr->octet[1] = buf[59];
			my_ip_addr->octet[2] = buf[60];
			my_ip_addr->octet[3] = buf[61];
	
			 // Extrahiere DHCP Option 1 (Subnet Mask) und aktualisiere die Subnetzadresse
			option_1 result_1;
			extract_option_1(buf, length, &result_1);
			*my_subnet_addr = result_1.subnet_mask;
	
			// Extrahiere DHCP Option 3 (Router) und aktualisiere die Gateway-Adresse
			option_3 result_3;
			extract_option_3(buf, length, &result_3);
			*my_gateway_addr = result_3.router;
			
			// Extrahiere DHCP Option 54 (DHCP Server) und aktualisiere die DHCP-Server-Adresse
			option_54 result_54;
			extract_option_54(buf, length, &result_54);
			*my_dhcp_server_addr = result_54.ip_addr;

			// Sende eine DHCP Request-Nachricht, um die zugewiesenen Konfigurationen zu bestätigen
			send_dhcp_req();
			return;
}

/**
 * Verarbeitet eine DHCP Acknowledgment-Nachricht und überprüft, ob die erhaltenen Konfigurationen korrekt sind.
 * 
 * @param buf Pointer auf den empfangenen Netzwerkpaket-Puffer
 * @param length Länge des empfangenen Netzwerkpakets
 * @param dhcp_rdy Ein Pointer auf den Status des DHCP-Dienstes.
 */
void get_dhcp_ack(const uint8_t* buf, uint16_t length, uint8_t* dhcp_rdy){
	
	// Extrahiere DHCP Option 1 (Subnetzmaske)
	option_1 result_1;
	extract_option_1(buf, length, &result_1);
	// Extrahiere DHCP Option 3 (Router)
	option_3 result_3;
	extract_option_3(buf, length, &result_3);
	// Extrahiere DHCP Option 54 (DHCP Server)
	option_54 result_54;
	extract_option_54(buf, length, &result_54);
	// Überprüfe, ob die erhaltenen Konfigurationen mit den erwarteten übereinstimmen
	if(
			my_ip_addr->octet[0] == buf[58] &&
			my_ip_addr->octet[1] == buf[59] &&
			my_ip_addr->octet[2] == buf[60] &&
			my_ip_addr->octet[3] == buf[61] &&
	
			my_subnet_addr->octet[0] == result_1.subnet_mask.octet[0] &&
			my_subnet_addr->octet[1] == result_1.subnet_mask.octet[1] &&
			my_subnet_addr->octet[2] == result_1.subnet_mask.octet[2] &&
			my_subnet_addr->octet[3] == result_1.subnet_mask.octet[3] &&
	
			my_gateway_addr->octet[0] == result_3.router.octet[0] &&
			my_gateway_addr->octet[1] == result_3.router.octet[1] &&
			my_gateway_addr->octet[2] == result_3.router.octet[2] &&
			my_gateway_addr->octet[3] == result_3.router.octet[3] &&
	
			my_dhcp_server_addr->octet[0] == result_54.ip_addr.octet[0] &&
			my_dhcp_server_addr->octet[1] == result_54.ip_addr.octet[1] &&
			my_dhcp_server_addr->octet[2] == result_54.ip_addr.octet[2] &&
			my_dhcp_server_addr->octet[3] == result_54.ip_addr.octet[3] 
	){
		// Setze den DHCP-Bereitschaftsstatus auf 1 (Abgeschlossen)
		*dhcp_rdy = 0x01;
	}
			return;
}


/**
 * Verarbeitet ein DHCP-Paket und ruft die entsprechenden Funktionen basierend auf der DHCP-Option 53 auf.
 * 
 * @param buf Pointer auf den empfangenen Netzwerkpaket-Puffer
 * @param length Länge des empfangenen Netzwerkpakets
 * 
 * @return Rückgabewert 0 für erfolgreiche Verarbeitung
 */
int handle_dhcp(const uint8_t* buf,uint16_t length){
		// Extrahiere DHCP Option 53 (DHCP Message Type)
		option_53 result;
		extract_option_53(buf, length, &result);
	
		// Überprüfe, ob die DHCP Option 53 vorhanden ist
		if (result.option_type == 53){
			if (result.dhcp_option == DHCP_OFFER){get_dhcp_offer(buf, length);} // Verarbeite DHCP Offer
			if (result.dhcp_option == DHCP_ACK){get_dhcp_ack(buf, length, dhcp_rdy_addr);} // Verarbeite DHCP Acknowledgment
		}
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