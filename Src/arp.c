/* Includes ------------------------------------------------------------------*/
#include "arp.h"

static arp_table* table;
static ip_address my_ip;
static mac_address my_mac;

/* Private functions prototypes ---------------------------------------------*/
int handle_arp(const uint8_t* buf, uint16_t length);
void add_to_arp_table(arp_entry entry);
int get_mac_from_table(ip_address ip, mac_address* mac);
void get_arp_rep(const uint8_t* buf);
void send_arp_req(ip_address src_ip, mac_address src_mac, ip_address target_ip);
void send_arp_rep(ip_address src_ip, mac_address src_mac, ip_address target_ip, mac_address target_mac);
void get_arp_req(const uint8_t* buf, ip_address my_ip, mac_address my_mac);

/* Functions -----------------------------------------------------------------*/

/**
 * Initialisiert die ARP-Tabelle mit den angegebenen Konfigurationen.
 *
 * @param table_adr Ein Pointer auf die ARP-Tabelle.
 * @param src_ip Die Quell-IP-Adresse.
 * @param src_mac Die Quell-MAC-Adresse.
 */
void arp_table_init(arp_table* table_adr, ip_address src_ip, mac_address src_mac) {
	// Fügt den ARP-EtherType und seine entsprechende Verarbeitungsfunktion zur Ethernet-Schicht hinzu
	eth_add_type(ARP_TYPE, &handle_arp);
	
	table = table_adr;
	
	 // Weist die bereitgestellten Quell-IP- und MAC-Adressen zu
	my_ip = src_ip;
	my_mac = src_mac;
	
  table->tail = 0;
}


/**
 * Fügt einen ARP-Eintrag zur ARP-Tabelle hinzu oder aktualisiert einen vorhandenen Eintrag.
 *
 * @param entry Der ARP-Eintrag, der zur Tabelle hinzugefügt oder aktualisiert werden soll.
 */
void add_to_arp_table(arp_entry entry) {
		// Durchsucht die ARP-Tabelle, um einen Eintrag mit derselben Ziel-MAC-Adresse zu finden
	  for (int i = 0; i <= table->tail; i++) {
			// Überprüft, ob die Ziel-MAC-Adresse mit dem aktuellen Eintrag in der Tabelle übereinstimmt
			if (table->data[i].dest_mac.octet[0] == entry.dest_mac.octet[0] &&
           table->data[i].dest_mac.octet[1] == entry.dest_mac.octet[1] &&
           table->data[i].dest_mac.octet[2] == entry.dest_mac.octet[2] &&
           table->data[i].dest_mac.octet[3] == entry.dest_mac.octet[3] &&
           table->data[i].dest_mac.octet[4] == entry.dest_mac.octet[4] &&
           table->data[i].dest_mac.octet[5] == entry.dest_mac.octet[5]){
					 // Aktualisiert den vorhandenen Eintrag mit den neuen Informationen
           table->data[i] = entry;
					 return;
        }
				
    }
		 // Wenn kein übereinstimmender Eintrag gefunden wird, füge den bereitgestellten Eintrag zur ARP-Tabelle hinzu
    table->data[table->tail] = entry;
		
		// Aktualisiert das Ende der Tabelle, wenn Platz verfügbar ist
    if(table->tail < ARP_TABLE_SIZE - 1) {
			table->tail += 1;
		}
}


/**
 * Sucht in der ARP-Tabelle nach einer MAC-Adresse für die angegebene Ziel-IP-Adresse.
 *
 * @param ip Die Ziel-IP-Adresse, für die die MAC-Adresse gesucht wird.
 * @param mac Ein Pointer auf die MAC-Adresse, die gefunden wurde (falls vorhanden).
 * @return 1, wenn die MAC-Adresse gefunden wurde; 0, wenn keine Übereinstimmung gefunden wurde.
 */
int get_mac_from_table(ip_address ip, mac_address* mac) {
	
    int foundIndex = -1;

		// Durchsucht die ARP-Tabelle, um einen Eintrag mit der angegebenen Ziel-IP-Adresse zu finden
    for (int i = 0; i <= table->tail; i++) {
				// Überprüft, ob die Ziel-IP-Adresse mit dem aktuellen Eintrag in der Tabelle übereinstimmt
        if (table->data[i].dest_ip.octet[0] == ip.octet[0] &&
            table->data[i].dest_ip.octet[1] == ip.octet[1] &&
            table->data[i].dest_ip.octet[2] == ip.octet[2] &&
            table->data[i].dest_ip.octet[3] == ip.octet[3]) {
						// Speichert den Index des gefundenen Eintrags und bricht die Schleife ab
            foundIndex = i;
            break; 
        }
    }

		// Wenn ein übereinstimmender Eintrag gefunden wurde, tauscht seine Position mit dem vorherigen Eintrag (falls vorhanden)
    if (foundIndex >= 0) {
        if (foundIndex > 0) {
            int prevIndex = (foundIndex - 1);
            arp_entry temp = table->data[foundIndex];
            table->data[foundIndex] = table->data[prevIndex];
            table->data[prevIndex] = temp;
        }
				
				// Gibt die MAC-Adresse zurück, die dem gefundenen Eintrag entspricht
        *mac = table->data[foundIndex].dest_mac; 
        return 1;
    }
		// Gibt 0 zurück, wenn kein übereinstimmender Eintrag und keine MAC-Adresse gefunden wurden
    return 0;
}

/**
 * Verarbeitet eine ARP-Antwort (Reply) und aktualisiert oder fügt den entsprechenden Eintrag zur ARP-Tabelle hinzu.
 *
 * @param buf Ein Pointer auf den Puffer, der die ARP-Antwort enthält.
 */
void get_arp_rep(const uint8_t* buf){
	
	arp_entry entry;
	
	// Extrahiert die Ziel-MAC-Adresse aus der ARP-Antwort
	entry.dest_mac.octet[0] = buf[22];
	entry.dest_mac.octet[1] = buf[23];
	entry.dest_mac.octet[2] = buf[24];
	entry.dest_mac.octet[3] = buf[25];
	entry.dest_mac.octet[4] = buf[26];
	entry.dest_mac.octet[5] = buf[27];
	
	// Extrahiert die Ziel-MAC-Adresse aus der ARP-Antwort
	entry.dest_ip.octet[0] = buf[28];
	entry.dest_ip.octet[1] = buf[29];
	entry.dest_ip.octet[2] = buf[30];
	entry.dest_ip.octet[3] = buf[31];
	
	 // Fügt den ARP-Eintrag zur ARP-Tabelle hinzu oder aktualisiert ihn
	add_to_arp_table(entry);
}



/**
 * Sendet eine ARP-Anfrage (Request) über das ENC28J60-Modul.
 *
 * @param src_ip Die IP-Adresse des Absenders.
 * @param src_mac Die MAC-Adresse des Absenders.
 * @param target_ip Die Ziel-IP-Adresse für die ARP-Anfrage.
 */
void send_arp_req(ip_address src_ip, mac_address src_mac, ip_address target_ip){
	// Definiert eine Struktur, um das gesamte ARP-Anfragepaket darzustellen
	struct package {
		mac_header mac_header;
		arp_package arp_package;
	};
	
	struct package req;
	
	// Layer 2 - MAC-Header
	req.mac_header.dest_mac = (mac_address){0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	req.mac_header.src_mac = src_mac;
	req.mac_header.ether_type = ARP_TYPE;
	
	// ARP-Paket
	req.arp_package.hw_type = ARP_HW_TYPE;
	req.arp_package.pr_type = ARP_PR_TYPE;
	req.arp_package.hw_size = ARP_HW_SIZE;
	req.arp_package.pr_size = ARP_PR_SIZE;
	req.arp_package.opcode = ARP_REQ;
	req.arp_package.sender_mac = src_mac;
	req.arp_package.sender_ip = src_ip;
	req.arp_package.target_mac = (mac_address){0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	req.arp_package.target_ip = target_ip;
	
	// Sendet das ARP-Anfragepaket
	enc28_packetSend(sizeof(req), (uint8_t*)&req); //sizeof()
}


/**
 * Sendet eine ARP-Antwort (Reply) über das ENC28J60-Modul.
 *
 * @param src_ip Die IP-Adresse des Absenders.
 * @param src_mac Die MAC-Adresse des Absenders.
 * @param target_ip Die IP-Adresse des Empfängers, für den die ARP-Antwort bestimmt ist.
 * @param target_mac Die MAC-Adresse des Empfängers, für den die ARP-Antwort bestimmt ist.
 */
void send_arp_rep(ip_address src_ip, mac_address src_mac, ip_address target_ip, mac_address target_mac){
	
	// Definiert eine Struktur, um das gesamte ARP-Antwortpaket darzustellen
	struct package {
		mac_header mac_header;
		arp_package arp_package;
	};
	
	// Create an instance of the package structure and initialize its fields
	struct package rep;
	
	// Layer 2 - MAC-Header
	rep.mac_header.dest_mac = target_mac;
	rep.mac_header.src_mac = src_mac;
	rep.mac_header.ether_type = ARP_TYPE;
	
	// ARP-Paket
	rep.arp_package.hw_type = ARP_HW_TYPE;
	rep.arp_package.pr_type = ARP_PR_TYPE;
	rep.arp_package.hw_size = ARP_HW_SIZE;
	rep.arp_package.pr_size = ARP_PR_SIZE;
	rep.arp_package.opcode = ARP_REPLY;
	rep.arp_package.sender_mac = src_mac;
	rep.arp_package.sender_ip = src_ip;
	rep.arp_package.target_mac = target_mac;
	rep.arp_package.target_ip = target_ip;
	
	// Sendet das ARP-Antwortpaket
	enc28_packetSend(sizeof(rep), (uint8_t*)&rep); //sizeof()
}

/**
 * Verarbeitet eine eingehende ARP-Anfrage (Request) und sendet eine ARP-Antwort (Reply) zurück,
 * falls die Anfrage an die angegebene IP-Adresse (my_ip) gerichtet ist.
 *
 * @param buf Der Puffer, der die empfangenen Daten enthält, einschließlich der ARP-Anfrage.
 * @param my_ip Die eigene IP-Adresse des Geräts.
 * @param my_mac Die eigene MAC-Adresse des Geräts.
 */
void get_arp_req(const uint8_t* buf, ip_address my_ip, mac_address my_mac) {
	// Überprüft, ob die ARP-Anfrage an die angegebene IP-Adresse (my_ip) gerichtet ist
	if (my_ip.octet[0] == buf[38] &&
      my_ip.octet[1] == buf[39] &&
      my_ip.octet[2] == buf[40] &&
      my_ip.octet[3] == buf[41]) {
			
			// Extrahiert Informationen aus der ARP-Anfrage
			arp_entry entry;
			entry.dest_mac.octet[0] = buf[22];
			entry.dest_mac.octet[1] = buf[23];
			entry.dest_mac.octet[2] = buf[24];
			entry.dest_mac.octet[3] = buf[25];
			entry.dest_mac.octet[4] = buf[26];
			entry.dest_mac.octet[5] = buf[27];
	
			entry.dest_ip.octet[0] = buf[28];
			entry.dest_ip.octet[1] = buf[29];
			entry.dest_ip.octet[2] = buf[30];
			entry.dest_ip.octet[3] = buf[31];	
			
			 // Sendet eine ARP-Antwort an die Quell-MAC- und IP-Adressen zurück
			send_arp_rep(my_ip, my_mac, entry.dest_ip, entry.dest_mac);
			}
			return;
}


/**
 * Versucht, die MAC-Adresse für die angegebene IP-Adresse zu erhalten.
 * Wenn die MAC-Adresse bereits in der ARP-Tabelle vorhanden ist, wird sie zurückgegeben.
 * Andernfalls wird eine ARP-Anfrage (Request) gesendet, um die MAC-Adresse zu ermitteln.
 *
 * @param ip Die Ziel-IP-Adresse, für die die MAC-Adresse abgerufen werden soll.
 * @param mac_addr Ein Pointer auf die MAC-Adresse, die zurückgegeben wird, wenn gefunden.
 * @return Gibt 1 zurück, wenn die MAC-Adresse in der ARP-Tabelle gefunden wurde, sonst 0.
 */
int get_mac(ip_address ip, mac_address* mac_addr){
	// Versucht, die MAC-Adresse aus der ARP-Tabelle abzurufen
	if(get_mac_from_table(ip, mac_addr)) {
		return 1; // MAC-Adresse in der ARP-Tabelle gefunden
	}
	// Falls nicht gefunden, sendet eine ARP-Anfrage, um die MAC-Adresse zu erhalten
	send_arp_req(my_ip, my_mac, ip);
	return 0; // ARP-Anfrage gesendet, die ARP-Antwort wird die ARP-Tabelle aktualisieren
}

/**
 * Verarbeitet ARP-Pakete und aktualisiert die ARP-Tabelle entsprechend.
 *
 * @param buf Pointer auf den empfangenen Netzwerkpaket-Puffer.
 * @param length Die Länge des ARP-Pakets.
 * @return Gibt 0 zurück, um anzuzeigen, dass die Verarbeitung erfolgreich war.
 */
int handle_arp(const uint8_t* buf, uint16_t length){
		// Extrahiere ARP-Opcode aus dem Paket
		if ((buf[20]  + (buf[21] << 8)) == ARP_REQ){get_arp_req(buf, my_ip, my_mac);}  // ARP-Anfrage: Verarbeiten und ARP-Antwort senden
		if ((buf[20]  + (buf[21] << 8)) == ARP_REPLY){get_arp_rep(buf);} // ARP-Antwort: Füge die IP- und MAC-Adresse des Absenders zur ARP-Tabelle hinzu
		return 0;
}
