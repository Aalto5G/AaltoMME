/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
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
#include "ECMSession.h"
#include "ECMSession_State.h"

typedef struct{
    S1Assoc          assoc;        /**< Lower layer*/
    gpointer         emm;          /**< Higher layer*/
    ECMSession_State *state;       /**< FSM */
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
}ECMSession_t;


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

const guint8 *ecmSession_getServingNetwork_TBCD(const ECMSession h);

/**
 * @brief generate_KeNB - KDF function to derive the K_eNB
 * @param [in]  kasme       derived key - 256 bits
 * @param [in]  ulNASCount  Uplink NAS COUNT
 * @param [out] keNB        eNB result key - 256 bits
 */
void generate_KeNB(const uint8_t *kasme, const uint32_t ulNASCount, uint8_t *keNB);

gpointer ecmSession_getS6a(const ECMSession h);


#endif /* ECMSESSION_PRIV_HFILE */
