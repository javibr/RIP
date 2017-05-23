
#include "arp.h"
#include "eth.h"
#include "eth.c"


#include <rawnet.h>
#include <timerms.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

#define ARP_HEADER_SIZE 28

//ipv4_addr_t maquina_ip ={0,0,0,0};
mac_addr_t maquina_mac = {0,0,0,0,0,0};


 /*Estructura de la cabecera de arp*/
struct arp_frame
{
 
    uint16_t hard_address;    /* Tipo hardware */
    uint16_t prot_address;    /*Tipo protocolo*/
    uint8_t  length_hard;    /*byte length of each hardware address*/
    uint8_t length_prot;    /* byte length of each protocol address*/
    uint16_t op_code;       /*tipo de respuesta */
    mac_addr_t mac_orig;    /*dir MAC origen */
    ipv4_addr_t ip_orig;    /*dir IP  origen  */
    mac_addr_t mac_dest;    /*dir MAC destino  */
    ipv4_addr_t ip_dest;    /*dir IP  destino  */
    
};


 /*int arp_resolve( et_iface_t *iface, ipv4_addr_t dest,mac_addr_t mac)
 
 DESCRIPCIÓN
 
 Esta función envia una trma arp request,solicitando una direcc MAC
 asociada a la dir IP pedida.
  */
 
 
 //CAMBIAR LA IP ORIGEN POR LA QUE COJEMOS DEL TXT
 
int arp_resolve (ipv4_addr_t ip_origen, eth_iface_t * iface, ipv4_addr_t dest, mac_addr_t mac )
    {
    
      
        unsigned char buffer[ETH_MTU];
        timerms_t timer;
        long int timeout = 2000;
        char src_addr_str[MAC_STR_LENGTH];
 
        mac_addr_t src_addr;
        struct arp_frame *recibido;

        int compara_ip=0;
        int len;
        int err;
 
 
      struct  arp_frame arp_frame;
    
        
        arp_frame.hard_address=htons(0x1);    
        arp_frame.prot_address=htons(0x800);   
        arp_frame.length_hard=6;   
        arp_frame.length_prot=4;  
        arp_frame.op_code=htons(0x1);

        eth_getaddr(iface, arp_frame.mac_orig);
        memcpy(arp_frame.ip_orig,ip_origen,IPv4_ADDR_SIZE);
        memset((arp_frame.mac_dest),0,MAC_ADDR_SIZE);
        memcpy(arp_frame.ip_dest,dest,IPv4_ADDR_SIZE );

        
        /* Enviar trama Ethernet al Servidor */

        err = eth_send( iface, MAC_BCAST_ADDR, 0x806, (unsigned char*)&(arp_frame),sizeof(arp_frame));
        printf("enviado\n");
	 
       if (err == -1)
           {
            fprintf(stderr, " ERROR en eth_send()\n");
            exit(-1);
            }   

     /* Recibir trama Ethernet del Servidor y procesar errores */
               
      timerms_reset(&timer,timeout);
      do{
	    long int time= timerms_left(&timer);
	    
            len = eth_recv(iface, src_addr, 0X0806, buffer,time);
	    if (len == -1)
	    {
                fprintf(stderr, ": ERROR en eth_recv()\n");
		return -1;
            } else if (len == 0) {
                fprintf(stderr, ": ERROR No hay respuesta del Servidor ARP\n");
                return 0;
            }
        
	     recibido=(struct arp_frame*)buffer;  
	     
	     compara_ip=memcmp(recibido->ip_orig,dest,IPv4_ADDR_SIZE)==0;
	     
            }
                        
        
        while(!(((ntohs(recibido->op_code)==2)&&(compara_ip))));
        

		mac_addr_str(src_addr, src_addr_str);
            
		printf("Recibidos %d bytes del Servidor ARP (%s)\n",len, src_addr_str);
		print_pkt(buffer, len, 0);
	

		memcpy(mac,(recibido->mac_orig),MAC_ADDR_SIZE);
     
        return 1;
	
	

    
    
    
    }




