/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ICMP_H
#define __ICMP_H

/* Includes ------------------------------------------------------------------*/
#include "eth.h"
#include "ipv4.h"
#include "arp.h"

/* Defines ------------------------------------------------------------------*/
//Little Endian
#define ICMP_TYPE 	0x01
#define ICMP_CODE	0x00
#define ICMP_CHECKSUM	0x584d
#define ICMP_IDENT 0x0100
#define ICMP_SEQ 0x0300
#define ICMP_REQ 0x08
#define ICMP_REPLY 0x00


typedef struct{
	uint8_t data[32];
} payload;

typedef struct{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint16_t ident;
	uint16_t seq;
	payload data;
} __attribute__((packed)) icmp_package;


/* Exported functions prototypes ---------------------------------------------*/
void icmp_init(ip_address* src_ip, ip_address *my_subnet,ip_address *my_gateway, mac_address src_mac);

void send_icmp_req(ip_address target_ip);

#endif /* __ICMP_H */