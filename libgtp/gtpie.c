/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   gtpie.c
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  GTPv2 encoding and decoding IE functions. Based on OpenGGSN.
 *
 *
 * Encapsulation
 * - gtpie_tlv, gtpie_tv0, gtpie_tv1, gtpie_tv2 ... Adds information
 * elements to a buffer.
 *
 * Decapsulation
 *  - gtpie_decaps: Returns array with pointers to information elements.
 *  - getie_getie: Returns the pointer of a particular element.
 *  - gtpie_gettlv: Copies tlv information element. Return 0 on success.
 *  - gtpie_gettv: Copies tv information element. Return 0 on success.
 *
 */


#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>

#include "gtpie.h"

struct gtp2_header_short { /*    Descriptions from 3GPP 29274 */
  uint8_t  flags;          /* 01 bitfield, with typical values */
                           /*    010..... Version: 2 */
                           /*    ...1.... Piggybacking flag (P) */
                           /*    ....1... TEID flag (T) */
                           /*    .....0.. Spare = 0 */
                           /*    ......0. Spare = 0 */
                           /*    .......0 Spare = 0 */
  uint8_t  type;           /* 02 Message type. */
  uint16_t length;         /* 03 Length seq(3)+1(spare)+IE length */
  uint32_t seq : 24;       /* 05 Sequence Number (3bytes)*/ /*Use of bit fields*/
  uint8_t  spare1;         /* 18 Spare */
}__attribute__((packed));


static int gtpie_getie(union gtpie_member* ie[], int type, int instance) {
  int j;
  for (j=0; j< GTPIE_SIZE; j++) {
      if(ie[j]==NULL)
            break;
      if ((ie[j] != 0) && (ie[j]->t == type)) {
          if (instance-- == 0){
              if (GTPIE_DEBUG) printf("IE found, IE #%d\n", j);
              return j;
          }
      }
      else{
          if (GTPIE_DEBUG) printf("IE # %d, Type %d != %d\n", j, ie[j]->t, type);
      }
  }
  return -1;
}

/*static int gtpie_exist(union gtpie_member* ie[], int type, int instance) {
  int j;
  for (j=0; j< GTPIE_SIZE; j++) {
    if ((ie[j] != 0) && (ie[j]->t == type)) {
      if (instance-- == 0) return 1;
    }
  }
  return 0;
}*/



int gtp2ie_tliv(void *p, unsigned int *length, unsigned int size,  uint8_t type, uint8_t instance,  uint8_t *value, uint16_t vsize) {
  int i;
  struct gtp2_header_short *header;
  header = (struct gtp2_header_short *)p;

  if ((*length + 8) >= size) return 1;
  ((union gtpie_member*) (p + *length))->tliv.t = hton8(type);
  ((union gtpie_member*) (p + *length))->tliv.l = hton16(vsize);
  ((union gtpie_member*) (p + *length))->tliv.i = hton8(instance);
  for (i=0 ; i<vsize ; i++)
  {
    memcpy((void*) (p + *length +4), value, sizeof(uint8_t));
    ((union gtpie_member*) (p + *length))->tliv.v[i] = hton8(value[i]);
  }
  *length += vsize+4;
  /*Modify the length field of the gtp2 packet*/
  header->length= hton16(ntoh16(header->length)+vsize+4);
  return 0;
}

int gtp2ie_gettliv(union gtpie_member* ie[], int type, int instance, uint8_t dst[], uint16_t *iesize){
  int ien;
  ien = gtpie_getie(ie, type, instance);

  if (ien>=0 && instance == ie[ien]->tliv.i){
      if (GTPIE_DEBUG) printf("IE type %d found, length %d\n", type, ntoh16(ie[ien]->tliv.l));
      memcpy(dst, ie[ien]->tliv.v, ntoh16(ie[ien]->tliv.l));
  }
  else{

      return EOF;
  }
  *iesize = ntoh16(ie[ien]->tliv.l);
  return 0;
}

int gtp2ie_decap(union gtpie_member* ie[], void *pack, unsigned len) {
  int i;
  int j = 0;
  unsigned char *p;
  unsigned char *end;

  struct gtp2_header_short *gtp2 = (struct gtp2_header_short*) pack;
  end = (unsigned char*) pack + len;
  if((gtp2->flags & 0xe8) == 0x40)
    p = pack+8;
  else
    p = pack+12;

  //memset(ie, 0, sizeof(union gtpie_member *) * GTPIE_SIZE);

  while ((p<end) && (j<GTPIE_SIZE)) {
    if (GTPIE_DEBUG) {
      printf("The packet looks like this:\n");
      for( i=0; i<(end-p); i++) {
        printf("%02x ", (unsigned char)*(char *)(p+i));
        if (!((i+1)%16)) printf("\n");
      };
      printf("\n");
    }
    ie[j] = (union gtpie_member*) p;
    if (GTPIE_DEBUG) printf("GTP2 IE found. Type %d\n", ie[j]->tliv.t);
    p+= 4 + ntoh16(ie[j]->tliv.l);
    j++;
  }
  if (p==end) {
    if (GTPIE_DEBUG) printf("GTPIE normal return. %lx %lx\n",
                            (unsigned long) p, (unsigned long) end);
    ie[j]=NULL;
    return 0; /* We landed at the end of the packet: OK */
  }
  else if (!(j<GTPIE_SIZE)) {
    if (GTPIE_DEBUG) printf("GTPIE too many elements.\n");
    return EOF; /* We received too many information elements */
  }
  else {
    if (GTPIE_DEBUG) printf("GTPIE exceeded end of packet. %lx %lx\n",
                            (unsigned long) p, (unsigned long) end);
    return EOF; /* We exceeded the end of the packet: Error */
  }
}


int gtp2ie_decaps_group(union gtpie_member **ie, unsigned int *size, void *from,  unsigned int len){
	int i;
	int j = 0;
	unsigned char *p;
	unsigned char *end;

	p = from;
	end = (unsigned char*) from + len;

	//memset(ie, 0, sizeof(union gtpie_member *) * GTPIE_SIZE);
	while ((p<end) && (j<GTPIE_SIZE)) {
		if (GTPIE_DEBUG) {
		printf("The group looks like this:\n");
			for( i=0; i<(end-p); i++) {
			  printf("%02x ", (unsigned char)*(char *)(p+i));
			  if (!((i+1)%16)) printf("\n");
			};
			printf("\n");
		}
		ie[j] = (union gtpie_member*) p;
		if (GTPIE_DEBUG) printf("GTP2 IE found. Type %d\n", ie[j]->tliv.t);
		p+= 4 + ntoh16(ie[j]->tliv.l);
		j++;
	}
	*size=j;

	if (p==end) {
		if (GTPIE_DEBUG) printf("GTPIE normal return. %lx %lx\n",
							(unsigned long) p, (unsigned long) end);
	return 0; /* We landed at the end of the packet: OK */
	}
	else if (!(j<GTPIE_SIZE)) {
		if (GTPIE_DEBUG) printf("GTPIE too many elements.\n");
		return EOF; /* We received too many information elements */
	}
	else {
		if (GTPIE_DEBUG) printf("GTPIE exceeded end of packet. %lx %lx\n",
							(unsigned long) p, (unsigned long) end);
		return EOF; /* We exceeded the end of the packet: Error */
	}

}

int gtp2ie_encaps(union gtpie_member ie[], unsigned int size, void *pack, unsigned *len) {
  unsigned int i, j;
  unsigned char *p;
  unsigned char *end;
  union gtpie_member *m;
  int iesize;
  struct gtp2_header_short *header;
  header = (struct gtp2_header_short *)pack;
  p = pack+*len;
  memset(p, 0, GTPIE_MAX);
  end = p + GTPIE_MAX;
  for (i=0; i<size; i++) {
    if (GTPIE_DEBUG) printf("gtpie_encaps. Number %d, Type %d\n", i, ie[i].t);
    m=(union gtpie_member *)p;
    iesize = 4 + ntoh16(ie[i].tliv.l);
    if (p+iesize < end) {
      memcpy(p, &ie[i], iesize);
      p += iesize;
      *len += iesize;
      /*Modify the gtp2 header length*/
      header->length= hton16(ntoh16(header->length)+iesize);
    }
    else return -1;              /* Out of space */
  }
  return 0;
}

/*     gtp2ie_encaps_group(GTPV2C_IE_BEARER_CONTEXT, 0, &ie[12], ie_bearer_ctx, 3);*/
int gtp2ie_encaps_group(int type, int instance, void *to, union gtpie_member ie[], unsigned int size){
    unsigned int i, j;
    unsigned char *p;
    unsigned char *end;
    int iesize;

    union gtpie_member *ie_dst;
    ie_dst= (union gtpie_member*)to;

    ie_dst->tliv.t=type;
    ie_dst->tliv.i=instance;
    ie_dst->tliv.l=0;
    p = to+4;
    /*memset(p, 0, sizeof(union gtpie_member));*/
    end = p + GTPIE_MAX;
    for (i=0; i<size; i++) {
      if (GTPIE_DEBUG) printf("gtpie_encaps_group. Number %d, Type %d\n", i, ie[i].t);
      iesize = 4 + ntoh16(ie[i].tliv.l);
      if (p+iesize < end) {
        memcpy(p, &ie[i], iesize);
        p += iesize;
        ie_dst->tliv.l = hton16(ntoh16(ie_dst->tliv.l)+iesize);
      }
      else return -1;              /* Out of space */
    }
    return 0;
}

void tbcd2dec(uint64_t *dec, const uint8_t *tbcd, const uint32_t length){
    uint64_t decimal = 0;
	uint32_t len = length;
	uint64_t bcd;

	memcpy(&bcd, tbcd, length);
	/*printf("test bcd 0x%16llx\n", bcd);*/

	while(len--){
		/* digit 2 	digit 1*/
    	/* 0101 	1001 -> 95*/
    	if((bcd & 0xF0)!= 0xF0){
    	    decimal = decimal * 100 + (bcd & 0x0F)* 10 + ((bcd  >> 4) & 0x0F) ;
    	}else{
    	    decimal = decimal * 10 + (bcd & 0x0F);
    	}
    	/*printf("decimal %16lld, bcd = %16llx\n", decimal, bcd);*/
        bcd>>=8;
    }
	*dec= decimal;
}

void dec2tbcd(uint8_t *tbcd, uint32_t *length, const uint64_t dec){
	uint64_t bcd=0, from = dec;
	uint8_t len=0;
	uint8_t l=1;

	while(from>9){ /*Count Digits*/
        l++;
        from/=10;
	}
    from = dec;
    if(l%2){ /*If odd, add a 0xF digit at the end*/
        bcd = 0xF;
        bcd <<= 4;
        bcd |= (from % 10);
        from /= 10;
        l++;
    }

    while(from>0){  /*Reorder the digits, network order*/
        bcd <<= 4;
        /*if(!(len%2)){
            bcd |= (from % 100)/10;
        }
        else{
            bcd |= (from % 10);
            from /= 100;
        }*/
        bcd |= (from % 10);
        from /= 10;
        len++;
    }
	*length = l/2;
	/*bcd = hton64(bcd);*/
    /*printf("bcd=0x%16llx\n", bcd);

	printf("#digits = %d\n", *length);
	printf("tbcd=0x%llx\n", tbcd);*/
	memcpy(tbcd, &bcd, *length);
	/*printf("tbcd=0x%llx\n", tbcd);*/

}
