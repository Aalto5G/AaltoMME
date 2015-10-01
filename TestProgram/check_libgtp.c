/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**@file   check_libgtp.c
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  Definition of Check tests
 *
 * Definition and implementation of test cases using check testing framework */

#include "check_libgtp.h"

#include "gtp.h"
#include "gtpie.h"
#include "MME_engine.h"
#include "storagesys.h"

#include <pcap/pcap.h>

/* strcmp*/
#include <string.h>

/* event_get_version function*/
#include <event2/event.h>


#define GTPIE_DEBUG 1

/** Structure used in test_echo_msg_pcap callback to pass the read message*/
struct packet_test{
    struct pcap_pkthdr    h;
    u_char                *p;
};


void test_echo_msg_pcap_cb(u_char *args, const struct pcap_pkthdr* pkthdr, const u_char* packet)
{
    struct packet_test *pack = (struct packet_test*) args;
    pack->h=(*pkthdr);
    pack->p=packet;
    /*fprintf(stdout, "Callback:\n ");
    print_packet(packet, pkthdr->len);
    fflush(stdout);*/
}

/*Dummy test checking the lib version*/
START_TEST (test_gtplib_version)
{
    const char *version = gtp_version();
    fail_unless(strcmp( version, "1.0")==0, "Incorrect gtplib Version: %s", version);
}
END_TEST

START_TEST (test_libevent_version)
{
    const char *version = event_get_version();
    fail_unless(strcmp( version, "2.0.21-stable")==0, "Incorrect libevent Version: %s", version);
}
END_TEST

/* Test ntoh hton 3 bytes */
START_TEST (test_ntoh24_hton24)
{
    uint32_t hw = 0x123456;
    fail_unless(hton24(hw) == 0x0563412, "hardware to network conversion fail, 0x%x, 0x%x", hw , hton24(hw));
    fail_unless(ntoh24(hton24(hw)) == 0x123456, "network to hardware conversion fail, 0x123456, 0x%x", ntoh24(hton24(hw)));
}
END_TEST

START_TEST (create_ctx_msg)
{
    struct in_addr listen_, destaddr;
    union gtp_packet packet;
    struct fteid_t  fteid;
    struct qos_t    qos;
    size_t len;
    struct sockaddr_in  src_addr;
    int addrlen = sizeof(struct sockaddr_in);
    union gtpie_member ie[GTPIE_SIZE], ie_bearer_ctx[GTPIE_SIZE];
    int hlen;
    uint32_t length, ielen;
    uint8_t b[30];

    listen_.s_addr=INADDR_ANY;

    inet_aton("10.11.0.142", &destaddr);
    //inet_aton("127.0.0.1", &destaddr);

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
    fteid.teid = hton32(1);
    inet_pton(AF_INET, "130.233.152.45", &(fteid.addr.addrv4));
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
    b[0]=0x01;  /*PDN Type  IPv4 */
    b[1]=0x0;   /*IP = 0.0.0.0*/
    b[2]=0x0;
    b[3]=0x0;
    b[4]=0x0;
    memcpy(ie[7].tliv.v, b, 5);
    /*Serving Network*/
    ie[8].tliv.i=0;
    ie[8].tliv.l=hton16(3);
    ie[8].tliv.t=GTPV2C_IE_SERVING_NETWORK;
    b[0]=0x01; /* 0000.... MCC digit 2 ....0000 MCC digit 1*/
    b[1]=0x00; /* 0000.... MNC digit 3 ....0000 MCC digit 3*/
    b[2]=0x01; /* 0000.... MNC digit 2 ....0000 MNC digit 1*/
    memcpy(ie[8].tliv.v, b, 3);
    /*PDN type*/
    ie[9].tliv.i=0;
    ie[9].tliv.l=hton16(1);
    ie[9].tliv.t=GTPV2C_IE_PDN_TYPE;
    b[0]=0x01; /* PDN type IPv4*/
    memcpy(ie[9].tliv.v, b, 1);
    /*APN restriction*/
    ie[10].tliv.i=0;
    ie[10].tliv.l=hton16(1);
    ie[10].tliv.t=GTPV2C_IE_APN_RESTRICTION;
    b[0]=0x01; /* PDN type IPv4*/
    memcpy(ie[10].tliv.v, b, 1);
    /*Selection Mode*/
    ie[11].tliv.i=0;
    ie[11].tliv.l=hton16(1);
    ie[11].tliv.t=GTPV2C_IE_SELECTION_MODE;
    b[0]=0x01; /* PDN type IPv4*/
    memcpy(ie[11].tliv.v, b, 1);
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
        b[0]=0x01; /**/
        b[1]=0x01;
        b[2]=0x01;
        memcpy(ie_bearer_ctx[1].tliv.v, b, 3);
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
    //printf("packet length %d\n",length );
    //print_packet(&packet, length);
}
END_TEST

/*Dummy test checking the lib version*/
START_TEST (test_tbcd)
{
	uint8_t tbcd[10], buf[8], testbuf[8];
	uint64_t dec, imei = 123456789012345;
	uint32_t bcdlen, n, pos;

	/*Decimal to TBCD conversion check*/
	dec2tbcd(tbcd, &bcdlen, imei);
	ck_assert_msg((bcdlen==8), "bcdlen not correct %d, expected 8", bcdlen);
	ck_assert_msg(*(uint64_t*)(tbcd)==0xf543210987654321,"conversion to TBCD not correct, tbcd =0x%16llx != 0xf543210987654321", *(uint64_t*)(tbcd));

	/*TBCD to decimal conversion check*/
	tbcd2dec(&dec, tbcd, bcdlen);
	ck_assert_msg(dec == imei, "TBCD to decimal concersion incorrect,  %lld != %lld", dec, imei);
}
END_TEST

Suite *
gtplib_suite (void)
{
    Suite *s = suite_create ("gtplib_Messages");

    /* Version test case */
    TCase *tc_version = tcase_create ("version_check");
    tcase_add_test (tc_version, test_gtplib_version);
    //tcase_add_test (tc_version, test_libevent_version);
    suite_add_tcase (s, tc_version);

    /* Echo Procedures test case */
    TCase *tc_hton24 = tcase_create ("Tool functions");
    tcase_add_test (tc_hton24, test_ntoh24_hton24);
    /*tcase_add_test (tc_echo, test_echo_msg_pcap);   *//*fail due to permissions*/
    suite_add_tcase (s, tc_hton24);

    /* Hash table, storage manager test case */
    TCase *tc_storage = tcase_create ("storage_test");
    tcase_add_test (tc_storage, test_hash_table_imsi);
    tcase_add_test (tc_storage, test_hash_table_teid);
    /*tcase_add_test (tc_storage, test_stress);*/
    tcase_set_timeout (tc_storage, 0);
    suite_add_tcase (s, tc_storage);

    /* GTPv2 messages test case */
    TCase *tc_gtpv2_msg = tcase_create ("msg_test");
    tcase_add_test (tc_gtpv2_msg, create_ctx_msg);
    suite_add_tcase (s, tc_gtpv2_msg);

    /* Unit testing  */
    TCase *tc_unit = tcase_create ("tbcd conversions");
    tcase_add_test (tc_unit, test_tbcd);
    suite_add_tcase (s, tc_unit);

    return s;
}
