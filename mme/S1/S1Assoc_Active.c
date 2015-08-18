/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S1Assoc_Active.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  S1 Assoc Active State
 *
 */

#include "S1Assoc_Active.h"
#include "logmgr.h"
#include "S1Assoc_priv.h"
#include "S1Assoc_FSMConfig.h"

static void processMsg(gpointer self, S1AP_Message_t *msg, int r_sid){
	log_msg(LOG_ERR, 0, "Not Implemented");
}


void linkS1AssocActive(S1Assoc_State* s){
	s->processMsg = processMsg;
}