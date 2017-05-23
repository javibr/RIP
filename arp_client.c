
#include "arp.h"
#include "eth.h"
#include "ipv4.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>

#define DEFAULT_PAYLOAD_LENGTH 1500

/* int main ( int argc, char * argv[] );
 *
 * DESCRIPCIÓN:
 *   Función principal del programa cliente arp.
 *
 * PARÁMETROS:
 *   'argc': Número de parámetros pasados al programa por la línea de
 *           comandos, incluyendo el propio nombre del programa.
 *   'argv': Array con los parámetros pasado al programa por línea de
 *           comandos.
 *           El primer parámetro (argv[0]) es el nombre del programa.
 *
 * VALOR DEVUELTO:
 *   Código de salida del programa.
 */



int main ( int argc, char * argv[] )
{
    /* Mostrar mensaje de ayuda si el número de argumentos es incorrecto */
    char * myself = basename(argv[0]);
    
    if (argc != 3) {
        printf("Uso: %s <iface> <tipo>\n", myself);
        printf("       <iface>: Nombre de la interfaz Ethernet\n");
        printf("        <tipo>: Campo Direccion ip a resolver\n");
        exit(-1);
    }
    
    char letra[MAC_STR_LENGTH];
    
    
    /* Procesar los argumentos */
    char * iface_name = argv[1];
    char* ip_dest= argv[2];
    
    mac_addr_t mac_dest;
    
    /*Cambiamos a binario la IP*/
    
    ipv4_addr_t ip_dest_addr;
    int error_ip=(ipv4_str_addr(ip_dest, ip_dest_addr));
    if(error_ip==-1){
     printf("\n La dirección proporcionada no se corresponde con IPv4.\n");
    }
    
  
    /* Abrir la interfaz Ethernet */
    eth_iface_t * eth_iface = eth_open(iface_name);
    if (eth_iface == NULL) {
        fprintf(stderr, "%s: ERROR en eth_open(\"%s\")\n", myself, iface_name);
        exit(-1);
    }
    
    mac_addr_t client_addr;
    eth_getaddr(eth_iface, client_addr);
    char client_addr_str[MAC_STR_LENGTH];
    mac_addr_str(client_addr, client_addr_str);
    
    printf("Abriendo interfaz Ethernet %s. Dirección MAC: %s\n",
           iface_name, client_addr_str);
    
	   
    /*arp_resolve*/
    int error= (arp_resolve ( eth_iface, ip_dest_addr, mac_dest ));    
    if (error==-1)
      {
        printf(" No se ha recibido respuesta ARP\n");
	
      }else{
      
	    mac_addr_str(mac_dest,letra );
	    printf("\n(%s)-->(%s)\n",ip_dest,letra);
	    }
    
    /* Cerrar interfaz Ethernet */
    printf("\nCerrando interfaz Ethernet %s.\n", iface_name);
    
    eth_close(eth_iface);
    
    
    return 0;
}
