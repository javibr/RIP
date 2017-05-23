

#define UDP_MTU 1472

int open_udp(char *nom_fichero_route_table,char *nom_fichero_config);

int send_udp(uint16_t p_origen,uint16_t p_destino,ipv4_addr_t ip_dest,unsigned char*payload,int payload_len);

int rec_udp(uint16_t p_src,uint16_t * p_destino,ipv4_addr_t ip_dest, unsigned char * buffer,long int timeout);

uint16_t genera_puerto();

int close_udp();


