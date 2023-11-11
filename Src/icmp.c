/* Includes ------------------------------------------------------------------*/
#include "icmp.h"


static ip_address my_ip;
static mac_address my_mac;

/* private functions prototypes ---------------------------------------------*/
int handle_icmp(uint8_t* buf);

void icmp_init(ip_address src_ip, mac_address src_mac) {

	ipv4_add_type(ICMP_TYPE, &handle_icmp); //add ICMP eth_type and icmp_handler function
	my_ip = src_ip;
	my_mac = src_mac;
}

uint16_t calculate_checksum(const void* data, size_t length) {
    const uint16_t* p = data;
    uint32_t sum = 0;

    // Summation of the 16-bit values in the data area
    while (length > 1) {
        sum += *p++;
        length -= 2;
    }

    // If the length is odd, add the last byte
    if (length > 0) {
        sum += *(uint8_t*)p;
    }

    // Add the carryovers
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    // Invert the bits to get checksum
    return (uint16_t)~sum;
}

void send_icmp_req(ip_address target_ip){
		struct package {
		mac_header mac_header;
		ipv4_header ipv4_header;
		icmp_package icmp_package;
	};
	struct package req;
	mac_address dest_mac;
	if(get_mac(target_ip, &dest_mac) != 1){return;}
	//layer2
	req.mac_header.dest_mac = dest_mac;
	req.mac_header.src_mac = my_mac;
	req.mac_header.ether_type = IPV4_TYPE;
	//layer3
	req.ipv4_header.version_length = IPV4_VERSION;
	req.ipv4_header.service_field = 0x00;
	req.ipv4_header.total_length = 0x3c00;
	req.ipv4_header.ident = calculate_next_id();
	req.ipv4_header.flags = 0x00;
	req.ipv4_header.ttl = 0xff;
	req.ipv4_header.prtcl = 0x01;
	req.ipv4_header.src = my_ip;
	req.ipv4_header.dst = target_ip;
	req.ipv4_header.header_checksum = calculate_checksum(&req.ipv4_header, sizeof(req.ipv4_header));
	//package
	req.icmp_package.type = ICMP_REQ;
	req.icmp_package.code = ICMP_CODE;
	req.icmp_package.ident = ICMP_IDENT;
	req.icmp_package.seq = ICMP_SEQ;
	req.icmp_package.data = (payload) {0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69};
	req.icmp_package.checksum = calculate_checksum(&req.icmp_package, sizeof(req.icmp_package));
	enc28_packetSend(74, (uint8_t*)&req);
}

void send_icmp_rep(ip_address target_ip, uint16_t ident, uint16_t seq, uint8_t ttl){
		struct package {
		mac_header mac_header;
		ipv4_header ipv4_header;
		icmp_package icmp_package;
	};
	struct package rep;
	mac_address dest_mac;
	if(get_mac(target_ip, &dest_mac) != 1){return;}
	//layer2
	rep.mac_header.dest_mac = dest_mac;
	rep.mac_header.src_mac = my_mac;
	rep.mac_header.ether_type = IPV4_TYPE;
	//layer3
	rep.ipv4_header.version_length = IPV4_VERSION;
	rep.ipv4_header.service_field = 0x00;
	rep.ipv4_header.total_length = 0x3c00;
	rep.ipv4_header.ident = calculate_next_id();
	rep.ipv4_header.flags = 0x00;
	rep.ipv4_header.ttl = ttl /2;
	rep.ipv4_header.prtcl = 0x01;
	rep.ipv4_header.src = my_ip;
	rep.ipv4_header.dst = target_ip;
	rep.ipv4_header.header_checksum = calculate_checksum(&rep.ipv4_header, sizeof(rep.ipv4_header));
	//package
	rep.icmp_package.type = ICMP_REPLY;
	rep.icmp_package.code = ICMP_CODE;
	rep.icmp_package.ident = ident;
	rep.icmp_package.seq = seq;
	rep.icmp_package.data = (payload) {0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69};
	rep.icmp_package.checksum = calculate_checksum(&rep.icmp_package, sizeof(rep.icmp_package));
	enc28_packetSend(74, (uint8_t*)&rep);
}

void get_icmp_req(uint8_t* buf){
			ip_address dest_ip;
			dest_ip.octet[0] = buf[26];
			dest_ip.octet[1] = buf[27];
			dest_ip.octet[2] = buf[28];
			dest_ip.octet[3] = buf[29];
	
			uint8_t ttl = buf[22];
			icmp_package pkg;
			pkg.ident = (buf[38]  + (buf[39] << 8));
			pkg.seq = (buf[40]  + (buf[41] << 8));
			
			send_icmp_rep(dest_ip, pkg.ident, pkg.seq, ttl);
			return;
}

int handle_icmp(uint8_t* buf){
		if (buf[34] == ICMP_REQ){get_icmp_req(buf);}
		if (buf[34] == ICMP_REPLY){return 0;}
		return 0;
}