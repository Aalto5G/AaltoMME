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

/* ======================================================================
 * S11 MME State Machine API
 * ====================================================================== */

/**@brief Attach new user
 * @param [in]  s11_h Engine reference
 * @param [in]  user  User session structure
 * @param [in]  cb    Callback to continue the workflow when the S11 procedure is done
 * @param [in]  args  Argument passed to the callback
 *
 * Used to pass the session to the S11 State machine.
 */
gpointer S11_newUserAttach(gpointer s11_h, struct user_ctx_t *user,
                       void(*cb)(gpointer), gpointer args);

/**@brief Modify Bearer on Attach procedure
 * @param [in]  s11_user  User structure
 * @param [in]  cb        Callback to continue the workflow when the S11 procedure is done
 * @param [in]  args      Argument passed to the callback
 *
 * The s11_user was created with S11_newUserAttach
 */
void S11_Attach_ModifyBearerReq(gpointer s11_user, void(*cb)(gpointer), gpointer args);

/**@brief Detach UE
 * @param [in]  engine Engine reference
 * @param [in]  session User session structure
 *
 */
void S11_dettach(struct t_engine_data *engine, struct SessionStruct_t *session);

/**@brief Trigger Indirect Data Forwarding Tunnel
 * @param [in]  engine Engine reference
 * @param [in]  session User session structure
 *
 */
void S11_CreateIndirectDataForwardingTunnel(struct t_engine_data *engine, struct SessionStruct_t *session);

/**@brief Release Access Bearers
 * @param [in]  engine Engine reference
 * @param [in]  session User session structure
 *
 */
/* void S11_ReleaseAccessBearers(struct t_engine_data *engine, struct SessionStruct_t *session); */


#endif /* MME_S11_HFILE */
