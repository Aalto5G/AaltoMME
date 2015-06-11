/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   eNBemulator.c
 * @Author Vicent Ferrer
 * @date   May, 2013
 * @brief  simple eNB emulator
 *
 * Tool to test S1 capabilities during development.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/sctp.h>
#include <event2/event.h>
#include <time.h>

#include <mysql.h>
#include <math.h>


#include "S1AP.h"
#include "NAS.h"


#define S1AP_CONTROL_STREAM0 0
#define S1AP_CONTROL_STREAM1 1

char *msgpath[] = {"/home/vicent/S1APmsg/S1AP_id-S1Setup.s1",
        "/home/vicent/S1APmsg/S1AP_id-initialUEMessageReq.s1",
        "/home/vicent/S1APmsg/S1AP_id-uplinkNASTransport.s1",
        "/home/vicent/S1APmsg/S1AP_id-InitialContextSetup_Rsp.s1",
        "/home/vicent/S1APmsg/S1AP_id-uplinkNASTransport_AttachComplete.s1"
};

#define MMEHOST "10.12.0.141"

/**************************************************
                    HSS connection
 **************************************************/

#define USER    "remote"
#define PASSWD  "remote"
#define HSS_DB  "hss_lte_db"

/**********************************************
 * GLOBAL VARIABLES
 **********************************************/

/* HSS connection global variable */
MYSQL                   *HSSConnection;

/* preload flag*/
uint8_t preload;
/* Number of users and reset Time*/
int numUE, resetTime;
struct timeval start;

/**********************************************
 * DB functions
 **********************************************/

void disconnect_hss(){

    /* Close datbase connection*/
    if(HSSConnection!= NULL){
        printf("Disconnecting from database, handler: %p\n", HSSConnection);
        mysql_close(HSSConnection);
        HSSConnection=NULL;
    }else{
        printf("Error while trying to disconnect MySQL. Handler not available\n");
    }

    mysql_library_end();
}

int init_hss(){
    MYSQL      *MySQLConRet;

    /*SQL */
    HSSConnection = NULL;

    if (mysql_library_init(0, NULL, NULL)) {
        printf("could not initialize MySQL library\n");
        return 1;
    }

    HSSConnection = mysql_init( NULL );
    if(HSSConnection == NULL){
        printf( "%u, %s", mysql_errno(HSSConnection), mysql_error(HSSConnection));
        disconnect_hss();
        return 1;
    }

    MySQLConRet = mysql_real_connect( HSSConnection, MMEHOST, USER, PASSWD, HSS_DB,
                                      0, NULL, CLIENT_MULTI_STATEMENTS );
    if ( MySQLConRet == NULL || MySQLConRet != HSSConnection){
        printf("%u, %s. Disconnecting. Handler %p", mysql_errno(HSSConnection), mysql_error(HSSConnection), HSSConnection);
        disconnect_hss();
        return 1;
    }
    return 0;
}

void deleteTestUEsonHSS(){
    int ret;

    ret = mysql_query(HSSConnection, "DELETE FROM auth_vec WHERE mcc='234' and mnc='77';");
    if(ret != 0){
        printf("ERROR 1 on deleteTestUEsonHSS: %u, %s\n", mysql_errno(HSSConnection), mysql_error(HSSConnection));
    }

    ret = mysql_query(HSSConnection, "DELETE FROM subscriber_profile WHERE mcc='234' and mnc='77';");
    if(ret != 0){
        printf("ERROR 2 on deleteTestUEsonHSS: %u, %s\n", mysql_errno(HSSConnection), mysql_error(HSSConnection));
    }

    ret = mysql_query(HSSConnection, "DELETE FROM pdn_subscription_ctx WHERE mcc='234' and mnc='77';");
    if(ret != 0){
        printf("ERROR 3 on deleteTestUEsonHSS: %u, %s\n", mysql_errno(HSSConnection), mysql_error(HSSConnection));
    }
}

void addNewUEonHSS(uint64_t imsi){
    int ret, err=0, i=0;
    struct timeval stop;

    const char insert1[] = "INSERT INTO auth_vec values (234, 77, x'%.10llu', 0, "
      "x'00000000000000000000000000000000', x'00000000000000000000000000000000', x'00000000000000000000000000000000', x'0000000000000000',"
      " x'00000000000000000000000000000000', x'000000000000', x'00000000000000000000000000000000', x'00000000000000000000000000000000'); ";
    const char insert2[] = "INSERT INTO subscriber_profile values (234, 77, x'%.10llu', 0,"
      " x'00000000000000000000000000000000', x'00000000000000000000000000000000', x'0000000000000000', 0, 0, 0, 0, 0, 0, x'0000'); ";
    const char insert3[] = "INSERT INTO pdn_subscription_ctx values (234, 77, x'%.10llu', 0,"
      " 0, 0, 0, x'0000', b'00', x'000000000000000000000000', 0, 0, 0, 0, 0, 0); ";

    char query[1000];

    gettimeofday(&stop, NULL);
    if((stop.tv_sec - start.tv_sec)>resetTime && resetTime!=0){
        deleteTestUEsonHSS();
        sleep(1);
        resetTime = 0;
    }

    //sprintf(query, insert, imsi%10000000000, imsi%10000000000, imsi%10000000000);
    sprintf(query, insert1, imsi%10000000000);

    do{
        ret = mysql_query(HSSConnection, query);

        if(ret != 0){
            printf("ERROR 1 on addNewUEonHSS: %u, %s\n", mysql_errno(HSSConnection), mysql_error(HSSConnection));
            sleep(15);
        }
        i++;
    }while(ret!= 0 && err == 2014 && i<2);

    sprintf(query, insert2, imsi%10000000000);
    ret = mysql_query(HSSConnection, query);
    if(ret != 0){
        printf("ERROR 2 on addNewUEonHSS: %u, %s\n", mysql_errno(HSSConnection), mysql_error(HSSConnection));
    }

    sprintf(query, insert3, imsi%10000000000);
    ret = mysql_query(HSSConnection, query);
    if(ret != 0){
        printf("ERROR 3 on addNewUEonHSS: %u, %s\n", mysql_errno(HSSConnection), mysql_error(HSSConnection));
    }
}


/**********************************************
 * Attach Message Functions
 **********************************************/

void forgeImsi(uint8_t *to, uint64_t imsi){
    uint8_t j;
    to[0] = 0x29;
    to[1] = 0x43;
    to[2] = 0x77;
    to+=7;
    for(j=0; j< 5;j++){
        *to = (imsi%100)/10;
        *to |= ((imsi%10)<<4);
        imsi/=100;
        to--;
    }
};

void sendInitUEMsg(uint32_t fd, uint16_t streamId, uint32_t eNBid, uint64_t imsi){

    /* Forge Initial UE message*/
     S1AP_Message_t *s1msg;
     ENB_UE_S1AP_ID_t *eNBUEId;
     Unconstrained_Octed_String_t *nAS_PDU;
     TAI_t  *tAI;
     EUTRAN_CGI_t *ecgi;
     RRC_Establishment_Cause_t *rrcCause;
     uint8_t nAS_buffer[500], *pointer, *pointer2, *tmp1;
     uint8_t buf[10000], tmp[50];
     uint32_t bsize, ret;

     s1msg = S1AP_newMsg();
     s1msg->choice = initiating_message;
     s1msg->pdu->procedureCode = id_initialUEMessage;
     s1msg->pdu->criticality = reject;

     /* eNB-UE-S1AP-ID*/
     eNBUEId = s1ap_newIE(s1msg, id_eNB_UE_S1AP_ID, mandatory, ignore);
     eNBUEId->eNB_id = eNBid;

     /* NAS-PDU*/
     nAS_PDU = s1ap_newIE(s1msg, id_NAS_PDU, mandatory, reject);

     /* TAI*/
     tAI = s1ap_newIE(s1msg, id_TAI, mandatory, reject);
     tAI->pLMNidentity = new_PLMNidentity();
     tAI->pLMNidentity->MCC = 234;
     tAI->pLMNidentity->MNC = 77;
     tAI->tAC->s[0]=0x05;
     tAI->tAC->s[1]=0x3d;

     /* id_EUTRAN_CGI*/
     ecgi = s1ap_newIE(s1msg, id_EUTRAN_CGI, mandatory, reject);
     ecgi->pLMNidentity = new_PLMNidentity();
     ecgi->pLMNidentity->MCC = 234;
     ecgi->pLMNidentity->MNC = 77;
     ecgi->cell_ID.id = 1;

     /* id_RRC_Establishment_Cause*/
     rrcCause = s1ap_newIE(s1msg, id_RRC_Establishment_Cause, mandatory, reject);
     rrcCause->cause.noext = RRC_mo_Signalling;

     /*FORGE NAS*/

     /* Forge Detach Accept*/
     pointer = nAS_buffer;
     newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

     encaps_EMM(&pointer, AttachRequest);

     /*EPSAttachType */
     nasIe_v_t1_l(&pointer, 1);
     /* NASKeySetId*/
     nasIe_v_t1_h(&pointer, 7);

     /*EPSMobileId*/
     forgeImsi(tmp, imsi);
     nasIe_lv_t4(&pointer, tmp, 8);

     /*UENetworkCapability*/
     tmp[0]=0xe0;
     tmp[1]=0xe0;
     nasIe_lv_t4(&pointer, tmp, 2);

     /*ESM_MessageContainer*/
         pointer2 = tmp;
         tmp1 = tmp;
         newNASMsg_ESM(&pointer2, EPSSessionManagementMessages, 0);

         encaps_ESM(&pointer2, 1, PDNConnectivityRequest);

         /*EPSAttachType */
         nasIe_v_t1_l(&pointer2, 1);
         /* NASKeySetId*/
         nasIe_v_t1_h(&pointer2, 1);

     nasIe_lv_t6(&pointer, tmp, pointer2-tmp1);

     nAS_PDU->len = pointer-nAS_buffer;
     nAS_PDU->str = nAS_buffer;

     /* Send Response*/
     memset(buf, 0, 10000);
     s1ap_encode(buf, &bsize, s1msg);

     /* sctp_sendmsg*/
     ret = sctp_sendmsg(fd, (void *)buf, (size_t)bsize, NULL, 0, SCTP_S1AP_PPID, 0, streamId, 0, 0 );

     if(ret==-1){
         printf("S1AP : Error sending SCTP message to MME");
     }

     /* Free msg*/
     s1msg->freemsg(s1msg);
}

void sendAuthRsp(uint32_t fd, uint16_t streamId, uint32_t mMEid, uint32_t eNBid, uint64_t imsi){

    S1AP_Message_t *s1out;

    MME_UE_S1AP_ID_t *mmeUEId;
    ENB_UE_S1AP_ID_t *eNBUEId;
    Unconstrained_Octed_String_t *nAS_PDU;

    uint8_t buf[10000], tmp[50], nAS_buffer[500], *pointer;
    uint32_t bsize, ret;

    s1out = S1AP_newMsg();
    s1out->choice = initiating_message;
    s1out->pdu->procedureCode = id_uplinkNASTransport;
    s1out->pdu->criticality = ignore;

    /* MME-UE-S1AP-ID*/
    mmeUEId = s1ap_newIE(s1out, id_MME_UE_S1AP_ID, mandatory, reject);
    mmeUEId->mme_id = mMEid;

    /* eNB-UE-S1AP-ID*/
    eNBUEId = s1ap_newIE(s1out, id_eNB_UE_S1AP_ID, mandatory, reject);
    eNBUEId->eNB_id = eNBid;

    /* NAS-PDU*/
    nAS_PDU = s1ap_newIE(s1out, id_NAS_PDU, mandatory, reject);

    /*FORGE NAS*/
         /* Forge Auth Resp */
         pointer = nAS_buffer;
         newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

         encaps_EMM(&pointer, AuthenticationResponse);

         memset(tmp, 0, 8);
         nasIe_lv_t4(&pointer, tmp, 8);

    nAS_PDU->len = pointer-nAS_buffer;
    nAS_PDU->str = nAS_buffer;

    /* Send Response*/
    memset(buf, 0, 10000);
    s1ap_encode(buf, &bsize, s1out);

    /* sctp_sendmsg*/
    ret = sctp_sendmsg(fd, (void *)buf, (size_t)bsize, NULL, 0, SCTP_S1AP_PPID, 0, streamId, 0, 0 );

    if(ret==-1){
        printf("S1AP : Error sending SCTP message to MME\n");
    }

    /* Free msg*/
    s1out->freemsg(s1out);
}

void sendInitialCtxSetupRsp(uint32_t fd, uint16_t streamId, uint32_t mMEid, uint32_t eNBid, uint64_t imsi){

    /* Forge Initial Context Setup Rsp*/
     S1AP_Message_t *s1msg;

     MME_UE_S1AP_ID_t *mmeUEId;
     ENB_UE_S1AP_ID_t *eNBUEId;
     E_RABSetupListCtxtSURes_t *eRABlist;
     E_RABSetupItemCtxtSURes_t *eRABitem;
     S1AP_PROTOCOL_IES_t *ie;
     uint8_t buf[10000];
     uint32_t bsize, ret;
     static uint32_t  ip=1;

     s1msg = S1AP_newMsg();
     s1msg->choice = successful_outcome;
     s1msg->pdu->procedureCode = id_InitialContextSetup;
     s1msg->pdu->criticality = reject;


     /* MME-UE-S1AP-ID*/
     mmeUEId = s1ap_newIE(s1msg, id_MME_UE_S1AP_ID, mandatory, reject);
     mmeUEId->mme_id = mMEid;

     /* eNB-UE-S1AP-ID*/
     eNBUEId = s1ap_newIE(s1msg, id_eNB_UE_S1AP_ID, mandatory, ignore);
     eNBUEId->eNB_id = eNBid;

     /* id_E_RABSetupListCtxtSURes*/
     eRABlist = s1ap_newIE(s1msg, id_E_RABSetupListCtxtSURes, mandatory, reject);

     ie = newProtocolIE();
     eRABitem = new_E_RABSetupItemCtxtSURes();
     ie->value = eRABitem;
     ie->showValue = eRABitem->showIE;
     ie->freeValue = eRABitem->freeIE;
     eRABlist->additem(eRABlist, ie);
     eRABitem->transportLayerAddress = new_TransportLayerAddress();
     ie->id = id_E_RABSetupItemCtxtSURes;
     ie->presence = optional;
     ie->criticality = ignore;
     eRABitem->eRAB_ID.id = 5;

     memcpy(eRABitem->transportLayerAddress->addr, &ip, sizeof(uint32_t));
     eRABitem->transportLayerAddress->len = 32;
     ip++;
     memcpy(eRABitem->gTP_TEID.teid, &ip, sizeof(uint32_t));

     /* Send Response*/
     memset(buf, 0, 10000);
     s1ap_encode(buf, &bsize, s1msg);

     /* sctp_sendmsg*/
     ret = sctp_sendmsg(fd, (void *)buf, (size_t)bsize, NULL, 0, SCTP_S1AP_PPID, 0, streamId, 0, 0 );

     if(ret==-1){
         printf("S1AP : Error sending SCTP message to MME");
     }

     /* Free msg*/
     s1msg->freemsg(s1msg);
};

void sendAttachComplete(uint32_t fd, uint16_t streamId, uint32_t mMEid, uint32_t eNBid, uint64_t imsi){

    S1AP_Message_t *s1out;

    MME_UE_S1AP_ID_t *mmeUEId;
    ENB_UE_S1AP_ID_t *eNBUEId;
    Unconstrained_Octed_String_t *nAS_PDU;

    uint8_t buf[10000], tmp[50], nAS_buffer[500], *pointer, *pointer2, *tmp1;
    uint32_t bsize, ret;

    s1out = S1AP_newMsg();
    s1out->choice = initiating_message;
    s1out->pdu->procedureCode = id_uplinkNASTransport;
    s1out->pdu->criticality = ignore;

    /* MME-UE-S1AP-ID*/
    mmeUEId = s1ap_newIE(s1out, id_MME_UE_S1AP_ID, mandatory, reject);
    mmeUEId->mme_id = mMEid;

    /* eNB-UE-S1AP-ID*/
    eNBUEId = s1ap_newIE(s1out, id_eNB_UE_S1AP_ID, mandatory, reject);
    eNBUEId->eNB_id = eNBid;

    /* NAS-PDU*/
    nAS_PDU = s1ap_newIE(s1out, id_NAS_PDU, mandatory, reject);

    /*FORGE NAS*/
         /* Forge Attach Complete*/
         pointer = nAS_buffer;
         newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

         encaps_EMM(&pointer, AttachComplete);

         /*ESM_MessageContainer*/
             pointer2 = tmp;
             tmp1 = tmp;
             newNASMsg_ESM(&pointer2, EPSSessionManagementMessages, 5);

             encaps_ESM(&pointer2, 0, ActivateDefaultEPSBearerContextAccept);

         nasIe_lv_t6(&pointer, tmp, pointer2-tmp1);

    nAS_PDU->len = pointer-nAS_buffer;
    nAS_PDU->str = nAS_buffer;

    /* Send Response*/
    memset(buf, 0, 10000);
    s1ap_encode(buf, &bsize, s1out);

    /* sctp_sendmsg*/
    ret = sctp_sendmsg(fd, (void *)buf, (size_t)bsize, NULL, 0, SCTP_S1AP_PPID, 0, streamId, 0, 0 );

    if(ret==-1){
        printf("S1AP : Error sending SCTP message to MME\n");
    }

    /* Free msg*/
    s1out->freemsg(s1out);
}


/**********************************************
 * General Attach Message Function Loop Manager
 **********************************************/

int newAttachMsg(int listener, int sid, S1AP_Message_t * s1msg){
    MME_UE_S1AP_ID_t    *mme_id;
    static int i=1, num_attach=0;
    static uint16_t enbid = 0;
    uint64_t imsi = 234770000000000ULL;

    if(i<4){

        switch(i){
        case 1: /*  Initial UE message / Attach Req*/
            num_attach++;
            imsi=imsi+1ULL;
            enbid ++;
            if(preload==0)
                addNewUEonHSS(imsi);
            sendInitUEMsg(listener, 0, enbid, imsi);
            break;
        case 2: /* Authentication Response */
            if(s1msg->pdu->procedureCode!= id_downlinkNASTransport){
                printf("Procedure Code error: %s\n", elementaryProcedureName[s1msg->pdu->procedureCode]);
                return 1;
            }
            mme_id = s1ap_findIe(s1msg, id_MME_UE_S1AP_ID);
            sendAuthRsp(listener, sid, mme_id->mme_id, enbid, imsi);
            break;
        case 3: /* Initial Context Setup*/
            mme_id = s1ap_findIe(s1msg, id_MME_UE_S1AP_ID);
            sendInitialCtxSetupRsp(listener, sid, mme_id->mme_id, enbid, imsi);

            /* Attach Complete*/
            sendAttachComplete(listener, sid, mme_id->mme_id, enbid, imsi);

            /* initial UE message / Attach request*/
            i = 1;
            if(preload==1 && num_attach>=numUE){
                exit(0);
            }
            num_attach++;
            imsi=imsi+num_attach;
            enbid++;
            printf("NEW attach #%u, imsi %llu\n", num_attach, imsi);
            if(preload==0)
                addNewUEonHSS(imsi);

            sendInitUEMsg(listener, 0, enbid, imsi);
            break;
        default:
            printf("ERROR\n");
            return 1;
            break;
        }
        i++;
    }
    return 0;
}

/**********************************************
 * Event Handler Callback
 **********************************************/

void s1_accept(evutil_socket_t listener, short event, void *arg){
    int in=0, flags;
    uint8_t buffer[500];
    struct sctp_sndrcvinfo sndrcvinfo;
    struct event *ev = *(struct event **)arg;
    S1AP_Message_t *s1msg=NULL;

    in = sctp_recvmsg( listener, (void *)buffer, sizeof(buffer),
                        (struct sockaddr *)NULL, 0, &sndrcvinfo, &flags );

    if (in > 0) {
        buffer[in] = 0;
        //printf("Recv from Stream %u \n", sndrcvinfo.sinfo_stream);
    }else{
        printf("Recv Error. flags %d\n", flags);
        return;
        //        goto error;
    }

    s1msg = s1ap_decode(buffer, in);

    if (newAttachMsg(listener, sndrcvinfo.sinfo_stream, s1msg) != 0){
        goto error;
    }

    s1msg->freemsg(s1msg);
    return;


error:
    if(s1msg!=NULL){
        s1msg->freemsg(s1msg);
    }
    /*Close socket*/
    close(listener);
    /*Delete event*/
    event_free(ev);
    deleteTestUEsonHSS();
    disconnect_hss();
    exit(1);
}


/**********************************************
 * MME connection function
 **********************************************/

int init_MME_connection(const char *peer, uint16_t port){

    int connSock, ret, in, optval;
    struct sctp_initmsg initmsg;
    struct sockaddr_in peerAddr, localAddr;
    struct sctp_event_subscribe events;
    struct sctp_status status;

    /* Create an SCTP TCP-Style Socket */
    connSock = socket( AF_INET, SOCK_STREAM, IPPROTO_SCTP );

    /* Accept connections from any interface */
    bzero( (void *)&localAddr, sizeof(struct sockaddr_in) );
    localAddr.sin_family = AF_INET;

    localAddr.sin_addr.s_addr = inet_addr( "10.12.0.142" );
    localAddr.sin_port = htons(port);

    if (bind( connSock, (struct sockaddr *)&localAddr, sizeof(struct sockaddr_in) ) < 0) {
        printf("bind(fd=%d, addr=%lx, len=%d) failed\n", connSock, (unsigned long) &localAddr, sizeof(struct sockaddr_in));
        return -1;
    }

    /* Disable Nagle algorithm */
    optval=1;
    if (setsockopt(connSock, IPPROTO_SCTP, SCTP_NODELAY, (void*)&optval, sizeof(optval)) ==-1){
        printf("Couldn't set SCTP_NODELAY socket option.\n");
        /* No critical*/
    }


    /* Specify that a maximum of 5 streams will be available per socket */
    memset( &initmsg, 0, sizeof(initmsg) );
    initmsg.sinit_num_ostreams = 5;
    initmsg.sinit_max_instreams = 5;
    initmsg.sinit_max_attempts = 4;
    ret = setsockopt( connSock, IPPROTO_SCTP, SCTP_INITMSG,
                     &initmsg, sizeof(initmsg) );

    /* Specify the peer endpoint to which we'll connect */
    bzero( (void *)&peerAddr, sizeof(peerAddr) );
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(port);
    peerAddr.sin_addr.s_addr = inet_addr( peer );

    /* Connect to the server */
    ret = connect( connSock, (struct sockaddr *)&peerAddr, sizeof(peerAddr) );
    if(ret==-1){
        printf("Connect error\n");
        return ret;
    }

    /* Enable receipt of SCTP Snd/Rcv Data via sctp_recvmsg */
    memset( (void *)&events, 0, sizeof(events) );
    events.sctp_data_io_event = 1;
    ret = setsockopt( connSock, SOL_SCTP, SCTP_EVENTS,
                     (const void *)&events, sizeof(events) );

    /* Read and emit the status of the Socket (optional step) */
    in = sizeof(status);
    ret = getsockopt( connSock, SOL_SCTP, SCTP_STATUS,
                     (void *)&status, (socklen_t *)&in );
    printf("******* Connection Information *******\n");
    printf("assoc id  = %d\n", status.sstat_assoc_id );
    printf("state     = %d\n", status.sstat_state );
    printf("instrms   = %d\n", status.sstat_instrms );
    printf("outstrms  = %d\n", status.sstat_outstrms );
    printf("**************************************\n\n");

    return connSock;
}


/**********************************************
 * Main
 **********************************************/
int main(int argc, char **argv)
{
    int n,connSock, ret;
    uint32_t ue;
    char buffer[500];
    FILE *f;
    uint64_t imsi = 234770000000000ULL;


    /*libevent*/
    struct event_base *base;
    struct event *listener_s1;
    struct event **cb_arg;


    if(!(argc==1 || argc==3)){
        printf("Parameter number not correct. Usage:\n%s [-p <number of UEs>]\n", argv[0]);
        exit(1);
    }

    preload = 0;
    resetTime = 0;
    if(argc==3){
        if(strcmp(argv[1], "-p")==0){
            numUE = atoi(argv[2]);
            if(numUE<0){
                printf("Negative UE number not allowed\n");
                exit(1);
            }
            printf("Preload detected, numUE %d\n", numUE);
            preload = 1;
        }else if(strcmp(argv[1], "-r")==0){
            resetTime = atoi(argv[2]);
            if(numUE<0){
                printf("Negative time not allowed\n");
                exit(1);
            }
        }else{
            printf("Parameters not correct. Usage:\n%s [-p <number of UEs>]\n", argv[0]);
            exit(1);
        }
    }

    gettimeofday(&start, NULL);

    base = event_base_new();
    if (!base)
    {
        printf("Failed to create libevent event-base\n");
        exit(1);
    }


    connSock = init_MME_connection(MMEHOST, S1AP_PORT);
    if(connSock==-1){
        return 1;
    }

    if(init_hss()==1){
        close(connSock);
        return 1;
    }

    if(preload==1){
        printf("Loading DB ...\n");
        for (ue=1; ue<=numUE; ue++){
            addNewUEonHSS(imsi+ue);
        }
        printf("Added %d UEs on DB\n", ue);
    }

    f = fopen(msgpath[0], "rb");
    if (f){
        memset(buffer,0,500);
        n = fread(buffer, 1, 500, f);
    }
    else{
        printf("error opening file main\n");
        close(connSock);
        return 1;
    }
    fclose(f);

    /* Add event*/
    cb_arg = &listener_s1;
    listener_s1 = event_new(base, connSock, EV_READ|EV_PERSIST, s1_accept, cb_arg);

    event_add(listener_s1, NULL);
    evutil_make_socket_nonblocking(connSock);

    /* Send First message*/
    ret = sctp_sendmsg( connSock, (void *)buffer, (size_t)n, NULL, 0, SCTP_S1AP_PPID, 0, S1AP_CONTROL_STREAM0, 0, 0 );

    if (ret<0){
      printf("sctp_sendmsg error\n");
      close(connSock);
      return 1;
    }

    printf("send %d bytes\n", ret);

    /* Init loop*/
    event_base_dispatch(base);
    printf("End of loop.\n");

    deleteTestUEsonHSS();
    disconnect_hss();

    return 0;
}
