/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   NAS.c
 * @Author Vicent Ferrer
 * @date   May, 2013
 * @brief  NAS
 */

#include "NAS.h"
#include "NASlog.h"
#include "NASHandler.h"
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>     /*htonl*/
#include <openssl/hmac.h>

/* ***** Decoding functions ***** */


void dec_EMM(EMM_Message_t *msg, uint8_t *buf, uint32_t size){

    msg->messageType = *(buf++);
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : messageType = %#x", msg->messageType);

    switch((NASMessageType_t)msg->messageType){
    case AttachRequest:
        dec_AttachRequest((AttachRequest_t*)msg, buf, size);
        break;
    case AttachAccept:
        dec_AttachAccept((AttachAccept_t*)msg, buf, size);
        break;
    case AttachComplete:
        dec_AttachComplete((AttachComplete_t*)msg, buf, size);
        break;
    case AttachReject:
        dec_AttachReject((AttachReject_t*)msg, buf, size);
        break;
    case DetachRequest:
        dec_DetachRequestUEOrig((DetachRequestUEOrig_t*)msg, buf, size);
        break;
    case DetachAccept:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case TrackingAreaUpdateRequest:
        dec_TrackingAreaUpdateRequest((TrackingAreaUpdateRequest_t*)msg, buf, size);
        break;
    case TrackingAreaUpdateAccept:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case TrackingAreaUpdateComplete:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case TrackingAreaUpdateReject:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case ExtendedServiceRequest:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case ServiceReject:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case GUTIReallocationCommand:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case GUTIReallocationComplete:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case AuthenticationRequest:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case AuthenticationResponse:
        dec_AuthenticationResponse((AuthenticationResponse_t*)msg, buf, size);
        break;
    case AuthenticationReject:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case AuthenticationFailure:
        dec_AuthenticationFailure((AuthenticationFailure_t*)msg, buf, size);
        break;
    case IdentityRequest:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case IdentityResponse:
        dec_IdentityResponse((IdentityResponse_t*)msg, buf, size);
        break;
    case SecurityModeCommand:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case SecurityModeComplete:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case SecurityModeReject:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case EMMStatus:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case EMMInformation:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case DownlinkNASTransport:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case UplinkNASTransport:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case CSServiceNotification:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case DownlinkGenericNASTransport:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case UplinkGenericNASTransport:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    default:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    }
}

void dec_ESM(ESM_Message_t *msg, uint8_t *buf, uint32_t size){

    msg->procedureTransactionIdentity = *(buf++);
    msg->messageType = *(buf++);
    size-=2;

    nas_msg(NAS_DEBUG, 0, "DEC : procedureTransactionIdentity %#x, messageType %#x",
            msg->procedureTransactionIdentity, msg->messageType);

    switch((NASMessageType_t)msg->messageType){
    case ActivateDefaultEPSBearerContextRequest:
        dec_ActivateDefaultEPSBearerContextAccept((ActivateDefaultEPSBearerContextAccept_t*)msg, buf, size);
        break;
    case ActivateDefaultEPSBearerContextAccept:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case ActivateDefaultEPSBearerContextReject:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case ActivateDedicatedEPSBearerContextRequest:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case ActivateDedicatedEPSBearerContextAccept:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case ActivateDedicatedEPSBearerContextReject:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case ModifyEPSBearerContextRequest:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case ModifyEPSBearerContextAccept:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case ModifyEPSBearerContextReject:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case DeactivateEPSBearerContextRequest:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case DeactivateEPSBearerContextAccept:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case PDNConnectivityRequest:
        dec_PDNConnectivityRequest((PDNConnectivityRequest_t*)msg, buf, size);
        break;
    case PDNConnectivityReject:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case PDNDisconnectRequest:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case PDNDisconnectReject:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case BearerResourceAllocationRequest:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case BearerResourceAllocationReject:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case BearerResourceModificationRequest:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case BearerResourceModificationReject:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case ESMInformationRequest:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case ESMInformationResponse:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case Notification:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    case ESMStatus:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    default:
        nas_msg(NAS_DEBUG, 0, "DEC : messageType decoder Not available: messageType %#x", msg->messageType);
        break;
    }
}

int dec_NAS(GenericNASMsg_t *msg, const uint8_t *buf, const uint32_t size){
    uint8_t const *pointer;
    ProtocolDiscriminator_t p;
    SecurityHeaderType_t s;

    if(!nas_getHeader(buf, size, &s, &p))
        return 0;

    pointer = buf;
    msg->header.protocolDiscriminator.v = p;
    msg->header.protocolDiscriminator.s = 0;
    msg->header.securityHeaderType.v = s;
    msg->header.securityHeaderType.s = 0;
    pointer++;

    nas_msg(NAS_DEBUG, 0,"DEC : protocolDiscriminator = %x,"
            " securityHeaderType = %x", p, s);

    if(s != PlainNAS ||
       !(p != EPSSessionManagementMessages || p != EPSMobilityManagementMessages)){
        return 0;
    }

    if(p == EPSMobilityManagementMessages){
        dec_EMM(&(msg->plain.eMM), pointer, size-1);
        return 1;
    }

    dec_ESM(&(msg->plain.eSM), pointer, size-1);
    return 1;
}


/* ***** Encoding functions ***** */


void newNASMsg_EMM(uint8_t **curpos,
                   ProtocolDiscriminator_t protocolDiscriminator,
                   SecurityHeaderType_t securityHeaderType){
    if(curpos == NULL){
        nas_msg(NAS_ERROR, 0,"ENC : buffer parameter not allocated");
        return;
    }
    nasIe_v_t1_l(curpos, protocolDiscriminator);
    nasIe_v_t1_h(curpos, securityHeaderType);
}

void newNASMsg_ESM(uint8_t **curpos,
                   ProtocolDiscriminator_t protocolDiscriminator,
                   uint8_t ePSBearerId){
    if(curpos == NULL){
        nas_msg(NAS_ERROR, 0,"ENC : buffer parameter not allocated");
        return;
    }
    nasIe_v_t1_l(curpos, protocolDiscriminator);
    nasIe_v_t1_h(curpos, ePSBearerId);
}

int newNASMsg_sec(const NAS h,
                  uint8_t *out, uint32_t *len,
                  const ProtocolDiscriminator_t p,
                  const SecurityHeaderType_t s,
                  const NAS_Direction direction,
                  const uint8_t *plain, const uint32_t pLen){

    uint8_t buf[pLen+1], count[4], mac[4], *pointer;
    uint32_t ncount;
    size_t cLen = 0;
    NASHandler *n = (NASHandler*)h;

    if(!(s == IntegrityProtected ||
         s == IntegrityProtectedAndCiphered ||
         s == IntegrityProtectedWithNewEPSSecurityContext ||
         s == IntegrityProtectedAndCipheredWithNewEPSSecurityContext)){
        /*We shouldn't be here*/
        return 0;
    }

    ncount = htonl(n->nas_count[direction]);
    memcpy(count, &ncount, 4);
    memset(mac, 0, 4);
    *len = 0;
    pointer = out;

    /* Cypher Message*/
    if(s == IntegrityProtectedAndCiphered ||
       s == IntegrityProtectedAndCipheredWithNewEPSSecurityContext){
        /* out+6 instead of buf+1 ?*/
        eea_cyph_cb[n->e](n->ekey, count, 0, direction, buf+1, &cLen, plain, pLen);
        if(pLen != cLen){
            return 0;
        }
    }else{
        memcpy(buf+1, plain, pLen);
    }

    /* NAS SQN*/
    buf[0] = count[3];

    /* Calculate MAC*/
    eia_cb[n->i](n->ikey, count, 0, direction, buf, (pLen +1)*8, mac);

    /* Encode new Message with Security header*/
    newNASMsg_EMM(&pointer, p, s);
    /* Add message authentication code*/
    nasIe_v_t3(&pointer, mac, 4);
    /* Add sequence number and cyphered message */
    nasIe_v_t3(&pointer, buf, pLen + 1);
    *len = pointer - out;
    /* Increment downlink counter*/
    nas_incrementNASCount(n, NAS_DownLink);
    return 1;
}


void encaps_ESM(uint8_t **curpos,
                ProcedureTransactionId_t procedureTransactionIdentity,
                NASMessageType_t messageType){
    nasIe_v_t3(curpos, (uint8_t*)&procedureTransactionIdentity, 1);
    nasIe_v_t3(curpos, (uint8_t*)&messageType, 1);
}

void encaps_EMM(uint8_t **curpos, NASMessageType_t messageType){
    nasIe_v_t3(curpos, (uint8_t*)&messageType, 1);
}

/* Tool functions*/

uint32_t encapPLMN(uint16_t mcc, uint16_t mnc){
    uint32_t tbcd;
    uint8_t tmp[3];
    tmp[0] = ((mcc%100)/10<<4) | mcc/100;
    tmp[1] = (mcc%10);
    if(mnc/100 == 0){
        tmp[1]|=0xf0;
        tmp[2] = (mnc/10) | (mnc%10)<<4;
    }else{
        tmp[1]|= (mnc%10)<<4;
        tmp[2] = ((mnc%100)/10<<4) | mnc/100;
    }
    memcpy(&tbcd, tmp, 3);
    return tbcd;
}

NAS nas_newHandler(){
    NASHandler *n = (NASHandler*)malloc(sizeof(NASHandler));
    memset(n, 0, sizeof(NASHandler));
    return n;
}

void nas_freeHandler(NAS h){
    NASHandler *n = (NASHandler*)h;
    free(n);
    return;
}

static void hmac_sha256(const unsigned char *key, unsigned int key_size,
                        const unsigned char *message, unsigned int message_len,
                        unsigned char *mac, unsigned mac_size)
{
    uint8_t res[EVP_MAX_MD_SIZE];
    uint32_t len;
    HMAC_CTX ctx;

    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key, key_size, EVP_sha256(), NULL);
    HMAC_Update(&ctx, message, message_len);
    HMAC_Final(&ctx, res, &len);
    HMAC_CTX_cleanup(&ctx);
    /* Use least significant bits */
    memcpy(mac, res + len - mac_size, mac_size);
}

/**
 * @brief Key derivation function
 * @param [in]  kasme          derived key - 256 bits
 * @param [in]  distinguisher  Algorithm distinguisher
 * @param [in]  algId          Algorithm identity
 * @param [out] k              Derived Key - 128 bits
 */
static void kdf(const uint8_t *kasme,
                const uint8_t distinguisher, const uint8_t algId,
                uint8_t *k){

    /*
    FC = 0x15,
    P0 = algorithm type distinguisher,
    L0 = length of algorithm type distinguisher (i.e. 0x00 0x01)
    P1 = algorithm identity
    L1 = length of algorithm identity (i.e. 0x00 0x01)
     */
    uint8_t s[7];
    s[0]=0x15;
    s[1]=distinguisher;
    s[2]=0x00;
    s[3]=0x01;
    s[4]=algId;
    s[5]=0x00;
    s[6]=0x01;

    hmac_sha256(kasme, 32, s, 7, k, 16);
}

void nas_setSecurity(NAS h, const NAS_EIA i, const NAS_EEA e,
                     const uint8_t *kasme){

    NASHandler *n = (NASHandler*)h;

    n->i = i;
    n->e = e;

    kdf(kasme, 0x02, i, n->ikey);
    kdf(kasme, 0x01, e, n->ekey);

    n->nas_count[0] = 0;
    n->nas_count[1] = 0;
    n->isValid = 1;
    return;
}

int nas_getHeader(const uint8_t *buf, const uint32_t size,
                   SecurityHeaderType_t *s, ProtocolDiscriminator_t *p){
    if(!buf || size<1){
        return 0;
    }
    if(s)
        *s = (buf[0]&0xf0)>>4;
    if(p)
        *p = buf[0]&0x0f;
    return 1;
}

const uint32_t nas_getLastCount(const NAS h, const NAS_Direction direction){
    NASHandler *n = (NASHandler*)h;
    /* -1 because we store the expected count message (UpLink), or the next count
     * to be used in Downlink*/
    return n->nas_count[direction]-1;
}

int nas_incrementNASCount(const NAS h, const NAS_Direction direction){
    NASHandler *n = (NASHandler*)h;
    if(!n->isValid)
        return 0;

    nas_msg(NAS_DEBUG, 0, "Increment counter for direction %u from %u",
            direction,
            n->nas_count[direction]);

    n->nas_count[direction]++;
    if(n->nas_count[direction] > 0xffffff-5){
        return 0;
    }
    return 1;
}

int nas_authenticateMsg(const NAS h,
                        const uint8_t *buf, const uint32_t size,
                        const NAS_Direction direction, uint8_t *isAuth){
    SecurityHeaderType_t s;
    NASHandler *n = (NASHandler*)h;
    uint8_t count[4], res = 0;
    uint32_t ncount;

    nas_getHeader(buf, size, &s, NULL);

    *isAuth = 0;
    if(!n->isValid)
        return 0;

    uint8_t mac[4], mac_x[4], nas_sqn;
    if( s == PlainNAS ){
        return 0;
    }

    if(s == IntegrityProtected ||
       s == IntegrityProtectedAndCiphered ||
       s == IntegrityProtectedWithNewEPSSecurityContext ||
       s == IntegrityProtectedAndCipheredWithNewEPSSecurityContext ){
        /*Check NAS SQN*/
        nas_sqn = buf[5];
        if((n->nas_count[direction]&0xff) != nas_sqn){
            return 2;
        }
        /*Calculate and validate MAC*/
        memcpy(mac, buf+1, 4);
        ncount = htonl(n->nas_count[direction]);
        memcpy(count, &ncount, 4);
        eia_cb[n->i](n->ikey, count, 0, direction, buf+5, (size-5)*8, mac_x);

        if(memcmp(mac, mac_x, 4) == 0 || n->i == NAS_EIA0){
            *isAuth = 1;
            nas_incrementNASCount(n, NAS_UpLink);
        }
        return 1;
    }else if(s == SecurityHeaderForServiceRequestMessage){
        return 0;
        /*Non-standard L3 message*/
    }else{
        return 0;
    }
}

int dec_secNAS(const NAS h,
               GenericNASMsg_t *msg, const NAS_Direction direction,
               const uint8_t *buf, const uint32_t size){

    SecurityHeaderType_t s;
    NASHandler *n = (NASHandler*)h;
    uint8_t plain[size], count[4];
    size_t len;
    uint32_t ncount;

    nas_getHeader(buf, size, &s, NULL);

    if(s ==  IntegrityProtected){
        dec_NAS(msg, buf+6, len-6);
        return 1;
    }else if( ! (s == IntegrityProtectedAndCiphered ||
                 s == IntegrityProtectedAndCipheredWithNewEPSSecurityContext ||
                 s == SecurityHeaderForServiceRequestMessage)){
        return 0;
    }

    /* Keep it here, so it can decode IntegrityProtected Messages
     * without a valid context. The context is only checked when decyphering
     * is required. */
    if(!n->isValid)
        return 0;

    ncount = htonl(n->nas_count[direction]);
    memcpy(count, &ncount, 4);
    eea_dec_cb[n->e](n->ekey, count, 0, direction,
                     buf + 6, size - 6,
                     plain, &len);

    return dec_NAS(msg, plain, len);
}
