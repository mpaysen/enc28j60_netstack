/* Includes ------------------------------------------------------------------*/
#include "icmp.h"

static ip_address my_ip;
static mac_address my_mac;

/* private functions prototypes ---------------------------------------------*/
int handle_icmp(uint8_t* buf);




/**
 * @brief  Initialize ICMP (Internet Control Message Protocol) module.
 *
 * This function initializes the ICMP module by adding ICMP as a supported Layer 3 protocol and
 * associating it with the corresponding handler function. It also sets the local IP address and MAC
 * address for use in ICMP packet handling.
 *
 * @param  src_ip   The source IP address to be set for the local device.
 * @param  src_mac  The source MAC address to be set for the local device.
 *
 * @note   This function assumes the existence of functions named ipv4_add_type and handle_icmp for
 *         adding ICMP as a supported protocol and handling ICMP packets, respectively. It also relies
 *         on structures named ip_address and mac_address for representing IP addresses and MAC addresses.
 */
void icmp_init(ip_address src_ip, mac_address src_mac) {
	// Add ICMP as a supported Layer 3 protocol and associate it with the handler function
	ipv4_add_type(ICMP_TYPE, &handle_icmp);
	
	// Set the local IP address and MAC address for ICMP packet handling
	my_ip = src_ip;
	my_mac = src_mac;
}



/**
 * @brief  Calculate Internet Checksum for the given data.
 *
 * This function calculates the Internet Checksum for the provided data. The Internet Checksum is a
 * simple mathematical checksum algorithm used to detect errors in data transmission.
 *
 * @param  data    Pointer to the data for which the checksum is to be calculated.
 * @param  length  Length of the data in bytes.
 *
 * @return The calculated 16-bit checksum value.
 */
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


/**
 * @brief Sends an ICMP echo request (ping) to the specified target IP address.
 *
 * This function constructs an ICMP echo request packet and sends it to the target IP address.
 *
 * @param target_ip The target IP address to which the ICMP echo request will be sent.
 */
void send_icmp_req(ip_address target_ip){
	// Create a package structure to hold the layers of the ICMP request packet
	struct package {
		mac_header mac_header;
		ipv4_header ipv4_header;
		icmp_package icmp_package;
	};
	 // Initialize the package structure
	struct package req;
	
	// Retrieve the MAC address of the target IP
	mac_address dest_mac;
	if(get_mac(target_ip, &dest_mac) != 1) {
		// Unable to get MAC address; return without sending the ICMP reply
		return;
	}
	
	// Layer 2
	req.mac_header.dest_mac = dest_mac;
	req.mac_header.src_mac = my_mac;
	req.mac_header.ether_type = IPV4_TYPE;
	
	// Layer 3 (IPv4)
	req.ipv4_header.version_length = IPV4_VERSION;
	req.ipv4_header.service_field = 0x00;
	req.ipv4_header.total_length = 0x3c00; // Update this based on the actual length
	req.ipv4_header.ident = calculate_next_id();
	req.ipv4_header.flags = 0x00;
	req.ipv4_header.ttl = 0xff;
	req.ipv4_header.prtcl = 0x01; // ICMP protocol
	req.ipv4_header.src = my_ip;
	req.ipv4_header.dst = target_ip;
	req.ipv4_header.header_checksum = calculate_checksum(&req.ipv4_header, sizeof(req.ipv4_header));
	
	// ICMP Package
	req.icmp_package.type = ICMP_REQ;
	req.icmp_package.code = ICMP_CODE;
	req.icmp_package.ident = ICMP_IDENT;
	req.icmp_package.seq = ICMP_SEQ;
	req.icmp_package.data = (payload) {0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69};
	req.icmp_package.checksum = calculate_checksum(&req.icmp_package, sizeof(req.icmp_package));
		
	// Send the ICMP request
	enc28_packetSend(74, (uint8_t*)&req);
}


/**
 * @brief Sends an ICMP echo reply in response to a received ICMP echo request.
 *
 * This function constructs an ICMP echo reply packet and sends it to the target IP address.
 *
 * @param target_ip The target IP address to which the ICMP echo reply will be sent.
 * @param ident The identifier field from the received ICMP echo request.
 * @param seq The sequence number field from the received ICMP echo request.
 * @param ttl The time-to-live value from the received ICMP echo request.
 */
void send_icmp_rep(ip_address target_ip, uint16_t ident, uint16_t seq, uint8_t ttl){
	// Create a package structure to hold the layers of the ICMP reply packet
	struct package {
		mac_header mac_header;
		ipv4_header ipv4_header;
		icmp_package icmp_package;
	};
	 // Initialize the package structure
	struct package rep;
	
	// Retrieve the MAC address of the target IP
	mac_address dest_mac;
	if(get_mac(target_ip, &dest_mac) != 1){
		// Unable to get MAC address; return without sending the ICMP reply
		return;
	}
	// Layer 2 (Ethernet)
	rep.mac_header.dest_mac = dest_mac;
	rep.mac_header.src_mac = my_mac;
	rep.mac_header.ether_type = IPV4_TYPE;
	// Layer 3 (IPv4)
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
	// Layer 4 (ICMP)
	rep.icmp_package.type = ICMP_REPLY;
	rep.icmp_package.code = ICMP_CODE;
	rep.icmp_package.ident = ident; // Use the identifier from the received ICMP echo request
	rep.icmp_package.seq = seq; // Use the sequence number from the received ICMP echo request
	rep.icmp_package.data = (payload) {0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69};
	rep.icmp_package.checksum = calculate_checksum(&rep.icmp_package, sizeof(rep.icmp_package));
	// Send the ICMP reply
	enc28_packetSend(74, (uint8_t*)&rep);
}


/**
 * @brief Handles incoming ICMP echo requests and sends ICMP echo replies.
 *
 * This function extracts information from the received ICMP echo request packet,
 * such as the destination IP address, TTL, identifier, and sequence number,
 * and sends an ICMP echo reply packet.
 *
 * @param buf Pointer to the buffer containing the received ICMP echo request packet.
 */
void get_icmp_req(uint8_t* buf){
			// Extract destination IP address from the received packet
			ip_address dest_ip;
			dest_ip.octet[0] = buf[26];
			dest_ip.octet[1] = buf[27];
			dest_ip.octet[2] = buf[28];
			dest_ip.octet[3] = buf[29];
	
			// Extract TTL (Time-to-Live) from the received packet
			uint8_t ttl = buf[22];
			// Extract identifier and sequence number from the received packet
			icmp_package pkg;
			pkg.ident = (buf[38]  + (buf[39] << 8));
			pkg.seq = (buf[40]  + (buf[41] << 8));
			// Send an ICMP echo reply
			send_icmp_rep(dest_ip, pkg.ident, pkg.seq, ttl);
			return;
}

/**
 * @brief Handles incoming ICMP packets.
 *
 * This function checks the type of incoming ICMP packet (Request or Reply)
 * and calls the corresponding function to handle the packet.
 *
 * @param buf Pointer to the buffer containing the received ICMP packet.
 * @return 0 (success).
 */
int handle_icmp(uint8_t* buf){
		 // Check the type of ICMP packet
		if (buf[34] == ICMP_REQ){get_icmp_req(buf);} // If it's an ICMP Request, handle the request
		if (buf[34] == ICMP_REPLY){return 0;} // If it's an ICMP Reply, (not implemented)
		return 0;
}