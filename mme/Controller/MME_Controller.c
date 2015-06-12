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


#define PACKET_MAX      8196


/* ======================================================================
 * TASK Prototypes
 * ====================================================================== */
static void TASK_MME_Controller___userAttach(Signal *signal);

static void TASK_MME_Controller___userDetach(Signal *signal);

static void TASK_MME_Controller___userHandover(Signal *signal);

void Controller_newAttach(struct t_engine_data *engine, struct SessionStruct_t *session);
void Controller_newDetach(struct t_engine_data *engine, struct SessionStruct_t *session);
void Controller_newHandover(struct t_engine_data *engine, struct SessionStruct_t *session);
int ctrlp_recv(int sockfd, struct sdn_packet *packet, size_t *len, struct sockaddr_in *peer, socklen_t *peerlen);
struct t_process *Controller_handler_create(struct t_engine_data *engine, struct t_process *owner);


/* ======================================================================
 * SDN Controller FSM State Implementations
 * ====================================================================== */
static int STATE_New_User_Attach(Signal *signal){

	INIT_TIME_MEASUREMENT_ENVIRONMENT
	log_msg(LOG_DEBUG, 0, "Entered the STATE_New_User_Attach");

	switch (signal->name){
	case Controller_new_user_attach:
		TASK_MME_Controller___userAttach(signal);
		log_msg(LOG_DEBUG, 0, "Controller: Sent new user attach message to SDN Controller - time = %u us", SELF_ON_SIG->procTime);
		break;
	default:
		log_msg(LOG_DEBUG, 0, "Controller: Received Signal is not implemented: %d", signal->name);
		break;
	}

	process_stop(PROC); /*Free process structure*/
	return 0;   /*free signal & send stored signals on process*/
}

static int STATE_Existing_User_Detach(Signal *signal){

	INIT_TIME_MEASUREMENT_ENVIRONMENT
	log_msg(LOG_DEBUG, 0, "Entered the STATE_Existing_User_Detach");

	switch (signal->name){
	case Controller_user_detach:
		TASK_MME_Controller___userDetach(signal);
		log_msg(LOG_DEBUG, 0, "Controller: Sent existing user detach message to SDN Controller - time = %u us", SELF_ON_SIG->procTime);
		break;
	default:
		log_msg(LOG_DEBUG, 0, "Controller: Received Signal is not implemented: %d", signal->name);
		break;
	}

	process_stop(PROC); /*Free process structure*/
	return 0;   /*free signal & send stored signals on process*/
}

static int STATE_Existing_User_Handover(Signal *signal){

	INIT_TIME_MEASUREMENT_ENVIRONMENT
	log_msg(LOG_DEBUG, 0, "Entered the STATE_Existing_User_Detach");

	switch (signal->name){
	case Controller_user_handover:
		TASK_MME_Controller___userHandover(signal);
		log_msg(LOG_DEBUG, 0, "Controller: Sent existing user detach message to SDN Controller - time = %u us", SELF_ON_SIG->procTime);
		break;
	default:
		log_msg(LOG_DEBUG, 0, "Controller: Received Signal is not implemented: %d", signal->name);
		break;
	}

	process_stop(PROC); /*Free process structure*/
	return 0;   /*free signal & send stored signals on process*/
}

static int STATE_Handle_Recv_Msg_C(Signal *signal){

	INIT_TIME_MEASUREMENT_ENVIRONMENT

	Signal *output;
	log_msg(LOG_DEBUG, 0, "Controller Enter STATE_Handle_Recv_Msg.");
	switch(signal->name){
	default:
		log_msg(LOG_DEBUG, 0, "Unknown message type received %d", signal->name);
		break;
	}

	process_stop(PROC); /*Free process structure*/
	return 0;   /*free signal & send stored signals on process*/
}


/* ======================================================================
 * SDN Controller API Implementation
 * ====================================================================== */

struct t_process *Controller_handler_create(struct t_engine_data *engine, struct t_process *owner)
{
	struct t_process *pSelf = (struct t_process *)NULL;  /* agent process */
	pSelf = process_create(engine, STATE_Handle_Recv_Msg_C, NULL, owner);
	return pSelf;
}

void Controller_newAttach(struct t_engine_data *engine, struct SessionStruct_t *session)
{
	Signal *output;
	struct t_process *proc;
	log_msg(LOG_DEBUG, 0, "Entered the Controller_newAttach method");

	/*Create a new process to manage the Controller state machine. The older session handler is stored as parent
	 * to return once the Controller state machine ends*/
	proc = process_create(engine, STATE_New_User_Attach, (void *)session, session->sessionHandler);

	output = new_signal(proc);
	output->name = Controller_new_user_attach;
	output->priority = MAXIMUM_PRIORITY;
	signal_send(output);

}

void Controller_newDetach(struct t_engine_data *engine, struct SessionStruct_t *session)
{
	Signal *output;
	struct t_process *proc;
	log_msg(LOG_DEBUG, 0, "Entered the Controller_newDetach method");

	/*Create a new process to manage the Controller state machine. The older session handler is stored as parent
	 * to return once the Controller state machine ends*/
	proc = process_create(engine, STATE_Existing_User_Detach, (void *)session, session->sessionHandler);

	output = new_signal(proc);
	output->name = Controller_user_detach;
	output->priority = MAXIMUM_PRIORITY;
	signal_send(output);

}

void Controller_newHandover(struct t_engine_data *engine, struct SessionStruct_t *session)
{
	Signal *output;
	struct t_process *proc;
	log_msg(LOG_DEBUG, 0, "Entered the Controller_newHandover method");

	/*Create a new process to manage the Controller state machine. The older session handler is stored as parent
	 * to return once the Controller state machine ends*/
	proc = process_create(engine, STATE_Existing_User_Handover, (void *)session, session->sessionHandler);

	output = new_signal(proc);
	output->name = Controller_user_handover;
	output->priority = MAXIMUM_PRIORITY;
	signal_send(output);

}

int ctrlp_recv(int sockfd, struct sdn_packet *packet, size_t *len, struct sockaddr_in *peer, socklen_t *peerlen)
{
	unsigned char buffer[PACKET_MAX];
	int status=0;
	struct sdn_packet *pack = NULL;
	*len = 0;

	*peerlen = sizeof(struct sockaddr_in);
	if ((status = recvfrom(sockfd, buffer, sizeof(buffer), 0,
			(struct sockaddr *) peer, peerlen)) < 0 ) {
		if (errno == EAGAIN)
			return 0;
		else return -1;
	}

	pack = (struct sdn_packet *) (buffer);
	/*
	 * Do some validation with the ctrl packet.
	 * 1. Check that the flags are proper. - No error flag
	 * 2. Some other status flags as per the requirement.
	 */
	memcpy((void *)packet, (void *)buffer, status);
	*len=status;
	return 0;
}


/* ======================================================================
 * TASK Implementations
 * ====================================================================== */
static void TASK_MME_Controller___userAttach(Signal *signal){

	/* This function is used to send the message to the SDN controller that a new user has
	 * attached to the eNB. For this, it creates a structure of type ctrl_message
	 * which contains the IP of the SGW, GTP tunnel Ids etc.
	 */

	struct sdn_packet packet = {0};
	struct user_ctx_t *user;
	struct sockaddr_in  *peer;
	uint32_t tunnelId;
	int sock;
	size_t peerlen;
	uint32_t length;
	struct nodeinfo_t ctrlInfo;
	uint8_t packet_str[PACKET_MAX], tmp_str[PACKET_MAX];
	uint8_t straddr[INET6_ADDRSTRLEN];
	struct in_addr ipv4enb, ipv4gw, ipv4addr;
    uint8_t ip_enb[INET_ADDRSTRLEN], ip_sgw[INET_ADDRSTRLEN];

	const uint8_t *packet_pattern = "{"
		"\"version\": \"%d\","
		"\"msg_type\": \"attach\","
		"\"msg\": {"
			"\"plmn\": \"%u%.2u\","
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

	user = PDATA->user_ctx;
	getNode(&ctrlInfo, CTRL, NULL);

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
		log_msg(LOG_DEBUG, 0, "UE: Both IPv4 & IPv6 addresses are present. Not Implemented");
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
			inet_ntop(AF_INET, &ipv4gw, ip_sgw, INET_ADDRSTRLEN),								/*SPGW endpoint IP Address*/
			ntohl(user->ebearer[0].s1u_sgw.teid),			/*UL TEID*/
			inet_ntop(AF_INET, &ipv4enb, ip_enb, INET_ADDRSTRLEN),							/*eNB endpoint IP Address*/
			ntohl(user->ebearer[0].s1u_eNB.teid),			/*DL TEID*/
			user->ebearer[0].qos.qci						/*QCI*/
			);


	log_msg(LOG_DEBUG, 0, "Uplink   Tunnel Id: %"PRIu32,ntohl(packet.uL_gtp_teid));
	log_msg(LOG_DEBUG, 0, "Downlink Tunnel Id: %"PRIu32,ntohl(packet.dL_gtp_teid));



	/* ======================================================================
	 * Sending packet
	 * ====================================================================== */

	sock = SELF_ON_SIG->ctrl.fd;
	peer = (struct sockaddr_in *)&(SELF_ON_SIG->ctrl.peerAddr);
	peer->sin_family = AF_INET;
	peer->sin_port = htons(CONTROLLER_PORT);
	peer->sin_addr.s_addr = ctrlInfo.addrv4.s_addr;
	peerlen = sizeof(struct sockaddr_in);
	length = strlen(packet_str);
	if (sendto(sock, &packet_str, length, 0, (struct sockaddr *)peer, peerlen) < 0) {
		log_errpack(LOG_ERR, errno, (struct sockaddr_in *)peer, &packet, length,
				"Sendto(fd=%d, msg=%lx, len=%d) failed", sock, (unsigned long) &packet, length);
		return;
	}
	log_msg(LOG_DEBUG, 0, "Sent the new user attach message to SDN Controller");
	/*print_packet(&packet, length);*/

}

static void TASK_MME_Controller___userDetach(Signal *signal){
	/*
	 * This function is used to send the message to the SDN controller that an existing user has
	 * detached from the eNB. For this, it creates a structure of type ctrl_message
	 * which contains the IP of the SGW, GTP tunnel Ids etc. */

	struct sdn_packet packet = {0};
	struct user_ctx_t *user;
	struct sockaddr_in  *peer;
	uint32_t tunnelId;
	int sock;
	size_t peerlen;
	uint32_t length;
	struct nodeinfo_t ctrlInfo;
	uint8_t packet_str[PACKET_MAX], tmp_str[PACKET_MAX];
	uint8_t straddr[INET6_ADDRSTRLEN];
	struct in_addr ipv4enb, ipv4gw, ipv4addr;
    uint8_t ip_enb[INET_ADDRSTRLEN], ip_sgw[INET_ADDRSTRLEN];

	const uint8_t *packet_pattern = "{"
		"\"version\": \"%d\","
		"\"msg_type\": \"detach\","
		"\"msg\": {"
			"\"plmn\": \"%u%.2u\","
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

	user = PDATA->user_ctx;
	getNode(&ctrlInfo, CTRL, NULL);

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


	log_msg(LOG_DEBUG, 0, "Uplink   Tunnel Id: %"PRIu32,ntohl(packet.uL_gtp_teid));
	log_msg(LOG_DEBUG, 0, "Downlink Tunnel Id: %"PRIu32,ntohl(packet.dL_gtp_teid));


	/* ======================================================================
	 * Sending packet
	 * ====================================================================== */

	sock = SELF_ON_SIG->ctrl.fd;
	peer = (struct sockaddr_in *)&(SELF_ON_SIG->ctrl.peerAddr);
	peer->sin_family = AF_INET;
	peer->sin_port = htons(CONTROLLER_PORT);
	peer->sin_addr.s_addr = ctrlInfo.addrv4.s_addr;
	peerlen = sizeof(struct sockaddr_in);
	length = strlen(packet_str);
	if (sendto(sock, &packet_str, length, 0, (struct sockaddr *)peer, peerlen) < 0) {
		log_errpack(LOG_ERR, errno, (struct sockaddr_in *)peer, &packet_str, length,
				"Sendto(fd=%d, msg=%lx, len=%d) failed", sock, (unsigned long) &packet, length);
		return;
	}
	log_msg(LOG_DEBUG, 0, "Sent the user detach message to SDN Controller");
	/*print_packet(&packet, length);*/
}

/* TODO: Get the correct GTP-TEIDs */
static void TASK_MME_Controller___userHandover(Signal *signal){
	/*
	 * This function is used to send two messages to the SDN controller that an existing user has
	 * moved from one eNB to another eNB. For this, it creates a structure of type ctrl_message
	 * which contains the IP of the SGW, GTP tunnel Ids etc. */


	struct user_ctx_t *user;
	struct sockaddr_in  *peer;
	uint32_t tunnelId;
	int sock;
	size_t peerlen;
	uint32_t length;
	struct nodeinfo_t ctrlInfo;
	uint8_t packet_str[PACKET_MAX], tmp_str[PACKET_MAX];
	uint8_t straddr[INET6_ADDRSTRLEN];
	struct in_addr ipv4enb, ipv4enb_tgt, ipv4gw, ipv4addr;
    uint8_t ip_enb[INET_ADDRSTRLEN], ip_tgtenb[INET_ADDRSTRLEN], ip_sgw[INET_ADDRSTRLEN];

	const uint8_t *packet_pattern = "{"
		"\"version\": \"%d\","
		"\"msg_type\": \"handover\","
		"\"msg\": {"
			"\"plmn\": \"%u%.2u\","
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

    user = PDATA->user_ctx;

    getNode(&ctrlInfo, CTRL, PDATA->user_ctx);

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


    //      /* 4. Target SGW ip address */
	//      packet.tgt_s_sgw.ipv4 = user->ebearer[0].s1u_sgw.addr.addrv4;
	//      /* 5. GTP Tunnel Endpoint ID at eNB */
	//      packet.uL_gtp_teid = user->ebearer[0].s1u_sgw.teid;
	//      /* 6. GTP Tunnel Endpoint ID at PGW */
	//      packet.dL_gtp_teid = user->ebearer[0].s5s8u.teid;
	//      /* 7. IP address of the eNB to which the UE is connected */
	//      packet.eNB.ipv4 = user->eNBAddr4.s_addr;
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

	sock = SELF_ON_SIG->ctrl.fd;
	peer = (struct sockaddr_in *)&(SELF_ON_SIG->ctrl.peerAddr);
	peer->sin_family = AF_INET;
	peer->sin_port = htons(CONTROLLER_PORT);
	peer->sin_addr.s_addr = ctrlInfo.addrv4.s_addr;
	peerlen = sizeof(struct sockaddr_in);
	length = strlen(packet_str);
	if (sendto(sock, &packet_str, length, 0, (struct sockaddr *)peer, peerlen) < 0) {
		log_errpack(LOG_ERR, errno, (struct sockaddr_in *)peer, &packet_str, length,
				"Sendto(fd=%d, msg=%lx, len=%d) failed", sock, (unsigned long) &packet_str, length);
		return;
	}
	log_msg(LOG_DEBUG, 0, "Sent the user detach message to SDN Controller");
	/*print_packet(&packet, length);*/
}
