/* Includes ------------------------------------------------------------------*/
#include "ipv4.h"

/* Private variables ---------------------------------------------------------*/
static prtcl_types* types;

/* Private functions prototypes ---------------------------------------------*/
int handle_ipv4(const uint8_t* buf, uint16_t length);

/* Functions -----------------------------------------------------------------*/

/**
 * Initialisiert die IPv4-Schicht, indem der IPv4-EtherType und die zugeh�rige Verarbeitungsfunktion
 * zur Ethernet-Schicht hinzugef�gt werden. Setzt den Pointer auf die Struktur der Layer-3-Protokolltypen
 * und initialisiert den Index auf Null, um anzuzeigen, dass noch keine Layer-3-Protokolle hinzugef�gt wurden.
 *
 * @param types_addr Ein Pointer auf die Struktur der Layer-3-Protokolltypen.
 */
void ipv4_init(prtcl_types* types_addr) {
	// �berpr�ft, ob der bereitgestellte Pointer nicht NULL ist
	if (types_addr != NULL) {
		// F�gt den IPv4-EtherType und die zugeh�rige Verarbeitungsfunktion zur Ethernet-Schicht hinzu
		eth_add_type(IPV4_TYPE, &handle_ipv4);
		
		// Setzt den Pointer auf die Struktur der Layer-3-Protokolltypen
		types = types_addr;
		
		types->idx = 0;
	}
}


/**
 * F�gt einen neuen Layer-3-Protokolltyp zur IPv4-Schicht hinzu.
 * Setzt die prtcl_type- und func-Felder der Struktur der Layer-3-Protokolltypen
 * an der aktuellen Indexposition und erh�ht den Index, wenn im Typenarray noch Platz vorhanden ist.
 *
 * @param type Der Protokolltyp des hinzuzuf�genden IPv4-Protokolls.
 * @param func Ein Pointer auf die Funktion, die f�r das hinzugef�gte Protokoll aufgerufen werden soll.
 */
void ipv4_add_type(uint8_t type, void* func){
	// Setzt die prtcl_type- und func-Felder der Struktur der Layer-3-Protokolltypen an der aktuellen Indexposition
	types->types[types->idx].prtcl_type = type;
	types->types[types->idx].func = func;
	
	// Erh�ht den Index, wenn im Array noch Platz vorhanden ist
	if (types->idx + 1 < PRTCL_TYPE_SIZE) {
		types->idx = types->idx + 1;
	}
}

/**
 * Verarbeitet IPv4-Pakete, indem der Protokolltyp (prtcl_type) aus dem IPv4-Paketheader extrahiert wird
 * und dann die registrierten Protokolltypen durchgegangen werden, um die entsprechende Verarbeitungsfunktion aufzurufen.
 *
 * @param buf Ein Pointer auf den Puffer, der das empfangene IPv4-Paket enth�lt.
 * @param length Die L�nge des empfangenen IPv4-Pakets.
 * @return 0, wenn die Verarbeitung erfolgreich war; 1, wenn kein passender Protokolltyp und Handler gefunden wurde.
 */
int handle_ipv4(const uint8_t* buf, uint16_t length) {
	uint8_t typ = buf[23]; // Extrahiert den Protokolltyp (prtcl_type) aus dem IPv4-Paketheader
	
	// Durchl�uft die registrierten Protokolltypen in der prtcl_types-Struktur
	for(uint8_t i = 0; i < PRTCL_TYPE_SIZE; i++) { 
		// �berpr�ft, ob der Protokolltyp (prtcl_type) des Pakets mit einem registrierten Protokolltyp �bereinstimmt
		if (typ == types->types[i].prtcl_type){
			
			// Ruft die entsprechende Verarbeitungsfunktion f�r den �bereinstimmenden Protokolltyp auf
			return types->types[i].func(buf, length);
		}
	}
	return 1;
}


/**
 * Berechnet und gibt eine eindeutige 16-Bit-Identifier (ID) zur�ck.
 * Verwendet einen statischen Z�hler, um die Identifikationsnummer zu verfolgen,
 * tauscht die Byte-Reihenfolge der ID und erh�ht anschlie�end den Z�hler f�r die n�chste ID.
 *
 * @return Die berechnete 16-Bit-Identifier (ID).
 */
uint16_t calculate_next_id() {
	// Statischer Z�hler zur Verfolgung der Identifikationsnummer
  static uint16_t id = 420;
	// Vertauscht die Byte-Reihenfolge der Identifikationsnummer ()
	uint16_t Id = ((id & 0xFF) << 8) | ((id & 0xFF00) >> 8);
  ++id;
  return Id;
}