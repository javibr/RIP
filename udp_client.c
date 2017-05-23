
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


int main ( int argc, char * argv[] )
{
    
    char * myself = basename(argv[0]);
    if (argc != 3)
    {
		printf("Uso: %s <ip_dest> <p_destino>\n", myself);
		printf("       <ip_dest>: dirección io destino.\n");
        printf("       <p_destino>:puerto destino.\n");
		exit(-1);
    }
	
	
    int payload_len=1472;
	unsigned char payload[payload_len];
    long int timeout=2000;
	ipv4_addr_t ip_dest_addr;
	unsigned char  buffer[1472];
	
	

	char* ip_dest= argv[1];
    uint16_t p_destino= atoi(argv[2]);
    
	char *nom_fichero_route_table="ipv4_route_table_client.txt";
	char *nom_fichero_config="ipv4_config_client.txt";
    
    
	/*Cambiamos a binario la IP*/
	int error_ip=(ipv4_str_addr(ip_dest, ip_dest_addr));
	if(error_ip==-1){
        printf("\n La dirección proporcionada no se corresponde con IPv4.\n");
	}
    /*generamos Payload*/
	int i;
	for (i=0; i<payload_len; i++) {
        payload[i] = (unsigned char)i;
	}
    
    /*Abrimos la interfaz*/
    
	int error=open_udp(nom_fichero_route_table,nom_fichero_config);
	if (error==-1) {
        printf("Error al abrir la interfaz\n");
        exit(-1);
    }
    uint16_t puerto=genera_puerto();
    
    error=send_udp(puerto,p_destino,ip_dest_addr,payload,payload_len);
	if (error==-1) {
        printf("Error al enviar (main.client)\n");
        exit(-1);
    }
    
    
    error = rec_udp(puerto,&p_destino,ip_dest_addr,buffer,timeout);
	if (error==-1) {
        printf("Error al recibir\n");
        exit(-1);
    }
  
	error=close_udp();
    if(error==-1){
        printf("Error al cerrar la interfaz\n");
        exit(-1);
    }
    
    
    
    
    return 0;
}



