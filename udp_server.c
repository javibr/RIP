#include "arp.h"
#include "eth.h"
#include "ipv4.h"
#include "ipv4_route_table.h"
#include "ipv4_config.h"
#include "udp.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>



int main(){
  
  
  
  
	int payload_len;
	ipv4_addr_t ip_dest_addr;
	char *nom_fichero_route_table="ipv4_route_table_server.txt";
	char *nom_fichero_config="ipv4_config_server.txt";
    uint16_t * p_destino;
    uint16_t p_escuchador=65000;
	
	int error=0;
  
  //abrir interfaz
  error=open_udp(nom_fichero_route_table,nom_fichero_config);
	if (error==-1) {
        printf("Error al abrir la interfaz\n");
        exit(-1);
    }
    unsigned char buffer[1472];
    long int timeout = -1;
  
  while(1){
      
  //recibir
   payload_len = rec_udp(p_escuchador, p_destino,ip_dest_addr,buffer,timeout);
	if (payload_len==-1) {
        printf("Error al recibir\n");
        exit(-1);
    }
  
  
  //enviar
  error=send_udp(p_escuchador,*p_destino,ip_dest_addr,buffer,payload_len);
	if (error==-1) {
        printf("Error al enviar (main.client)\n");
        exit(-1);
    }
    
  printf("Trama enviada (MAIN)\n");
  }
  //cerrar interfaz
  
  error=close_udp();
    if(error==-1){
        printf("Error al cerrar la interfaz\n");
        exit(-1);
    }
    
    
    
    
    return 0;
  
  
  
 
  
}
