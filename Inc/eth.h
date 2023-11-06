/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ETH_H
#define __ETH_H

#include "stm32g0xx_hal.h"


/* Define ether_types -------------------------------------*/


#define ETHER_TYPE_SIZE 2
//Big Endian
#define IPV4_TYPE 0x0008

typedef struct {
	uint16_t ether_type;
	int (*func)(uint8_t* buf);
} ether_type;

typedef struct {
	ether_type types[ETHER_TYPE_SIZE];
	uint8_t idx;
} ether_types;

typedef struct {
    uint8_t octet[4];
} ip_address;

typedef struct {
    uint8_t octet[6];
} mac_address;

typedef struct {
	mac_address dest_mac;
	mac_address src_mac;
	uint16_t ether_type;
} mac_header;


void eth_init(ether_types* types);

void eth_add_type(uint16_t type, void* func);

int eth_handler(uint8_t* buf);

#endif /* __ETH_H */