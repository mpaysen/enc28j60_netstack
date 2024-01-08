/* Includes ------------------------------------------------------------------*/
#include "udp.h"

static ip_address my_ip;
static mac_address my_mac;
static udp_serivces* serivces;

/* private functions prototypes ---------------------------------------------*/
int handle_udp(uint8_t* buf);


/**
 * @brief Initializes the UDP layer.
 *
 * This function registers the UDP protocol type with the IPv4 layer
 * and sets the pointer to a structure that holds information about UDP services.
 *
 * @param serivces_addr Pointer to the UDP services structure.
 * @param src_ip Source IP address for the UDP layer.
 * @param src_mac Source MAC address for the UDP layer.
 */
void udp_init(udp_serivces* serivces_addr, ip_address src_ip, mac_address src_mac) {
	// Check if the UDP services structure pointer is not NULL
	if (serivces_addr != NULL) {
		// Add UDP to the IPv4 layer's protocol types and associate it with the handle_udp function
		ipv4_add_type(UDP_TYPE, &handle_udp); 
		// Set the pointer to the UDP services structure
		serivces = serivces_addr;
		// Set the index to zero, assuming no protocols have been added yet
		serivces->idx = 0;
	}
	// Set the source IP and MAC addresses for the UDP layer
	my_ip = src_ip;
	my_mac = src_mac;
}


/**
 * @brief Adds support for a specific UDP service.
 *
 * This function registers a function to handle UDP packets for a given local port.
 *
 * @param lport Local port for the UDP service.
 * @param func Pointer to the function that handles UDP packets for the specified local port.
 */
void udp_add_type(uint16_t lport, void* func){
	// Set UDP local port and associated function in the UDP services structure
	serivces->serivces[serivces->idx].lport = lport;
	serivces->serivces[serivces->idx].func = func;
	
	// Increment the index, assuming successful addition
	if (serivces->idx + 1 < UDP_SERVICES_SIZE){
		serivces->idx = serivces->idx + 1;
	}
}



/**
 * @brief Handles incoming UDP packets.
 *
 * This function identifies the local port of an incoming UDP packet and dispatches
 * the packet to the appropriate handler function registered for that port.
 *
 * @param buf Pointer to the UDP packet data.
 * @return 0 if the packet is handled successfully, 1 otherwise.
 */
int handle_udp(uint8_t* buf) {
	// Extract UDP destination port from the packet
	uint16_t lport = (buf[36]  + (buf[37] << 8));
	
	// Iterate through the UDP services to find a match for the local port
	for(uint8_t i = 0; i < UDP_SERVICES_SIZE; i++) { 
		if (lport == serivces->serivces[i].lport){
			
			// Call the registered handler function for the identified local port
			return serivces->serivces[i].func(buf);
		}
	}
	// No matching service found for the UDP destination port
	return 1;
}




// Has already been written in icmp.c, maybe add it somewhere more generally?

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

