/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S1Assoc Session.h
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  ECM logic
 *
 * This Module implements the S1 Association logic, the user associated messages
 * of S1AP
 */

#ifndef S1ASSOC_PRIV_HFILE
#define S1ASSOC_PRIV_HFILE

#include <glib.h>
#include <event2/event.h>
#include <netinet/in.h>

#include "S1Assoc.h"
#include "S1Assoc_State.h"
#include "MME_S1.h"
#include "S1AP.h"
#include "MMEutils.h"

/** Macro to check mandatory IE presence */
#define CHECKIEPRESENCE(p) if(p==NULL){ \
    log_msg(LOG_ERR, 0, "IE not found on message"); return; }


typedef struct{
    S1                  s1;
    int                 fd;             /**< File Descriptor*/
    struct sockaddr     peerAddr;       /**< Peer IP address, IPv4 or IPv6*/
    socklen_t           socklen;        /**< Peer Socket addr length returned by recvfrom*/
    guint16             nonue_rsid;     /**< non-UE associated signaling remote stream id*/
    guint16             nonue_lsid;     /**< non-UE associated signaling remote stream id*/
    S1Assoc_State       *state;
    GString             *eNBname;
    mme_GlobaleNBid     global_eNB_ID;
    //Global_ENB_ID_t     *global_eNB_ID;
    SupportedTAs_t      *suportedTAs;
    CSG_IdList_t        *cGS_IdList;
    GHashTable          *ecm_sessions;  /**< ECM sessions allocated in this association*/
}S1Assoc_t;


/**
 * @brief S1 Send message to non UE signaling
 * @param [in] s1       S1 Association used to send the message
 * @param [in] s1msg    Message to be sent
 *
 * This function send the S1 message to non UE associated signaling.
 * It uses the stream id used during the S1 Setup procedure
 * */
void s1Assoc_sendNonUE(gpointer s1, S1AP_Message_t *s1msg);

/**@brief S1 Send message
 * @param [in] s1       S1 Association used to send the message
 * @param [in] streamId Strem to send the message
 * @param [in] s1msg    Message to be sent
 *
 * This function send the S1 message using the SCTP protocol
 * */
void s1Assoc_send(gpointer s1, guint32 streamId, S1AP_Message_t *s1msg);

/**
 * @brief S1 get ECM session
 * @param [in] h  S1 Association handler
 * @param [in] id MME UE S1AP ID of the ECM session
 *
 * This function returns the ECM session by MME UE S1AP ID
 * */
gpointer *s1Assoc_getECMSession(const S1Assoc h, guint32 id);

/* ************************************************** */
/*                      Accessors                     */
/* ************************************************** */

void s1Assoc_setState(gpointer s1, S1Assoc_State *s);

void s1Assoc_setGlobalID(gpointer h, const Global_ENB_ID_t *gid);

S1 s1Assoc_getS1(gpointer h);


#endif /* S1ASSOC_PRIV_HFILE */
