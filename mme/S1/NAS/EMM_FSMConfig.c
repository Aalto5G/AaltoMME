/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EMM_FSMConfig.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  Functions to modify EMM FSM
 *
 */


#include "EMM_FSMConfig.h"

#include "EMM_Deregistered.h"
#include "EMM_Registered.h"
#include "EMM_SpecificProcedureInitiated.h"
#include "EMM_CommonProcedureInitiated.h"
#include "EMM_DeregisteredInitiated.h"

#include "ESM_FSMConfig.h"

#include "logmgr.h"

EMM_State *emm_states;

const char *EMMStateName[] = {"Deregistered",
                              "Registered",
                              "SpecificProcedureInitiated",
                              "CommonProcedureInitiated",
                              "DeregisteredInitiated"};

void emmConfigureFSM(){
    emm_states = g_new(EMM_State, 5);

    esmConfigureFSM();

    linkEMMDeregistered(&emm_states[0]);
    linkEMMRegistered(&emm_states[1]);
    linkEMMSpecificProcedureInitiated(&emm_states[2]);
    linkEMMCommonProcedureInitiated(&emm_states[3]);
    linkEMMDeregisteredInitiated(&emm_states[4]);
}

void emmDestroyFSM(){
	esmDestroyFSM();
    g_free(emm_states);
}


void emmChangeState(gpointer ctx, EMMState s){
    log_msg(LOG_DEBUG, 0, "Change to EMM %s", EMMStateName[s]);
    emm_setState(ctx, &(emm_states[s]));
}

void emmNotImplemented(gpointer self){
    log_msg(LOG_ERR, 0, "Not Implemented");
}
