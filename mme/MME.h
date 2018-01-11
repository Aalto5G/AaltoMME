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

/**
 * @file   MME.h
 * @Author Vicent Ferrer
 * @date   March, 2013
 * @brief  MME type definition and functions.
 *
 * The goal of this file is to define the structures and generic functions of the MME and its interfaces.
 * Allowing to pass process between interfaces.
 */

#ifndef MME_HFILE
#define MME_HFILE

#include <stdint.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <time.h>
#include <arpa/inet.h>

/* Required after removing uthash */
#include <string.h>
#include <inttypes.h>

#include <event2/event.h>
#include <glib.h>

#include "Subscription.h"

#include "gtp.h"
#include "NAS_Definitions.h"
#include "S1AP.h"
#include "S6a.h"
#include "EMM_FSMConfig.h"
#include "timermgr.h"

#define MAX_UE 500000 /*< Max number of active users on this MME*/
#define FIRST_UE_SCTP_STREAM 1 /*< The minimum UE SCTP stream value*/


/* ====================================================================== */

typedef gpointer MME;

extern void mme_main();

extern MME mme_init();

extern void mme_free(MME mme);


struct t_message{
    union{
        uint8_t             raw[65547];
        union gtp_packet    gtp;
    }packet;                 ;  /*  data received as part of the message*/
    size_t                  length;         /*  Packet lenght*/
    struct sockaddr         peer;
    socklen_t               peerlen;
};


/*@TODO Node structure*/
struct mme_t{
    struct event_base       *evbase;
    struct event            *kill_event;                     /*< Kill Posix signal event*/
    TimerMgr                tm;
    MMEname_t               *name;
    char                    ipv4[INET_ADDRSTRLEN];
    char                    ipv6[INET6_ADDRSTRLEN]; /* Not used*/
    gchar                   *stateDir;
    ServedGUMMEIs_t         *servedGUMMEIs;
    RelativeMMECapacity_t   *relativeCapacity;
    gchar                   *s6a_db_host;
    gchar                   *s6a_db;
    gchar                   *s6a_db_user;
    gchar                   *s6a_db_passwd;
    GHashTable              *ev_readers;                     /*< Listener events accessed by socket*/
    gpointer                s6a;
    gpointer                s11;
    gpointer                s1;
    gpointer                cmd;
    gpointer                sdnCtrl;
    GHashTable              *s1_by_GeNBid;                   /**< S1 Associations By GlobaleNBid */
    GHashTable              *s1_localIDs;                    /**< Used MME UE S1AP IDs */
    GHashTable              *emm_sessions;                   /**< Store all EMM session of the MME */
    GHashTable              *ecm_sessions_by_localID;        /**< Store all ECM session of the MME */

    struct timeval          start;   /* Test Variable*/
    uint32_t                procTime;
};

extern int init_udp_srv(const char* src, int port);

extern int init_sctp_srv(const char *src, int port);

extern struct t_message *newMsg();
extern void freeMsg( void *msg);

extern void kill_handler(evutil_socket_t listener, short event, void *arg);

extern unsigned int newTeid();

extern uint32_t mme_newLocalUEid(struct mme_t *self);

extern void mme_freeLocalUEid(struct mme_t *self, uint32_t id);


/**************************************************/
/* Accessors                                      */
/**************************************************/

extern const ServedGUMMEIs_t *mme_getServedGUMMEIs(const struct mme_t *mme);

extern const char *mme_getLocalAddress(const struct mme_t *mme);

extern const char *mme_getStateDir(const struct mme_t *mme);

/**************************************************/
/* API towards state machines                     */
/**************************************************/



/**
 * @brief  Get timer manager
 * @param [in] self MME pointer
 * @return Timer manager
 *
 */
TimerMgr mme_getTimerMgr(struct mme_t *self);

/**
 * @brief Register a read callback for a socket
 * @param [in] self MME pointer
 * @param [in] fd   File descriptor to register
 * @param [in] cb   Callback invoked when the fd becomes active
 * @param [in] args Arguments used in the callback
 *
 * The socket needs to be deregistered before exiting the program with
 * the function mme_deregisterRead
 */
void mme_registerRead(struct mme_t *self, int fd,
                      event_callback_fn cb, void * args);

/**
 * @brief Deregister a read callback
 * @param [in] self MME pointer
 * @param [in] fd   File descriptor to register
 *
 * Deregisters a callback registered with mme_registerRead
 */
void mme_deregisterRead(struct mme_t *self, int fd);

struct event_base *mme_getEventBase(struct mme_t *self);


void mme_registerS1Assoc(struct mme_t *self, gpointer assoc);

void mme_deregisterS1Assoc(struct mme_t *self, gpointer assoc);

void mme_lookupS1Assoc(struct mme_t *self, gconstpointer geNBid, gpointer *assoc);


void mme_registerEMMCtxt(struct mme_t *self, gpointer emm);

void mme_deregisterEMMCtxt(struct mme_t *self, gpointer emm);

void mme_lookupEMMCtxt(struct mme_t *self, const guint32 m_tmsi, gpointer *emm);

void mme_lookupEMMCtxt_byIMSI(struct mme_t *self, const guint64 imsi, gpointer *emm);


void mme_registerECM(struct mme_t *self, gpointer ecm);

void mme_deregisterECM(struct mme_t *self, gpointer ecm);

void mme_lookupECM(struct mme_t *self, const guint32 id, gpointer *ecm);


gpointer mme_getS6a(struct mme_t *self);

gpointer mme_getS11(struct mme_t *self);

GList *mme_getS1Assocs(struct mme_t *self);

void mme_paging(struct mme_t *self, gpointer emm);

gboolean mme_GUMMEI_IsLocal(const struct mme_t *self,
                            const guint32 plmn,
                            const guint16 mmegi,
                            const guint8 mmec);

gboolean mme_containsSupportedTAs(const struct mme_t *self, SupportedTAs_t *tas);

#endif /* MME_HFILE */
