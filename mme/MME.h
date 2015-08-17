/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   MME.h
 * @Author Vicent Ferrer
 * @date   March, 2013
 * @brief  MME type definition and functions.
 *
 * The goal of this file is to define the structures and generic functions of the MME and its interfaces.
 * Allowing to pass process between interfaces.
 */

#ifndef MME_HFILE
#define MME_HFILE

#include <stdint.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <time.h>
#include <arpa/inet.h>

#include <event2/event.h>
#include <glib.h>

#include "Subscription.h"

#include "uthash.h"
#include "gtp.h"
#include "NAS_Definitions.h"
#include "S1AP.h"
#include "S6a.h"


#define MAX_UE 500000 /*< Max number of active users on this MME*/
#define FIRST_UE_SCTP_STREAM 1 /*< The minimum UE SCTP stream value*/

#define PROC signal->processTo
#define PDATA (&(*(struct  SessionStruct_t  *)PROC->data))
#define SELF_ON_SIG signal->processTo->engine->mme

/* ====================================================================== */
enum port_state
{
    closed      = 0,
    opening     = 1,
    listening   = 2,
    connecting  = 3,
    opened      = 4,
    closing     = 5
};

/** EndpointStruct, stores the Interface parameters*/
struct EndpointStruct_t
{
    int                 fd;             /**<File Descriptor*/
    enum port_state     portState;      /**<Port State*/
    struct sockaddr     peerAddr;       /**<Peer IP address, IPv4 or IPv6*/
    socklen_t           socklen;        /**<Peer Socket length returned by recvfrom*/
    void                *info;          /**<Endpoint info, interface dependent*/
    struct t_process    *handler;       /**<Process to handle new incoming messages */
    struct event        *ev;
};


/**
 * Session Structure*/
struct SessionStruct_t
{
    struct t_process            *sessionHandler;    /*Current process handler*/
    struct t_signal_queue       *storedSignals;      /*Stored signals, used when changing between State Machines*/
    struct user_ctx_t           *user_ctx;          /*User information*/
    struct EndpointStruct_t     *s1;
    struct EndpointStruct_t     *s11;
    uint16_t                    sid;                /*SCTP stream used*/
    uint8_t                     pendingRsp;         /*Pending Response Flag*/
    UT_hash_handle              hh1;                /* TEID as index*/
    UT_hash_handle              hh2;                /* MME UE S1AP ID as  index*/
};

struct t_message{
    union{
        uint8_t             raw[65547];
        union gtp_packet    gtp;
    }packet;                 ;  /*  data received as part of the message*/
    size_t                  length;         /*  Packet lenght*/
    struct EndpointStruct_t peer;           /*  Peer endpoint struct*/
	char                    srcAddr[INET6_ADDRSTRLEN];
};

/* Procedure transaction states in the network : 3FPP 24.301 clause 6.1.3.3*/
enum procedure_state{
    proc_inactive,           /*PROCEDURE TRANSACTION INACTIVE*/
    proc_pending             /*PROCEDURE TRANSACTION PENDING */
};

typedef struct Bearer_Ctx_c{
	uint8_t id;
	struct fteid_t	s1u_sgw;
    struct fteid_t  s1u_eNB;
	struct fteid_t	s5s8u;
	struct qos_t	qos;
}Bearer_Ctx_t;

/* Structure to store MME MM and EPS bearer Contexts
 * Avoid the use of pointer to allow the persistent storage in a DB
 * After creating the structure, it is mandatory to initialize it to 0 to avoid table hash errors */
/* There shall be only one pair of TEID-C per UE over the S11 and the S4 interfaces. The same tunnel shall be
shared for the control messages related to the same UE operation. A TEID-C on the S11/S4 interface shall be
released after all its associated EPS bearers are deleted.  */
/* @TODO: finish the struct definition*/
struct user_ctx_t{
	uint64_t          imsi;
    uint64_t          msisdn;
	Subscription      subs;
	/*MSISDN*/
	/*MM State*/
	ESM_State_t       stateNAS_ESM;
	EMM_State_t       stateNAS_EMM;
	/*GUTI Global Unique Temporary Identity*/
    guti_t              guti;
	/*MEI Mobile Equipment Identity*/

	/*MME IP addr for S11*/
	gpointer	s11; //TEMPORAL LOCATION

	/*F-TEID PGW S5/S8 Address for Control Plane or PMIP */
	struct fteid_t	s5s8;

	uint32_t          S11MMETeid;     /**< MME TEID for S11*/
	/*struct in_addr    S11SgwAddr4;    *//**< SGW IP addr for S11*/
	/*uint32_t          S11SgwTeid;     *//**< SGW IP TEID for S11*/
	struct in_addr    eNBAddr4;       /**< eNB IP addr*/

	struct PAA_t    pAA;

	uint32_t			eNB_UE_S1AP_ID;	/**< eNB UE S1AP ID*/
	uint32_t			mME_UE_S1AP_ID;	/**< MME UE S1AP ID*/

	/** TAI Tracking Area ID*/
	struct User_TAI_c{
	  uint16_t		MCC;			/**< Mobile Country Code*/
	  uint16_t		MNC;			/**< Mobile Network Code*/
	  uint8_t       sn[3];          /**< TA TBCD encoded*/
	  uint16_t		tAC;			/**< Tracking Area Code*/
	}tAI;

	/** E-UTRAN CGI (Cell Global ID)*/
	struct User_ECGI_c{
	  uint16_t		MCC;
	  uint16_t		MNC;
	  uint32_t		cellID:28;
	}ECGI;

	struct ksi_c{
	    uint8_t id:3;   /*< K_si*/
	    uint8_t tsc:1;    /*< Type of security context flag (TSC) */
	}ksi;

	SecurityCtx_t sec_ctx;

	struct ho_ctx_c{
	    uint32_t                        target_eNB_id;
        uint32_t                        source_eNB_id;
	    E_RAB_ID_t                      eRAB_ID;
	    struct EndpointStruct_t         *target_s1;
        struct EndpointStruct_t         source_s1;
        Bearer_Ctx_t	                old_ebearers[1];
        GTP_TEID_t                      GTP_TEID;
        TransportLayerAddress_t         transportLayerAddress;
	    GTP_TEID_t                      dL_Forward_GTP_TEID;
	    TransportLayerAddress_t         dL_Forward_transportLayerAddress;
	    GTP_TEID_t                      uL_Forward_GTP_TEID;
	    TransportLayerAddress_t         uL_Forward_transportLayerAddress;
	    Unconstrained_Octed_String_t    target2sourceTransparentContainer;
	    struct fteid_t                  dataforwarding_sgw;
	}hoCtx;


	/*For each active PDN connection*/
	/*APN in Use*/
	/*IP addresses*/
	/*PDN GW Address in Use*/

	/*For each EPS Bearer within the PDN connection:*/
	Bearer_Ctx_t	ebearer[1];
	/*EPS Bearer ID */
	/*IP address for S1-u */
	/*TEID for S1u        */

	/*Others (needed IE)*/
	/*RAT type*/
	/*APN*/
    uint8_t aPname[256];
	/*PAA*/
	/*Serving Network*/
	/*PDN type*/
    uint8_t pdn_type;
	/*APN restriction*/
	/*Selection Mode*/
	/*Bearer contex*/
	  /*EPS Bearer ID */
	  /*EPS Bearer TFT */
	  /* Bearer QoS */
    struct qos_t qos;
	uint64_t ue_ambr_dl;
	uint64_t ue_ambr_ul;
	uint8_t pco[0xff+2];   /* TLV Protocol Configuration Options*/
};


/*@TODO Node structure*/
struct mme_t{
    int                     *run;
    struct t_engine_data    *engine;                /*Multiple engines on the future?*/
    struct event_base       *evbase;
    MMEname_t               *name;
	char                    ipv4[INET_ADDRSTRLEN];
	char                    ipv6[INET6_ADDRSTRLEN]; /* Not used*/
    ServedGUMMEIs_t         *servedGUMMEIs;
    RelativeMMECapacity_t   *relativeCapacity;
    struct EndpointStruct_t ctrl;                           /*< Server endpoint*/
	GHashTable *            ev_readers;                     /*< Listener events accessed by socket*/
	gpointer                s6a;
	gpointer                s11;
	gpointer                s1;
	gpointer                cmd;
	gpointer                sdnCtrl;
    uint32_t                nums1conn;                      /*< Number of S1 Connections*/
    struct SessionStruct_t  *s1apUsersbyLocalID[MAX_UE];    /*< UE MME ID to session relation vector*/
    struct SessionStruct_t  *sessionht_byTEID;    /*Session Hash table to store the processes waiting for a response, */
    struct SessionStruct_t  *sessionht_byS1APID;  /*Session Hash table to store the processes waiting for a response, */
    struct timeval          start;   /* Test Variable*/
    uint32_t                procTime;
	uint32_t                uE_DNS;  /*IP address to be used on the PDN by the UEs*/
};

#define INIT_TIME_MEASUREMENT_ENVIRONMENT \
    struct timeval stop;

#define MME_RESET_TIME \
    gettimeofday(&(SELF_ON_SIG->start), NULL);

#define MME_MEASURE_PROC_TIME \
    gettimeofday(&stop, NULL);\
    SELF_ON_SIG->procTime=(stop.tv_usec - SELF_ON_SIG->start.tv_usec) + (stop.tv_sec - SELF_ON_SIG->start.tv_sec)*1000000; \
    //log_msg(LOG_WARNING, 0, "time mme->procTime %u us", mme->procTime);

extern int init_udp_srv(const char* src, int port);

extern int init_sctp_srv(const char *src, int port);

extern int mme_run();

extern struct t_message *newMsg();
extern void freeMsg( void *msg);

extern void kill_handler(evutil_socket_t listener, short event, void *arg);

/**@brief store pendent response
 * @param [in] session Session structure pointer
 * @return 1 if error, else 0 */
extern int addToPendingResponse(struct SessionStruct_t *session);

extern int removePendentResponse(struct SessionStruct_t *session);

/* Removes session from hash table */
extern struct SessionStruct_t *getPendingResponseByTEID(struct mme_t *mme, uint32_t teid);

/* Removes session from hash table */
extern struct SessionStruct_t *getPendingResponseByUES1APID(struct mme_t *mme, uint32_t mME_UE_S1AP_ID);

extern unsigned int newTeid();

extern uint32_t getNewLocalUEid(struct  SessionStruct_t  * s);


/**************************************************/
/* Accessors                                      */
/**************************************************/

extern const ServedGUMMEIs_t *mme_getServedGUMMEIs(const struct mme_t *mme);

extern const char *mme_getLocalAddress(const struct mme_t *mme);

/**************************************************/
/* API towards state machines                     */
/**************************************************/

/**@brief Register a read callback for a socket
 * @param [in] self MME pointer
 * @param [in] fd   File descriptor to register
 * @param [in] cb   Callback invoked when the fd becomes active
 * @param [in] args Arguments used in the callback
 *
 * The socket needs to be deregistered before exiting the program with
 * the function mme_deregisterRead
 */
void mme_registerRead(struct mme_t *self, int fd,
                      event_callback_fn cb, void * args);

/**@brief Deregister a read callback
 * @param [in] self MME pointer
 * @param [in] fd   File descriptor to register
 *
 * Deregisters a callback registered with mme_registerRead
 */
void mme_deregisterRead(struct mme_t *self, int fd);

struct event_base *mme_getEventBase(struct mme_t *self);

#endif /* MME_HFILE */
