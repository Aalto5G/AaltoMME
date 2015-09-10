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
    size=-2;

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

void dec_NAS(GenericNASMsg_t *msg, const uint8_t *buf, const uint32_t size){
    uint8_t const *pointer;
    pointer = buf;
    msg->header.protocolDiscriminator.v = (*pointer)&0x0F;
    msg->header.protocolDiscriminator.s = 0;
    msg->header.securityHeaderType.v = ((*pointer)&0xF0)>>4;
    msg->header.securityHeaderType.s = 0;
    pointer++;

    nas_msg(NAS_DEBUG, 0,"DEC : protocolDiscriminator = %x, securityHeaderType = %x",
            msg->header.protocolDiscriminator.v, msg->header.securityHeaderType.v);

    if( (ProtocolDiscriminator_t)msg->header.protocolDiscriminator.v == EPSMobilityManagementMessages &&
            (SecurityHeaderType_t)msg->header.securityHeaderType.v != PlainNAS){
        memcpy(msg->ciphered.messageAuthCode, pointer,4);
        pointer+=4;
        msg->ciphered.sequenceNum = *pointer;
        pointer++;
        msg->ciphered.msg = pointer;
        msg->ciphered.len = size-6;
        return;
    }else{
        if((ProtocolDiscriminator_t)msg->header.protocolDiscriminator.v == EPSMobilityManagementMessages){
            dec_EMM(&(msg->plain.eMM), pointer, size-1);

        }else if((ProtocolDiscriminator_t)msg->header.protocolDiscriminator.v == EPSSessionManagementMessages){
            dec_ESM(&(msg->plain.eSM), pointer, size-1);
        }else{
            nas_msg(NAS_ERROR, 0, "Protocol discriminator not recognized: %u", msg->header.protocolDiscriminator.v);
            msg=NULL;
            return;
        }
    }
}


/* ***** Encoding functions ***** */


void newNASMsg_EMM(uint8_t **curpos, ProtocolDiscriminator_t protocolDiscriminator, SecurityHeaderType_t securityHeaderType){
    if(curpos == NULL){
        nas_msg(NAS_ERROR, 0,"ENC : buffer parameter not allocated");
        return;
    }
    nasIe_v_t1_l(curpos, protocolDiscriminator);
    nasIe_v_t1_h(curpos, securityHeaderType);
}

void newNASMsg_ESM(uint8_t **curpos, ProtocolDiscriminator_t protocolDiscriminator, uint8_t ePSBearerId){
    if(curpos == NULL){
        nas_msg(NAS_ERROR, 0,"ENC : buffer parameter not allocated");
        return;
    }
    nasIe_v_t1_l(curpos, protocolDiscriminator);
    nasIe_v_t1_h(curpos, ePSBearerId);
}

void encaps_ESM(uint8_t **curpos, ProcedureTransactionId_t procedureTransactionIdentity, NASMessageType_t messageType){
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

void nas_setSecurity(NAS h,
                     NAS_EIA i, const uint8_t *ikey,
                     NAS_EEA e, const uint8_t *ekey,
                     void (*cb)(void*), void* user_data){

    NASHandler *n = (NASHandler*)h;

    n->i = i;
    n->e = e;

    if(ikey)
        memcpy(n->ikey, ikey, 16);
    if(ekey)
        memcpy(n->ekey, ekey, 16);

    n->nas_count[0] = 0;
    n->nas_count[1] = 0;

    if(cb)
        n->countOverflow = cb;
    if(user_data)
        n->udata = user_data;

    return;
}

int nas_getHeader(const uint8_t *buf, const uint32_t size,
                   SecurityHeaderType_t *s, ProtocolDiscriminator_t *p){
    NAS_Header_t h;
    if(!buf || size<1){
        return 0;
    }
    memcpy(&h, buf, 1);
    if(s)
        *s = h.securityHeaderType.v;
    if(p)
        *p = h.protocolDiscriminator.v;
    return 1;
}

int nas_authenticateMsg(const NAS h,
                        const uint8_t *buf, const uint32_t size,
                        const uint8_t direction, uint8_t *isAuth){
    SecurityHeaderType_t s;
    NASHandler *n = (NASHandler*)h;

    nas_getHeader(buf, size, &s, NULL);
    isAuth = 0;
    uint8_t const *pointer;
    uint8_t mac[4], mac_x[4], nas_sqn;
    size_t len;
    if( s == PlainNAS ){
        return 0;
    }else if(s == IntegrityProtected ||
             s == IntegrityProtectedAndCiphered ||
             s == IntegrityProtectedWithNewEPSSecurityContext ||
             s == IntegrityProtectedAndCipheredWithNewEPSSecurityContext ){
        /*Check NAS SQN*/
        nas_sqn = buf[5];
        if(n->nas_count[direction]&0xff != nas_sqn){
            *isAuth = 0;
            return 1;
        }
        /*Calculate and validate MAC*/
        memcpy(mac, buf+1, 4);

        pointer = buf + 5;
        len = size - 5;
    }else if(s == SecurityHeaderForServiceRequestMessage){
        return 0;
        /*Non-standard L3 message*/
    }else{
        return 0;
    }
}

int dec_secNAS(const NAS h,
                GenericNASMsg_t *msg,
                const uint8_t *buf, const uint32_t size){
    GenericNASMsg_t tmp;
    uint8_t const *pointer;
    NASHandler *n = (NASHandler*)h;

    pointer = buf;
    msg->header.protocolDiscriminator.v = (*pointer)&0x0F;
    msg->header.protocolDiscriminator.s = 0;
    msg->header.securityHeaderType.v = ((*pointer)&0xF0)>>4;
    msg->header.securityHeaderType.s = 0;
    pointer++;

    nas_msg(NAS_DEBUG, 0,"DEC : protocolDiscriminator = %x, securityHeaderType = %x",
            msg->header.protocolDiscriminator.v, msg->header.securityHeaderType.v);

    if((ProtocolDiscriminator_t)msg->header.protocolDiscriminator.v == EPSMobilityManagementMessages &&
       (SecurityHeaderType_t)msg->header.securityHeaderType.v != PlainNAS){
        memcpy(msg->ciphered.messageAuthCode, pointer,4);
        pointer+=4;
        msg->ciphered.sequenceNum = *pointer;
        pointer++;
        msg->ciphered.msg = pointer;
        msg->ciphered.len = size-6;
        return;
    }else{
        dec_NAS(msg, buf, size);
    }
}
