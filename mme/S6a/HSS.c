/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   HSS.c
 * @Author Vicent Ferrer
 * @date   June, 2013
 * @brief  Functions to access to HSS database
 *
 * The current database is a MariaDB.
 */

#include "HSS.h"
#include "logmgr.h"
#include "SQLqueries.h"
#include "MME.h"
#include "EMMCtx.h"
#include "hmac_sha2.h"
#include "milenage.h"

#include <mysql.h>
#include <stdlib.h>

/* HSS connection global variable */
MYSQL                   *HSSConnection;

G_DEFINE_QUARK(diameter, diameter);


static char *bin_to_strhex(uint8_t *hexbuf, uint32_t size, char *result){
    char          hex_str[]= "0123456789abcdef";
    unsigned int  i;

    if (!size)
        return NULL;

    if(hexbuf == NULL || result == NULL)
        log_msg(LOG_ERR, 0, "An input buffer was null. hexbuf = %x, result= %x", hexbuf, result);

    result[size * 2] = 0;

    for (i = 0; i < size; i++){
        result[i * 2 + 0] = hex_str[hexbuf[i] >> 4  ];
        result[i * 2 + 1] = hex_str[hexbuf[i] & 0x0F];
    }
    return result;
}

void increaseSQN(uint8_t *sqn_b){
    uint64_t sqn, sqn_tmp;
    uint8_t str_sqn[6*2+1];
    int j;
    /*Increase SQN*/
    /* Using annex C.1.1.2 of TS 33.102
     * Generation of sequence numbers which are not time-based*/

    bin_to_strhex(sqn_b,6,  str_sqn);
    sqn = strtoll(str_sqn, NULL, 16);
    //sqn = ((sqn/32+1)<<5)|(sqn%32); /* SQN_HE = SEQ_HE (43 bits)|| IND_HE (5 bits)*/
    /* Copy back to buffer*/
    /*sqn = htobe64(sqn<<16);
      memcpy(sqn_b, &sqn, 6);*/
    sqn+=0x20;
    sqn_tmp = sqn;
    for(j=5; j>=0;j--){
        sqn_b[j] = sqn_tmp&0xFF;
        sqn_tmp>>=8;
    }
}

int init_hss(const char *host, const char *db, const char *usr, const char *pw){
    MYSQL      *MySQLConRet;
    int reconnect;

    /*SQL */
    HSSConnection = NULL;

    if (mysql_library_init(0, NULL, NULL)) {
        log_msg(LOG_ERR, 0, "could not initialize MySQL library");
        return 1;
    }

    HSSConnection = mysql_init( NULL );
    if(HSSConnection == NULL){
        log_msg(LOG_ERR, mysql_errno(HSSConnection), "%s", mysql_error(HSSConnection));
        disconnect_hss();
        return 1;
    }

    MySQLConRet = mysql_real_connect( HSSConnection, host, usr, pw, db,
                                      0, NULL, CLIENT_MULTI_STATEMENTS );
    if ( MySQLConRet == NULL || MySQLConRet != HSSConnection){
        log_msg(LOG_ERR, mysql_errno(HSSConnection), "%s. Disconnecting. Handler %x", mysql_error(HSSConnection), HSSConnection);
        disconnect_hss();
        return 1;
    }

    reconnect = 1;
    mysql_options(HSSConnection, MYSQL_OPT_RECONNECT, &reconnect);
    return 0;
}

void disconnect_hss(){

    /* Close datbase connection*/
    if(HSSConnection!= NULL){
        log_msg(LOG_DEBUG, 0, "Disconnecting from database, handler: %#x", HSSConnection);
        mysql_close(HSSConnection);
        HSSConnection=NULL;
    }else{
        log_msg(LOG_ERR, 0, "Error while trying to disconnect MySQL. Handler not available");
    }

    mysql_library_end();
}

/* ============================================================== */

/**
 * get_random - Get cryptographically strong pseudo random data
 * @buf: Buffer for pseudo random data
 * @len: Length of the buffer
 * Returns: 0 on success, -1 on failure
 *
 * Orignal function from hostapd:
 * http://svn0.us-east.freebsd.org/base/vendor/hostapd/0.5.8/contrib/hostapd/os_unix.c
 */
static int get_random(unsigned char *buf, size_t len)
{
    FILE *f;
    size_t rc;

    f = fopen("/dev/urandom", "rb");
    if (f == NULL) {
        log_msg(LOG_ERR, errno, "Could not open /dev/urandom.");
        return -1;
    }

    rc = fread(buf, 1, len, f);
    fclose(f);

    return rc != len ? -1 : 0;
}

/**
 * @brief generate_Kasme - KDF function to derive the K_ASME
 * @param [in]  ck      Cipher Key - 128 bits
 * @param [in]  ik      Integrity Key - 128 bits
 * @param [in]  sn      Serving Network, it is the Tracking area encoded with TBCD
 * @param [in]  sqnAk   XOR of SQN and Ak - 48 bits
 * @param [out] kasme   derived key - 256 bits
 */
static void generate_Kasme(const uint8_t *ck, const uint8_t *ik, const uint8_t *sn, const uint8_t *sqnAk, uint8_t *kasme){
    uint8_t k[32], s[14];

    /*
    -    FC = 0x10,
    -    P0 = SN id,
    -    L0 = length of SN id (i.e. 0x00 0x03),
    -    P1 = SQN ^ AK
    -    L1 = length of SQN ⊕ AK (i.e. 0x00 0x06)

    S = FC || P0 || L0 || P1 || L1
    KEY = Ck || Ik

    derived key = HMAC-SHA-256 ( Key , S ) */

    memcpy(k, ck, 16);
    memcpy(k+16, ik, 16);

    s[0]=0x10;
    memcpy(s+1, sn, 3);
    s[4]=0x00;
    s[5]=0x03;
    memcpy(s+6, sqnAk, 6);
    s[12]=0x00;
    s[13]=0x06;
    hmac_sha256(k, 32, s, 14, kasme, 32);
}

/* ============================================================== */

static void HSS_newAuthVec(EMMCtx emm, GError **err){
    MYSQL_RES *result;
    MYSQL_ROW row;
    my_ulonglong num_rows;

    uint8_t op[16], amf[2], k[16], opc[16], ik[16], ck[16], sqn_b[6];
    uint8_t str_ik[16*2+1], str_ck[16*2+1], str_rand[16*2+1], str_autn[16*2+1], str_kasme[16*2+1], str_opc[16*2+1];
    uint8_t str_res[8*2+1], str_sqn[6*2+1];
    uint64_t sqn, sqn_tmp;

    uint8_t i;
    int j;
    size_t resLen=8;

    char query[1000];
    uint16_t mcc;
    uint8_t mnc;

    AuthQuadruplet *authVec;
    const guint64 imsi = emmCtx_getIMSI(emm);

    mcc = imsi/1000000000000;
    mnc = (imsi/10000000000)%100;

    sprintf(query, authparams, mcc, mnc, imsi%10000000000ULL);

    if (mysql_query(HSSConnection, query)){
        log_msg(LOG_ERR, mysql_errno(HSSConnection), "%s", mysql_error(HSSConnection));
    }

    result = mysql_store_result(HSSConnection);

    if (result == NULL){

    }

    num_rows =mysql_num_rows(result);
    if(num_rows == 0){
        mysql_free_result(result);
        g_set_error(err, DIAMETER, DIAMETER_UNKNOWN_EPS_SUBSCRIPTION,
                    "Unknown EPS subscription: %" PRIu64, imsi);
        return;
    }else if(num_rows != 1){
        mysql_free_result(result);
        log_msg(LOG_ERR, 0, "Unexpected number of rows %" PRIu64 ". \n %s", num_rows, query);
        return;
    }
    row = mysql_fetch_row(result);

    memcpy(k, row[0], 16); /* k*/
    memcpy(sqn_b, row[2], 6); /* sqn*/
    memcpy(op, row[3], 16); /* op*/
    memcpy(amf, row[4], 2); /* amf*/

    increaseSQN(sqn_b);

    if(row[1]==NULL){
        getOPC(op, k, opc);
    }else{
        memcpy(opc, row[1], 16); /* opc*/
    }

    mysql_free_result(result);

    authVec = g_new0(AuthQuadruplet, 1);
    get_random(authVec->rAND, 16);

    milenage_generate(opc, amf, k, sqn_b, authVec->rAND, authVec->aUTN, ik, ck, authVec->xRES, &resLen);

    /* The first 6 bytes of AUTN are SQN^Ak*/
    generate_Kasme(ck, ik,
                   emmCtx_getServingNetwork_TBCD(emm),
                   authVec->aUTN, authVec->kASME);

    emmCtx_setNewAuthQuadruplet(emm, authVec);

    /* Store Auth vector*/
    sprintf(query, insertAuthVector,
            1,
            mcc,
            mnc,
            imsi%10000000000ULL,
            bin_to_strhex(ik,16, str_ik),
            bin_to_strhex(ck,16, str_ck),
            bin_to_strhex(authVec->rAND,16, str_rand),
            bin_to_strhex(authVec->xRES,8, str_res),
            bin_to_strhex(authVec->aUTN,16, str_autn),
            bin_to_strhex(sqn_b, 6, str_sqn),
            bin_to_strhex(authVec->kASME,16, str_kasme),
            "000000000000",   /*AK not stored for the moment*/
            bin_to_strhex(sqn_b, 6, str_sqn),
            bin_to_strhex(opc,16, str_opc),
            mcc,
            mnc,
            imsi%10000000000ULL);

    /*log_msg(LOG_DEBUG, 0, "%s", query);*/

    if (mysql_query(HSSConnection, query)){
        log_msg(LOG_ERR, mysql_errno(HSSConnection), "%s", mysql_error(HSSConnection));
    }

    /*The last query is a multiple statement query, so it is needed to get
      all free all potential results to avoid sync errors.*/
    for(; mysql_next_result(HSSConnection) == 0;)/* do nothing */;
}

/* static void HSS_recoverAuthVec(struct user_ctx_t *user){ */
/*     char query[1000]; */
/*     MYSQL_RES *result; */
/*     MYSQL_ROW row; */
/*     uint16_t mcc; */
/*     uint8_t mnc; */

/*     mcc = user->imsi/1000000000000; */
/*     mnc = (user->imsi/10000000000)%100; */

/*     /\*Chech if there is any Auth vector already stored*\/ */
/*     sprintf(query, get_auth_vec, mcc, mnc, (uint64_t)user->imsi%10000000000ULL, 0); */

/*     if (mysql_query(HSSConnection, query)){ */
/*         log_msg(LOG_ERR, mysql_errno(HSSConnection), "%s", mysql_error(HSSConnection)); */
/*         return; */
/*     } */
/*     /\* log_msg(LOG_DEBUG, 0, "%s", query);*\/ */
/*     result = mysql_store_result(HSSConnection); */
/*     row = mysql_fetch_row(result); */

/*     if(row == NULL){ */
/*         mysql_free_result(result); */
/*         log_msg(LOG_ERR, 0, "No row fetched"); */
/*         return; */
/*     } */

/*     memcpy(user->sec_ctx.rAND,  row[0], 16); */
/*     memcpy(user->sec_ctx.aUTN,  row[1], 16); */
/*     memcpy(user->sec_ctx.xRES,  row[2],  8); */
/*     memcpy(user->sec_ctx.kASME, row[3], 16); */

/*     mysql_free_result(result); */
/* } */

/* ============================================================== */

void HSS_getAuthVec(EMMCtx emm, GError **err){
    HSS_newAuthVec(emm, err);

}

void HSS_syncAuthVec(EMMCtx emm, uint8_t * auts,  GError **err){
    uint8_t sqn[6];
    MYSQL_RES *result;
    MYSQL_ROW row;
    my_ulonglong num_rows;

    uint8_t op[16], amf[2], k[16], opc[16], ik[16], ck[16], sqn_b[6];
    uint8_t str_ik[16*2+1], str_ck[16*2+1], str_rand[16*2+1], str_autn[16*2+1], str_kasme[16*2+1], str_opc[16*2+1];
    uint8_t str_res[8*2+1], str_sqn[6*2+1], sqn_old[6*2+1], sqn_new[6*2+1];

    uint8_t i;
    int j;
    size_t resLen=8;

    char query[1000];
    uint16_t mcc;
    uint8_t mnc;

    const AuthQuadruplet *authVec;
    AuthQuadruplet *newAuthVec;
    guint64 imsi = emmCtx_getIMSI(emm);

    log_msg(LOG_DEBUG, 0, "ENTER");

    mcc = imsi/1000000000000;
    mnc = (imsi/10000000000)%100;

    sprintf(query, authparams, mcc, mnc, (uint64_t)imsi%10000000000ULL);

    if (mysql_query(HSSConnection, query)){
        log_msg(LOG_ERR, mysql_errno(HSSConnection), "%s", mysql_error(HSSConnection));
    }

    result = mysql_store_result(HSSConnection);

    if (result == NULL){
        g_error("'mysql_store_result()' returned NULL");
    }

    num_rows =mysql_num_rows(result);
    if(num_rows != 1){
        mysql_free_result(result);
        log_msg(LOG_ERR, 0, "Unexpected number of rows %" PRIu64 ". \n %s", num_rows, query);
        return;
    }
    row = mysql_fetch_row(result);

    memcpy(k, row[0], 16); /* k*/
    memcpy(sqn_b, row[2], 6); /* sqn*/
    memcpy(op, row[3], 16); /* op*/
    memcpy(amf, row[4], 2); /* amf*/

    if(row[1]==NULL){
        getOPC(op, k, opc);
    }else{
        memcpy(opc, row[1], 16); /* opc*/
    }

    mysql_free_result(result);

    authVec = emmCtx_getFirstAuthQuadruplet(emm);

    if(milenage_auts(opc, k, authVec->rAND, auts, sqn) == 0){
        if (memcmp(sqn, sqn_b, 6) == 0){
            log_msg(LOG_ERR, 0, "SEQ Already synchronized");
            emmCtx_freeAuthQuadruplets(emm);
            g_set_error(err, DIAMETER, 0,
                        "SEQ Already synchronized: %" PRIu64, imsi);
            return;
        }

        emmCtx_freeAuthQuadruplets(emm);
        increaseSQN(sqn);
        log_msg(LOG_INFO, 0, "SEQ sync old:0x%s, new:0x%s",
                bin_to_strhex(sqn_b, 6, sqn_old), bin_to_strhex(sqn, 6, sqn_new));

        newAuthVec = g_new0(AuthQuadruplet, 1);
        get_random(newAuthVec->rAND, 16);

        milenage_generate(opc, amf, k, sqn, newAuthVec->rAND,
                          newAuthVec->aUTN, ik, ck, newAuthVec->xRES, &resLen);

        /* The first 6 bytes of AUTN are SQN^Ak*/
        generate_Kasme(ck, ik, emmCtx_getServingNetwork_TBCD(emm), newAuthVec->aUTN,
                       newAuthVec->kASME);

        emmCtx_setNewAuthQuadruplet(emm, newAuthVec);

        /* Store Auth vector*/
        sprintf(query, insertAuthVector,
                1,
                mcc,
                mnc,
                imsi%10000000000ULL,
                bin_to_strhex(ik,16, str_ik),
                bin_to_strhex(ck,16, str_ck),
                bin_to_strhex(newAuthVec->rAND,16, str_rand),
                bin_to_strhex(newAuthVec->xRES,8, str_res),
                bin_to_strhex(newAuthVec->aUTN,16, str_autn),
                bin_to_strhex(sqn, 6, sqn_new),
                bin_to_strhex(newAuthVec->kASME,16, str_kasme),
                "000000000000",   /*AK not stored for the moment*/
                bin_to_strhex(sqn, 6, sqn_new),
                bin_to_strhex(opc,16, str_opc),
                mcc,
                mnc,
                imsi%10000000000ULL);

        /*log_msg(LOG_DEBUG, 0, "%s", query);*/

        if (mysql_query(HSSConnection, query)){
            log_msg(LOG_ERR, mysql_errno(HSSConnection), "%s", mysql_error(HSSConnection));
        }

        /*The last query is a multiple statement query, so it is needed to get
          all free all potential results to avoid sync errors.*/
        for(; mysql_next_result(HSSConnection) == 0;)/* do nothing */;

    }else{
        log_msg(LOG_ERR, 0, "Invalid AUTS");
    }
}

void HSS_UpdateLocation(EMMCtx emm, const ServedGUMMEIs_t * sGUMMEIs){

    MYSQL_RES *result;
    MYSQL_ROW row;
    uint8_t mmegi[5], apn[100];

    char query[1000];
    uint16_t mcc;
    uint8_t mnc;
    const guint64 imsi = emmCtx_getIMSI(emm);
    Subscription subs;
    PDNCtx subs_pdn;
    struct qos_t qos;

    mcc = imsi/1000000000000;
    mnc = (imsi/10000000000)%100;
    /*Update Location*/
    sprintf(query, update_location,
            sGUMMEIs->item[0]->servedMMECs->item[0]->s[0],
            bin_to_strhex(sGUMMEIs->item[0]->servedGroupIDs->item[0]->s,2,mmegi),
            0,
            mcc, mnc, imsi%10000000000ULL);

    if (mysql_query(HSSConnection, query)){
        log_msg(LOG_ERR, mysql_errno(HSSConnection), "%s", mysql_error(HSSConnection));
        return;
    }

    /*Get Subscriber info*/
    sprintf(query, get_subscriber_profile,
            mcc, mnc, imsi%10000000000ULL, 0);

    if (mysql_query(HSSConnection, query)){
        log_msg(LOG_ERR, mysql_errno(HSSConnection), "%s", mysql_error(HSSConnection));
        return;
    }

    result = mysql_store_result(HSSConnection);
    row = mysql_fetch_row(result);

    subs = emmCtx_getSubscription(emm);
    subs_pdn = subs_newPDNCtx(subs);

    emmCtx_setMSISDN(emm, strtoull(row[0], NULL, 0));
    subs_setUEAMBR(subs, strtoull(row[1], NULL, 0), strtoull(row[2], NULL, 0) );

    qos.qci = atoi(row[13]);
    qos.pl  = atoi(row[14]);
    qos.pci = *row[15];
    qos.pvi = *row[16];
    qos.mbr_ul = 0;
    qos.mbr_dl = 0;
    qos.gbr_ul = 0;
    qos.gbr_dl = 0;

    pdnCtx_setDefaultBearerQoS(subs_pdn, &qos);

    pdnCtx_setPDNtype(subs_pdn, *row[9]);
    sprintf(apn, "%s.mnc%.3u.mcc%.3u.gprs", row[5], mnc, mcc);
    pdnCtx_setAPN(subs_pdn, apn);
    mysql_free_result(result);

}
