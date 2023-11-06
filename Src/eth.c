/* Includes ------------------------------------------------------------------*/

#include "eth.h"

static ether_types* types;

void eth_init(ether_types* types_addr) {
	types = types_addr; //set ptr to Layer2 protocol struct
	types->idx = 0; //set index to zero, none protocol added
}

void eth_add_type(uint16_t type, void* func){
	types->types[types->idx].ether_type = type; //set ether_type
	types->types[types->idx].func = func; //set ptr to handel function for ether_type
	if (types->idx + 1 < ETHER_TYPE_SIZE){types->idx = types->idx + 1;} //increment idx
}

int eth_handler(uint8_t* buf) {
	uint16_t typ = (buf[12]  + (buf[13] << 8)); //get ether_type from package
	for(uint8_t i = 0; i < ETHER_TYPE_SIZE; i++) { 
		if (typ == types->types[i].ether_type){ //check ether_type from package is in protocol struct
			return types->types[i].func(buf); //call ether_type handel function
		}
	}
	return 1;
}