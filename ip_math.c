#include <stdio.h>
#include <math.h>
#include <inttypes.h>

void str_to_byte_strings( char * ip_string, char ret[4][4] ) {

  char dot = '.';

  int  pos    = 0;
  int  ip_pos = 0;
  char *counter = ip_string;

  while (*counter != '\0') {

    ret[ip_pos][pos] = *counter;
    pos++;
    counter++;

    if (*counter == dot ) {
        counter++;
        pos = 0;
        ip_pos++;
    }
  }

}

uint8_t str_to_num( char * num_str ) {

  uint8_t num_list[3] = {};
  uint8_t place       = 0;

  while ( *num_str != '\0' ) {
    num_list[place++] = *num_str;
    num_str++;
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


int str_to_ipv4addr( char * ip_string, Ipv4Addr * ipv4 ) {

  unsigned int  pos = 0;
  unsigned int  oct = 0;

  char ip_byte_strs[4][4] = {"\0\0\0\0","\0\0\0\0","\0\0\0\0","\0\0\0\0" };
  char dot      = '.';
  char *counter = ip_string;

  while (*counter != '\0' ) {

    if ( pos > 2 || oct > 3 ) {
        return -1;
    }

    ip_byte_strs[oct][pos] = *counter;
    pos++;
    counter++;

    if ( *counter == dot ) {
      counter++;
      pos = 0;
      oct++;
    }
  }

  ipv4->octets[0] = str_to_num(ip_byte_strs[0]);
  ipv4->octets[1] = str_to_num(ip_byte_strs[1]);
  ipv4->octets[2] = str_to_num(ip_byte_strs[2]);
  ipv4->octets[3] = str_to_num(ip_byte_strs[3]);

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

   unsigned int cidr= 0;

   for (int i = 0; i <= 3; i++) {
       unsigned int octet = mask->octets[i];

       unsigned int count = 0;
       unsigned int bit = 1;

       while (bit == 1) {
          if (count == 8)
            break;

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

int main() {

  char * addr = "192.168.10.5";
  char * mask = "255.255.248.0";

  char ip_bytes[4][4] = { "\0\0\0\0","\0\0\0\0","\0\0\0\0","\0\0\0\0" };

  str_to_byte_strings(addr, ip_bytes);

  Ipv4Addr ip;

  ip.octets[0] = str_to_num(ip_bytes[0]);

  printf("int: %hhu\n", ip.octets[0] );


  Ipv4Addr ip2;

  int status = str_to_ipv4addr(addr, &ip2);

  printf("IP: %" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\n",
          ip2.octets[0], ip2.octets[1], ip2.octets[2], ip2.octets[3] );

  Ipv4Addr ip_mask;

  status = str_to_ipv4addr(mask, &ip_mask);

  printf("Mask: %" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\n",
          ip_mask.octets[0], ip_mask.octets[1], ip_mask.octets[2], ip_mask.octets[3] );

  int cidr = ipv4_mask_cidr(&ip_mask);

  printf("cidr: %i\n", cidr);

  Ipv4Addr ip_fail;

  char bad_attr[] = {'H', 'e', 'l', 'l', 'o'};
  status = str_to_ipv4addr(bad_attr, &ip_fail);

  printf("status: %i\n", status);
  printf("Mask: %" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\n",
          ip_fail.octets[0], ip_fail.octets[1], ip_fail.octets[2], ip_fail.octets[3] );

  char * short_mask = "255.248";

  Ipv4Addr ip_short_mask;

  status = str_to_ipv4addr(short_mask, &ip_short_mask);

  printf("status: %i\n", status);
  printf("Mask: %" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\n",
          ip_short_mask.octets[0], ip_short_mask.octets[1], ip_short_mask.octets[2], ip_short_mask.octets[3] );

  return 0;
}
