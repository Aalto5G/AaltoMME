/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   gtp.c
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  GTPv2 encoding and decoding functions. Based on OpenGGSN.
 *
 */

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include <syslog.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <arpa/inet.h>

/* #include <stdint.h>  ISO C99 types */

#include "gtp.h"
#include "gtpie.h"
//#include "queue.h"


/* Error reporting functions */
void gtp_err(int priority, char *filename, int linenum, char *fmt, ...) {
  va_list args;
  char buf[ERRMSG_SIZE];

  va_start(args, fmt);
  vsnprintf(buf, ERRMSG_SIZE, fmt, args);
  va_end(args);
  buf[ERRMSG_SIZE-1] = 0;
  syslog(priority, "%s: %d: %s", filename, linenum, buf);
}

void gtp_errpack(int pri, char *fn, int ln, struct sockaddr_in *peer,
		 void *pack, unsigned len, char *fmt, ...) {

  va_list args;
  char buf[ERRMSG_SIZE];
  char buf2[ERRMSG_SIZE];
  unsigned int n;
  int pos;

  va_start(args, fmt);
  vsnprintf(buf, ERRMSG_SIZE, fmt, args);
  va_end(args);
  buf[ERRMSG_SIZE-1] = 0;

  snprintf(buf2, ERRMSG_SIZE, "Packet from %s:%u, length: %d, content:",
	   inet_ntoa(peer->sin_addr),
	   ntohs(peer->sin_port),
	   len);
  buf2[ERRMSG_SIZE-1] = 0;
  pos = strlen(buf2);
  for(n=0; n<len; n++) {
    if ((pos+4)<ERRMSG_SIZE) {
      sprintf((buf2+pos), " %02hhx", ((unsigned char*)pack)[n]);
      pos += 3;
    }
  }
  buf2[pos] = 0;

  syslog(pri, "%s: %d: %s. %s", fn, ln, buf, buf2);

}





/* API Functions */

const char* gtp_version()
{
  return VERSION;
}


/**
 * get_default_gtp()
 * Generate a GPRS Tunneling Protocol signalling packet header, depending
 * on GTP version and message type. pdp is used for teid/flow label.
 * *packet must be allocated by the calling function, and be large enough
 * to hold the packet header.
 * returns the length of the header. 0 on error.
 **/
unsigned int get_default_gtp(int version, uint8_t type, void *packet) {
  struct gtp0_header *gtp0_default = (struct gtp0_header*) packet;
  struct gtp1_header_long *gtp1_default = (struct gtp1_header_long*) packet;
  struct gtp2_header_long *gtp2_defaultl = (struct gtp2_header_long*) packet;
  struct gtp2_header_short *gtp2_defaults = (struct gtp2_header_short*) packet;

  switch (version) {
  case 0:
    /* Initialise "standard" GTP0 header *//*
    memset(gtp0_default, 0, sizeof(struct gtp0_header));
    gtp0_default->flags=0x1e;
    gtp0_default->type=hton8(type);
    gtp0_default->spare1=0xff;
    gtp0_default->spare2=0xff;
    gtp0_default->spare3=0xff;
    gtp0_default->number=0xff;
    return GTP0_HEADER_SIZE;
  case 1:
    /* Initialise "standard" GTP1 header */
    /* 29.060: 8.2: S=1 and PN=0 */
    /* 29.060 9.3.1: For GTP-U messages Echo Request, Echo Response */
    /* and Supported Extension Headers Notification, the S field shall be */
    /* set to 1 */
    /* Currently extension headers are not supported */
    /*memset(gtp1_default, 0, sizeof(struct gtp1_header_long));
    gtp1_default->flags=0x32; /* No extension, enable sequence, no N-PDU *//*
    gtp1_default->type=hton8(type);
    return GTP1_HEADER_SIZE_LONG;*/
  case 2:
    if ( (type == GTP2_ECHO_REQ) ||
                (type == GTP2_ECHO_RSP)){ /* short header*/
        memset(gtp2_defaults, 0, sizeof(struct gtp2_header_short));
        gtp2_defaults->flags=0x40; /* No piggybacked, TEID disable, */
        gtp2_defaults->length = hton16(4);
    }
    else{
        memset(gtp2_defaultl, 0, sizeof(struct gtp2_header_long));
        gtp2_defaultl->flags=0x48; /* No piggybacked, TEID enable, */
        gtp2_defaultl->length = hton16(8);
    }
    gtp2_defaultl->type=hton8(type);
    return ntoh16(gtp2_defaultl->length)+4;
  default:
    gtp_err(LOG_ERR, __FILE__, __LINE__, "Unknown GTP packet version");
    return 0;
  }
}

int print_packet(void *packet, unsigned len)
{
  unsigned int i;
  printf("The packet looks like this (%d bytes):\n", len);
  for( i=0; i<len; i++) {
    printf("%02x ", (unsigned char)*(char *)(packet+i));
    if (!((i+1)%16)) printf("\n");
  };
  printf("\n");
  return 0;
}

int gtp2_recv(int sockfd, union gtp_packet *packet, size_t *len, struct sockaddr_in *peer, socklen_t *peerlen)
{
  unsigned char buffer[PACKET_MAX];
  int status=0;
  union gtp_packet *pack = NULL;
  *len = 0;

  *peerlen = sizeof(struct sockaddr_in);
  if ((status = recvfrom(sockfd, buffer, sizeof(buffer), 0,
               (struct sockaddr *) peer, peerlen)) < 0 ) {
    if (errno == EAGAIN) return 0;
    //gsn->err_readfrom++;
    gtp_err(LOG_ERR, __FILE__, __LINE__, "recvfrom(fd=%d, buffer=%lx, len=%d) failed: status = %d error = %s", sockfd, (unsigned long) buffer, sizeof(buffer), status, status ? strerror(errno) : "No error");
    return -1;
  }

  /* Need at least 1 byte in order to check version */
  if (status < (1)) {
    //gsn->empty++;
    gtp_errpack(LOG_ERR, __FILE__, __LINE__, peer, buffer, status,
                "Discarding packet - too small");
    return 0; /* Silently discard  */
  }

  pack = (union gtp_packet *) (buffer);

  /* Version must be no GTPv1 or GTPv2*/
  if (((pack->flags & 0xe0) != 0x20 && (pack->flags & 0xe0) != 0x40)) {
    //gsn->unsup++;
    gtp_errpack(LOG_ERR, __FILE__, __LINE__, peer, buffer, status,
                "Unsupported GTP version");
    //TODO @vicent manage unsuported req
    //gtp_unsup_req(gsn, version, &peer, fd, buffer, status);
    return 0; /* Silently discard  */
  }

  /* Check length of packet */
  if (status < GTP2_MINIMUM_HEADER_SIZE) {
    //gsn->tooshort++;
    gtp_errpack(LOG_ERR, __FILE__, __LINE__, peer, buffer, status,
                "GTP packet too short");
    return 0;  /* Silently discard  */
  }

  /* Check packet length field versus length of packet */
  if (status < (ntoh16(pack->gtp2s.h.length) + 4)) {
    //gsn->tooshort++;
    gtp_errpack(LOG_ERR, __FILE__, __LINE__, peer, buffer, status,
                "GTP packet length field does not match actual length");
    return 0;  /* Silently discard */
  }
  memcpy((void *)packet, (void *)buffer, status);
  *len=status;
  return 0;
}

