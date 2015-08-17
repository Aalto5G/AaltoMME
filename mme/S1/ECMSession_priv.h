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

typedef struct{
	guint32  l_sid;   /**< SCTP local  Stream ID*/
	guint32  r_sid;   /**< SCTP remote Stream ID*/
	guint32  mmeUEId; /**< S1AP MME UE ID*/
	guint32  eNBUEId; /**< S1AP eNB UE ID*/

	struct{
	  guint16		MCC;			/**< Mobile Country Code*/
	  guint16		MNC;			/**< Mobile Network Code*/
	  guint8        sn[3];          /**< TA TBCD encoded*/
	  guint16		tAC;			/**< Tracking Area Code*/
	}tAI; /**< TAI Tracking Area ID*/
	
	struct{
	  guint16		MCC;
	  guint16		MNC;
	  guint32		cellID:28;
	}eCGI; /**< E-UTRAN CGI (Cell Global ID)*/
}ECMSession_t;

#endif /* ECMSESSION_PRIV_HFILE */
