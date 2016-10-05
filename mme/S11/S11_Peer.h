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

typedef struct{
    struct sockaddr addr;
    socklen_t       len;
    guint32         num_sessions;
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
 * p is filled with the peer struct, if it is the first time, it is created
 */
gboolean s11peer_isFirstSession(GHashTable *peers,
                                const struct sockaddr *rAddr,
                                const socklen_t rAddrLen,
                                Peer_t *p);

gboolean s11peer_hasRestarted(GHashTable *peers,
                              const struct sockaddr *rAddr,
                              const socklen_t rAddrLen,
                              guint8 counter);
