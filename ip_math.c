#include <stdio.h>
//#include <math.h>
#include <inttypes.h>
#include <stdlib.h>

#define OCTET_LEN 3

#define OCTET_STR_LEN 4

uint8_t oct_str_to_num( char * num_str, uint8_t * err ) {

  uint8_t num_list[OCTET_LEN] = {};
  uint8_t place       = 0;

  while ( *num_str != '\0' ) {

    if (place > OCTET_LEN) {
      *err |= 1;
      return 0;
    }

    num_list[place++] = *num_str;
    num_str++;
  }

  uint8_t num = 0;

  for ( int i=0; i < place; i++ ) {

    uint8_t char_int = ( (uint8_t)(num_list[i]) - '0' );
    uint16_t t = num * 10 + char_int;

    if ( t > 255 ) {
      *err |= 1;
      return 0;
    }
    else {
      num = t;
    }
  }

  return num;
}


typedef union {
  uint32_t address;
  uint8_t  octets[4];
} Ipv4Addr;


int str_to_ipv4addr( const char * ip_string_ptr, Ipv4Addr * ipv4 ) {

  const char dot = '.';

  char ip_byte_strs[OCTET_STR_LEN][OCTET_STR_LEN] = {"\0\0\0\0","\0\0\0\0","\0\0\0\0","\0\0\0\0" };
  /* Who needs to manage multi-dimensional array
   * indexes ?
   *
   * NOT MY COMPILER
   * */
  char * ip_byte_strs_ptr = (char*)ip_byte_strs;
  unsigned int pos = 0;
  unsigned int seen_dot = 0;

  while (*ip_string_ptr != '\0' ) {

    if ( pos > 31 )
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

      /* align ip_byte_strs_ptr pointer to the start
       * of the next row in ip_byte_strs
       */
      int octet_pos = pos % OCTET_LEN;
      if (octet_pos) {

         int next_row_offset = ( OCTET_LEN - octet_pos );

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
  ipv4->octets[0] = oct_str_to_num(ip_byte_strs[0], &err );
  ipv4->octets[1] = oct_str_to_num(ip_byte_strs[1], &err );
  ipv4->octets[2] = oct_str_to_num(ip_byte_strs[2], &err );
  ipv4->octets[3] = oct_str_to_num(ip_byte_strs[3], &err );

  if ( err )
    return -1;

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


unsigned int ipv4_mask_cidr( Ipv4Addr * mask ) {

  unsigned int cidr = 0;

  for (int i = 0; i <= 3; i++) {

    uint8_t octet = mask->octets[i];
    uint8_t bit   = 1;
    uint8_t count = 0;

    while (bit == 1) {

      if (count == 8)
        break;

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


void print_ipv4( Ipv4Addr ip ) {

  printf("%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8,
         ip.octets[0], ip.octets[1], ip.octets[2], ip.octets[3] );
}

int format_ipv4( char *buffer, Ipv4Addr ip ) {

  return sprintf( buffer,
                  "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8,
                  ip.octets[0], ip.octets[1], ip.octets[2], ip.octets[3] );
}

Ipv4Addr * get_network_addr( Ipv4Addr *addr, Ipv4Addr *mask ) {

  const int ip_addr = addr->address;
  const int ip_mask = mask->address;

  const int network_addr = ip_addr & ip_mask;

  Ipv4Addr* network_addr_ptr = malloc(sizeof(Ipv4Addr));

  network_addr_ptr->address = network_addr;

  return network_addr_ptr;
}

Ipv4Addr * get_broadcast_addr( Ipv4Addr *addr, Ipv4Addr *mask ) {

  const int ip_addr        = addr->address;
  const int wildcard_mask  = ~(mask->address);
  const int broadcast_addr = ip_addr | wildcard_mask;

  Ipv4Addr* broadcast_addr_ptr = malloc(sizeof(Ipv4Addr));

  broadcast_addr_ptr->address = broadcast_addr;

  return broadcast_addr_ptr;
}


Ipv4Addr * get_low_addr( Ipv4Addr *addr, Ipv4Addr *broadcast_addr ) {

  const int ip_addr = addr->address;

  Ipv4Addr* low_addr_ptr = malloc(sizeof(Ipv4Addr));
  low_addr_ptr->address  = ip_addr + (1<<24);

  return low_addr_ptr;
}

Ipv4Addr * get_high_addr( Ipv4Addr *addr, Ipv4Addr *broadcast_addr ) {

  const int broadcast_ip = broadcast_addr->address;

  Ipv4Addr* high_addr_ptr = malloc(sizeof(Ipv4Addr));
  high_addr_ptr->address  = broadcast_ip - (1<<24);

  return high_addr_ptr;
}


int main() {

  const char * addr = "192.8.18.5";
  const char * mask = "255.255.254.0";

  Ipv4Addr ip2;

  int status = str_to_ipv4addr(addr, &ip2);
  printf("status: %i\n", status);


  char buffer[12];
  format_ipv4(buffer, ip2);
  printf("IP: %s\n", buffer);

  Ipv4Addr ip_mask;

  status = str_to_ipv4addr(mask, &ip_mask);
  printf("status: %i\n", status);

  printf("Mask: ");
  print_ipv4(ip_mask);
  printf("\n");

  int cidr = ipv4_mask_cidr(&ip_mask);

  printf("CIDR: %i\n", cidr);


  Ipv4Addr* network_addr = get_network_addr( &ip2, &ip_mask );

  printf("Network Addr: ");
  print_ipv4(*network_addr);
  printf("\n");

  Ipv4Addr* broadcast_addr = get_broadcast_addr( &ip2, &ip_mask );

  printf("Broadcast Addr: ");
  print_ipv4(*broadcast_addr);
  printf("\n");


  Ipv4Addr * low_addr = get_low_addr( network_addr, broadcast_addr );

  printf("Lowest Addr: ");
  print_ipv4(*low_addr);
  printf("\n");

  Ipv4Addr * high_addr = get_high_addr( network_addr, broadcast_addr );


  printf("Highest Addr: ");
  print_ipv4(*high_addr);
  printf("\n");


  free(network_addr);
  free(broadcast_addr);
  free(low_addr);
  free(high_addr);

  const char * short_mask = "255.248";

  Ipv4Addr ip_short_mask;

  status = str_to_ipv4addr(short_mask, &ip_short_mask);

  printf("Short Mask: ");
  print_ipv4(ip_short_mask);
  printf("\n");

  Ipv4Addr ip_fail;

  const char bad_addr[] = {'H', 'e', 'l', 'l', 'o'};
  status = str_to_ipv4addr(bad_addr, &ip_fail);

  printf("status: %i\n", status);

  const char bad_addr2[] = "555.555.555.300";
  status = str_to_ipv4addr(bad_addr2, &ip_fail);

  printf("status: %i\n", status);

  return 0;
}
