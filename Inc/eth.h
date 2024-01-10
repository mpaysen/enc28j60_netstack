/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ETH_H
#define __ETH_H

#include "stm32g0xx_hal.h"


/* Defines -------------------------------------*/
#define ETHER_TYPE_SIZE 2

typedef struct {
	uint16_t ether_type;
	int (*func)(const uint8_t* buf, uint16_t length);
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
} __attribute__((packed)) mac_header;

/* Exported functions prototypes ---------------------------------------------*/
void eth_init(ether_types* types_addr);

void eth_add_type(uint16_t type, void* func);

int eth_handler(const uint8_t* buf, uint16_t lenght);

int isInSameNetwork(ip_address* my_ip, ip_address* dst_ip, ip_address* sub_netmask);

uint32_t swapEndian32(uint32_t value);

uint16_t swapEndian16(uint16_t value);

#endif /* __ETH_H */