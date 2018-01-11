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
 * @file   ECMSession.h
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  ECM logic
 *
 * This Module implements the ECM session logic, the user associated messages
 * of S1AP
 */

#ifndef ECMSESSION_PRIV_HFILE
#define ECMSESSION_PRIV_HFILE

#include <glib.h>
#include "NAS.h"
#include "ECMSession.h"
#include "ECMSession_State.h"
#include "ECMSession_FSMConfig.h"

/*API HACK*/
typedef  E_RABSetupListCtxtSURes_t E_RABsToBeModified_t;

typedef struct{
    S1Assoc          assoc;        /**< Lower layer*/
    gpointer         emm;          /**< Higher layer*/
    ECMSession_State *state;       /**< FSM */
    ECMSessionState  stateName;
    guint32          l_sid;        /**< SCTP local  Stream ID*/
    gboolean         r_sid_valid;  /**< SCTP remote sid valid flag*/
    guint32          r_sid;        /**< SCTP remote Stream ID*/
    guint32          mmeUEId;      /**< S1AP MME UE ID*/
    guint32          eNBUEId;      /**< S1AP eNB UE ID*/

    struct{
      guint16    MCC;			/**< Mobile Country Code*/
      guint16    MNC;			/**< Mobile Network Code*/
      guint8     sn[3];          /**< TA TBCD encoded*/
      guint16    tAC;			/**< Tracking Area Code*/
    }                tAI;     /**< TAI Tracking Area ID*/

    struct{
      guint16    MCC;
      guint16    MNC;
      guint32    cellID:28;
    }                eCGI;    /**< E-UTRAN CGI (Cell Global ID)*/
    Cause_t     *causeRelease;
}ECMSession_t;

#define ecm_log(self, p, en, ...) ecm_log_(self, p, __FILE__, __func__, __LINE__, en, __VA_ARGS__)

void ecm_log_(ECMSession ecm, int pri, char *fn, const char *func, int ln,
              int en, char *fmt, ...);

void ecmSession_setState(ECMSession ecm, ECMSession_State *s, ECMSessionState name);

/* API to NAS */

/**@brief S1 Send message
 * @param [in] h        ECM Session handler
 * @param [in] msg      Message buffer pointer
 * @param [in] len      Lenght of the message buffer
 *
 * This function sends the downlinkNASTransport S1AP message
 * to forward a NAS message
 * */
void ecm_send(ECMSession h, gpointer msg, size_t len);

void ecm_sendCtxtSUReq(ECMSession h, gpointer msg, size_t len, GList *bearers);

const guint8 *ecmSession_getServingNetwork_TBCD(const ECMSession h);

gpointer ecmSession_getS6a(const ECMSession h);

gpointer ecmSession_getS11(const ECMSession h);

void ecmSession_getTAIlist(const ECMSession h, NAS_tai_list_t *tail, gsize *len);

void ecmSession_getTAI(const ECMSession h, guint8 *sn, guint16 *tac);

void ecmSession_getGUMMEI(const ECMSession h, guint32* sn, guint16 *mmegi, guint8 *mmec);

void ecm_sendUEContextReleaseCommand(const ECMSession h, cause_choice_t choice, uint32_t cause);

void ecmSession_setEMM(ECMSession h, gpointer emm);

void ecmSession_newGUTI(ECMSession h, guti_t *guti);


#endif /* ECMSESSION_PRIV_HFILE */
