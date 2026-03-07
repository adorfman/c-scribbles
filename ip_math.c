#include <stdio.h>
//#include <math.h>
#include <inttypes.h>
#include <stdlib.h>
#include <netinet/in.h>

#define OCTET_LEN 3

#define OCTET_STR_LEN 4

#define IPV4_STR_LITERAL_SIZE 16

uint8_t oct_str_to_num( const char *num_str, uint8_t *const err ) {

  uint8_t place = 0;
  uint8_t num   = 0;

  while ( *num_str != '\0' ) {

    if (++place > OCTET_LEN) {
      *err |= 1;
      return 0;
    }

    // Using a wider type for overflow checking
    uint16_t t = num * 10 + ( (uint8_t)(*num_str) - '0' );

    if ( t > 255 ) {
      *err |= 1;
      return 0;
    }
    else {
      num = t;
    }

    num_str++;
  }

  return num;
}


typedef union {
  uint32_t address;   // LSB
  uint8_t  octets[4];
} Ipv4Addr;


int str_to_ipv4addr( const char *ip_string_ptr, Ipv4Addr *const ipv4 ) {

  const char dot = '.';

  char ip_byte_strs[OCTET_STR_LEN][OCTET_STR_LEN] = {{},{},{},{}};
  /* Who needs to manage multi-dimensional array
   * indexes ?
   *
   * NOT MY COMPILER
   * */
  char *ip_byte_strs_ptr = (char*)ip_byte_strs;
  unsigned int pos = 0;
  unsigned int seen_dot = 0;

  while (*ip_string_ptr != '\0' ) {

    if ( pos > 12 )
      return -1;

    /* some input validation, as a treat */
    if ( (int)(pos - ( seen_dot * OCTET_STR_LEN )) > OCTET_LEN )
      return -1;

    *ip_byte_strs_ptr = *ip_string_ptr;

    pos++;
    ip_byte_strs_ptr++;
    ip_string_ptr++;

    if ( *ip_string_ptr == dot ) {

      seen_dot++;
      ip_string_ptr++;
      ip_byte_strs_ptr++;

      /* align ip_byte_strs_ptr to the start
       * of the next row in ip_byte_strs
       */
      const int octet_pos_left = pos % OCTET_LEN;

      if (octet_pos_left) {

        const int next_row_offset = ( OCTET_LEN - octet_pos_left );

        ip_byte_strs_ptr += next_row_offset;
        pos += next_row_offset;
      }
    }
  }

  /*
  unsigned int  oct = 0;
  unsigned int  pos = 0;
  while (*ip_string_ptr != '\0' ) {

    if ( pos > 2 || oct > 3 ) {
        return -1;
    }

    ip_byte_strs[oct][pos] = *ip_string_ptr;
    pos++;
    ip_string_ptr++;

    if ( *ip_string_ptr == dot ) {
      ip_string_ptr++;
      pos = 0;
      oct++;
    }
  }
  */

  uint8_t err = 0;
  for ( int i = 0; i <= 3; i++) {
    ipv4->octets[i] = oct_str_to_num(ip_byte_strs[i], &err );
    if (err)
      return -1;
  }

  return 0;
}


/* Converts to MSB 32 uint
int ipv4_mask_cidr( Ipv4Addr * mask ) {

   unsigned int bit = 1;
   int cidr  = 0;
   int count = 0;

   unsigned int msb_order = ( mask->octets[0] << 24) | ( mask->octets[1] << 16) | ( mask->octets[2] << 8) | mask->octets[3];

   printf("addr %u\n", msb_order);
   while (bit == 1) {
      if (count == 32)
        break ;

      bit = (msb_order & 0x80000000) >> 31;
      printf("bit %u\n", bit);
      if (bit == 1) {
         cidr++;
      }

      msb_order = msb_order << 1;
      count++;
   }

   return cidr;
}
*/

uint32_t get_net_addr( const Ipv4Addr *const addr ) {

  uint32_t net_order = ( addr->octets[0] << 24) |
                       ( addr->octets[1] << 16) |
                       ( addr->octets[2] << 8)  |
                       addr->octets[3];

  return net_order;
}

unsigned int ipv4_mask_cidr( const Ipv4Addr *const mask ) {

  unsigned int cidr = 0;

  for (int i = 0; i <= 3; i++) {

    uint8_t octet = mask->octets[i];
    unsigned int bit   = 1;
    unsigned int count = 0;

    while (count < 8) {

      /*
        Clearing the higher order bits here
        shouldn't be necessary with octet
        being a uint8_t, but better safe than
        sorry?
      */
      bit = (octet & 0x80) >> 7;

      if (bit == 0)
         return cidr;

      cidr++;
      count++;
      octet <<= 1;
    }

  }

  return cidr;
}


void print_ipv4( const Ipv4Addr *const ip ) {

  printf("%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8,
         ip->octets[0], ip->octets[1], ip->octets[2], ip->octets[3] );
}


int format_ipv4( char *buffer, const Ipv4Addr *const ip ) {

  return sprintf( buffer,
                  "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8,
                  ip->octets[0], ip->octets[1], ip->octets[2], ip->octets[3] );
}


Ipv4Addr* get_network_addr( const Ipv4Addr *const addr, const Ipv4Addr *const mask ) {

  const int network_addr = addr->address & mask->address;

  Ipv4Addr *network_addr_ptr = malloc(sizeof(Ipv4Addr));

  network_addr_ptr->address = network_addr;

  return network_addr_ptr;
}


Ipv4Addr* get_broadcast_addr( const Ipv4Addr *const addr, const Ipv4Addr *const mask ) {

  const int broadcast_addr = addr->address | ~(mask->address);

  Ipv4Addr *broadcast_addr_ptr = malloc(sizeof(Ipv4Addr));

  broadcast_addr_ptr->address = broadcast_addr;

  return broadcast_addr_ptr;
}


Ipv4Addr* get_low_addr( const Ipv4Addr *const addr, const Ipv4Addr *const broadcast_addr ) {

  Ipv4Addr *low_addr_ptr = malloc(sizeof(Ipv4Addr));

  if (broadcast_addr->address == addr->address )  // 32 case
    low_addr_ptr->address = addr->address;
  else if ( ( get_net_addr(broadcast_addr) - get_net_addr(addr) ) == 1 )  // 31 case
    low_addr_ptr->address = addr->address;
  else
    low_addr_ptr->address = addr->address + (1<<24); // NOT PORTABLE assumes LSB

  return low_addr_ptr;
}


Ipv4Addr* get_high_addr( const Ipv4Addr *const addr, const Ipv4Addr *const broadcast_addr ) {

  Ipv4Addr *high_addr_ptr = malloc(sizeof(Ipv4Addr));

  if (broadcast_addr->address == addr->address )
    high_addr_ptr->address = addr->address;
  else if ( ( get_net_addr(broadcast_addr) - get_net_addr(addr) ) == 1 )
    high_addr_ptr->address = broadcast_addr->address;
  else
    high_addr_ptr->address = broadcast_addr->address - (1<<24);

  return high_addr_ptr;
}


void print_ip_data( const char *ip_addr, const char *netmask ) {

  Ipv4Addr ip;

  int status = str_to_ipv4addr(ip_addr, &ip);

  if (status) {
     printf("\nError parsing [%s] status: %i\n", ip_addr, status);
     return;
  }

  char addr_buffer[IPV4_STR_LITERAL_SIZE];
  format_ipv4(addr_buffer, &ip);

  Ipv4Addr ip_mask;

  status = str_to_ipv4addr(netmask, &ip_mask);

  if (status) {
     printf("\nError parsing [%s] status: %i\n", netmask, status);
     return;
  }

  char mask_buffer[IPV4_STR_LITERAL_SIZE];
  format_ipv4(mask_buffer, &ip_mask);


  int cidr = ipv4_mask_cidr(&ip_mask);


  Ipv4Addr *network_addr = get_network_addr( &ip, &ip_mask );

  char net_buffer[IPV4_STR_LITERAL_SIZE];
  format_ipv4(net_buffer, network_addr);


  Ipv4Addr *broadcast_addr = get_broadcast_addr( &ip, &ip_mask );

  char bcast_buffer[IPV4_STR_LITERAL_SIZE];
  format_ipv4(bcast_buffer, broadcast_addr);


  Ipv4Addr *low_addr = get_low_addr( network_addr, broadcast_addr );

  char laddr_buffer[IPV4_STR_LITERAL_SIZE];
  format_ipv4(laddr_buffer, low_addr);


  Ipv4Addr *high_addr = get_high_addr( network_addr, broadcast_addr );

  char haddr_buffer[IPV4_STR_LITERAL_SIZE];
  format_ipv4(haddr_buffer, high_addr);


  printf("\n----------------------------\n");
  printf("IP: %s\n", addr_buffer);
  printf("Netmask: %s\n", mask_buffer);
  printf("CIDR: %i\n", cidr);
  printf("Network Addr: %s\n", net_buffer);
  printf("Broadcast Addr: %s\n", bcast_buffer);
  printf("Lowest Addr: %s\n", laddr_buffer);
  printf("Highest Addr: %s\n", haddr_buffer);

  free(network_addr);
  free(broadcast_addr);
  free(low_addr);
  free(high_addr);

}


int main() {

  print_ip_data("192.168.10.100","255.255.255.0");
  print_ip_data("192.8.18.5","255.255.248.0");
  print_ip_data("8.555.18.5","255.255.254.0");
  print_ip_data("8.8.8.8","255.255.255.255");
  print_ip_data("6.6.6.6","255.255.255.254");
  print_ip_data("127.0.0.2","255.255.255.252");
  print_ip_data("0.0.0.0","0.0.0.0");
  print_ip_data("4.4.4.4","WUT_R_U_DOING___AAAAAAAAAAAAAAAAAAHHHH");


  const char * short_mask = "255.248";

  Ipv4Addr ip_short_mask;

  int status = str_to_ipv4addr(short_mask, &ip_short_mask);

  printf("\nShort Mask: ");
  print_ipv4(&ip_short_mask);
  printf("\n");

  return 0;
}
