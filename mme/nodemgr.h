/* AaltoMME - Mobility Management Entity for LTE networks
 * Copyright (C) 2013 Vicent Ferrer Guash & Jesus Llorente Santos
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**@file   nodemgr.h
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  Node information manager
 *
 * API to store and acquire node (eNB, SGW, PGW, other MME) information (connection status, IP addr, name)
 */

#ifndef NODEMGR_HFILE
#define NODEMGR_HFILE

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <glib.h>
#include <stdbool.h>
#include "MME.h"
#include "Subscription.h"

#define MAX_HOST_NAME 30

#define MME_CONFIG conf_quark()

GQuark conf_quark();

/*connection status enum*/
enum nodeStatus{
    down,
    up,
    busy
};

/*Node type enum*/
enum nodeType{
    invalid = -1,
    node_MME,
    eNB,
    SGW,
    PGW,
    UE,
    CTRL
};

/*Possible IPv6 useful doc: http://long.ccaba.upc.es/long/045Guidelines/eva/ipv6.html*/
/*Struct to store node information*/
struct nodeinfo_t{
    char            name[MAX_HOST_NAME];    /*name*/
    struct in_addr  addrv4;                 /*IP addr v4*/
    struct in6_addr addrv6;                 /*IP addr v4*/
    enum nodeStatus status;                 /*connection status*/
    enum nodeType   type;
};

/*Public API*/

/**@brief Init node manager structures
 * @returns false when error
 * Used to store new node information or to refresh a existing one. */
extern bool init_nodemgr();

/**@brief Free node manager structures
 * Used to store new node information or to refresh a existing one. */
extern void free_nodemgr();

/**@brief Acquire node information
 * @param [out] node Return a valid SGW information struct
 * @param [in]  node type to be returned
 *
 * Used to acquire a random node from the stored ones.
 * TODO @ Vicent Use user info
 */
extern void getNode(struct nodeinfo_t *node, const enum nodeType type, Subscription subs);

/**@brief Save node information
 * @param [in] node node info to be stored
 *
 * Used to store new node information or to refresh a existing one.
 */
extern void saveNode(const struct nodeinfo_t *node);

/**@brief Get Node by name
 * @param [in] name Node name
 * @param [in] type Node type
 * @param [out] node information
 */
extern void getNodeByName(const char *name, const enum nodeType type, struct nodeinfo_t *node);

/**@brief Get Node by addr v4
 * @param [in] addr IPv4 addr
 * @param [in] type Node type
 * @param [out] node information
 */
extern void getNodeByAddr4(const struct in_addr *addr, const enum nodeType type, struct nodeinfo_t *node);

/**@brief Get Node by addr v6
 * @param [in] addr IPv4 addr
 * @param [in] type Node type
 * @param [out] node information
 */
extern void getNodeByAddr6(const struct in6_addr *addr, const enum nodeType type, struct nodeinfo_t *node);

/**@brief Load MME information from config file
 * @param [in] mme  MME structure reference
 */
extern void loadMMEinfo(struct mme_t *mme, GError **err);

/**@brief free MME information from config file
 * @param [in] mme  MME structure reference
 */
extern void freeMMEinfo(struct mme_t *mme);

#endif /* NODEMGR_HFILE */
