/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   MME_S6a.c
 * @Author Vicent Ferrer
 * @date   June, 2013
 * @brief  MME S6a interface protocol state machine.
 *
 * This module implements the S6a interface state machine.
 * It is currently only and emulator
 */

#include "MME_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include "signals.h"

#include "MME_S6a.h"
#include "logmgr.h"

#include "hmac_sha2.h"
#include "milenage.h"

#include "HSS.h"

/**
 * @brief generate_KeNB - KDF function to derive the K_eNB
 * @param [in]  kasme       derived key - 256 bits
 * @param [in]  ulNASCount  Uplink NAS COUNT
 * @param [out] keNB        eNB result key - 256 bits
 */
static void generate_KeNB(const uint8_t *kasme, const uint32_t ulNASCount, uint8_t *keNB){

    /*
    FC = 0x11,
    P0 = Uplink NAS COUNT,
    L0 = length of uplink NAS COUNT (i.e. 0x00 0x04)
     */
    uint8_t s[7];
    s[0]=0x11;
    memcpy(s+1, &ulNASCount, 4);
    s[5]=0x00;
    s[6]=0x04;

    hmac_sha256(kasme, 32, s, 7, keNB, 32);
}


/* ====================================================================== */

static int STATE_S6a_Authentication(Signal *signal){
    struct user_ctx_t *user = PDATA->user_ctx;
    INIT_TIME_MEASUREMENT_ENVIRONMENT

    MME_MEASURE_PROC_TIME
    if( signal->name == S6a_getAuthVector){
	    log_msg(LOG_DEBUG, 0, "S6a: Creating Auth vector -  time = %u us", SELF_ON_SIG->procTime);

	    HSS_getAuthVec(signal);

	    generate_KeNB(user->sec_ctx.kASME, user->sec_ctx.ulNAScnt, user->sec_ctx.keNB);
    }else if(signal->name == S6a_SynchAuthVector){
	    log_msg(LOG_DEBUG, 0, "S6a: Synch NAS SQN -  time = %u us", SELF_ON_SIG->procTime);

	    HSS_syncAuthVec(signal);

	    generate_KeNB(user->sec_ctx.kASME, user->sec_ctx.ulNAScnt, user->sec_ctx.keNB);
    }

    /*  Recover old process and old signal to continue the flow to original State Machine*/    
    run_parent(signal);

    MME_MEASURE_PROC_TIME
    log_msg(LOG_DEBUG, 0, "S6a: Auth vector available-  time = %u us", SELF_ON_SIG->procTime);
    return 0;

}

static int STATE_S6a_UpdateLocation(Signal *signal){
    INIT_TIME_MEASUREMENT_ENVIRONMENT

    MME_MEASURE_PROC_TIME
    log_msg(LOG_DEBUG, 0, "S6a: Update Location -  time = %u us", SELF_ON_SIG->procTime);

    HSS_UpdateLocation(signal);

    /*  Recover old process and old signal to continue the flow to original State Machine*/
    run_parent(signal);

    MME_MEASURE_PROC_TIME
    log_msg(LOG_DEBUG, 0, "S6a: Location Updated -  time = %u us", SELF_ON_SIG->procTime);

    return 0;

}

/* ====================================================================== */

void s6a_GetAuthVector(struct t_engine_data *engine, struct SessionStruct_t *session){

    Signal *output;
    struct t_process proc;
    log_msg(LOG_DEBUG, 0, "Enter S6a State Machine");

    /*Create a new process to manage the S6a state machine. The older session handler is stored as parent
     * to return once the S6a state machine ends*/
    session->sessionHandler = process_create(engine, STATE_S6a_Authentication, (void *)session, session->sessionHandler);

    output = new_signal(session->sessionHandler);
    /*output->data = (void *)session;*/
    output->name = S6a_getAuthVector;
    output->priority = MAXIMUM_PRIORITY;
    signal_send(output);
}

void s6a_SynchAuthVector(struct t_engine_data *engine, struct SessionStruct_t *session, uint8_t *auts){

    Signal *output;
    struct t_process proc;
    void *d;
    log_msg(LOG_DEBUG, 0, "Enter S6a State Machine");

    /*Create a new process to manage the S6a state machine. The older session handler is stored as parent
     * to return once the S6a state machine ends*/
    session->sessionHandler = process_create(engine, STATE_S6a_Authentication, (void *)session, session->sessionHandler);

    output = new_signal(session->sessionHandler);
    d = malloc(14);
    memcpy(d, auts, 14);
    output->data = d;
    output->freedataFunc=&free;
    output->name = S6a_SynchAuthVector;
    output->priority = MAXIMUM_PRIORITY;
    signal_send(output);
}


void s6a_UpdateLocation(struct t_engine_data *engine, struct SessionStruct_t *session){

    Signal *output;
    struct t_process proc;
    log_msg(LOG_DEBUG, 0, "Enter S6a State Machine");

    /*Create a new process to manage the S6a state machine. The older session handler is stored as parent
     * to return once the S6a state machine ends*/
    session->sessionHandler = process_create(engine, STATE_S6a_UpdateLocation, (void *)session, session->sessionHandler);

    output = new_signal(session->sessionHandler);
    /*output->data = (void *)session;*/
    /*output->name = S6a_getAuthVector;*/
    output->priority = MAXIMUM_PRIORITY;
    signal_send(output);
}

/* ====================================================================== */
