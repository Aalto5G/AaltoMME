/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and funded by EIT ICT labs.
 */

/**
 * @file   MME_Controller.c
 * @Author Robin Babujee Jerome
 * @date November 2013
 * @brief MME Controller interface protocol state machine.
 *
 * This module implements the SDN Controller interface state machine. The states are separated on STATE_* and TASK_MME_Controller___*
 * The firsts ones implements the flow control and the second ones the processing
 */

#include <stdio.h>
#include <stdlib.h>
#include "signals.h"
#include <syslog.h>
#include <errno.h>
#include "MME_engine.h"
#include "logmgr.h"
#include "MME_Controller.h"
#include "MME.h"
#include "nodemgr.h"
#include "storagesys.h"

#define CONTROLLER_PORT 12345
#define PACKET_MAX      8196

typedef struct{
	gpointer        mme;
	int             fd;
	struct sockaddr addr;
	size_t          addrlen;
}SDNCtrl_t;

void ctrl_accept(evutil_socket_t listener, short event, void *arg){

    uint32_t teid;

    SDNCtrl_t *self = (SDNCtrl_t *)arg;

    log_msg(LOG_DEBUG, 0, "New SDN Controller message recv");
}

gpointer sdnCtrl_init(gpointer mme){
	struct nodeinfo_t ctrlInfo;
	struct sockaddr_in *peer;

	SDNCtrl_t *self = g_new0(SDNCtrl_t, 1);

	self->mme = mme;
	self->fd =init_udp_srv(mme_getLocalAddress(self->mme), CONTROLLER_PORT);
    log_msg(LOG_INFO, 0, "Open SDN Controller server on file descriptor %d, port %d", self->fd, CONTROLLER_PORT);
	getNode(&ctrlInfo, CTRL, NULL);

	peer = (struct sockaddr_in *)&(self->addr);
	peer->sin_family = AF_INET;
	peer->sin_port = htons(CONTROLLER_PORT);
	peer->sin_addr.s_addr = ctrlInfo.addrv4.s_addr;
	self->addrlen = sizeof(struct sockaddr_in);

	/*mme_registerRead(self->mme, self->fd, ctrl_accept, self);*/

	return self;
}

void sdnCtrl_free(gpointer ctrl_h){
	SDNCtrl_t *self = (SDNCtrl_t*)ctrl_h;
	close(self->fd);
	g_free(self);
}

static void sdnCtrl_send(SDNCtrl_t *self, char* msg){
	size_t len = strlen(msg);
	if (sendto(self->fd, msg, len, 0,
	           &(self->addr), self->addrlen) < 0) {
		log_errpack(LOG_ERR, errno, (struct sockaddr_in *)&(self->addr),
		            msg, len,
		            "Sendto(fd=%d, msg=%s) failed",
		            self->fd, msg);
	}
}


/* ======================================================================
 * TASK Implementations
 * ====================================================================== */
static void TASK_MME_Controller___userAttach(SDNCtrl_t* self, struct user_ctx_t *user){

	/* This function is used to send the message to the SDN controller that a new user has
	 * attached to the eNB. For this, it creates a structure of type ctrl_message
	 * which contains the IP of the SGW, GTP tunnel Ids etc.
	 */

	uint32_t tunnelId;
	uint8_t packet_str[PACKET_MAX], tmp_str[PACKET_MAX];
	uint8_t straddr[INET6_ADDRSTRLEN];
	struct in_addr ipv4enb, ipv4gw, ipv4addr;
    uint8_t ip_enb[INET_ADDRSTRLEN], ip_sgw[INET_ADDRSTRLEN];

	const uint8_t *packet_pattern = "{"
		"\"version\": \"%d\","
		"\"msg_type\": \"attach\","
		"\"msg\": {"
			"\"plmn\": \"%.3llu%.2llu\","
			"\"ue_msisdn\": \"%llu\","
			"%s"
			/*"\"ue_ipv4addr\": \"10.10.255.254\","*/
			/*"\"ue_ipv6addr\": \"2001:0db8:85a3:0042:1000:8a2e:0370:7334\","*/
			"\"gtp_spgw_ipv4addr\": \"%s\","
			"\"gtp_teid_ul\": \"%u\","
			"\"gtp_enb_ipv4addr\": \"%s\","
			"\"gtp_teid_dl\": \"%u\","
		    "\"gtp_qci\": \"%u\""
		"},"
		"\"notes\": \"Attach of an UE to the network\"}";

	log_msg(LOG_DEBUG, 0, "Entered the task for sending user Attach to SDN Controller");

	/* ======================================================================
	 * Setting values in the packet
	 * ====================================================================== */


	/* UE IP - IP address of the User Equipment. This is fetched from the PAA (PDN Allocated Address) */
	switch(user->pAA.type){
	case  1: /* IPv4*/
		ipv4addr.s_addr = user->pAA.addr.ipv4;
		sprintf(tmp_str, "\"ue_ipv4addr\": \"%s\",", inet_ntoa(ipv4addr));
		log_msg(LOG_DEBUG, 0, "UE: Only IPv4 address is present %s", inet_ntoa(ipv4addr));
		break;
	case 2: /* IPv6*/
		inet_ntop(AF_INET6, &user->pAA.addr.ipv6, straddr, sizeof(straddr));
		sprintf(tmp_str, "\"ue_ipv6addr\": \"%s\",", straddr);
		log_msg(LOG_DEBUG, 0, "UE: Only IPv6 address is present");
		break;
	case 3: /*IPv4v6*/
		/*sprintf(tmp_str, "\"ue_ipv4addr\": \"%s\",\"ue_ipv6addr\": \"%s\",", inet_ntoa(user->pAA.addr.ipv4), inet_ntoa(user->pAA.addr.ipv6));*/
		log_msg(LOG_DEBUG, 0, "UE: Both IPv4 & IPv6 addresses are present. Not Implemented");
		break;
	}

	ipv4enb.s_addr = user->ebearer[0].s1u_eNB.addr.addrv4;
	ipv4gw.s_addr = user->ebearer[0].s1u_sgw.addr.addrv4;

	snprintf(packet_str, PACKET_MAX, packet_pattern,
	         1,												/* Version*/
	         user->imsi/1000000000000,						/*MCC*/
	         (user->imsi/10000000000)%100,					/*MNC*/
	         user->msisdn,									/*MSISDN*/
	         tmp_str,
	         inet_ntop(AF_INET, &ipv4gw, ip_sgw, INET_ADDRSTRLEN),								/*SPGW endpoint IP Address*/
	         ntohl(user->ebearer[0].s1u_sgw.teid),			/*UL TEID*/
	         inet_ntop(AF_INET, &ipv4enb, ip_enb, INET_ADDRSTRLEN),							/*eNB endpoint IP Address*/
	         ntohl(user->ebearer[0].s1u_eNB.teid),			/*DL TEID*/
	         user->ebearer[0].qos.qci						/*QCI*/
	         );

	/* log_msg(LOG_DEBUG, 0, "Uplink   Tunnel Id: %"PRIu32,ntohl(packet.uL_gtp_teid)); */
	/* log_msg(LOG_DEBUG, 0, "Downlink Tunnel Id: %"PRIu32,ntohl(packet.dL_gtp_teid)); */

	/* ======================================================================
	 * Sending packet
	 * ====================================================================== */
	sdnCtrl_send(self, packet_str);
	log_msg(LOG_DEBUG, 0, "Sent the new user attach message to SDN Controller");
}

static void TASK_MME_Controller___userDetach(SDNCtrl_t* self, struct user_ctx_t *user){
	/*
	 * This function is used to send the message to the SDN controller that an existing user has
	 * detached from the eNB. For this, it creates a structure of type ctrl_message
	 * which contains the IP of the SGW, GTP tunnel Ids etc. */

	uint32_t tunnelId;
	uint8_t packet_str[PACKET_MAX], tmp_str[PACKET_MAX];
	uint8_t straddr[INET6_ADDRSTRLEN];
	struct in_addr ipv4enb, ipv4gw, ipv4addr;
    uint8_t ip_enb[INET_ADDRSTRLEN], ip_sgw[INET_ADDRSTRLEN];

	const uint8_t *packet_pattern = "{"
		"\"version\": \"%d\","
		"\"msg_type\": \"detach\","
		"\"msg\": {"
    		"\"plmn\": \"%.3llu%.2llu\","
			"\"ue_msisdn\": \"%llu\","
			"%s"
			/*"\"ue_ipv4addr\": \"10.10.255.254\","*/
			/*"\"ue_ipv6addr\": \"2001:0db8:85a3:0042:1000:8a2e:0370:7334\","*/
			"\"gtp_spgw_ipv4addr\": \"%s\","
			"\"gtp_teid_ul\": \"%u\","
			"\"gtp_enb_ipv4addr\": \"%s\","
			"\"gtp_teid_dl\": \"%u\","
			"\"gtp_qci\": \"%u\""
			"},"
		"\"notes\": \"Detach of an UE from the network\"}";

	log_msg(LOG_DEBUG, 0, "Entered the task for sending user Detach to SDN Controller");

	/* ======================================================================
	 * Setting values in the packet
	 * ====================================================================== */

	/* UE IP - IP address of the User Equipment. This is fetched from the PAA (PDN Allocated Address) */
	switch(user->pAA.type){
	case  1: /* IPv4*/
		ipv4addr.s_addr = user->pAA.addr.ipv4;
		sprintf(tmp_str, "\"ue_ipv4addr\": \"%s\",", inet_ntoa(ipv4addr));
		log_msg(LOG_DEBUG, 0, "UE: Only IPv4 address is present %d",user->pAA.addr.ipv4);
		break;
	case 2: /* IPv6*/
		inet_ntop(AF_INET6, &user->pAA.addr.ipv6, straddr, sizeof(straddr));
		sprintf(tmp_str, "\"ue_ipv6addr\": \"%s\",", straddr);
		log_msg(LOG_DEBUG, 0, "UE: Only IPv6 address is present");
		break;
	case 3: /*IPv4v6*/
		/*sprintf(tmp_str, "\"ue_ipv4addr\": \"%s\",\"ue_ipv6addr\": \"%s\",", inet_ntoa(user->pAA.addr.ipv4), inet_ntoa(user->pAA.addr.ipv6));*/
		log_msg(LOG_DEBUG, 0, "UE: Both Ipv4 & IPv6 addresses are present. Not Implemented");
		break;
	}

	ipv4enb.s_addr = user->ebearer[0].s1u_eNB.addr.addrv4;
	ipv4gw.s_addr = user->ebearer[0].s1u_sgw.addr.addrv4;

	sprintf(packet_str, packet_pattern,
			1,												/* Version*/
			user->imsi/1000000000000,						/*MCC*/
			(user->imsi/10000000000)%100,					/*MNC*/
			user->msisdn,									/*MSISDN*/
			tmp_str,
			inet_ntop(AF_INET, &ipv4gw, ip_sgw, INET_ADDRSTRLEN),							/*SPGW endpoint IP Address*/
			ntohl(user->ebearer[0].s1u_sgw.teid),			/*UL TEID*/
			inet_ntop(AF_INET, &ipv4enb, ip_enb, INET_ADDRSTRLEN),							/*eNB endpoint IP Address*/
			ntohl(user->ebearer[0].s1u_eNB.teid),			/*DL TEID*/
			user->ebearer[0].qos.qci						/*QCI*/
			);


	/* ======================================================================
	 * Sending packet
	 * ====================================================================== */
	sdnCtrl_send(self, packet_str);
	log_msg(LOG_DEBUG, 0, "Sent the user detach message to SDN Controller");
}

static void TASK_MME_Controller___userHandover(SDNCtrl_t* self, struct user_ctx_t *user){
	/*
	 * This function is used to send two messages to the SDN controller that an existing user has
	 * moved from one eNB to another eNB. For this, it creates a structure of type ctrl_message
	 * which contains the IP of the SGW, GTP tunnel Ids etc. */


	uint32_t tunnelId;
	uint8_t packet_str[PACKET_MAX], tmp_str[PACKET_MAX];
	uint8_t straddr[INET6_ADDRSTRLEN];
	struct in_addr ipv4enb, ipv4enb_tgt, ipv4gw, ipv4addr;
    uint8_t ip_enb[INET_ADDRSTRLEN], ip_tgtenb[INET_ADDRSTRLEN], ip_sgw[INET_ADDRSTRLEN];

	const uint8_t *packet_pattern = "{"
		"\"version\": \"%d\","
		"\"msg_type\": \"handover\","
		"\"msg\": {"
			"\"plmn\": \"%.3llu%.2llu\","
			"\"ue_msisdn\": \"%llu\","
			"%s"
			/*"\"ue_ipv4addr\": \"10.10.255.254\","*/
			/*"\"ue_ipv6addr\": \"2001:0db8:85a3:0042:1000:8a2e:0370:7334\","*/
			"\"gtp_spgw_ipv4addr\": \"%s\","
			"\"gtp_teid_ul\": \"%u\","
			"\"gtp_src_enb_ipv4addr\": \"%s\","
			"\"gtp_src_teid_dl\": \"%u\","
			"\"gtp_tgt_enb_ipv4addr\": \"%s\","
			"\"gtp_tgt_teid_dl\": \"%u\","
			"\"gtp_qci\": \"%u\""
			"},"
		"\"notes\": \"Handover of an UE\"}";

    log_msg(LOG_DEBUG, 0, "Entered the task for sending user Handover to SDN Controller");

    /* ======================================================================
	 * Setting values in the packet
	 * ====================================================================== */

	/* UE IP - IP address of the User Equipment. This is fetched from the PAA (PDN Allocated Address) */
	switch(user->pAA.type){
	case  1: /* IPv4*/
		ipv4addr.s_addr = user->pAA.addr.ipv4;
		sprintf(tmp_str, "\"ue_ipv4addr\": \"%s\",", inet_ntoa(ipv4addr));
		log_msg(LOG_DEBUG, 0, "UE: Only IPv4 address is present %d",user->pAA.addr.ipv4);
		break;
	case 2: /* IPv6*/
		inet_ntop(AF_INET6, &user->pAA.addr.ipv6, straddr, sizeof(straddr));
		sprintf(tmp_str, "\"ue_ipv6addr\": \"%s\",", straddr);
		log_msg(LOG_DEBUG, 0, "UE: Only IPv6 address is present");
		break;
	case 3: /*IPv4v6*/
		/*sprintf(tmp_str, "\"ue_ipv4addr\": \"%s\",\"ue_ipv6addr\": \"%s\",", inet_ntoa(user->pAA.addr.ipv4), inet_ntoa(user->pAA.addr.ipv6));*/
		log_msg(LOG_DEBUG, 0, "UE: Both Ipv4 & IPv6 addresses are present. Not Implemented");
		break;
	}


    ipv4enb_tgt.s_addr = user->ebearer[0].s1u_eNB.addr.addrv4;
    ipv4enb.s_addr = user->hoCtx.old_ebearers[0].s1u_eNB.addr.addrv4;
	ipv4gw.s_addr = user->ebearer[0].s1u_sgw.addr.addrv4;

	sprintf(packet_str, packet_pattern,
			1,												              /* Version*/
			user->imsi/1000000000000,						              /*MCC*/
			(user->imsi/10000000000)%100,					              /*MNC*/
			user->msisdn,									              /*MSISDN*/
			tmp_str,                                                      /*UE IP*/
			inet_ntop(AF_INET, &ipv4gw, ip_sgw, INET_ADDRSTRLEN),	      /*SPGW endpoint IP Address*/
			ntohl(user->ebearer[0].s1u_sgw.teid),			              /*UL TEID*/
			inet_ntop(AF_INET, &ipv4enb, ip_enb, INET_ADDRSTRLEN),	      /*SRC eNB endpoint IP Address*/
			ntohl(user->hoCtx.old_ebearers[0].s1u_eNB.teid),			  /*SRC DL TEID*/
            inet_ntop(AF_INET, &ipv4enb_tgt, ip_tgtenb, INET_ADDRSTRLEN), /*TGT eNB endpoint IP Address*/
			ntohl(user->ebearer[0].s1u_eNB.teid),			              /*TGT DL TEID*/
			user->ebearer[0].qos.qci						              /*QCI*/
			);


	/* ======================================================================
	 * Sending packet
	 * ====================================================================== */
	sdnCtrl_send(self, packet_str);
	log_msg(LOG_DEBUG, 0, "Sent the user detach message to SDN Controller");
}

/* ======================================================================
 * SDN Controller API Implementation
 * ====================================================================== */

void Controller_newAttach(gpointer ctrl, struct user_ctx_t *user){
	SDNCtrl_t* self = (SDNCtrl_t*)ctrl;
	log_msg(LOG_DEBUG, 0, "Entered the Controller_newAttach method");

	TASK_MME_Controller___userAttach(self, user);
}

void Controller_newDetach(gpointer ctrl, struct user_ctx_t *user){
	SDNCtrl_t* self = (SDNCtrl_t*)ctrl;
	log_msg(LOG_DEBUG, 0, "Entered the Controller_newDetach method");
	TASK_MME_Controller___userDetach(self, user);
}

void Controller_newHandover(gpointer ctrl, struct user_ctx_t *user){
	SDNCtrl_t* self = (SDNCtrl_t*)ctrl;
	log_msg(LOG_DEBUG, 0, "Entered the Controller_newHandover method");
	TASK_MME_Controller___userHandover(self, user);
}
