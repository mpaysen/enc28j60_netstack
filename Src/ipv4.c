/* Includes ------------------------------------------------------------------*/
#include "ipv4.h"

static prtcl_types* types;


/**
 * @brief  Initialize the IPv4 layer with a pointer to the Layer 3 protocol types structure.
 *
 * This function initializes the IPv4 layer by adding the IPv4 EtherType and its corresponding handling
 * function to the Ethernet layer using the `eth_add_type` function. It also sets the pointer to the
 * provided `types_addr` and initializes the index to zero, indicating that no Layer 3 protocols have
 * been added yet.
 *
 * @param  types_addr  A pointer to the Layer 3 protocol types structure.
 *
 * @note   This function assumes the existence of a structure named prtcl_types that contains the index
 *         for tracking Layer 3 protocol types, and it relies on the `eth_add_type` and `handle_ipv4`
 *         functions.
 *
 * @retval None
 */
void ipv4_init(prtcl_types* types_addr) {
	// Check if the provided pointer is not NULL
	if (types_addr != NULL) {
		// Add the IPv4 EtherType and its corresponding handling function to the Ethernet layer
		eth_add_type(IPV4_TYPE, &handle_ipv4);
		
		// Set the pointer to the Layer 3 protocol types structure
		types = types_addr;
		
		// Initialize the index to zero, indicating no Layer 3 protocols added yet
		types->idx = 0;
	}
}


/**
 * @brief  Add a Layer 3 protocol type and its corresponding handling function to the IPv4 layer.
 *
 * This function adds a Layer 3 protocol type and its corresponding handling function to the IPv4 layer.
 * It sets the prtcl_type and func fields of the Layer 3 protocol types structure at the current index,
 * then increments the index if there is space available in the types array.
 *
 * @param  type  The Layer 3 protocol type to be added.
 * @param  func  A pointer to the handling function for the specified prtcl_type.
 *
 * @note   This function assumes the existence of a structure named prtcl_types that contains an array
 *         of Layer 3 protocol types and their handling functions, as well as an index to track the
 *         number of added protocols.
 *
 * @retval None
 */
void ipv4_add_type(uint8_t type, void* func){
	// Set the prtcl_type and func fields of the Layer 3 protocol types structure at the current index
	types->types[types->idx].prtcl_type = type;
	types->types[types->idx].func = func;
	
	// Increment the index if there is space available in the types array
	if (types->idx + 1 < PRTCL_TYPE_SIZE) {
		types->idx = types->idx + 1;
	}
}


/**
 * @brief  Handle an IPv4 packet by identifying the Layer 3 protocol type and calling the corresponding function.
 *
 * This function handles an IPv4 packet by extracting the Layer 3 protocol type from the packet header,
 * comparing it with the registered protocol types in the prtcl_types structure, and calling the corresponding
 * handling function if a match is found.
 *
 * @param  buf  A pointer to the IPv4 packet buffer.
 *
 * @note   This function assumes the existence of a structure named prtcl_types that contains an array
 *         of Layer 3 protocol types and their handling functions, as well as an index to track the
 *         number of added protocols.
 *
 * @retval int  Returns 0 if a matching protocol type and handler were found, otherwise returns 1.
 */
int handle_ipv4(uint8_t* buf) {
	uint8_t typ = buf[23]; // Get prtcl_type from the IPv4 packet header
	
	// Iterate through the registered protocol types in the prtcl_types structure
	for(uint8_t i = 0; i < PRTCL_TYPE_SIZE; i++) { 
		// Check if the prtcl_type from the packet matches a registered protocol type
		if (typ == types->types[i].prtcl_type){
			
			// Call the corresponding handling function for the matched prtcl_type
			return types->types[i].func(buf);
		}
	}
	// Return 1 if no matching protocol type and handler were found
	return 1;
}


/**
 * @brief  Calculate and return the next unique identifier value.
 *
 * This function calculates the next unique identifier value using a static counter.
 * The counter is incremented with each call, and the resulting identifier is swapped
 * in its byte order before being returned.
 *
 * @note   The initial value of the counter is set to 420.
 *
 * @retval uint16_t  The next unique identifier value.
 */
uint16_t calculate_next_id() {
	// Static counter to keep track of the identifier
  static uint16_t id = 420;
	// Swap the byte order of the identifier
	uint16_t Id = ((id & 0xFF) << 8) | ((id & 0xFF00) >> 8);
  ++id;
  return Id;
}