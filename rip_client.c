


#include "arp.h"
#include "eth.h"
#include "ipv4.h"
#include "ipv4_route_table.h"
#include "ipv4_config.h"
#include "udp.h"
#include "rip_route_table.h"
#include "rip.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>





int main (int argc,char *argv[]){
  
  ipv4_addr_t ip_zero ={0,0,0,0};

  
    char * myself = basename(argv[0]);
    if (argc != 2)
    {
		printf("Uso: %s <ip_dest> <p_destino>\n", myself);
		printf("       <ip_dest>: dirección ip destino.\n");

		exit(-1);
    }
    
    
    ipv4_addr_t ip_dest_addr;
	char* ip_dest= argv[1];
	char *nom_fichero_route_table="ipv4_route_table_client.txt";
	char *nom_fichero_config="ipv4_config_client.txt";
    uint16_t p_destino=0;
   // uint16_t p_src=300;
    unsigned char buffer[RIPV2_ENTRYS_MAX_LENGTH+RIPV2_HEADER_LENGTH];
    long int timeout=2000;
    int len_receive;
    
	/*procesamos argumentos*/

    
    int error_ip=(ipv4_str_addr (ip_dest, ip_dest_addr));
    if(error_ip==-1){
        printf("\n La dirección proporcionada no se corresponde con IPv4.\n");
    }
    
     /*creamos la struct rip y la rellenamos*/
    struct rip_segmento rip;
    rip.comand=1; //tipo Request
    rip.version=2; //version 2;
    rip.domain=0;
    
    /*rellenamos solo una entrada*/
    rip.entrys[0].family=0;
    rip.entrys[0].tag=0;
    memcpy(&(rip.entrys[0].subnet_addr),ip_zero,IPv4_ADDR_SIZE);
    memcpy(&(rip.entrys[0].subnet_mask),ip_zero,IPv4_ADDR_SIZE);
    memcpy(&(rip.entrys[0].gateway_addr),ip_zero,IPv4_ADDR_SIZE);
    rip.entrys[0].metric=htonl(16);
    
    
    
    /*abrimos la interfaz*/
    int error=open_udp(nom_fichero_route_table,nom_fichero_config);
	if (error==-1) {
        printf("Error al abrir la interfaz\n");
        exit(-1);
    }
        uint16_t p_src=genera_puerto();
                 
                 
    /*enviar mensaje Request*/
    error=send_udp(p_src,520,ip_dest_addr, (unsigned char*)& (rip),24);
	if (error==-1) {
        printf("Error al enviar (main.client)\n");
        exit(-1);
    }

    /*recibimos la respuesta*/
    len_receive= rec_udp(p_src,&p_destino,ip_dest_addr,buffer,timeout);
	if (len_receive==-1) {
        printf("Error al recibir(main.rip_client\n");
        exit(-1);
    }
    if(len_receive>0)
    {
        
         printf("Respuesta recibida \n");
        printf("Rutas Rip: \n");
        int i=0;
        int numero_entradas = ((len_receive-4)/RIPV2_ENTRY_LEN);
        
        struct rip_segmento *recibido=(struct rip_segmento*)buffer;
        
        char  subnet_ip[16];
        char  mask_ip[256];
        char  gateway_ip[256];
        
        printf("\nTipo:%d",recibido->comand);
        printf("\nVersion:%d",recibido->version);
        printf("\nDominio encam:%d",recibido->domain);
	
        printf("\nnº   fam   tag IP Addr.    Netmask  Next Hop    Metric");
        
        for(i=0;i<numero_entradas;i++)
            {
            ipv4_addr_str(recibido->entrys[i].subnet_addr,&subnet_ip);
            ipv4_addr_str(recibido->entrys[i].subnet_mask,&mask_ip);
            ipv4_addr_str(recibido->entrys[i].gateway_addr,&gateway_ip);
            
            printf("\n%d    %d      %d    %s    %s    %s   %d",i+1,ntohs(recibido->entrys[i].family), ntohs(recibido->entrys[i].tag),subnet_ip,mask_ip,gateway_ip,ntohl(recibido->entrys[i].metric));
            }
        
    }
    if(len_receive==0){
        printf("\nNo hay respuesta servidor RIP\n");
    }
    
    
  
   if(close_udp()==1){
        printf("\nInterfaz cerrada(rip)");
        return 1;
    }else{
        
        printf("\nError al cerrar la interfaz(rip)");
        
        return -1;
    }
    
    
    
    
    return 0;
}