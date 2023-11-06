/* Includes ------------------------------------------------------------------*/
#include "arp.h"

static arp_table* table;
static ip_address my_ip;
static mac_address my_mac;

/* private functions prototypes ---------------------------------------------*/
int handle_arp(uint8_t* buf);

void arp_table_init(arp_table* table_adr, ip_address src_ip, mac_address src_mac) {
	//ip_address default_ip = {0x00, 0x00, 0x00, 0x00};
	//mac_address default_mac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	//arp_entry default_entry = {default_ip, default_mac};
	
	//for (int i = 0; i < ARP_TABLE_SIZE; i++) {
  //      table->data[i] = default_entry;
  //  }
	eth_add_type(ARP_TYPE, &handle_arp); //add ARP eth_type and arp_handler function
	table = table_adr; //set ptr to ARP Table
	my_ip = src_ip;
	my_mac = src_mac;
  table->tail = 0; //set ARP Table tail to zero
}

void add_to_arp_table(arp_entry entry) {
	  for (int i = 0; i <= table->tail; i++) {
			if (table->data[i].dest_mac.octet[0] == entry.dest_mac.octet[0] &&
           table->data[i].dest_mac.octet[1] == entry.dest_mac.octet[1] &&
           table->data[i].dest_mac.octet[2] == entry.dest_mac.octet[2] &&
           table->data[i].dest_mac.octet[3] == entry.dest_mac.octet[3] &&
           table->data[i].dest_mac.octet[4] == entry.dest_mac.octet[4] &&
           table->data[i].dest_mac.octet[5] == entry.dest_mac.octet[5]){
           table->data[i] = entry;
					 return;
        }
				
    }
    table->data[table->tail] = entry;
    if(table->tail < ARP_TABLE_SIZE - 1){table->tail += 1;}
}
		
int get_mac_from_table(mac_address* mac, ip_address ip) {
    int foundIndex = -1;


    for (int i = 0; i <= table->tail; i++) {
        if (table->data[i].dest_ip.octet[0] == ip.octet[0] &&
            table->data[i].dest_ip.octet[1] == ip.octet[1] &&
            table->data[i].dest_ip.octet[2] == ip.octet[2] &&
            table->data[i].dest_ip.octet[3] == ip.octet[3]) {
            foundIndex = i;
            break; 
        }
    }

    if (foundIndex >= 0) {
        
        if (foundIndex > 0) {
            int prevIndex = (foundIndex - 1);
            arp_entry temp = table->data[foundIndex];
            table->data[foundIndex] = table->data[prevIndex];
            table->data[prevIndex] = temp;
        }

        *mac = table->data[foundIndex].dest_mac; 
        return 1;
    }

    return 0; 
}

void get_arp_rep(uint8_t* buf){
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
	
	add_to_arp_table(entry);
}

void send_arp_req(ip_address src_ip, mac_address src_mac, ip_address target_ip){
	arp_package req;
	req.mac_header.dest_mac = (mac_address){0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	req.mac_header.src_mac = src_mac;
	req.mac_header.ether_type = ARP_TYPE;
	req.hw_type = ARP_HW_TYPE;
	req.pr_type = ARP_PR_TYPE;
	req.hw_size = ARP_HW_SIZE;
	req.pr_size = ARP_PR_SIZE;
	req.opcode = ARP_REQ;
	req.sender_mac = src_mac;
	req.sender_ip = src_ip;
	req.target_mac = (mac_address){0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	req.target_ip = target_ip;
	enc28_packetSend(42, (uint8_t*)&req);
}

void send_arp_rep(ip_address src_ip, mac_address src_mac, ip_address target_ip, mac_address target_mac){
	arp_package rep;
	rep.mac_header.dest_mac = target_mac;
	rep.mac_header.src_mac = src_mac;
	rep.mac_header.ether_type = ARP_TYPE;
	rep.hw_type = ARP_HW_TYPE;
	rep.pr_type = ARP_PR_TYPE;
	rep.hw_size = ARP_HW_SIZE;
	rep.pr_size = ARP_PR_SIZE;
	rep.opcode = ARP_REPLY;
	rep.sender_mac = src_mac;
	rep.sender_ip = src_ip;
	rep.target_mac = target_mac;
	rep.target_ip = target_ip;
	enc28_packetSend(42, (uint8_t*)&rep);
}

void get_arp_req(uint8_t* buf, ip_address my_ip, mac_address my_mac){
	if (my_ip.octet[0] == buf[38] &&
      my_ip.octet[1] == buf[39] &&
      my_ip.octet[2] == buf[40] &&
      my_ip.octet[3] == buf[41]) {
			
			arp_entry entry;
			entry.dest_mac.octet[0] = buf[6];
			entry.dest_mac.octet[1] = buf[7];
			entry.dest_mac.octet[2] = buf[8];
			entry.dest_mac.octet[3] = buf[9];
			entry.dest_mac.octet[4] = buf[10];
			entry.dest_mac.octet[5] = buf[11];
	
			entry.dest_ip.octet[0] = buf[28];
			entry.dest_ip.octet[1] = buf[29];
			entry.dest_ip.octet[2] = buf[30];
			entry.dest_ip.octet[3] = buf[31];	
				
			send_arp_rep(my_ip, my_mac, entry.dest_ip, entry.dest_mac);
			}
			return;
}


int handle_arp(uint8_t* buf){
		if ((buf[20]  + (buf[21] << 8)) == ARP_REQ){get_arp_req(buf, my_ip, my_mac);}
		if ((buf[20]  + (buf[21] << 8)) == ARP_REPLY){get_arp_rep(buf);}
		return 0;
}
