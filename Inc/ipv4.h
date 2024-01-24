/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __IPV4_H
#define __IPV4_H

#include "eth.h"


/* Defines -------------------------------------*/
#define PRTCL_TYPE_SIZE 2
//Little Endian
#define IPV4_TYPE 0x0008
#define IPV4_VERSION 0x45

typedef struct {
	uint8_t prtcl_type;
	int (*func)(const uint8_t* buf, uint16_t length);
} prtcl_type;

typedef struct {
	prtcl_type types[PRTCL_TYPE_SIZE];
	uint8_t idx;
} prtcl_types;

typedef struct {
	uint8_t version_length;
	uint8_t service_field;
	uint16_t total_length;
	uint16_t ident;
	uint16_t flags;
	uint8_t ttl;
	uint8_t prtcl;
	uint16_t header_checksum;
	ip_address src;
	ip_address dst;	
} __attribute__((packed)) ipv4_header;

/* Exported functions prototypes ---------------------------------------------*/
void ipv4_init(prtcl_types* types_addr);

void ipv4_add_type(uint8_t type, void* func);

//int handle_ipv4(uint8_t* buf, uint16_t length);

uint16_t calculate_next_id();

#endif /* __IPV4_H */