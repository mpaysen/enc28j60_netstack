/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DHCP_H
#define __DHCP_H

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"
#include "enc28_j60.h"
#include "eth.h"
#include "ipv4.h"
#include "arp.h"
#include "udp.h"

/* Defines ------------------------------------------------------------------*/
#define UDP_SERVICES_SIZE 2
//Little Endian
#define DHCP_LPORT 	0x4400 // Port 68
#define DHCP_RPORT 	0x4300 // Port 67

#define DHCP_DISCOVER	0x01 
#define DHCP_OFFER 		0x02
#define DHCP_REQUEST 	0x03
#define DHCP_ACK			0x05

#define DHCP_OP_53			0x35


// Option 1 Subnet Mask
typedef struct{
	uint8_t option_type; //1
	uint8_t length;
	ip_address subnet_mask;
} option_1;

// Option 3 Router IP Address
typedef struct{
	uint8_t option_type; //3
	uint8_t length;
	ip_address router;
} option_3;

// Option 50 Request IP Address
typedef struct{
	uint8_t option_type; //50
	uint8_t length;
	ip_address ip_addr;
} option_50;

// Option 51 IP Address Lease Time
typedef struct{
	uint8_t option_type; //51
	uint8_t length;
	uint32_t ip_address_lease_time;
} option_51;

// Option 53 DHCP Message Type
typedef struct{
	uint8_t option_type; //53
	uint8_t length;
	uint8_t dhcp_option;
} option_53;

// Option 54 DHCP Server Identifier
typedef struct{
	uint8_t option_type; //54
	uint8_t length;
	ip_address ip_addr;
} option_54;

// Option 55 Parameter Request List
typedef struct{
	uint8_t option_type; //55
	uint8_t length;
	uint8_t sub_mask;
	uint8_t router;
	uint8_t dns;
	uint8_t ntps;
} option_55;

// Option 58 Renewal Time Value
typedef struct{
	uint8_t option_type; //58
	uint8_t length;
	uint32_t renewal_time_value;
} option_58;

// Option 59 Rebinding Time Value
typedef struct{
	uint8_t option_type; //59
	uint8_t length;
	uint32_t rebinding_time_value;
} option_59;

// Option 61 Client identifier
typedef struct{
	uint8_t option_type; //61
	uint8_t length;
	uint8_t hw_type;
	mac_address mac_addr;
} option_61;

// Option 255 End
typedef struct{
	uint8_t option_type; //255
} option_255;


typedef struct{
	uint8_t data[10];
} padding;

typedef struct{
	uint8_t data[64];
} server;

typedef struct{
	uint8_t data[128];
} file;


typedef struct{
	uint8_t type;
	uint8_t hw_type;
	uint8_t hw_len;
	uint8_t hops;
	uint32_t id;
	uint16_t secs;
	uint16_t flags;
	ip_address ip_client;
	ip_address ip_your;
	ip_address ip_server;
	ip_address ip_relay;
	mac_address mac_addr;
	padding addr_padding;
	server name;
	file boot;
	uint32_t cookie;
}__attribute__((packed)) dhcp_header;


/* Exported functions prototypes ---------------------------------------------*/
void dhcp_init(ip_address *my_ip, ip_address *my_subnet, ip_address *my_gateway, ip_address *my_dhcp_server, uint8_t *dhcp_rdy, mac_address src_mac);

void send_dhcp_disc();

//int handle_dhcp(uint8_t* buf, uint16_t lenght);


#endif /* __DHCP_H */