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

#include "MME.h"

/* ======================================================================
 * S11 MME State Machine API
 * ====================================================================== */

/**@brief Attach new user
 * @param [in]  engine Engine reference
 * @param [in]  session User session structure
 *
 * Used to pass the session to the S11 State machine. The previous state machine shall include the signal to return.
 */
void S11_newUserAttach(struct t_engine_data *engine, struct SessionStruct_t *session);

/**@brief Modify Bearer on Attach procedure
 * @param [in]  engine Engine reference
 * @param [in]  session User session structure
 *
 */
void S11_Attach_ModifyBearerReq(struct t_engine_data *engine, struct SessionStruct_t *session);

/**@brief Detach UE
 * @param [in]  engine Engine reference
 * @param [in]  session User session structure
 *
 */
void S11_dettach(struct t_engine_data *engine, struct SessionStruct_t *session);

/**@brief Create S11 process structure
 * @param [in]  engine Engine reference
 * @param [in]  owner Parent process.
 *
 */
struct t_process *S11_handler_create(struct t_engine_data *engine, struct t_process *owner);

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
void S11_ReleaseAccessBearers(struct t_engine_data *engine, struct SessionStruct_t *session);


#endif /* MME_S11_HFILE */
