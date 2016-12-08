/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   MME_S11.h
 * @Author Vicent Ferrer
 * @date   March, 2013
 * @brief  MME S11 interface protocol state machine.
 *
 * This module implements the S11 interface state machine.
 */

#ifndef MME_S11_HFILE
#define MME_S11_HFILE

#include <glib.h>
#include "MME.h"
#include "EPS_Session.h"
#include "EMMCtx_iface.h"

typedef event_callback_fn s11_event_cb;
typedef void*             s11_event_arg;

/**
 * @brief initiates the S11 stack
 * @param [in]  mme   pointer to mme structure to access the MME API
 * @return pointer to s11 handler
 *
 * The returned pointer needs to be freed with s11_free
 */
gpointer s11_init(gpointer mme);

/**
 * @brief removes the s11 handler
 * @param [in]  s11 s11 stack handler
 *
 * Deallocates the s11 stack handler allocated with s11_init
 */
void s11_free(gpointer s11);


/**
 * @brief Register file descriptor to accept messages
 * @param [in]  s11 s11 stack handler
 * @param [in]  fd  File descriptor to monitor
 * @param [in]  cb  callback to notify new message
 * @param [in]  arg parameter to pass to the callback
 *
 * Deallocates the s11 stack handler allocated with s11_init
 */
void s11_register_fd(gpointer s11_h, const int fd, s11_event_cb cb, s11_event_arg arg);

void s11_send(gpointer s11_h,
              union gtp_packet *oMsg, guint32 oMsglen,
              struct sockaddr *rAddr, socklen_t rAddrLen,
              GError **err);

/**
 * @brief removes the s11 session
 * @param [in]  s11 s11 stack handler
 *
 * Deregisters the user session, used in the State Machine
 */
void s11_deleteSession(gpointer s11_h, gpointer u);


/**
 * @brief Local restart counter accessor
 * @param [in]  s11_h s11 stack handler
 * @return local restart counter
 */
const guint8 getRestartCounter(gpointer s11_h);


/**
 * @brief Check if this is the first session for that peer
 * @param [in] s11_h s11 stack handler
 * @param [in] peer address
 * @param [in] peer address length
 * @return FALSE when the peer address has other sessions registered, TRUE otherwise
 *
 * This function starts tracking the number of sessions registered for that S-GW
 */
gboolean S11_isFirstSession(gpointer  s11_h,
                            const struct sockaddr *rAddr,
                            const socklen_t rAddrLen);


/**
 * @brief Remove one session from the peer session tracking
 * @param [in] s11_h s11 stack handler
 * @param [in] peer address
 * @param [in] peer address length
 *
 */
void S11_unrefSession(gpointer  s11_h,
                      const struct sockaddr *rAddr,
                      const socklen_t rAddrLen);

void S11_checkPeerRestart(gpointer  s11_h,
                          const struct sockaddr *rAddr,
                          const socklen_t rAddrLen,
                          guint8 restartCounter,
                          gpointer ongoingUser);

void S11_paging(gpointer s11_h, gpointer emm);

/**
 * @brief Provides the next sequence number to be sent
 * @param [in]  s11_h s11 stack handler
 * @return sequence number
 */
const unsigned int getNextSeq(gpointer s11_h);

/**
 * @brief Provides the local IP address in string format
 * @param [in]  s11_h s11 stack handler
 * @return Local IP address in string format
 */
const char *s11_getLocalAddress(gpointer s11_h);

/* ======================================================================
 * S11 MME State Machine API
 * ====================================================================== */

/**
 * @brief Attach new user
 * @param [in]  s11_h Engine reference
 * @param [in]  emm   Mobility Manamegent Handler
 * @param [in]  s     EPS session Handler
 * @param [in]  cb    Callback to continue the workflow when the S11 procedure is done
 * @param [in]  args  Argument passed to the callback
 *
 * Used to pass the session to the S11 State machine.
 */
gpointer S11_newUserAttach(gpointer s11_h, EMMCtx emm, EPS_Session s,
                       void(*cb)(gpointer), gpointer args);

/**
 * @brief Modify Bearer on Attach procedure
 * @param [in]  s11_user  User structure
 * @param [in]  cb        Callback to continue the workflow when the S11 procedure is done
 * @param [in]  args      Argument passed to the callback
 *
 * The s11_user was created with S11_newUserAttach
 */
void S11_Attach_ModifyBearerReq(gpointer s11_user, void(*cb)(gpointer), gpointer args);

/**
 * @brief Detach UE
 * @param [in]  engine Engine reference
 * @param [in]  session User session structure
 *
 */
void S11_dettach(gpointer s11_user, void(*cb)(gpointer), gpointer args);

/* /\** */
/*  * @brief Trigger Indirect Data Forwarding Tunnel */
/*  * @param [in]  engine Engine reference */
/*  * @param [in]  session User session structure */
/*  * */
/*  *\/ */
/* void S11_CreateIndirectDataForwardingTunnel(struct t_engine_data *engine, struct SessionStruct_t *session); */

/**
 * @brief Release Access Bearers
 * @param [in]  engine Engine reference
 * @param [in]  session User session structure
 *
 */
void S11_ReleaseAccessBearers(gpointer s11_user, void(*cb)(gpointer), gpointer args);

#endif /* MME_S11_HFILE */
