/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   NAS_FSM.h
 * @Author Vicent Ferrer
 * @date   May, 2013
 * @brief  NAS FSM header
 *
 * This module implements the NAS interface state machine on the MME EndPoint.
 */

#ifndef NAS_FSM_H
#define NAS_FSM_H

#include "MME.h"
#include "MME_engine.h"

#include <stdint.h>

/* ======================================================================
 * NAS MME State Machine API
 * ====================================================================== */


/**@brief NAS processing function
 * @param [out] returnbuffer return information send to the lower layer protocol
 * @param [out] bsize size of returnbuffer
 * @param [in]  pack buffer containing the received information from the lower layer protocol
 * @param [in]  size pack buffer size
 * @param [in]  user user structure to store the parsed information
 *
 *  This functions detects the used protocol and triggers the state processing function.
 */
void NAS_process(uint8_t *returnbuffer, uint32_t *bsize, void *msg, uint32_t size, Signal *signal);

/**@brief This function indicates Session is available
 * @param [out] returnbuffer return information send to the lower layer protocol
 * @param [out] bsize size of returnbuffer
 * @param [in]  signal signal structure
 *
 * Use this function when the lower layer has to fulfill any NAS container.
 * It trigguers the NAS state machine and returns the buffer to be send on the NAS container.
 * For example, this function indicates to the NAS layer when the E-RAB is available after the S11 Create Session Procedure is done.
 */
void NAS_sessionAvailable(uint8_t *returnbuffer, uint32_t *bsize, Signal *signal);

#endif /* NAS_FSM_H */
