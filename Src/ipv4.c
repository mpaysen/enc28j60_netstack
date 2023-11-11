/* Includes ------------------------------------------------------------------*/
#include "ipv4.h"

static prtcl_types* types;

void ipv4_init(prtcl_types* types_addr) {

	eth_add_type(IPV4_TYPE, &handle_ipv4); //add IPv4 eth_type and ipv4_handler function
	types = types_addr; //set ptr to Layer2 protocol struct
	types->idx = 0; //set index to zero, none protocol added
}

void ipv4_add_type(uint8_t type, void* func){
	types->types[types->idx].prtcl_type = type; //set prtcl_type
	types->types[types->idx].func = func; //set ptr to handel function for prtcl_type
	if (types->idx + 1 < PRTCL_TYPE_SIZE){types->idx = types->idx + 1;} //increment idx
}

int handle_ipv4(uint8_t* buf) {
	uint8_t typ = buf[23]; //get prtcl_type from package
	for(uint8_t i = 0; i < PRTCL_TYPE_SIZE; i++) { 
		if (typ == types->types[i].prtcl_type){ //check prtcl_type from package is in protocol struct
			return types->types[i].func(buf); //call prtcl_type handel function
		}
	}
	return 1;
}

uint16_t calculate_next_id() {
    static uint16_t id = 420;
	  uint16_t Id = ((id & 0xFF) << 8) | ((id & 0xFF00) >> 8);
    ++id;
    return Id;
}