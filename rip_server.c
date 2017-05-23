

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

ipv4_addr_t rip_multicast = {224, 0, 0, 9};

int main (int argc,char *argv[])

{
    
    int i=0;
    
    for (i=1;i<(argc);i++)
        {
    
            if (strcmp("-m",argv[i])==0)
            {
                i++;
                if(strcmp("sh",argv[i])==0){
				s_horizon=1;
				printf("\nServidor Rip con Técnica Split Horizon Activada...\n");
                }
                if(strcmp("pr", argv[i]) == 0){
				poison=1;
				printf("\nServidor Rip con Técnica Split Horizon with Poison Reverse Activada...\n");
                }
            }
            if(strcmp("-r",argv[i])==0){
                first_request=1;
                printf("\nPetición de rutas activa cuando se inicia el servidor...\n");
            }
        
            if(strcmp("-g",argv[i])==0){
                garbage_collection=1;
                printf("\nGarbage Collection activado.\n");
            }
        
            if(strcmp("-s",argv[i])==0){
                sinc_ip=1;
                printf("\nSincronización con la tabla IP activada...\n");
            }
	    
            if(strcmp("-l",argv[i])==0){
                read_routes=1;
                printf("\nRutas estaticas leidas\n");
            }
            if(strcmp("-j",argv[i])==0){
                jitter=1;
                printf("\nJitter activado\n");
            }
        
        }

    uint16_t p_destino;
    uint16_t p_src=520;
    ipv4_addr_t ip_dest_addr;
    
    
    unsigned char buffer[RIPV2_ENTRYS_MAX_LENGTH+RIPV2_HEADER_LENGTH];
    char *nom_fichero_route_table="ipv4_route_table_server.txt";
	char *nom_fichero_config="ipv4_config_server.txt";
    int error=0;
    
    timerms_t table_update; // cuando este temp se ponga a cero, se deberia enviar un response
    
    long int min_timeout=0;
    
    /*creamos tabla rutas vacía y ponemos contador a 30 segundos*/
    rip_route_table_t * table = rip_route_table_create();


    
        if(read_routes)
        {

                /*leemos archivo de rutas*/
            int rutas_anadidas=rip_route_table_read ("rip_rutas.txt", table );
            if(error==-1)
            {
                printf("\nno se pudo añadir rutas");
            }else{
                printf("\nSe han añadido %d rutas :\n",rutas_anadidas);
            }
            /* imprimimos nuestra tabla de rutas*/
            rip_route_table_print ( table );

        }
    
    /*abrir interfaz*/
    error=open_udp(nom_fichero_route_table,nom_fichero_config);
	if (error==-1) {
        printf("Error al abrir la interfaz\n");
        exit(-1);
    }
    
    
    
    if(jitter)
        {
            
            rip_table_timeout =50+(uint32_t)(101.0*rand()/(RAND_MAX +1.0));
            rip_table_timeout=( rip_table_timeout* RIPV2_TABLE_UPDATE)/100;
            // rip_table_timeout = RIPV2_TABLE_UPDATE + ((rand() % RIPV2_TABLE_UPDATE*2*JITTER) - RIPV2_TABLE_UPDATE*JITTER);

            //printf("\n jitter: %d\n",rip_table_timeout);
            timerms_reset(&table_update,rip_table_timeout);

        }else{
            timerms_reset(&table_update,RIPV2_TABLE_UPDATE);
            }
    
    
    if(first_request)
        {
      
            send_request(rip_multicast,p_src );
      
        }
    
    
    
    while(1)
    
    {
        
        /*tiempo de que escucharemos mensajes*/
        min_timeout = minimum_timeout(table,table_update);
        if(timerms_left(&table_update)==0)
        {
                        if(jitter)
                        {
                            rip_table_timeout =50+(uint32_t)(101.0*rand()/(RAND_MAX +1.0));
                            rip_table_timeout=( rip_table_timeout* RIPV2_TABLE_UPDATE)/100;
                            timerms_reset(&table_update,rip_table_timeout);
                          } else
            timerms_reset(&table_update,RIPV2_TABLE_UPDATE);
        }
        
        if(min_timeout!=0){
        printf("\nEscuchando mensajes por (%ld) segundos \n",min_timeout/1000);
        /*Escuchando mensajes */
        int len_receive= rec_udp(p_src,&p_destino,ip_dest_addr,buffer,min_timeout);
        
        
        
        if(len_receive ==0)
                {
                    printf("\nActualizando rutas...\n");
                    int update=update_routes(table);
                    
                    if(update>0){
                    rip_route_table_print ( table );
                    }
                
                    if(timerms_left(&table_update)==0)
                        {
                            send_response(table,rip_multicast,520);
			     rip_route_table_print ( table );

                        }
            
                }
                else
                    {
                        printf("\nProcesando paquete recibido..\n");
                        process_pack(len_receive, ip_dest_addr,buffer,p_destino,table);
                    }
            
        }
    
     
    }
    

}
    



