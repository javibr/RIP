
#include <rawnet.h>
#include <timerms.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>


#include "ipv4_config.h"
#include "ipv4_route_table.h"
#include "eth.h"
#include "arp.h"
#include "ipv4.h"


#define UDP_HEADER_LENGTH 8

#define UDP_SEGMENT_MAX_LENGTH 1480 

uint16_t p_maquina;



struct udp_segmento{

    uint16_t p_origen;
    uint16_t p_destino;
    uint16_t longitud;
    uint16_t checksum;
    unsigned char payload[1472];
    
};


int open_udp(char *nom_fichero_route_table,char *nom_fichero_config){
  
 srand((unsigned int)time(NULL));
    
    if(open_ip(nom_fichero_route_table,nom_fichero_config)==1){
        
        return 1;
    }else{
        return -1;
    }
    
    
    
}


int send_udp(uint16_t p_origen,uint16_t p_destino,ipv4_addr_t ip_dest,unsigned char*payload,int payload_len){
    
    int len;
    int udp_segmento_len=payload_len+UDP_HEADER_LENGTH ; // TAMAÑO DEL segemento que vamos a enviar
    
    struct udp_segmento udp_segmento;
    
    udp_segmento.p_origen=htons(p_origen) ;
    udp_segmento.p_destino=htons(p_destino);
    udp_segmento.longitud=htons(UDP_HEADER_LENGTH + payload_len);
    udp_segmento.checksum=0x0;
    
    memcpy(udp_segmento.payload,payload,payload_len);
    
    len=send_ip(ip_dest,0x11,(unsigned char*)&(udp_segmento),udp_segmento_len);
    if(len==-1)
    {
        printf("\nError al enviar (send_udp).\n");
        return -1;
    }
    printf("\nTRAMA UDP ENVIADA\n");
      printf("\nEnviada al puerto %d\n",p_destino);
      printf("\nEnviada desde el puerto %d\n",p_origen);

      printf("\n....................................\n");
      
    return len;
    
}





int rec_udp(uint16_t p_src,uint16_t * p_destino,ipv4_addr_t ip_dest, unsigned char * buffer,long int timeout){
    
    
    
    int compara_puerto;
    
    unsigned char udp_buffer[1480]; // 1480 COMO MAXIMO
    struct udp_segmento *recibido=NULL;
    int payload_len;
    int len;
    
    timerms_t timer;

    
    timerms_reset(&timer,timeout);
    do{
	    long int time= timerms_left(&timer);
	    
        len = rec_ip(ip_dest,0x11,udp_buffer,time);
	
	//  printf("\nTRAMA UDP RECIBIDA\n");

        
	    if (len == -1){
            fprintf(stderr, ": ERROR en rec_ip()\n");
            return -1;
	    } 
	  if (len == 0) {
           // fprintf(stderr, ": ERROR No hay respuesta del Servidor UDP\n");
            return 0;
        }
	
        recibido=(struct udp_segmento*)udp_buffer;

        compara_puerto= (p_src == ntohs(recibido->p_destino));
	
//	printf("\nPuerto recibido :%d\n",ntohs(recibido->p_destino));
	

    }
    while( !(compara_puerto) );
    
    payload_len= len - 8;
    uint16_t aux = ntohs(recibido->p_origen);
    *p_destino = aux;
       
    memcpy(buffer ,recibido->payload,payload_len );

    return payload_len;

}


uint16_t genera_puerto(){
    uint16_t p_maquina;
    
    p_maquina =1024+(uint32_t)(64512.0*rand()/(RAND_MAX +1.0));
    //j=1+(int)(10.0*rand()/(RAND_MAX+1.0));

  //  p_maquina= (rand()%66559)+1024;
    return p_maquina;
}





int close_udp(){
 // close_ip();
    
   if(close_ip()==1){
        printf("\nInterfaz cerrada(UDP)");
        return 1;
    }else{
        
        printf("\nError al cerrar la interfaz(UDP)");
        
        return -1;
    }
    
    
  return 1;
}


/*
 
uint16_t udp_checksum (unsigned char * data, int len, ipv4_addr_t src, ipv4_addr_t dest_addr)
{

  int pseudoheader_size = 2*IPv4_ADDR_SIZE+4;
  int packet_size = len+pseudoheader_size;
  //Creamos el datagrama entero para calcular su checksum
  //Tiene que ser multiplo de dos octetos, 2 bytes.
  if(packet_size % 2 != 0)
  {
   //Si no lo es creamos el segmento multiplo de dos octetos y ponemos padding.
    packet_size ++;
  }
  unsigned char packet[packet_size];
  memset(packet, 0, packet_size);
 
  uint16_t length = (uint16_t) len;
  //Copiamos la pseudocabecera
  memcpy(packet, src, IPv4_ADDR_SIZE);
  memcpy((unsigned char *)&(packet[IPv4_ADDR_SIZE]), dest_addr, IPv4_ADDR_SIZE);
  packet[2*IPv4_ADDR_SIZE] = 0x00;
  packet[2*IPv4_ADDR_SIZE+1] = IP_UDP_PROT;
  packet[2*IPv4_ADDR_SIZE+2] = length >> 8;
  packet[2*IPv4_ADDR_SIZE+3] = length & 0x00FF;
  memcpy((unsigned char *)&(packet[pseudoheader_size]), data, len);
   
  //Ahora ya tenemos el segmento UDP con su cabecera y su pseudocabecera para calcular su checksum.
  unsigned int checksum = 0;
  uint16_t word;
  int i = 0;
  for(i = 0; i<packet_size; i=i+2)
  {
    word = (0xFF00 & (packet[i] << 8)) + (0x00FF & packet[i+1]);
    checksum = checksum + (unsigned int) word;
  }
 
  //Ojo con el resto.
  while (checksum >> 16) {
    checksum = (checksum & 0xFFFF) + (checksum >> 16);
  }
 
  checksum = ~checksum;
  return (uint16_t) checksum;
}
*/



