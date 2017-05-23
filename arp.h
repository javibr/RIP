//
//  arp.h
//  arp
//
//  Created by Christian on 26/09/13.
//  Copyright (c) 2013 Christian J. Calva. All rights reserved.
//

#ifndef arp_arp_h
#define arp_arp_h


#include "eth.h"
#include "ipv4.h"

// void ipv4_addr_str(ipv4_addr_t addr, char str[]);
// int ipv4_str_addr(char* str, ipv4_addr_t addr);



int arp_resolve (ipv4_addr_t ip_origen, eth_iface_t * iface, ipv4_addr_t dest, mac_addr_t mac );




#endif
