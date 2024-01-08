/* Includes ------------------------------------------------------------------*/

#include "eth.h"

static ether_types* types;


/**
 * @brief  Initialize the Ethernet layer with a pointer to the Layer 2 protocol types structure.
 *
 * This function initializes the Ethernet layer with a pointer to the Layer 2 protocol types structure.
 * It sets the pointer to the provided `types_addr` and initializes the index to zero, indicating that
 * no Layer 2 protocols have been added yet.
 *
 * @param  types_addr  A pointer to the Layer 2 protocol types structure.
 *
 * @note   This function assumes the existence of a structure named ether_types that contains the index
 *         for tracking Layer 2 protocol types.
 *
 * @retval None
 */
void eth_init(ether_types* types_addr) {
	// Check if the provided pointer is not NULL
	if (types_addr != NULL) {
		// Set the pointer to the Layer 2 protocol types structure
		types = types_addr;
		
		// Initialize the index to zero, indicating no Layer 2 protocols added yet
		types->idx = 0;
	}
}



/**
 * @brief  Add a Layer 2 protocol type and its corresponding handling function to the Ethernet layer.
 *
 * This function adds a Layer 2 protocol type and its corresponding handling function to the Ethernet layer.
 * It sets the ether_type and func fields of the Layer 2 protocol types structure at the current index,
 * then increments the index if there is space available in the types array.
 *
 * @param  type  The Layer 2 protocol type to be added.
 * @param  func  A pointer to the handling function for the specified ether_type.
 *
 * @note   This function assumes the existence of a structure named ether_types that contains an array
 *         of Layer 2 protocol types and their handling functions, as well as an index to track the
 *         number of added protocols.
 *
 * @retval None
 */
void eth_add_type(uint16_t type, void* func){
	// Set the ether_type and func fields of the Layer 2 protocol types structure at the current index
	types->types[types->idx].ether_type = type;
	types->types[types->idx].func = func;
	
	// Increment the index if there is space available in the types array
	if (types->idx + 1 < ETHER_TYPE_SIZE){
		types->idx = types->idx + 1;
	}
}


/**
 * @brief  Handle an Ethernet packet by identifying the Layer 2 protocol type and calling the corresponding function.
 *
 * This function handles an Ethernet packet by extracting the Layer 2 protocol type from the packet header,
 * comparing it with the registered protocol types in the ether_types structure, and calling the corresponding
 * handling function if a match is found.
 *
 * @param  buf  A pointer to the Ethernet packet buffer.
 *
 * @note   This function assumes the existence of a structure named ether_types that contains an array
 *         of Layer 2 protocol types and their handling functions, as well as an index to track the
 *         number of added protocols.
 *
 * @retval int  Returns 0 if a matching protocol type and handler were found, otherwise returns 1.
 */
int eth_handler(uint8_t* buf) {
	// Get ether_type from the Ethernet packet header
	uint16_t typ = (buf[12]  + (buf[13] << 8));
	
	// Iterate through the registered protocol types in the ether_types structure
	for(uint8_t i = 0; i < ETHER_TYPE_SIZE; i++) { 
		// Check if the ether_type from the packet matches a registered protocol type
		if (typ == types->types[i].ether_type){
			// Call the corresponding handling function for the matched ether_type
			return types->types[i].func(buf);
		}
	}
	// Return 1 if no matching protocol type and handler were found
	return 1;
}