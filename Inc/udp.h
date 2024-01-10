/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UDP_H
#define __UDP_H

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"
#include "enc28_j60.h"
#include "eth.h"
#include "ipv4.h"
#include "arp.h"

/* Defines ------------------------------------------------------------------*/
#define UDP_SERVICES_SIZE 2
//Little Endian
#define UDP_TYPE 	0x11

typedef struct {
	uint16_t lport;
	int (*func)(const uint8_t* buf, uint16_t length);
} udp_serivce;

typedef struct {
	udp_serivce serivces[UDP_SERVICES_SIZE];
	uint8_t idx;
} udp_serivces;

typedef struct{
	uint16_t src;
	uint16_t dest;
	uint16_t length; // = sizeof(payload) + sizeof(src) + sizeof(dest) + sizeof(checksum)
	uint16_t checksum;
} __attribute__((packed)) udp_header;


/* Exported functions prototypes ---------------------------------------------*/
void udp_init(udp_serivces* types_addr, ip_address src_ip, mac_address src_mac);

void udp_add_type(uint16_t lport, void* func);

uint16_t udp_checksum(ipv4_header *ip_header, udp_header *udp_header, uint8_t *payload, size_t payload_size);

//void send_udp(ip_address target_ip, uint16_t src, uint16_t dest, uint8_t* payload);

//int handle_udp(uint8_t* buf, uint16_t length);


#endif /* __UDP_H */