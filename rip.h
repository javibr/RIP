

#include <stdint.h>

#define RIPV2_HEADER_LENGTH 4
#define RIPV2_ENTRYS_MAX_LENGTH 500
#define RIPV2_ENTRY_LEN 20
#define RIPV2_MAX_ENTRYS 25
#define RIPV2_TABLE_UPDATE 30000
#define RIPV2_TABLE_TIMEOUT 180000
#define RIPV2_GARBAGE_TIMEOUT 120000
#include "rip_route_table.h"

//#include "rip_route_table.h"
 

int poison;
int s_horizon;
int first_request;
int garbage_collection;
int sinc_ip;
int read_routes;
int jitter;
int rip_table_timeout;
typedef struct rip_route_entry{
    
    uint16_t family;
    uint16_t tag;
    ipv4_addr_t subnet_addr;
    ipv4_addr_t subnet_mask;
    ipv4_addr_t gateway_addr;
    uint32_t metric;
    
}rip_route_entry_t;


struct rip_segmento{
    
    uint8_t comand;
    uint8_t version;
    uint16_t domain;
    
    rip_route_entry_t entrys[RIPV2_MAX_ENTRYS];// 25 ENTRADAS MAXIMO
};


/*  METODOS PARA EL RIP SERVER*/

long int minimum_timeout(rip_route_table_t * table,timerms_t table_update);

int update_routes(rip_route_table_t * table);

int send_response(rip_route_table_t * table,ipv4_addr_t ip_dest,uint16_t p_dest);

int process_pack(int len_receive, ipv4_addr_t ip_dest_addr,unsigned char * buffer,uint16_t p_dest, rip_route_table_t *table);

int send_request(ipv4_addr_t ip_dest_addr,uint16_t p_src);

