/* Includes ------------------------------------------------------------------*/
#include "arp.h"

static arp_table* table;
static ip_address my_ip;
static mac_address my_mac;

/* private functions prototypes ---------------------------------------------*/
int handle_arp(uint8_t* buf);


/**
 * @brief  Initialize the ARP table with a specified source IP and MAC address.
 *
 * This function initializes the ARP table by adding the ARP EtherType and its corresponding handling
 * function to the Ethernet layer using the `eth_add_type` function. It sets the pointer to the provided
 * `table_adr` as the ARP Table, assigns the provided source IP and MAC addresses, and initializes the
 * ARP Table tail to zero.
 *
 * @param  table_adr  A pointer to the ARP table structure.
 * @param  src_ip     The source IP address for ARP requests and responses.
 * @param  src_mac    The source MAC address for ARP requests and responses.
 *
 * @note   This function assumes the existence of a structure named arp_table that represents the ARP table,
 *         a structure named ip_address for representing IP addresses, a structure named mac_address for
 *         representing MAC addresses, and a function named handle_arp for handling ARP packets.
 *
 * @retval None
 */
void arp_table_init(arp_table* table_adr, ip_address src_ip, mac_address src_mac) {
	//ip_address default_ip = {0x00, 0x00, 0x00, 0x00};
	//mac_address default_mac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	//arp_entry default_entry = {default_ip, default_mac};
	
	//for (int i = 0; i < ARP_TABLE_SIZE; i++) {
  //      table->data[i] = default_entry;
  //  }
	
	// Add the ARP EtherType and its corresponding handling function to the Ethernet layer
	eth_add_type(ARP_TYPE, &handle_arp);
	
	// Set the pointer to the ARP Table
	table = table_adr;
	
	// Assign the provided source IP and MAC addresses
	my_ip = src_ip;
	my_mac = src_mac;
	
	// Initialize the ARP Table tail to zero
  table->tail = 0;
}



/**
 * @brief  Add an ARP entry to the ARP table or update an existing entry with the provided entry.
 *
 * This function iterates through the ARP table to find an entry with the same destination MAC address as
 * the provided entry. If a matching entry is found, it updates the entry with the new information. If no
 * matching entry is found, it adds the provided entry to the ARP table, updating the table's tail if space
 * is available.
 *
 * @param  entry  The ARP entry to be added or updated in the ARP table.
 *
 * @note   This function assumes the existence of a structure named arp_table that represents the ARP table,
 *         a structure named arp_entry for representing ARP entries, and a structure named mac_address for
 *         representing MAC addresses.
 *
 * @retval None
 */
void add_to_arp_table(arp_entry entry) {
		// Iterate through the ARP table to find an entry with the same destination MAC address
	  for (int i = 0; i <= table->tail; i++) {
			// Check if the destination MAC address matches the current entry in the table
			if (table->data[i].dest_mac.octet[0] == entry.dest_mac.octet[0] &&
           table->data[i].dest_mac.octet[1] == entry.dest_mac.octet[1] &&
           table->data[i].dest_mac.octet[2] == entry.dest_mac.octet[2] &&
           table->data[i].dest_mac.octet[3] == entry.dest_mac.octet[3] &&
           table->data[i].dest_mac.octet[4] == entry.dest_mac.octet[4] &&
           table->data[i].dest_mac.octet[5] == entry.dest_mac.octet[5]){
					 // Update the existing entry with the new information
           table->data[i] = entry;
					 return;
        }
				
    }
		// If no matching entry is found, add the provided entry to the ARP table
    table->data[table->tail] = entry;
		
		// Update the table's tail if space is available
    if(table->tail < ARP_TABLE_SIZE - 1) {
			table->tail += 1;
		}
}




/**
 * @brief  Get the MAC address corresponding to the specified IP address from the ARP table.
 *
 * This function searches the ARP table for an entry with the specified destination IP address.
 * If a matching entry is found, it swaps the position of that entry with the previous entry in
 * the ARP table (if applicable) and returns the corresponding MAC address. If no matching entry
 * is found, it returns 0, and the MAC parameter is left unchanged.
 *
 * @param  ip   The destination IP address for which to find the MAC address.
 * @param  mac  A pointer to a mac_address structure to store the resulting MAC address.
 *
 * @note   This function assumes the existence of a structure named arp_table that represents the ARP table,
 *         a structure named arp_entry for representing ARP entries, and a structure named mac_address for
 *         representing MAC addresses.
 *
 * @retval int  Returns 1 if a matching entry and MAC address were found, otherwise returns 0.
 */
int get_mac_from_table(ip_address ip, mac_address* mac) {
		// Initialize the index of the found entry to -1
    int foundIndex = -1;

		// Iterate through the ARP table to find an entry with the specified destination IP address
    for (int i = 0; i <= table->tail; i++) {
				// Check if the destination IP address matches the current entry in the table
        if (table->data[i].dest_ip.octet[0] == ip.octet[0] &&
            table->data[i].dest_ip.octet[1] == ip.octet[1] &&
            table->data[i].dest_ip.octet[2] == ip.octet[2] &&
            table->data[i].dest_ip.octet[3] == ip.octet[3]) {
						// Save the index of the found entry and break the loop
            foundIndex = i;
            break; 
        }
    }

		// If a matching entry is found, swap its position with the previous entry (if applicable)
    if (foundIndex >= 0) {
        if (foundIndex > 0) {
            int prevIndex = (foundIndex - 1);
            arp_entry temp = table->data[foundIndex];
            table->data[foundIndex] = table->data[prevIndex];
            table->data[prevIndex] = temp;
        }
				
				// Return the MAC address corresponding to the found entry
        *mac = table->data[foundIndex].dest_mac; 
        return 1;
    }
		// Return 0 if no matching entry and MAC address were found
    return 0;
}


/**
 * @brief  Process an ARP reply packet and update the ARP table with the new ARP entry.
 *
 * This function extracts the destination MAC address and IP address from an ARP reply packet,
 * creates an ARP entry with the obtained information, and adds or updates the entry in the ARP table.
 *
 * @param  buf  A pointer to the buffer containing the ARP reply packet.
 *
 * @note   This function assumes the existence of a structure named arp_entry for representing ARP entries,
 *         a structure named mac_address for representing MAC addresses, and a structure named ip_address for
 *         representing IP addresses. It also assumes the availability of the add_to_arp_table function.
 *
 * @retval None
 */
void get_arp_rep(uint8_t* buf){
	// Create an ARP entry to store the information from the ARP reply packet
	arp_entry entry;
	
	// Extract the destination MAC address from the ARP reply packet
	entry.dest_mac.octet[0] = buf[22];
	entry.dest_mac.octet[1] = buf[23];
	entry.dest_mac.octet[2] = buf[24];
	entry.dest_mac.octet[3] = buf[25];
	entry.dest_mac.octet[4] = buf[26];
	entry.dest_mac.octet[5] = buf[27];
	
	// Extract the destination IP address from the ARP reply packet
	entry.dest_ip.octet[0] = buf[28];
	entry.dest_ip.octet[1] = buf[29];
	entry.dest_ip.octet[2] = buf[30];
	entry.dest_ip.octet[3] = buf[31];
	
	// Add or update the ARP entry in the ARP table
	add_to_arp_table(entry);
}




/**
 * @brief  Send an ARP request packet.
 *
 * This function constructs and sends an ARP request packet over the network using the ENC28J60 module.
 * The function creates the necessary layers of the packet, including the MAC header and ARP package,
 * and sends the packet to the specified target IP address.
 *
 * @param  src_ip     The source IP address for the ARP request packet.
 * @param  src_mac    The source MAC address for the ARP request packet.
 * @param  target_ip  The target IP address for which the ARP request is sent.
 *
 * @note   This function assumes the existence of structures named mac_address, ip_address, mac_header,
 *         and arp_package for representing MAC addresses, IP addresses, MAC headers, and ARP packages, respectively.
 *         It also assumes the availability of the enc28_packetSend function for sending packets using the ENC28J60 module.
 *
 * @retval None
 */
void send_arp_req(ip_address src_ip, mac_address src_mac, ip_address target_ip){
	// Define a structure to represent the entire ARP request packet
	struct package {
		mac_header mac_header;
		arp_package arp_package;
	};
	
	// Create an instance of the package structure and initialize its fields
	struct package req;
	
	// Layer 2 - MAC header
	req.mac_header.dest_mac = (mac_address){0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	req.mac_header.src_mac = src_mac;
	req.mac_header.ether_type = ARP_TYPE;
	
	// ARP package
	req.arp_package.hw_type = ARP_HW_TYPE;
	req.arp_package.pr_type = ARP_PR_TYPE;
	req.arp_package.hw_size = ARP_HW_SIZE;
	req.arp_package.pr_size = ARP_PR_SIZE;
	req.arp_package.opcode = ARP_REQ;
	req.arp_package.sender_mac = src_mac;
	req.arp_package.sender_ip = src_ip;
	req.arp_package.target_mac = (mac_address){0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	req.arp_package.target_ip = target_ip;
	
	// Send the ARP request packet
	enc28_packetSend(42, (uint8_t*)&req);
}




/**
 * @brief  Send an ARP reply packet.
 *
 * This function constructs and sends an ARP reply packet over the network using the ENC28J60 module.
 * The function creates the necessary layers of the packet, including the MAC header and ARP package,
 * and sends the packet to the specified target IP address with its corresponding MAC address.
 *
 * @param  src_ip       The source IP address for the ARP reply packet.
 * @param  src_mac      The source MAC address for the ARP reply packet.
 * @param  target_ip    The target IP address to which the ARP reply is sent.
 * @param  target_mac   The target MAC address corresponding to the target IP address.
 *
 * @note   This function assumes the existence of structures named mac_address, ip_address, mac_header,
 *         and arp_package for representing MAC addresses, IP addresses, MAC headers, and ARP packages, respectively.
 *         It also assumes the availability of the enc28_packetSend function for sending packets using the ENC28J60 module.
 *
 * @retval None
 */
void send_arp_rep(ip_address src_ip, mac_address src_mac, ip_address target_ip, mac_address target_mac){
	
	// Define a structure to represent the entire ARP reply packet
	struct package {
		mac_header mac_header;
		arp_package arp_package;
	};
	
	// Create an instance of the package structure and initialize its fields
	struct package rep;
	
	 // Layer 2 - MAC header
	rep.mac_header.dest_mac = target_mac;
	rep.mac_header.src_mac = src_mac;
	rep.mac_header.ether_type = ARP_TYPE;
	
	// ARP package
	rep.arp_package.hw_type = ARP_HW_TYPE;
	rep.arp_package.pr_type = ARP_PR_TYPE;
	rep.arp_package.hw_size = ARP_HW_SIZE;
	rep.arp_package.pr_size = ARP_PR_SIZE;
	rep.arp_package.opcode = ARP_REPLY;
	rep.arp_package.sender_mac = src_mac;
	rep.arp_package.sender_ip = src_ip;
	rep.arp_package.target_mac = target_mac;
	rep.arp_package.target_ip = target_ip;
	
	// Send the ARP reply packet
	enc28_packetSend(42, (uint8_t*)&rep);
}


/**
 * @brief  Process an incoming ARP request and send a corresponding ARP reply.
 *
 * This function checks if the incoming ARP request is addressed to the specified IP address (my_ip).
 * If the destination IP matches, the function extracts information from the ARP request,
 * constructs an ARP reply packet, and sends it back to the source MAC and IP addresses.
 *
 * @param  buf       Pointer to the buffer containing the incoming ARP request packet.
 * @param  my_ip     The IP address of the device processing the ARP request.
 * @param  my_mac    The MAC address of the device processing the ARP request.
 *
 * @note   This function assumes the existence of structures named ip_address, mac_address, and arp_entry
 *         for representing IP addresses, MAC addresses, and ARP entries, respectively.
 *         It also assumes the availability of the send_arp_rep function for sending ARP reply packets.
 *
 * @retval None
 */
void get_arp_req(uint8_t* buf, ip_address my_ip, mac_address my_mac){
	// Check if the ARP request is addressed to the specified IP address (my_ip)
	if (my_ip.octet[0] == buf[38] &&
      my_ip.octet[1] == buf[39] &&
      my_ip.octet[2] == buf[40] &&
      my_ip.octet[3] == buf[41]) {
			
			// Extract information from the ARP request
			arp_entry entry;
			entry.dest_mac.octet[0] = buf[22];
			entry.dest_mac.octet[1] = buf[23];
			entry.dest_mac.octet[2] = buf[24];
			entry.dest_mac.octet[3] = buf[25];
			entry.dest_mac.octet[4] = buf[26];
			entry.dest_mac.octet[5] = buf[27];
	
			entry.dest_ip.octet[0] = buf[28];
			entry.dest_ip.octet[1] = buf[29];
			entry.dest_ip.octet[2] = buf[30];
			entry.dest_ip.octet[3] = buf[31];	
			
			// Send an ARP reply to the source MAC and IP addresses
			send_arp_rep(my_ip, my_mac, entry.dest_ip, entry.dest_mac);
			}
			return;
}


/**
 * @brief  Get the MAC address corresponding to the given IP address.
 *
 * This function attempts to retrieve the MAC address associated with the specified IP address from
 * the ARP table. If the MAC address is found in the table, it is returned in the mac_addr parameter,
 * and the function returns 1. If the MAC address is not found, an ARP request is sent to obtain it,
 * and the function returns 0. The ARP reply, when received, will update the ARP table with the mapping.
 *
 * @param  ip          The target IP address for which to retrieve the MAC address.
 * @param  mac_addr    Pointer to the mac_address structure to store the resulting MAC address.
 *
 * @note   This function assumes the availability of structures named ip_address and mac_address
 *         for representing IP addresses and MAC addresses, respectively. It also assumes the existence
 *         of the get_mac_from_table and send_arp_req functions for ARP table lookup and ARP request, respectively.
 *
 * @retval 1 if the MAC address is successfully retrieved from the ARP table.
 * @retval 0 if an ARP request is sent to obtain the MAC address, and the table is updated upon receiving the ARP reply.
 */
int get_mac(ip_address ip, mac_address* mac_addr){
	// Attempt to retrieve the MAC address from the ARP table
	if(get_mac_from_table(ip, mac_addr)) {
		return 1; // MAC address found in the ARP table
	}
	// If not found, send an ARP request to obtain the MAC address
	send_arp_req(my_ip, my_mac, ip);
	return 0; // ARP request sent; the ARP reply will update the ARP table
}


/**
 * @brief  Handle ARP (Address Resolution Protocol) packets.
 *
 * This function processes ARP packets and performs corresponding actions based on the ARP opcode.
 * If the opcode indicates an ARP request, the function responds with an ARP reply containing the MAC
 * address associated with the local IP address. If the opcode indicates an ARP reply, the function adds
 * the sender's IP and MAC address to the ARP table.
 *
 * @param  buf  Pointer to the ARP packet data.
 *
 * @note   This function assumes the existence of functions named get_arp_req and get_arp_rep for
 *         handling ARP requests and replies, respectively. It also relies on structures named ip_address
 *         and mac_address for representing IP addresses and MAC addresses.
 *
 * @retval 0 indicating successful handling of the ARP packet.
 */
int handle_arp(uint8_t* buf){
		// Extract ARP opcode from the packet
		if ((buf[20]  + (buf[21] << 8)) == ARP_REQ){get_arp_req(buf, my_ip, my_mac);}  // ARP Request: Process and send ARP Reply
		if ((buf[20]  + (buf[21] << 8)) == ARP_REPLY){get_arp_rep(buf);} // ARP Reply: Add sender's IP and MAC address to the ARP table
		return 0;
}
