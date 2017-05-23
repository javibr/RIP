

#include <stdio.h>
#include <stdlib.h>

#include <rawnet.h>
#include <timerms.h>
#include "ipv4_config.h"
#include "ipv4_route_table.h"
#include "eth.h"
#include "arp.h"
#include "ipv4.h"
#include "udp.h"
#include "rip_route_table.h"
#include "rip.h"

#include <string.h>
#include <netinet/in.h>



ipv4_addr_t ip_rip_multicast = {224,0,0,9};
ipv4_addr_t ip_zero={0,0,0,0};



int send_response(rip_route_table_t * table,ipv4_addr_t ip_dest,uint16_t p_dest){
    
    struct rip_segmento rip;
    rip_route_t *aux;
    rip.comand=2; //tipo Response
    rip.version=2; //version 2;
    rip.domain=0;
    
    int payload_len=0;
    
    int i=0;
	for(i=0; i<25; i++)
    {
       aux=rip_route_table_get(table,i);
            if(aux!= NULL)
                {
                    rip.entrys[i].family=htons(2);
                    rip.entrys[i].tag=htons(0);
                    memcpy( rip.entrys[i].subnet_addr  ,aux->subnet_addr , IPv4_ADDR_SIZE);
                    memcpy( rip.entrys[i].subnet_mask  ,aux->subnet_mask , IPv4_ADDR_SIZE);
                    memcpy( rip.entrys[i].gateway_addr ,ip_zero ,IPv4_ADDR_SIZE);
                    rip.entrys[i].metric=htonl(aux->metric);
                
                    /*Técnica Split Horizon wth Poison Reverse*/
             
                    if(poison==1)
                        {
                        char *iface=eth_getname (getIface());
                        int comp= strcmp(iface,aux->iface)==0;
                        //printf("Rawnet:%s prueba\n",iface);
                        //printf("tabla:%s prueba\n\n",aux->iface);
                        if(comp){
                        	rip.entrys[i].metric=htonl(16);
                            printf("\nRuta puesta a infinita al enviar por Poison Reverse\n");
                        	}
                        }
                    /*******************************************/
     			
                    payload_len+=RIPV2_ENTRY_LEN;

                    /*Técnica Split Horizon*/
                    if(s_horizon==1)
                    {
                        char*iface=eth_getname(getIface());
                        int comp=strcmp(iface,aux->iface)==0;
                        if(comp){
                            payload_len=payload_len-RIPV2_ENTRY_LEN;
                            printf("\nRuta eliminada al enviar por Split Horizon\n");
                            }
                    }
                    /***************************************************/
                
            }
    }
    
   send_udp(520,p_dest,ip_dest,(unsigned char*)&(rip), payload_len+4);
    return 0;
    
}






/*obtemos el tiempo de espera*/
long int minimum_timeout(rip_route_table_t *table,timerms_t table_update)
{
    long int min = timerms_left(&table_update);
    int i=0;
    long int min_entry_time=0;
    rip_route_t *aux;
    
    for(i=0; i<RIP_ROUTE_TABLE_SIZE; i++)
        
    {
        aux=rip_route_table_get(table,i);
        
        if(aux!= NULL)
            {
                min_entry_time = timerms_left(&aux->timer);
            
                if((min_entry_time==0)&&(aux->metric<16))
                {
                    return 0;
                }else
                    {
                        if( (min_entry_time< min) && (min_entry_time > 0))
                            {
                            min = min_entry_time;
                            }
                    }
            }
    }
    
    return min;
}



/*actualiza la tabla de rutas, las rutas cuyo temporizador haya caducado se ponen con metrica a 16 */

int update_routes(rip_route_table_t * table)
{
    int i=0;
    int rutas=0;
    rip_route_t * aux;
    for(i=0; i<RIP_ROUTE_TABLE_SIZE; i++)
    {
        aux=rip_route_table_get(table,i);
        
        if(aux!= NULL)
        {
                if((timerms_left(&aux->timer)==0) && (aux->garbage_flag!=1))
                    {
                    aux->metric=16;
                    rutas++;
                            /*Comprobamos si el flag garbage global  esta activado*/
                            if(garbage_collection==1){
                                timerms_reset(&aux->timer,RIPV2_GARBAGE_TIMEOUT);
                                aux->garbage_flag=1;
                            }
                            /*********************************************/
                    }
            
                    /*Comprobamos si el flag garbage esta activado*/
                    if(garbage_collection==1){
                        if(aux->garbage_flag==1)
                        {
                            if((aux->metric==16)&&(timerms_left(&aux->timer)==0))
                            {
                                rip_route_t * aux2;
                                aux2=rip_route_table_remove (table,i);
                                rip_route_free(aux2);
                                printf("\nRuta borrada por Timeout de Garbage Collection.\n");
                                rutas++;
                            }
                        }
                    }
                    /********************************************/
            

        }
    }

    return rutas;
}




/*metodo para procesar un paquete recibido, toma decisiones a partir del comando del paquete recibido*/
int process_pack(int len_receive, ipv4_addr_t ip_dest_addr,unsigned char * buffer,uint16_t p_dest, rip_route_table_t *table)
{
    
   struct rip_segmento *recibido= (struct rip_segmento*) buffer;
    
    int len= len_receive-4;
    int i=0;
	rip_route_t *aux;
    int indice=0;
    if(recibido->version==1){
        return -1;
    }
    
    if(recibido->comand==1)
    {
        if(len==20)
            {
                int comp1=recibido->entrys[0].family;
                int comp2=recibido->entrys[0].tag;
                int comp3=memcmp(ip_zero ,recibido->entrys[0].subnet_addr ,IPv4_ADDR_SIZE)==0;
                int comp4=memcmp(ip_zero ,recibido->entrys[0].subnet_mask ,IPv4_ADDR_SIZE)==0;
                int comp5=memcmp(ip_zero ,recibido->entrys[0].gateway_addr ,IPv4_ADDR_SIZE)==0;
                long int comp6=ntohl(recibido->entrys[0].metric);

                if((comp1==0)&&(comp2==0)&&(comp3)&&(comp4)&&(comp5)&&(comp6==16))
                    {
                        send_response(table,ip_dest_addr,p_dest);
                    }
            }
    }
    
    
    if(recibido->comand==2)
    {
        for(i=0;i<len/20;i++)
        {
            indice =rip_route_table_find ( table, recibido->entrys[i].subnet_addr, recibido->entrys[i].subnet_mask);
            /*Caso en el que no tenemos guardada la entrada*/
            if(indice==-1)
                {
                if((ntohl(recibido->entrys[i].metric)+1)<16)
                    {
                        char *iface=eth_getname (getIface());
                        rip_route_t * new_rute= rip_route_create (recibido->entrys[i].subnet_addr,  recibido->entrys[i].subnet_mask,iface, ip_dest_addr, ntohl(recibido->entrys[i].metric)+1,RIPV2_TABLE_TIMEOUT);

                        int n=rip_route_table_add (table, new_rute);
                        if(n==-1){
                            printf("\nNo se ha podido añadir ruta \n");
                            }
                    }
                }
            /*Caso en el que tenemos la ruta guardada*/
            if(indice>=0)
                {
                
                    aux=rip_route_table_get(table,indice);
                    
                    int comparacion =memcmp(ip_dest_addr ,aux->gateway_addr ,IPv4_ADDR_SIZE)==0;
                    /*Comprobamos si el que nos envia la ruta es el proximo salto de la ruta , anteriormente habiamos aprendido de ella*/
                    if(comparacion==1)
                                    {
                                            if(ntohl(recibido->entrys[i].metric)<16)
                                                {
                                                    aux->metric =ntohl(recibido->entrys[i].metric)+1;
                                                    timerms_reset(&(aux->timer),RIPV2_TABLE_TIMEOUT);

                                                }
                                                else{
                                                    aux->metric =ntohl(recibido->entrys[i].metric);
                                        
                                                    timerms_reset(&(aux->timer),RIPV2_TABLE_TIMEOUT);
                                                    
                                                    
                                                    
                                                    if(garbage_collection==1){
                                                        if(aux->garbage_flag!=1)
                                                        {
                                                        timerms_reset(&aux->timer,RIPV2_GARBAGE_TIMEOUT);
                                                        aux->garbage_flag=1;
                                                        }
                                                    }
                                                    
                                                    
                                                    
                                                }
                                    }
                    
                                else
                                    {
                                        /*Caso que el que nos actualiza la ruta es diferente al sig salto*/
                                            if(aux->metric>ntohl(recibido->entrys[i].metric)+1)
                                            {
                                                aux->metric =ntohl(recibido->entrys[i].metric)+1;
                                                memcpy( aux->gateway_addr,ip_dest_addr,IPv4_ADDR_SIZE);
                                                timerms_reset(&(aux->timer),RIPV2_TABLE_TIMEOUT);
                                            }
			
                                    }
                    
                }
            
            }
      
            rip_route_table_print ( table );
        
    }
    
    return 1;
    
}




int send_request(ipv4_addr_t ip_dest_addr,uint16_t p_src)

{
  
  
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
    
    
     /*enviar mensaje Request*/
    int error=send_udp(p_src,520,ip_dest_addr, (unsigned char*)& (rip),24);
	if (error==-1) {
        printf("Error al enviar (send_request\n");
        exit(-1);
    }
  
 return 0; 
}


