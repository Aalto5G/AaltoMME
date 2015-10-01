/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S1Assoc_NotConfigured.h
 * @Author Vicent Ferrer
 * @date   June, 2015
 * @brief  ESM State
 *
 */

#ifndef S1ASSOC_NOTCONFIGURED_HFILE
#define S1ASSOC_NOTCONFIGURED_HFILE

#include "S1Assoc_State.h"

typedef struct{
    S1STATE;
}S1Assoc_NotConfigured;

void linkS1AssocNotConfigured(S1Assoc_State* s);

#endif /* S1ASSOC_NOTCONFIGURED_HFILE */
