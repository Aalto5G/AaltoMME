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
 * @brief removes the s11 session
 * @param [in]  s11 s11 stack handler
 *
 * Deregisters the user session, used in the State Machine
 */
void s11_deleteSession(gpointer s11_h, gpointer u);

/**
 * @brief File descriptor accessor
 * @param [in]  s11_h s11 stack handler
 * @return file descriptor
 */
const int s11_fg(gpointer s11_h);

/**
 * @brief Local restart counter accessor
 * @param [in]  s11_h s11 stack handler
 * @return local restart counter
 */
const guint8 getRestartCounter(gpointer s11_h);

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
