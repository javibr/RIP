#include "rip_route_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
//#include "rip.h"

/* Dirección IPv4 cero */
//ipv4_addr_t IPv4_ZERO_ADDR = { 0, 0, 0, 0 };



struct rip_route_table {
  rip_route_t * routes[RIP_ROUTE_TABLE_SIZE];
    
};

/* rip_route_t * rip_route_create
 * ( ipv4_addr_t subnet, ipv4_addr_t mask, char* iface, ipv4_addr_t gw );
 * 
 * DESCRIPCIÓN: 
 *   Esta función crea una ruta IPv4 con los parámetros especificados:
 *   dirección de subred, máscara, nombre de interfaz y dirección de siguiente
 *   salto.
 *
 *   Esta función reserva memoria para la estructura creada. Debe utilizar la
 *   función 'ipv4_route_free()' para liberar dicha memoria.
 *
 * PARÁMETROS:
 *   'subnet': Dirección IPv4 de la subred destino de la nueva ruta.
 *     'mask': Máscara de la subred destino de la nueva ruta.
 *    'iface': Nombre del interfaz empleado para llegar a la subred destino de
 *             la nueva  ruta.
 *             Debe tener una longitud máxima de 'IFACE_NAME_LENGTH' caracteres.
 *       'gw': Dirección IPv4 del encaminador empleado para llegar a la subred
 *             destino de la nueva ruta.
 *    'adist': Distancia administrativa del protocolo de encaminamiento que 
 *             ha creado la nueva ruta.
 *
 * VALOR DEVUELTO:
 *   La función devuelve un puntero a la ruta creada.
 * 
 * ERRORES:
 *   La función devuelve 'NULL' si no ha sido posible reservar memoria para
 *   crear la ruta.
 */
rip_route_t * rip_route_create
(ipv4_addr_t subnet, ipv4_addr_t mask, char* iface, ipv4_addr_t gw, uint32_t metric,long int timeout)
{
  rip_route_t * route = (rip_route_t *) malloc(sizeof(struct rip_route));

  if (route != NULL) {
    memcpy(route->subnet_addr, subnet, IPv4_ADDR_SIZE);
    memcpy(route->subnet_mask, mask, IPv4_ADDR_SIZE);
    memcpy(route->iface, iface, IFACE_NAME_LENGTH);
    memcpy(route->gateway_addr, gw, IPv4_ADDR_SIZE);
    route->metric  = metric;
    
  }
  
  timerms_reset(&(route->timer),timeout);
  return route;
}


/* int rip_route_lookup ( ipv4_route_t * route, ipv4_addr_t addr );
 *
 * DESCRIPCIÓN:
 *   Esta función indica si la dirección IPv4 especificada pertence a la
 *   subred indicada. En ese caso devuelve la longitud de la máscara de la
 *   subred.
 *
 *   Esta función NO está implementada, debe implementarla usted para que
 *   funcione correctamente la función 'ipv4_route_table_lookup()'.
 * 
 * PARÁMETROS:
 *   'route': Ruta a la subred que se quiere comprobar.
 *    'addr': Dirección IPv4 destino.
 *
 * VALOR DEVUELTO:
 *   Si la dirección IPv4 pertenece a la subred de la ruta especificada, debe
 *   devolver un número positivo que indica la longitud del prefijo de
 *   subred. Esto es, el número de bits a uno de la máscara de subred.
 *   La función devuelve '-1' si la dirección IPv4 no pertenece a la subred
 *   apuntada por la ruta especificada.
 */
int rip_route_lookup ( rip_route_t * route, ipv4_addr_t addr )
{
    int prefix_length = -1;
    int i;
    int result=0;
    
    ipv4_addr_t new_subred;
    
    for(i=0;i<4;i++){
        
        new_subred[i] =((addr[i]) & (route->subnet_mask[i]));
	
        }
        
    
    //result = 0 si es negativo
    //result = 1 si es verdadera la comparación
   result=(( memcmp(new_subred,route->subnet_addr,IPv4_ADDR_SIZE))==0);
   
   
    if(result){
      
        prefix_length = 0;
        
        for(i=0;i<4;i++){
            switch((int)route->subnet_mask[i]){
        case 255:
             prefix_length=prefix_length+8;
            break;
        case 254:
             prefix_length=prefix_length+7;
            break;
        case 252:
             prefix_length=prefix_length+6;
            break;
        case 248:
             prefix_length=prefix_length+5;
            break;
        case 240:
             prefix_length=prefix_length+4;
            break;
        case 224:
             prefix_length=prefix_length+3;
            break;
        case 192:
             prefix_length=prefix_length+2;
            break;
        case 128:
             prefix_length=prefix_length+1;
            break;  
	default:
	  break;
                }
            }  
      
    }
  
  return prefix_length;
}


/* void rip_route_print ( ipv4_route_t * route );
 *
 * DESCRIPCIÓN:
 *   Esta función imprime la ruta especificada por la salida estándar.
 *
 * PARÁMETROS:
 *   'route': Ruta que se desea imprimir.
 */
void rip_route_print ( rip_route_t * route )
{
  if (route != NULL) {
    char subnet_str[IPv4_STR_MAX_LENGTH];
    ipv4_addr_str(route->subnet_addr, subnet_str);
    char mask_str[IPv4_STR_MAX_LENGTH];
    ipv4_addr_str(route->subnet_mask, mask_str);
    char* iface_str = route->iface;
    char gw_str[IPv4_STR_MAX_LENGTH];
    ipv4_addr_str(route->gateway_addr, gw_str);
    uint32_t metric = route->metric;

    printf("%s/%s -> %s/%s (%d)", subnet_str, mask_str, iface_str, gw_str, 
	   metric);
  }
}


/* void ipv4_route_free ( ipv4_route_t * route );
 *
 * DESCRIPCIÓN:
 *   Esta función libera la memoria reservada para la ruta especificada, que
 *   ha sido creada con 'ipv4_route_create()'.
 *
 * PARÁMETROS:
 *   'route': Ruta que se desea liberar.
 */
void rip_route_free ( rip_route_t * route )
{
  if (route != NULL) {
    free(route);
  }
}


/* rip_route_table_t * ipv4_route_table_create();
 * 
 * DESCRIPCIÓN: 
 *   Esta función crea una tabla de rutas IPv4 vacía.
 *
 *   Esta función reserva memoria para la tabla de rutas creada, para
 *   liberarla es necesario llamar a la función 'ipv4_route_table_free()'.
 *
 * VALOR DEVUELTO:
 *   La función devuelve un puntero a la tabla de rutas creada.
 *
 * ERRORES:
 *   La función devuelve 'NULL' si no ha sido posible reservar memoria para
 *   crear la tabla de rutas.
 */
rip_route_table_t * rip_route_table_create()
{
  rip_route_table_t * table;

  table = (rip_route_table_t *) malloc(sizeof(struct rip_route_table));
  if (table != NULL) {
    int i;
    for (i=0; i<RIP_ROUTE_TABLE_SIZE; i++) {
      table->routes[i] = NULL;
    }
  }

  return table;
}


/* int rip_route_table_add ( rip_route_table_t * table,
 *                            rip_route_t * route );
 * DESCRIPCIÓN: 
 *   Esta función añade la ruta especificada en la primera posición libre de
 *   la tabla de rutas.
 *
 * PARÁMETROS:
 *   'table': Tabla donde añadir la ruta especificada.
 *   'route': Ruta a añadir en la tabla de rutas.
 * 
 * VALOR DEVUELTO:
 *   La función devuelve el indice de la posición [0,IPv4_ROUTE_TABLE_SIZE-1]
 *   donde se ha añadido la ruta especificada.
 * 
 * ERRORES:
 *   La función devuelve '-1' si no ha sido posible añadir la ruta
 *   especificada.
 */
int rip_route_table_add ( rip_route_table_t * table, rip_route_t * route )
{
  int route_index = -1;

  if (table != NULL) {
    /* Find an empty place in the route table */
    int i;
    for (i=0; i<RIP_ROUTE_TABLE_SIZE; i++) {
      if (table->routes[i] == NULL) {
        table->routes[i] = route;
        route_index = i;
        break;
      }
    }
  }

  return route_index;
}


/* ipv4_route_t * ipv4_route_table_remove ( ipv4_route_table_t * table, 
 *                                          int index );
 *
 * DESCRIPCIÓN:
 *   Esta función borra la ruta almacenada en la posición de la tabla de rutas
 *   especificada.
 *   
 *   Esta función NO libera la memoria reservada para la ruta borrada. Para
 *   ello es necesario utilizar la función 'ipv4_route_free()' con la ruta
 *   devuelta.
 *
 * PARÁMETROS:
 *   'table': Tabla de rutas de la que se desea borrar una ruta.
 *   'index': Índice de la ruta a borrar. Debe tener un valor comprendido
 *            entre [0, IPv4_ROUTE_TABLE_SIZE-1].
 * 
 * VALOR DEVUELTO:
 *   Esta función devuelve la ruta que estaba almacenada en la posición
 *   indicada.
 *
 * ERRORES:
 *   Esta función devuelve 'NULL' si la ruta no ha podido ser borrada, o no
 *   existía ninguna ruta en dicha posición.
 */
rip_route_t * rip_route_table_remove ( rip_route_table_t * table, int index )
{
  rip_route_t * removed_route = NULL;
  
  if ((table != NULL) && (index >= 0) && (index < RIP_ROUTE_TABLE_SIZE)) {
    removed_route = table->routes[index];
    table->routes[index] = NULL;
  }

  return removed_route;
}


/* ipv4_route_t * ipv4_route_table_get ( ipv4_route_table_t * table, int index );
 * 
 * DESCRIPCIÓN:
 *   Esta función devuelve la ruta almacenada en la posición de la tabla de
 *   rutas especificada.
 *
 * PARÁMETROS:
 *   'table': Tabla de rutas de la que se desea obtener una ruta.
 *   'index': Índice de la ruta consultada. Debe tener un valor comprendido
 *            entre [0, IPv4_ROUTE_TABLE_SIZE-1].
 * 
 * VALOR DEVUELTO:
 *   Esta función devuelve la ruta almacenada en la posición de la tabla de
 *   rutas indicada.
 *
 * ERRORES:
 *   Esta función devuelve 'NULL' si no ha sido posible consultar la tabla de
 *   rutas, o no existe ninguna ruta en dicha posición.
 */
rip_route_t * rip_route_table_get ( rip_route_table_t * table, int index )
{
  rip_route_t * route = NULL;

  if ((table != NULL) && (index >= 0) && (index < RIP_ROUTE_TABLE_SIZE)) {
    route = table->routes[index];
  }
  
  return route;
}


/* int rip_route_table_find ( rip_route_table_t * table, ipv4_addr_t subnet,
 *                                                         ipv4_addr_t mask );
 *
 * DESCRIPCIÓN:
 *   Esta función devuelve el índice de la ruta para llegar a la subred
 *   especificada.
 *
 * PARÁMETROS:
 *    'table': Tabla de rutas en la que buscar la subred.
 *   'subnet': Dirección de la subred a buscar.
 *     'mask': Máscara de la subred a buscar.
 * 
 * VALOR DEVUELTO:
 *   Esta función devuelve la posición de la tabla de rutas donde se encuentra
 *   la ruta que apunta a la subred especificada.
 *
 * ERRORES:
 *   La función devuelve '-1' si no se ha encontrado la ruta especificada o
 *   '-2' si no ha sido posible realizar la búsqueda.
 */
int rip_route_table_find
( rip_route_table_t * table, ipv4_addr_t subnet, ipv4_addr_t mask )
{
  int route_index = -2;

  if (table != NULL) {
    route_index = -1;
    int i;
    for (i=0; i<RIP_ROUTE_TABLE_SIZE; i++) {
      rip_route_t * route_i = table->routes[i];
      if (route_i != NULL) {
        int same_subnet = 
          (memcmp(route_i->subnet_addr, subnet, IPv4_ADDR_SIZE) == 0);
        int same_mask = 
          (memcmp(route_i->subnet_mask, mask, IPv4_ADDR_SIZE) == 0);
        
        if (same_subnet && same_mask) {
          route_index = i;
          break;
        }
      }
    }
  }

  return route_index;
}


/* ipv4_route_t * ipv4_route_table_lookup ( ipv4_route_table_t * table, 
 *                                          ipv4_addr_t addr );
 * 
 * DESCRIPCIÓN:
 *   Esta función devuelve la mejor ruta almacenada en la tabla de rutas para
 *   alcanzar la dirección IPv4 destino especificada.
 *
 *   Esta función recorre toda la tabla de rutas buscando rutas que contengan
 *   a la dirección IPv4 indicada. Para ello emplea la función
 *   'ipv4_route_lookup()'. De todas las rutas posibles se devuelve aquella
 *   con el prefijo más específico, esto es, aquella con la máscara de subred
 *   mayor.
 * 
 * PARÁMETROS:
 *   'table': Tabla de rutas en la que buscar la dirección IPv4 destino.
 *    'addr': Dirección IPv4 destino a buscar.
 *
 * VALOR DEVUELTO:
 *   Esta función devuelve la ruta más específica para llegar a la dirección
 *   IPv4 indicada.
 *
 * ERRORES:
 *   Esta función devuelve 'NULL' si no no existe ninguna ruta para alcanzar
 *   la dirección indicada, o si no ha sido posible realizar la búsqueda.
 */
rip_route_t * rip_route_table_lookup ( rip_route_table_t * table,
                                         ipv4_addr_t addr )
{
  rip_route_t * best_route = NULL;
  int best_route_prefix = -1;

  if (table != NULL) {
    int i;
    for (i=0; i<RIP_ROUTE_TABLE_SIZE; i++) {
      rip_route_t * route_i = table->routes[i];
      if (route_i != NULL) {
        int route_i_lookup = rip_route_lookup(route_i, addr);
        if (route_i_lookup > best_route_prefix) {
          best_route = route_i;
          best_route_prefix = route_i_lookup;
        }
      }
    }
  }
  
  return best_route;
}


/* void ipv4_route_table_free ( ipv4_route_table_t * table );
 *
 * DESCRIPCIÓN:
 *   Esta función libera la memoria reservada para la tabla de rutas
 *   especificada, incluyendo todas las rutas almacenadas en la misma,
 *   mediante la función 'ipv4_route_free()'.
 *
 * PARÁMETROS:
 *   'table': Tabla de rutas a borrar.
 */
void rip_route_table_free ( rip_route_table_t * table )
{
  if (table != NULL) {
    int i;
    for (i=0; i<RIP_ROUTE_TABLE_SIZE; i++) {
      rip_route_t * route_i = table->routes[i];
      if (route_i != NULL) {
        table->routes[i] = NULL;
        rip_route_free(route_i);
      }
    }
    free(table);
  }
}


/* int ipv4_route_table_read ( char * filename, ipv4_route_table_t * table );
 *
 * DESCRIPCIÓN:
 *   Esta función lee el fichero especificado y añade las rutas IPv4
 *   estáticas leídas en la tabla de rutas indicada.
 *
 * PARÁMETROS:
 *   'filename': Nombre del fichero con rutas IPv4 que se desea leer.
 *      'table': Tabla de rutas donde añadir las rutas leidas.
 *
 * VALOR DEVUELTO:
 *   La función devuelve el número de rutas leidas y añadidas en la tabla, o
 *   '0' si no se ha leido ninguna ruta.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error al leer el
 *   fichero de rutas.
 */
int rip_route_table_read ( char * filename, rip_route_table_t * table )
{
  int read_routes = 0;

  FILE * routes_file = fopen(filename, "r");
  if (routes_file == NULL) {
    fprintf(stderr, "Error opening input IPv4 Routes file \"%s\": %s.\n",
            filename, strerror(errno));
    return -1;
  }

  int linenum = 0;
  char line_buf[1024];
  char subnet_str[256];
  char mask_str[256];
  char iface_name[256];
  char gw_str[256];
    char metric[16];
  int err = 0;

  while ((! feof(routes_file)) && (err==0)) {

    linenum++;

    /* Read next line of file */
    char* line = fgets(line_buf, 1024, routes_file);
    if (line == NULL) {
      break;
    }

    /* If this line is empty or a comment, just ignore it */
    if ((line_buf[0] == '\n') || (line_buf[0] == '#')) {
      err = 0;
      continue;
    }

    /* Parse line: Format "<subnet> <mask> <iface> <gw>\n" */
    err = sscanf(line, "%s %s %s %s %s\n",
                 subnet_str, mask_str, iface_name, gw_str,metric);
    if (err != 5) {
      fprintf(stderr, "%s:%d: Invalid IPv4 Route format: \"%s\" (%d items)\n",
              filename, linenum, line, err);
      fprintf(stderr, 
              "%s:%d: Format must be: <subnet> <mask> <iface> <gw>\n",
              filename, linenum);
      err = -1;

    } else {

      /* Parse IPv4 route subnet address */
      ipv4_addr_t subnet;
      err = ipv4_str_addr(subnet_str, subnet);
      if (err == -1) {
        fprintf(stderr, "%s:%d: Invalid <subnet> value: \"%s\"\n", 
                filename, linenum, subnet_str);
        break;
      }

      /* Parse IPv4 route subnet mask */
      ipv4_addr_t mask;
      err = ipv4_str_addr(mask_str, mask);
      if (err == -1) {
        fprintf(stderr, "%s:%d: Invalid <mask> value: \"%s\"\n",
                filename, linenum, mask_str);
        break;
      }
      
      /* Parse IPv4 route gateway */
      ipv4_addr_t gateway;
      err = ipv4_str_addr(gw_str, gateway);
      if (err == -1) {
        fprintf(stderr, "%s:%d: Invalid <gw> value: \"%s\"\n",
                filename, linenum, gw_str);
        break;
      }
      
      /* Check the administrative distance
      uint32_t admin_dist = ADMIN_DIST_STATIC;
      if (memcmp(gateway, IPv4_ZERO_ADDR, IPv4_ADDR_SIZE) == 0) {
	  admin_dist = ADMIN_DIST_DIRECT;
      }
       */

      /* Create new route & add it to Route Table */
      rip_route_t * new_route =
        rip_route_create(subnet, mask, iface_name, gateway,atoi(metric),-1);

      if (table != NULL) {
        err = rip_route_table_add(table, new_route);
        if (err >= 0) {
          err = 0;
          read_routes++;
        }
      }
    }
  } /* while() */

  if (err == -1) {
    read_routes = -1;
  }

  /* Close IP Route Table file */
  fclose(routes_file);

  return read_routes;
}


/* void ipv4_route_table_output ( ipv4_route_table_t * table, FILE * out );
 *
 * DESCRIPCIÓN:
 *   Esta función imprime por la salida indicada la tabla de rutas IPv4
 *   especificada.
 *
 * PARÁMETROS:
 *        'out': Salida por la que imprimir la tabla de rutas.
 *      'table': Tabla de rutas a imprimir.
 *      'adist': Cero si no se desea imprimir la distancia administrativa.
 *
 * VALOR DEVUELTO:
 *   La función devuelve el número de rutas impresas por la salida indicada, o
 *   '0' si la tabla de rutas estaba vacia.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error al escribir por
 *   la salida indicada.
 *
 */

int rip_route_table_output ( FILE * out, rip_route_table_t * table, uint32_t metric)
{
  int err;

  if (metric== 0) {
    err = fprintf
      (out, "# SubnetAddr  \tSubnetMask    \tIface  \tGateway\n");
  } else {
    err = fprintf
      (out, "# SubnetAddr  \tSubnetMask    \tIface  \tGateway       \tMetric   \tTime\n");
  }
  if (err < 0) {
    return -1;
  }
  
  char subnet_str[IPv4_STR_MAX_LENGTH];
  char mask_str[IPv4_STR_MAX_LENGTH];
  char* ifname = NULL;
  char gw_str[IPv4_STR_MAX_LENGTH];
  long int timeout;
  int i;
  for (i=0; i<RIP_ROUTE_TABLE_SIZE; i++) {
    rip_route_t * route_i = rip_route_table_get(table, i);
    if (route_i != NULL) {
      timeout=timerms_left(&route_i->timer);
      ipv4_addr_str(route_i->subnet_addr, subnet_str);
      ipv4_addr_str(route_i->subnet_mask, mask_str);
      ifname = route_i->iface;
      ipv4_addr_str(route_i->gateway_addr, gw_str);
      metric = route_i->metric;
      
      if (metric == 0) {
	err = fprintf(out, "%-15s\t%-15s\t%s\t%-15s\t%ld\n",
		      subnet_str, mask_str, ifname, gw_str,timeout/1000);
      } else {
	err = fprintf(out, "%-15s\t%-15s\t%s\t%-15s\t%d\t%ld\n",
		      subnet_str, mask_str, ifname, gw_str, metric,timeout/1000);
      }
      if (err < 0) {
        return -1;
      }
    }
  }

  return 0;
}


/* int ipv4_route_table_write ( ipv4_route_table_t * table, char * filename );
 *
 * DESCRIPCIÓN:
 *   Esta función almacena en el fichero especificado la tabla de rutas IPv4
 *   indicada.
 *
 * PARÁMETROS:
 *      'table': Tabla de rutas a almacenar.
 *   'filename': Nombre del fichero donde se desea almacenar la tabla de
 *               rutas.
 *
 * VALOR DEVUELTO:
 *   La función devuelve el número de rutas almacenadas en el fichero de
 *   rutas, o '0' si la tabla de rutas estaba vacia.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error al escribir el
 *   fichero de rutas.
 */
int rip_route_table_write ( rip_route_table_t * table, char * filename )
{
  int num_routes = 0;

  FILE * routes_file = fopen(filename, "w");
  if (routes_file == NULL) {
    fprintf(stderr, "Error opening output IPv4 Routes file \"%s\": %s.\n",
            filename, strerror(errno));
    return -1;
  }

  fprintf(routes_file, "# %s\n", filename);
  fprintf(routes_file, "#\n");

  if (table != NULL) {
    num_routes = rip_route_table_output (routes_file, table, 0);
    if (num_routes == -1) {
      fprintf(stderr, "Error writing IPv4 Routes file \"%s\": %s.\n",
              filename, strerror(errno));
      return -1;
    }
  }

  fclose(routes_file);
  
  return num_routes;
}


/* void ipv4_route_table_print ( ipv4_route_table_t * table );
 *
 * DESCRIPCIÓN:
 *   Esta función imprime por la salida estándar la tabla de rutas IPv4
 *   especificada.
 *
 * PARÁMETROS:
 *      'table': Tabla de rutas a imprimir.
 */
void rip_route_table_print ( rip_route_table_t * table )
{
  if (table != NULL) {
    rip_route_table_output (stdout, table, 1);
  }
}













/*
 rip_route_t * comprueba_timeout(rip_route_table_t * table);
 
 Funcion que me devuelve la ruta que mas tiempo lleva en la tabla de rutas
 
 */


rip_route_t * comprueba_timeout(rip_route_table_t * table)
{
rip_route_t * route = NULL;
//int worst_time = 180000;
long int time;
if (table != NULL) 
{
    int i;
    for (i=0; i<RIP_ROUTE_TABLE_SIZE; i++) 
    {
        rip_route_t * route_i = table->routes[i];
        if (route_i != NULL) 
	{
          time = timerms_left(&(route_i->timer));
            
            if (time == 0)
	    {
                 route = route_i;
                
            }
        }
    }
}

return route;
    
    
}
