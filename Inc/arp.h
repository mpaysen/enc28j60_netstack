/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ARP_H
#define __ARP_H

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"
#include "enc28_j60.h"
#include "eth.h"

/* Defines ------------------------------------------------------------------*/
#define ARP_TABLE_SIZE 10
//Little Endian
#define ARP_TYPE 	0x0608
#define ARP_HW_TYPE 0x0100
#define ARP_PR_TYPE 0x0008
#define ARP_HW_SIZE 0x06
#define ARP_PR_SIZE 0x04
#define ARP_REQ 0x0100
#define ARP_REPLY 0x0200

typedef struct{
	uint16_t hw_type;
	uint16_t pr_type;
	uint8_t hw_size;
	uint8_t pr_size;
	uint16_t opcode;
	mac_address sender_mac;
	ip_address sender_ip;
	mac_address target_mac;
	ip_address target_ip;
} arp_package;

typedef struct{
	ip_address dest_ip;
	mac_address dest_mac;
} arp_entry;

typedef struct {
    arp_entry data[ARP_TABLE_SIZE];
    int tail;
} arp_table;


/* Exported functions prototypes ---------------------------------------------*/
void arp_table_init(arp_table* table_adr, ip_address src_ip, mac_address src_mac);

int get_mac(ip_address ip, mac_address* mac_addr);


#endif /* __ARP_H */