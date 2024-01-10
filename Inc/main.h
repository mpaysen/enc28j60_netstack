#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"
#include "eth.h"
#include "ipv4.h"
#include "enc28_j60.h"
#include "arp.h"
#include "icmp.h"
#include "udp.h"
#include "dhcp.h"



/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

#endif /* __MAIN_H */
