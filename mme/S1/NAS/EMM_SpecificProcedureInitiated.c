/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EMM_SpecificProcedureInitiated.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  EMM State
 *
 */

#include "EMM_SpecificProcedureInitiated.h"
#include "logmgr.h"
#include "EMM_FSMConfig.h"
#include "NAS.h"
#include "ECMSession_priv.h"
#include "NAS_EMM_priv.h"
#include <string.h>

static void emmProcessMsg(gpointer emm_h, GenericNASMsg_t* msg){
    log_msg(LOG_ERR, 0, "Not Implemented");
}

static void emm_processSecMsg(gpointer emm_h, gpointer buf, gsize len){
	EMMCtx_t *emm = (EMMCtx_t*)emm_h;
	GenericNASMsg_t msg;
	SecurityHeaderType_t s;
    ProtocolDiscriminator_t p;
    gboolean isAuth = FALSE, res;
	nas_getHeader(buf, len, &s, &p);
	log_msg(LOG_INFO, 0, "PING");
	if(emm->sci){
		res = nas_authenticateMsg(emm->parser, buf, len, NAS_UpLink, (uint8_t*)&isAuth);
		if(res==2){
			log_msg(LOG_WARNING, 0, "Wrong SQN Count");
			return;
		}else if(res==0){
			g_error("NAS Authentication Error");
		}
	}
	if(!isAuth){
		log_msg(LOG_INFO, 0, "Received Message with wrong MAC");
		return;
	}

	if(!dec_secNAS(emm->parser, &msg, NAS_UpLink, buf, len)){
		g_error("NAS Decyphering Error");
	}

	if(p == EPSSessionManagementMessages){
		esm_processMsg(emm->esm, &(msg.plain.eSM));
		return;
	}
}

static void emmAttachAccept(gpointer emm_h, gpointer esm_msg, gsize msgLen, GList *bearers){
	EMMCtx_t *emm = (EMMCtx_t*)emm_h;
	guint8 *pointer, out[256], plain[250], count, t3412, guti_b[11];
	guti_t guti;
	guint32 len;
    gsize tlen;
	NAS_tai_list_t tAIl;

	memset(out, 0, 156);
	memset(plain, 0, 150);
    pointer = plain;

	if (emm->attachStarted == TRUE){
		emm->attachStarted = FALSE;
		/* Build Attach Accept*/
		newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

		encaps_EMM(&pointer, AttachAccept);

		/* EPS attach result */
		nasIe_v_t1_l(&pointer, 1); /* EPS only */
		pointer++; /*Spare half octet*/
		/* T3412 value */
		t3412 = 0x23;
		nasIe_v_t3(&pointer, &t3412, 1); /* EPS only */
		/* TAI list */
		ecmSession_getTAIlist(emm->ecm, &tAIl, &tlen);
		nasIe_lv_t4(&pointer, (uint8_t*)&tAIl, tlen);
		/* ESM message container */
		nasIe_lv_t6(&pointer, esm_msg, msgLen);

		/* GUTI */
		emmCtx_newGUTI(emm, &guti);
		guti_b[0]=0xF6;   /*1111 0 110 - spare, odd/even , GUTI id*/
		memcpy(guti_b+1, &guti, 10);
		nasIe_tlv_t4(&pointer, 0x50, guti_b, 11);

		
		newNASMsg_sec(emm->parser, out, &len,
		              EPSMobilityManagementMessages,
		              IntegrityProtectedAndCiphered,
		              NAS_DownLink,
		              plain, pointer-plain);
		nas_incrementNASCount(emm->parser, NAS_DownLink);
	}else{
		return;
	}
	ecm_sendCtxtSUReq(emm->ecm, out, len, bearers);
	emmChangeState(emm, EMM_Registered);
}

void linkEMMSpecificProcedureInitiated(EMM_State* s){
    s->processMsg = emmProcessMsg;
    /* s->authInfoAvailable = emmAuthInfoAvailable; */
    s->attachAccept = emmAttachAccept;
    s->processSecMsg = emm_processSecMsg;
    s->sendESM = emm_internalSendESM;
}


