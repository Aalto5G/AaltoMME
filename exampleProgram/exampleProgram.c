/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

#include "gtp.h"
#include "MME_engine.h"
#include "storagesys.h"
#include "logmgr.h"
#include "MME.h"
#include "gtpie.h"

/* Libevent. */
//#include <event.h>
#include <event2/event.h>
#include <event2/util.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <error.h>

#include "commands.h"


int
check_for_old_version(void)
{
    const char *v = event_get_version();
    /* This is a dumb way to do it, but it is the only thing that works
       before Libevent 2.0. */
    if (!strncmp(v, "0.", 2) ||
        !strncmp(v, "1.1", 3) ||
        !strncmp(v, "1.2", 3) ||
        !strncmp(v, "1.3", 3)) {

        printf("Your version of Libevent is very old.  If you run into bugs,"
               " consider upgrading.\n");
        return -1;
    } else {
        printf("Running with Libevent version %s\n", v);
        return 0;
    }
}

void do_accept( evutil_socket_t listener, short event, void *arg)
{
  //struct gsn_t *gsn = arg;
  /*if (listener == gsn->fd0){
    //gtp_decaps0(gsn);
  }
  else if (listener == gsn->fd1c){
    //gtp_decaps1u(gsn);
  }
  else
    return;
*/
}

void do_parse_test(evutil_socket_t listener, short event, void *arg){
    struct t_message *msg;
    struct fteid_t fteid;
    uint8_t value[20];
    uint16_t vsize;
    union gtpie_member* ie[10];

    /*for(i=0; i<10; i++){
        ie[i] = malloc(sizeof(union gtpie_member*));
    }*/

    msg = malloc(sizeof(struct t_message));
    printf("sizeof(struct t_message) = %d\n", sizeof(struct t_message));

    if(&(msg->packet) != NULL){
        printf("&(msg->packet) = %p\n", &(msg->packet));
        gtp2_recv(listener, (union gtp_packet*)&(msg->packet), &(msg->length), (struct sockaddr_in*)&(msg->peer.peerAddr), &(msg->peer.socklen));

        printf("Recv packet :\n");
        print_packet(&(msg->packet), msg->length);

        printf("Type = %d, length %d\n", ((union gtp_packet*)&(msg->packet))->gtp2l.h.type, msg->length);
        //memset(ie, 0, sizeof(union gtpie_member*)*GTPIE_SIZE);
        //for (i=0; i)
        gtp2ie_decap(ie, &(msg->packet), msg->length);
        gtp2ie_gettliv(ie, GTPV2C_IE_FTEID, 0, value, &vsize);
        printf("vsize = %d\n", vsize);
        memcpy(&fteid, value, vsize);
        printf("teid = %x into\n", hton32(fteid.teid));
    }
    else
    {
        printf("sizeof(msg) %d\n", sizeof(*msg));
    }
    free(msg);
}


void do_send_echo_req( evutil_socket_t listener, short event, void *arg)
{
  //struct gsn_t *gsn = arg;
  struct in_addr destaddr;
  inet_aton("127.0.0.1", &destaddr);

  //gtp_echo_req(gsn, 2, NULL, &destaddr);
}

void do_send_create_new_session_req(evutil_socket_t listener, short event, void *arg){
     union gtp_packet packet;
     struct fteid_t  fteid;
     struct qos_t    qos;
     struct sockaddr_in  peer;
     union gtpie_member ie[14], ie_bearer_ctx[4];
     int sock;
     uint32_t length, ielen, peerlen;
     uint8_t bytefield[30];

     //struct gsn_t *gsn = arg;

     length = get_default_gtp(2, GTP2_CREATE_SESSION_REQ, &packet);

         /*IMSI*/
         ie[0].tliv.i=0;
         ie[0].tliv.t=GTPV2C_IE_IMSI;
         dec2tbcd(ie[0].tliv.v, &ielen, 1234567890123456);
         ie[0].tliv.l=hton16(ielen);
         /*MSISDN*/
         ie[1].tliv.i=0;
         ie[1].tliv.t=GTPV2C_IE_MSISDN;
         dec2tbcd(ie[1].tliv.v, &ielen, 1234567890123456);
         ie[1].tliv.l=hton16(ielen);
         /*MEI*/
         ie[2].tliv.i=0;
         ie[2].tliv.t=GTPV2C_IE_MEI;
         dec2tbcd(ie[2].tliv.v, &ielen, 1234567890123456);
         ie[2].tliv.l=hton16(ielen);
         /*RAT type*/
         ie[3].tliv.i=0;
         ie[3].tliv.l=hton16(1);
         ie[3].tliv.t=GTPV2C_IE_RAT_TYPE;
         ie[3].tliv.v[0]=6;
         //memset(ie[3].tliv.v, 6, 1); /*Type 6= EUTRAN*/
         /*F-TEID*/
         ie[4].tliv.i=0;
         ie[4].tliv.l=hton16(9);
         ie[4].tliv.t=GTPV2C_IE_FTEID;
         fteid.ipv4=1;
         fteid.ipv6=0;
         fteid.iface= hton8(S11_MME);
         fteid.teid = hton32(0);
         inet_pton(AF_INET, "10.0.2.15", &(fteid.addr.addrv4));
         /*memcpy(ie[4].tliv.v, b, 9);*/
         ie[4].tliv.l=hton16(FTEID_IP4_SIZE);
         memcpy(ie[4].tliv.v, &fteid, FTEID_IP4_SIZE);
         /*F-TEID PGW S5/S8 Address for Control Plane or PMIP */
         ie[5].tliv.i=1;
         ie[5].tliv.l=hton16(FTEID_IP4_SIZE);
         ie[5].tliv.t=GTPV2C_IE_FTEID;
         fteid.ipv4=1;
         fteid.ipv6=0;
         fteid.iface= hton8(S5S8C_PGW);
         fteid.teid = hton32(1);
         inet_pton(AF_INET, "10.11.0.145", &(fteid.addr.addrv4));
         memcpy(ie[5].tliv.v, &fteid, FTEID_IP4_SIZE);
         /*APN*/
         ie[6].tliv.i=0;
         ie[6].tliv.l=hton16(2);
         ie[6].tliv.t=GTPV2C_IE_APN;
         memset(ie[6].tliv.v, 0x61, 2);
         /*PAA*/
         ie[7].tliv.i=0;
         ie[7].tliv.l=hton16(5);
         ie[7].tliv.t=GTPV2C_IE_PAA;
         bytefield[0]=0x01;  /*PDN Type  IPv4 */
         bytefield[1]=0x0;   /*IP = 0.0.0.0*/
         bytefield[2]=0x0;
         bytefield[3]=0x0;
         bytefield[4]=0x0;
         memcpy(ie[7].tliv.v, bytefield, 5);
         /*Serving Network*/
         ie[8].tliv.i=0;
         ie[8].tliv.l=hton16(3);
         ie[8].tliv.t=GTPV2C_IE_SERVING_NETWORK;
         bytefield[0]=0x01; /* 0000.... MCC digit 2 ....0000 MCC digit 1*/
         bytefield[1]=0x00; /* 0000.... MNC digit 3 ....0000 MCC digit 3*/
         bytefield[2]=0x01; /* 0000.... MNC digit 2 ....0000 MNC digit 1*/
         memcpy(ie[8].tliv.v, bytefield, 3);
         /*PDN type*/
         ie[9].tliv.i=0;
         ie[9].tliv.l=hton16(1);
         ie[9].tliv.t=GTPV2C_IE_PDN_TYPE;
         bytefield[0]=0x01; /* PDN type IPv4*/
         memcpy(ie[9].tliv.v, bytefield, 1);
         /*APN restriction*/
         ie[10].tliv.i=0;
         ie[10].tliv.l=hton16(1);
         ie[10].tliv.t=GTPV2C_IE_APN_RESTRICTION;
         bytefield[0]=0x01; /* PDN type IPv4*/
         memcpy(ie[10].tliv.v, bytefield, 1);
         /*Selection Mode*/
         ie[11].tliv.i=0;
         ie[11].tliv.l=hton16(1);
         ie[11].tliv.t=GTPV2C_IE_SELECTION_MODE;
         bytefield[0]=0x01; /* PDN type IPv4*/
         memcpy(ie[11].tliv.v, bytefield, 1);

         /*Bearer contex*/
             /*EPS Bearer ID */
             ie_bearer_ctx[0].tliv.i=0;
             ie_bearer_ctx[0].tliv.l=hton16(1);
             ie_bearer_ctx[0].tliv.t=GTPV2C_IE_EBI;
             ie_bearer_ctx[0].tliv.v[0]=0x05; /*EBI = 5,  EBI > 4, see 3GPP TS 24.007 11.2.3.1.5  EPS bearer identity */
             /*EPS Bearer TFT */
             ie_bearer_ctx[1].tliv.i=0;
             ie_bearer_ctx[1].tliv.l=hton16(3);
             ie_bearer_ctx[1].tliv.t=GTPV2C_IE_BEARER_TFT;
             bytefield[0]=0x01; /**/
             bytefield[1]=0x01;
             bytefield[2]=0x01;
             memcpy(ie_bearer_ctx[1].tliv.v, bytefield, 3);
             /* Bearer QoS */
             ie_bearer_ctx[2].tliv.i=0;
             ie_bearer_ctx[2].tliv.l=hton16(sizeof(struct qos_t));
             ie_bearer_ctx[2].tliv.t=GTPV2C_IE_BEARER_LEVEL_QOS;
             /*memcpy(ie_bearer_ctx[2].tliv.v, b, 22);*/
             qos.pci = 0;
             qos.pl  = 1;
             qos.pvi = 1;
             qos.qci = 1;
             qos.mbr_ul = 0;
             qos.mbr_dl = 0;
             qos.gbr_ul = 0;
             qos.gbr_dl = 0;
             memcpy(ie_bearer_ctx[2].tliv.v, &qos, sizeof(struct qos_t));
         gtp2ie_encaps_group(GTPV2C_IE_BEARER_CONTEXT, 0, &ie[12], ie_bearer_ctx, 3);
         gtp2ie_encaps(ie, 13, &packet, &length);

         /*Packet header modifications*/
         packet.gtp2l.h.seq = hton24(0);
         packet.gtp2l.h.tei = hton32(0);

         /*Debug information*/
         printf("packet length %d\n",length);
         print_packet(&packet, length);

         /*******************************************************************/
     /*Get SGW addr*/
     //sock = gsn->fd1c;
     peer.sin_family = AF_INET;
     peer.sin_port = htons(GTP2C_PORT);
     inet_pton(AF_INET, "10.11.0.142", &(peer.sin_addr));

     peerlen = sizeof(struct sockaddr_in);
     printf("Peerlen = %d\n", peerlen);

     printf("sock %p, packet %p, lenght %p, peer %p , peerlen %p\n", &sock, &packet, &length, &peer, &peerlen);
     if (sendto(sock, &packet, length, 0, (struct sockaddr *)&peer, peerlen) < 0) {
         log_errpack(LOG_ERR, errno, (struct sockaddr_in *)&peer, &packet, length,
                 "Sendto(fd=%d, msg=%lx, len=%d) failed", sock, (unsigned long) &packet, length);
         return;
     }
     /*******************************************************************/
}

/*int main(){
    uint8_t tbcd[10], buf[32];
    uint64_t dec, imei = 123456789012345, test = 0x5F34129078563412;
    uint32_t bcdlen, n, pos;

    *//*Decimal to TBCD conversion check*//*
    dec2tbcd(tbcd, &bcdlen, imei);
    //ck_assert_msg((bcdlen==8), "bcdlen not correct %d, expected 8. tbcd = 0x%10llx", bcdlen, *tbcd);

    buf[32-1] = 0;
    pos = 0;
    for(n=0; n<bcdlen; n++) {
        if ((pos+4)<32) {
          sprintf((buf+pos), " %02hhx", ((unsigned char*)tbcd)[n]);
          pos += 3;
        }
    }
    buf[pos] = 0;
    printf("tbcd = %s\n", buf);
    //ck_assert_msg(memcmp(tbcd, &test, bcdlen)==0,"conversion to TBCD not correct, tbcd = %s != 0x%16llx", buf, test);

    *//*TBCD to decimal conversion check*//*
    tbcd2dec(&dec, tbcd, bcdlen);
    printf("dec %lld\n", dec);
    //ck_assert_msg(dec == imei, "TBCD to decimal concersion incorrect,  %d != %d", dec, imei);
    return 0;

}*/

/*int main(void)
{
  struct gsn_t *gsn;              *//* GSN instance            *//*
  struct in_addr listen_, destaddr;
  union gtp_packet packet;
  size_t len;
  struct sockaddr_in src_addr;
  int addrlen = sizeof(src_addr);

  listen_.s_addr=INADDR_ANY;

  gtp_new(&gsn, "",  &listen_, GTP_MODE_GGSN);

  inet_aton("127.0.0.1", &destaddr);
  gtp_echo_req(gsn, 2, NULL, &destaddr);

  gtp2_recv(gsn->fd1c, &packet, &len, &src_addr, &addrlen);

  printf("src_addr: %s \n port: %d\n", inet_ntoa(src_addr.sin_addr), ntohs(src_addr.sin_port));

  gtp_echo_resp(gsn, 2, &src_addr, gsn->fd1c, &packet, len);
  gtp_free(gsn);
}*/


int main(void)
{

    struct event_base *base;
    //struct event *listener_event0, *listener_event1c, *send_echo_req;// *listener_event1u;
    //struct timeval second_tv = {1,0};


    base = event_base_new();
    if (!base)
    {
        error(1, errno, "Failed to libevent event-base");
        exit(1);
    }

    openlog(PACKAGE, (LOG_PID | LOG_PERROR), LOG_DAEMON);
    printf("eGTP-PING 0.1.\n");
    printf("gtplib version: %s\n", gtp_version());
    check_for_old_version();

    //struct gsn_t *gsn;              /* GSN instance            */
    /*int listen_fd;
    struct sockaddr_in listen_addr;*/
    //struct in_addr listen_;
    //listen_.s_addr=INADDR_ANY;

    /*if (gtp_new(&gsn, ".",  &listen_, GTP_MODE_GGSN)) {
        error(1, errno, "Failed to create gtp");
        exit(1);
    }*/

    /*evutil_make_socket_nonblocking(gsn->fd0);
    evutil_make_socket_nonblocking(gsn->fd1c);
    evutil_make_socket_nonblocking(gsn->fd1u);
*/
    struct in_addr destaddr;
    inet_aton("127.0.0.1", &destaddr);

    /* Initialize libevent. */
    //gtp_echo_req(gsn, 0, NULL, &destaddr);
    //gtp_echo_req(gsn, 1, NULL, &destaddr);
    //gtp_echo_req(gsn, 2, NULL, &destaddr);
/*
    listener_event0 = event_new(base, gsn->fd0, EV_READ|EV_PERSIST, do_accept, (void*)gsn);
    //listener_event1c = event_new(base, gsn->fd1c, EV_READ|EV_PERSIST, do_accept, (void*)gsn);
    listener_event1c = event_new(base, gsn->fd1c, EV_READ|EV_PERSIST, do_parse_test, (void*)gsn);
    //listener_event1u = event_new(base, gsn->fd1u, EV_READ|EV_PERSIST, gtp_decaps1u, (void*)base);*/


    /*send_echo_req = evtimer_new(base, do_send_echo_req, (void*)gsn);*/
    //send_echo_req = evtimer_new(base, do_send_create_new_session_req, (void*)gsn);

    //event_add(listener_event0, NULL);
    //event_add(listener_event1c, NULL);
    //evtimer_add(send_echo_req, &second_tv);

    event_base_dispatch(base);


    //check_version_match();

    //gtp_free(gsn);

    return 0;

}
