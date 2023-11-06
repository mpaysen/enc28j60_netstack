/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ARP_H
#define __ARP_H

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"
#include "enc28_j60.h"
#include "eth.h"

/* Defines ------------------------------------------------------------------*/

#define ARP_TABLE_SIZE 10
//Big Endian
#define ARP_TYPE 	0x0608
#define ARP_HW_TYPE 0x0100
#define ARP_PR_TYPE 0x0008
#define ARP_HW_SIZE 0x06
#define ARP_PR_SIZE 0x04
#define ARP_REQ 0x0100
#define ARP_REPLY 0x0200

typedef struct{
	mac_header mac_header;
	uint16_t hw_type;
	uint16_t pr_type;
	uint8_t hw_size;
	uint8_t pr_size;
	uint16_t opcode;
	mac_address sender_mac;
	ip_address sender_ip;
	mac_address target_mac;
	ip_address target_ip;
} __attribute__((packed)) arp_package;

typedef struct{
	ip_address dest_ip;
	mac_address dest_mac;
} __attribute__((packed)) arp_entry;

typedef struct {
    arp_entry data[ARP_TABLE_SIZE];
    int tail;
} __attribute__((packed)) arp_table;


/* Exported functions prototypes ---------------------------------------------*/

void arp_table_init(arp_table* table_adr, ip_address src_ip, mac_address src_mac);

int handle_arp(uint8_t* buf);

void send_arp_req(ip_address src_ip, mac_address src_mac, ip_address target_ip);


#endif /* __ARP_H */