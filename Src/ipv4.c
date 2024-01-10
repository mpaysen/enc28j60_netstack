/* Includes ------------------------------------------------------------------*/
#include "ipv4.h"

/* Private variables ---------------------------------------------------------*/
static prtcl_types* types;

/* Private functions prototypes ---------------------------------------------*/
int handle_ipv4(const uint8_t* buf, uint16_t length);

/* Functions -----------------------------------------------------------------*/

/**
 * Initialisiert die IPv4-Schicht, indem der IPv4-EtherType und die zugehörige Verarbeitungsfunktion
 * zur Ethernet-Schicht hinzugefügt werden. Setzt den Pointer auf die Struktur der Layer-3-Protokolltypen
 * und initialisiert den Index auf Null, um anzuzeigen, dass noch keine Layer-3-Protokolle hinzugefügt wurden.
 *
 * @param types_addr Ein Pointer auf die Struktur der Layer-3-Protokolltypen.
 */
void ipv4_init(prtcl_types* types_addr) {
	// Überprüft, ob der bereitgestellte Pointer nicht NULL ist
	if (types_addr != NULL) {
		// Fügt den IPv4-EtherType und die zugehörige Verarbeitungsfunktion zur Ethernet-Schicht hinzu
		eth_add_type(IPV4_TYPE, &handle_ipv4);
		
		// Setzt den Pointer auf die Struktur der Layer-3-Protokolltypen
		types = types_addr;
		
		types->idx = 0;
	}
}


/**
 * Fügt einen neuen Layer-3-Protokolltyp zur IPv4-Schicht hinzu.
 * Setzt die prtcl_type- und func-Felder der Struktur der Layer-3-Protokolltypen
 * an der aktuellen Indexposition und erhöht den Index, wenn im Typenarray noch Platz vorhanden ist.
 *
 * @param type Der Protokolltyp des hinzuzufügenden IPv4-Protokolls.
 * @param func Ein Pointer auf die Funktion, die für das hinzugefügte Protokoll aufgerufen werden soll.
 */
void ipv4_add_type(uint8_t type, void* func){
	// Setzt die prtcl_type- und func-Felder der Struktur der Layer-3-Protokolltypen an der aktuellen Indexposition
	types->types[types->idx].prtcl_type = type;
	types->types[types->idx].func = func;
	
	// Erhöht den Index, wenn im Array noch Platz vorhanden ist
	if (types->idx + 1 < PRTCL_TYPE_SIZE) {
		types->idx = types->idx + 1;
	}
}

/**
 * Verarbeitet IPv4-Pakete, indem der Protokolltyp (prtcl_type) aus dem IPv4-Paketheader extrahiert wird
 * und dann die registrierten Protokolltypen durchgegangen werden, um die entsprechende Verarbeitungsfunktion aufzurufen.
 *
 * @param buf Ein Pointer auf den Puffer, der das empfangene IPv4-Paket enthält.
 * @param length Die Länge des empfangenen IPv4-Pakets.
 * @return 0, wenn die Verarbeitung erfolgreich war; 1, wenn kein passender Protokolltyp und Handler gefunden wurde.
 */
int handle_ipv4(const uint8_t* buf, uint16_t length) {
	uint8_t typ = buf[23]; // Extrahiert den Protokolltyp (prtcl_type) aus dem IPv4-Paketheader
	
	// Durchläuft die registrierten Protokolltypen in der prtcl_types-Struktur
	for(uint8_t i = 0; i < PRTCL_TYPE_SIZE; i++) { 
		// Überprüft, ob der Protokolltyp (prtcl_type) des Pakets mit einem registrierten Protokolltyp übereinstimmt
		if (typ == types->types[i].prtcl_type){
			
			// Ruft die entsprechende Verarbeitungsfunktion für den übereinstimmenden Protokolltyp auf
			return types->types[i].func(buf, length);
		}
	}
	return 1;
}


/**
 * Berechnet und gibt eine eindeutige 16-Bit-Identifier (ID) zurück.
 * Verwendet einen statischen Zähler, um die Identifikationsnummer zu verfolgen,
 * tauscht die Byte-Reihenfolge der ID und erhöht anschließend den Zähler für die nächste ID.
 *
 * @return Die berechnete 16-Bit-Identifier (ID).
 */
uint16_t calculate_next_id() {
	// Statischer Zähler zur Verfolgung der Identifikationsnummer
  static uint16_t id = 420;
	// Vertauscht die Byte-Reihenfolge der Identifikationsnummer ()
	uint16_t Id = ((id & 0xFF) << 8) | ((id & 0xFF00) >> 8);
  ++id;
  return Id;
}