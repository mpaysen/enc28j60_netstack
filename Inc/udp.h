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
	int (*func)(uint8_t* buf);
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
	uint8_t* payload;
} udp_package;


/* Exported functions prototypes ---------------------------------------------*/

void udp_init(udp_serivces* types_addr, ip_address src_ip, mac_address src_mac);

void udp_add_type(uint16_t dst, void* func);

void send_udp(ip_address target_ip, uint16_t src, uint16_t dest, uint8_t* payload);

int handle_udp(uint8_t* buf);


#endif /* __UDP_H */