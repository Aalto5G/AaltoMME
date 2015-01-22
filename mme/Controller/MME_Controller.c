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

static int STATE_Handle_Recv_Msg(Signal *signal){

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
	pSelf = process_create(engine, STATE_Handle_Recv_Msg, NULL, owner);
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
	struct in_addr ipv4addr, ipv4gw;

	const uint8_t *packet_pattern = "{"
	  "\"version\": \"%d\","
	  "\"msg_type\": \"attach\","
	  "\"msg\": {"
	    "\"plmn\": \"%u%.2u\","
	    "\"ue_msisdn\": \"%u\","
	    "%s"
	    /*"\"ue_ipv4addr\": \"10.10.255.254\","*/
	    /*"\"ue_ipv6addr\": \"2001:0db8:85a3:0042:1000:8a2e:0370:7334\","*/
	    "\"gtp_spgw_ipv4addr\": \"%s\","
	    "\"gtp_teid_ul\": \"%u\","
	    "\"gtp_enb_ipv4addr\": \"%s\","
	    "\"gtp_teid_dl\": \"%u\","
	    "\"gtp_qci\": \"%u\""
	    "},"
	  "\"notes\": \"Attach of a UE to the network\"}"

	  ;

	log_msg(LOG_DEBUG, 0, "Entered the task for sending user Attach to SDN Controller");

	user = PDATA->user_ctx;
	getNode(&ctrlInfo, CTRL, NULL);

	/* ======================================================================
	 * Setting values in the packet
	 * ====================================================================== */


	/* 6. UE IP - IP address of the User Equipment. This is fetched from the PAA (PDN Allocated Address) */
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

	ipv4addr.s_addr = user->ebearer[0].s1u_eNB.addr.addrv4;
	ipv4gw.s_addr = user->ebearer[0].s1u_sgw.addr.addrv4;

	sprintf(packet_str, packet_pattern,
			1,												/* Version*/
			user->imsi/1000000000000,						/*MCC*/
			(user->imsi/10000000000)%100,					/*MNC*/
			user->msisdn,									/*MSISDN*/
			tmp_str,
			inet_ntoa(ipv4gw),								/*SPGW endpoint IP Address*/
			user->ebearer[0].s1u_sgw.teid,					/*UL TEID*/
			inet_ntoa(ipv4addr),							/*eNB endpoint IP Address*/
			user->ebearer[0].s1u_eNB.teid,					/*DL TEID*/
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

	log_msg(LOG_DEBUG, 0, "Entered the task for sending user Detach to SDN Controller");

	user = PDATA->user_ctx;
	getNode(&ctrlInfo, CTRL, NULL);

	/* ======================================================================
	 * Setting values in the packet
	 * ====================================================================== */

	/* 1.Setting the version */
	packet.version = 1;
	/* 2.Setting the operation Attach (0x1) */
	packet.operation = 2;
	/* 3. MSISDN field can be fetched from user context */
	packet.msisdn = hton64(user->msisdn);
	/* 4. GTP Tunnel Endpoint ID at eNB */
	packet.uL_gtp_teid = user->ebearer[0].s1u_sgw.teid;
	/* 5. GTP Tunnel Endpoint ID at PGW */
	packet.dL_gtp_teid = user->ebearer[0].s1u_eNB.teid;

	log_msg(LOG_DEBUG, 0, "Uplink   Tunnel Id: %"PRIu32,ntohl(packet.uL_gtp_teid));
	log_msg(LOG_DEBUG, 0, "Downlink Tunnel Id: %"PRIu32,ntohl(packet.dL_gtp_teid));


	/* 6. UE IP - IP address of the User Equipment. This is fetched from the PAA (PDN Allocated Address) */
	packet.UE.spare = 0x0;
	switch(user->pAA.type){
	case  1: /* IPv4*/
		packet.UE.ipv4 = 0x1;
		packet.eNB.ipv6 = 0x0;
		packet.UE.ipAddr.addr_v4 = user->pAA.addr.ipv4;
		log_msg(LOG_DEBUG, 0, "UE: Only IPv4 address is present");
		break;
	case 2: /* IPv6*/
		packet.UE.ipv4 = 0x0;
		packet.eNB.ipv6 = 0x1;
		memcpy(&packet.UE.ipAddr.addr_v6, &user->pAA.addr.ipv6, sizeof user->pAA.addr.ipv6);
		log_msg(LOG_DEBUG, 0, "UE: Only IPv6 address is present");
		break;
	case 3: /*IPv4v6*/
		packet.UE.ipv4 = 0x1;
		packet.eNB.ipv6 = 0x1;
		memcpy(&packet.UE.ipAddr.both, &user->pAA.addr.both, sizeof user->pAA.addr.both);
		log_msg(LOG_DEBUG, 0, "UE: Both Ipv4 & IPv6 addresses are present");
		break;
	}
	/* 7. eNB IP - IP address of the eNB to which the UE is connected  */
	packet.eNB.spare = 0x0;
	packet.eNB.ipv4 = 0x1;
	packet.eNB.ipv6 = 0x0;
	packet.eNB.ipAddr.addr_v4 = packet.eNB.ipAddr.addr_v4 = ((struct sockaddr_in *)&SELF_ON_SIG->s1apUsersbyLocalID[PDATA->user_ctx->mME_UE_S1AP_ID]->s1->peerAddr)->sin_addr.s_addr;

	/* 8. SGW IP - IP address of the concerned SGW  */
	packet.SGW.spare = 0x0;
	packet.SGW.ipv4 = user->ebearer[0].s1u_sgw.ipv4;
	packet.SGW.ipv6 = user->ebearer[0].s1u_sgw.ipv6;
	if(user->ebearer[0].s1u_sgw.ipv4 & !user->ebearer[0].s1u_sgw.ipv6){
		log_msg(LOG_DEBUG, 0, "SGW: Only Ipv4 addresses is present");
		packet.SGW.ipAddr.addr_v4 = user->ebearer[0].s1u_sgw.addr.addrv4;
	} else if(!user->ebearer[0].s1u_sgw.ipv4 & user->ebearer[0].s1u_sgw.ipv6){
		log_msg(LOG_DEBUG, 0, "SGW: Only Ipv6 addresses is present");
		memcpy(&packet.SGW.ipAddr.addr_v6, &user->ebearer[0].s1u_sgw.addr.addrv6, sizeof user->ebearer[0].s1u_sgw.addr.addrv6);
	} else if(user->ebearer[0].s1u_sgw.ipv4 & user->ebearer[0].s1u_sgw.ipv6){
		log_msg(LOG_DEBUG, 0, "SGW: Both Ipv4 & IPv6 addresses are present");
		memcpy(&packet.SGW.ipAddr.both, &user->ebearer[0].s1u_sgw.addr.both, sizeof user->ebearer[0].s1u_sgw.addr.both);
	}

	/* ======================================================================
	 * Sending packet
	 * ====================================================================== */

	sock = SELF_ON_SIG->ctrl.fd;
	peer = (struct sockaddr_in *)&(SELF_ON_SIG->ctrl.peerAddr);
	peer->sin_family = AF_INET;
	peer->sin_port = htons(CONTROLLER_PORT);
	peer->sin_addr.s_addr = ctrlInfo.addrv4.s_addr;
	peerlen = sizeof(struct sockaddr_in);
	length = sizeof(packet);
	if (sendto(sock, &packet, length, 0, (struct sockaddr *)peer, peerlen) < 0) {
		log_errpack(LOG_ERR, errno, (struct sockaddr_in *)peer, &packet, length,
				"Sendto(fd=%d, msg=%lx, len=%d) failed", sock, (unsigned long) &packet, length);
		return;
	}
	log_msg(LOG_DEBUG, 0, "Sent the user detach message to SDN Controller");
	print_packet(&packet, length);}

/* TODO: Get the correct GTP-TEIDs */
static void TASK_MME_Controller___userHandover(Signal *signal){
	/*
	 * This function is used to send two messages to the SDN controller that an existing user has
	 * moved from one eNB to another eNB. For this, it creates a structure of type ctrl_message
	 * which contains the IP of the SGW, GTP tunnel Ids etc. */

	//      struct ctrl_packet packet;
	//      struct user_ctx_t *user;
	//      struct sockaddr_in  *peer;
	//      uint32_t tunnelId;
	//      int sock;
	//      size_t peerlen;
	//      uint32_t length;
	//      struct nodeinfo_t ctrlInfo;
	//
	//      log_msg(LOG_DEBUG, 0, "Entered the task for sending user Handover to SDN Controller");
	//
	//
	//      user = PDATA->user_ctx;
	//      //      getNode(&ctrlInfo, CTRL, PDATA->user_ctx);
	//      /* Populate the packet with relevant values */
	//      /* 1. IP address of the User Equipment. This is fetched from the PAA (PDN Allocated Address)   */
	//      packet.ue.ipv4 = user->pAA.addr.ipv4;
	//      /* 2. MSISDN field can be fetched from user context */
	//      packet.msisdn = user->msisdn;
	//      /* 3. Flag 0x2 corresponds to detach */
	//      packet.flags = 0x2;
	//      /* 4. Target SGW ip address */
	//      packet.tgt_s_sgw.ipv4 = user->ebearer[0].s1u_sgw.addr.addrv4;
	//      /* 5. GTP Tunnel Endpoint ID at eNB */
	//      packet.uL_gtp_teid = user->ebearer[0].s1u_sgw.teid;
	//      /* 6. GTP Tunnel Endpoint ID at PGW */
	//      packet.dL_gtp_teid = user->ebearer[0].s5s8u.teid;
	//      /* 7. IP address of the eNB to which the UE is connected */
	//      packet.eNB.ipv4 = user->eNBAddr4.s_addr;
	//
	//
	//      sock = SELF_ON_SIG->ctrl.fd;
	//      peer = (struct sockaddr_in *)&(SELF_ON_SIG->ctrl.peerAddr);
	//      peer->sin_family = AF_INET;
	//      peer->sin_port = htons(CONTROLLER_PORT);
	//      peer->sin_addr.s_addr = ctrlInfo.addrv4.s_addr;
	//      peerlen = sizeof(struct sockaddr_in);
	//      length = sizeof(packet);
	//      if (sendto(sock, &packet, length, 0, (struct sockaddr *)peer, peerlen) < 0) {
	//              log_errpack(LOG_ERR, errno, (struct sockaddr_in *)peer, &packet, length,
	//                              "Sendto(fd=%d, msg=%lx, len=%d) failed", sock, (unsigned long) &packet, length);
	//              return;
	//      }
	//
	//      log_msg(LOG_DEBUG, 0, "Sent Handover Message 1 to SDN Controller");
	//
	//      /* 8. Update the flag with new value*/
	//      packet.flags = 0x1;
	//      /* 9. GTP Tunnel Endpoint ID at eNB */
	//      packet.uL_gtp_teid = user->hoCtx.uL_Forward_GTP_TEID.teid;
	//      /* 10. GTP Tunnel Endpoint ID at PGW */
	//      packet.dL_gtp_teid = user->hoCtx.dL_Forward_GTP_TEID.teid;
	//
	//      length = sizeof(packet);
	//      if (sendto(sock, &packet, length, 0, (struct sockaddr *)peer, peerlen) < 0) {
	//              log_errpack(LOG_ERR, errno, (struct sockaddr_in *)peer, &packet, length,
	//                              "Sendto(fd=%d, msg=%lx, len=%d) failed", sock, (unsigned long) &packet, length);
	//              return;
	//      }
	//      log_msg(LOG_DEBUG, 0, "Sent Handover Message 2 to SDN Controller");


}
