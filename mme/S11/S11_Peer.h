/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */
/**
 * @file   S11_Peer.h
 * @Author Vicent Ferrer
 * @date   October, 2016
 * @brief  S11 peer mgmt.
 *
 */

#pragma once

#include <netinet/in.h>
#include <glib.h>

#include "gtp.h"
#include "timermgr.h"

typedef struct{
    gpointer        s11;
    TimerMgr        tm;
    Timer           t;
    struct sockaddr addr;
    socklen_t       len;
    guint32         num_sessions;
    gboolean        restartValid;
    guint8          restartCounter;
}Peer_t;

GHashTable *s11peer_buildTable();

void s11peer_destroyTable(GHashTable *peers);

/**
 *@brief Check if the peer has other sessions
 *@param [in]  peers
 *@param [in]  rAddr
 *@paran [in]  rAddrLen
 *@param [out] p         Peer struct
 *@return TRUE if the peer has other sessions
 *
 * p is filled with the peer struct, if it is the first time, it is created.
 * This function starts tracking the number of session active on that peer, use function
 * unrefSession after each detach.
 */
gboolean s11peer_isFirstSession(GHashTable *peers,
                                const struct sockaddr *rAddr,
                                const socklen_t rAddrLen,
                                Peer_t **p);

/**
 *@brief Update restart counter of peer and check if the peer has restarted
 *@param [in]  peers
 *@param [in]  rAddr
 *@paran [in]  rAddrLen
 *@param [in]  counter
 *@return TRUE if the peer has restarted
 *
 * This function is used also to update the restart counter.
 * The first update always returns FALSE as no previous counter is available.
 *
 */
gboolean s11peer_hasRestarted(GHashTable *peers,
                              const struct sockaddr *rAddr,
                              const socklen_t rAddrLen,
                              const guint8 counter);

Peer_t *s11peer_get(GHashTable *peers,
                    const struct sockaddr *rAddr,
                    const socklen_t rAddrLen);

void s11peer_track(Peer_t *p);


void s11peer_untrack(Peer_t *p);


void s11peer_processEchoRsp(GHashTable *peers,
                            const struct sockaddr *rAddr,
                            const socklen_t rAddrLen,
                            union gtp_packet *msg,
                            size_t msg_len);
