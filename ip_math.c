#include <stdio.h>
#include <math.h>
#include <inttypes.h>

#define OCTET_LEN 3

#define OCTET_STR_LEN 4

uint8_t oct_str_to_num( char * num_str ) {

  uint8_t num_list[OCTET_LEN] = {};
  uint8_t place       = 0;

  while ( *num_str != '\0' ) {
    num_list[place++] = *num_str;
    num_str++;

    if (place > OCTET_LEN - 1)
      break;
  }

  uint8_t num = 0;

  for ( int i=0; i < place; i++ ) {
      uint8_t char_int = ( (uint8_t)(num_list[i]) - '0' );

      num = num + ( char_int * (uint8_t)pow(10.0, place - i - 1) );
  }

  return num;
}


typedef union {
    uint32_t address;
    uint8_t  octets[4];
} Ipv4Addr;


int str_to_ipv4addr( const char * ip_string_ptr, Ipv4Addr * ipv4 ) {

  const char dot = '.';

  char ip_byte_strs[OCTET_STR_LEN][OCTET_STR_LEN] = {};
  /* Who needs to manage multi-dimensional array
   * indexes ?
   *
   * NOT MY COMPILER
   * */
  char * ip_byte_strs_ptr = (char*)ip_byte_strs;
  unsigned int  pos = 0;
  unsigned int  seen_dot = 0;

  while (*ip_string_ptr != '\0' ) {

    if ( pos > 31 ) {
        return -1;
    }

    /* some string validation, as a treat */
    if ( (int)(pos - ( seen_dot * OCTET_STR_LEN )) > OCTET_LEN ) {
        return -1;
    }

    *ip_byte_strs_ptr = *ip_string_ptr;

    pos++;
    ip_byte_strs_ptr++;
    ip_string_ptr++;

    if ( *ip_string_ptr == dot ) {
      seen_dot++;
      ip_string_ptr++;
      /* align ip_byte_strs_ptr pointer to the start
       * of the next row ip_byte_strs
       */
      ip_byte_strs_ptr += 1 + ( pos % OCTET_LEN );
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

  ipv4->octets[0] = oct_str_to_num(ip_byte_strs[0]);
  ipv4->octets[1] = oct_str_to_num(ip_byte_strs[1]);
  ipv4->octets[2] = oct_str_to_num(ip_byte_strs[2]);
  ipv4->octets[3] = oct_str_to_num(ip_byte_strs[3]);

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


int main() {

  const char * addr = "192.168.10.5";
  const char * mask = "255.255.248.0";

  Ipv4Addr ip2;

  int status = str_to_ipv4addr(addr, &ip2);

  printf("status: %i\n", status);
  printf("IP: ");
  print_ipv4(ip2);
  printf("\n");

  Ipv4Addr ip_mask;

  status = str_to_ipv4addr(mask, &ip_mask);

  printf("status: %i\n", status);
  printf("Mask: ");
  print_ipv4(ip_mask);
  printf("\n");


  int cidr = ipv4_mask_cidr(&ip_mask);

  printf("cidr: %i\n", cidr);

  Ipv4Addr ip_fail;

  const char bad_addr[] = {'H', 'e', 'l', 'l', 'o'};
  status = str_to_ipv4addr(bad_addr, &ip_fail);

  printf("status: %i\n", status);
  printf("Bad: ");
  print_ipv4(ip_fail);
  printf("\n");


  const char * short_mask = "255.248";

  Ipv4Addr ip_short_mask;

  status = str_to_ipv4addr(short_mask, &ip_short_mask);

  printf("status: %i\n", status);
  printf("Short Mask: ");
  print_ipv4(ip_short_mask);
  printf("\n");

  return 0;
}
