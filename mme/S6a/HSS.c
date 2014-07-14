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

#include <mysql.h>
#include <stdlib.h>

#define HOST    "localhost"
#define USER    "hss"
#define PASSWD  "hss"
#define HSS_DB  "hss_lte_db"


/* HSS connection global variable */
MYSQL                   *HSSConnection;


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

int init_hss(){
    MYSQL      *MySQLConRet;

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

    MySQLConRet = mysql_real_connect( HSSConnection, HOST, USER, PASSWD, HSS_DB, 
                                      0, NULL, CLIENT_MULTI_STATEMENTS );
    if ( MySQLConRet == NULL || MySQLConRet != HSSConnection){
        log_msg(LOG_ERR, mysql_errno(HSSConnection), "%s. Disconnecting. Handler %x", mysql_error(HSSConnection), HSSConnection);
        disconnect_hss();
        return 1;
    }
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

static void HSS_newAuthVec(struct user_ctx_t *user){
    MYSQL_RES *result;
    MYSQL_ROW row;
    my_ulonglong num_rows;

    uint8_t op[16], amf[2], k[16], opc[16], ik[16], ck[16], sqn_b[6];
    uint8_t str_ik[16*2+1], str_ck[16*2+1], str_rand[16*2+1], str_autn[16*2+1], str_kasme[16*2+1], str_opc[16*2+1];
    uint8_t str_res[8*2+1], str_sqn[6*2+1];
    uint64_t sqn;

    uint8_t i;
    size_t resLen=8;

    char query[1000];
    uint16_t mcc;
    uint8_t mnc;

    mcc = user->imsi/1000000000000;
    mnc = (user->imsi/10000000000)%100;

    sprintf(query, authparams, mcc, mnc, user->imsi%10000000000);

    if (mysql_query(HSSConnection, query)){
        log_msg(LOG_ERR, mysql_errno(HSSConnection), "%s", mysql_error(HSSConnection));
    }

    result = mysql_store_result(HSSConnection);

    if (result == NULL){

    }

    num_rows =mysql_num_rows(result);
    if(num_rows != 1){
        mysql_free_result(result);
        log_msg(LOG_ERR, 0, "Unexpected number of rows %llu. \n %s", num_rows, query);
        return;
    }
    row = mysql_fetch_row(result);

    memcpy(k, row[0], 16); /* k*/
    memcpy(sqn_b, row[2], 6); /* sqn*/
    memcpy(op, row[3], 16); /* op*/
    memcpy(amf, row[4], 2); /* amf*/

    /*Increase SQN*/
    /* Using annex C.1.1.2 of TS 33.102
	 * Generation of sequence numbers which are not time-based*/
	bin_to_strhex(sqn_b,6,  str_sqn);
   	sqn = strtoll(str_sqn, NULL, 16);
	sqn = ((sqn/32+1)<<5)|(sqn%32); /* SQN_HE = SEQ_HE (43 bits)|| IND_HE (5 bits)*/
	/* Copy back to buffer*/
	sqn = htobe64(sqn<<16);
	memcpy(sqn_b, &sqn, 6);

    if(row[1]==NULL){
        getOPC(op, k, opc);
    }else{
        memcpy(opc, row[1], 16); /* opc*/
    }

    mysql_free_result(result);

    /* KSI*/
    user->ksi.id = 0;
    user->ksi.tsc = 0;

    get_random(user->sec_ctx.rAND, 16);

    milenage_generate(opc, amf, k, sqn_b, user->sec_ctx.rAND, user->sec_ctx.aUTN, ik, ck, user->sec_ctx.xRES, &resLen);

    /* The first 6 bytes of AUTN are SQN^Ak*/
    generate_Kasme(ck, ik, user->tAI.sn, user->sec_ctx.aUTN, user->sec_ctx.kASME);

    /* Store Auth vector*/
    sprintf(query, insertAuthVector,
            mcc,
            mnc,
            user->imsi%10000000000,
            user->ksi.id,
            bin_to_strhex(ik,16, str_ik),
            bin_to_strhex(ck,16, str_ck),
            bin_to_strhex(user->sec_ctx.rAND,16, str_rand),
            bin_to_strhex(user->sec_ctx.xRES,8, str_res),
            bin_to_strhex(user->sec_ctx.aUTN,16, str_autn),
            sqn,
            bin_to_strhex(user->sec_ctx.kASME,16, str_kasme),
            "00000000000000000000000000000000",   /*AK not stored for the moment*/
            sqn,
            bin_to_strhex(opc,16, str_opc),
            mcc,
            mnc,
            user->imsi%10000000000 );

    /*log_msg(LOG_DEBUG, 0, "%s", query);*/

    if (mysql_query(HSSConnection, query)){
        log_msg(LOG_ERR, mysql_errno(HSSConnection), "%s", mysql_error(HSSConnection));
    }

    /*The last query is a multiple statement query, so it is needed to get
      all free all potential results to avoid sync errors.*/
    for(; mysql_next_result(HSSConnection) == 0;)/* do nothing */;
}

static void HSS_recoverAuthVec(struct user_ctx_t *user){
    char query[1000];
    MYSQL_RES *result;
    MYSQL_ROW row;
    uint16_t mcc;
    uint8_t mnc;

    mcc = user->imsi/1000000000000;
    mnc = (user->imsi/10000000000)%100;

    /*Chech if there is any Auth vector already stored*/
    sprintf(query, get_auth_vec, mcc, mnc, user->imsi%10000000000, 0);

    if (mysql_query(HSSConnection, query)){
        log_msg(LOG_ERR, mysql_errno(HSSConnection), "%s", mysql_error(HSSConnection));
        return;
    }
    /* log_msg(LOG_DEBUG, 0, "%s", query);*/
    result = mysql_store_result(HSSConnection);
    row = mysql_fetch_row(result);
    
    if(row == NULL){
        mysql_free_result(result);
        log_msg(LOG_ERR, 0, "No row fetched");
        return;
    }

    /* KSI*/
    user->ksi.id = 0;
    user->ksi.tsc = 0;

    memcpy(user->sec_ctx.rAND,  row[0], 16);
    memcpy(user->sec_ctx.aUTN,  row[1], 16);
    memcpy(user->sec_ctx.xRES,  row[2],  8);
    memcpy(user->sec_ctx.kASME, row[3], 16);

    mysql_free_result(result);
}

/* ============================================================== */

void HSS_getAuthVec(Signal *signal){
	/*
    MYSQL_RES *result;
    MYSQL_ROW row;

    char query[1000];
    struct user_ctx_t *user = PDATA->user_ctx;
    uint16_t mcc;
    uint8_t mnc;

    mcc = user->imsi/1000000000000;
    mnc = (user->imsi/10000000000)%100;*/
	/*Chech if there is any Auth vector already stored*//*
    sprintf(query, exists_auth_vec, mcc, mnc, user->imsi%10000000000);

    if (mysql_query(HSSConnection, query)){
        log_msg(LOG_ERR, mysql_errno(HSSConnection), "%s", mysql_error(HSSConnection));
        return;
    }
    result = mysql_store_result(HSSConnection);
    row = mysql_fetch_row(result);*/
    /*log_msg(LOG_DEBUG, 0, "%s", query);*/
	/*log_msg(LOG_DEBUG, 0, "row[0] %d %d",*row[0], (int)*row[0] );*//*
    if(*row[0] == '1'){
        HSS_recoverAuthVec(user);
    }else{
        HSS_newAuthVec(user);
    }

    mysql_free_result(result);*/
	struct user_ctx_t *user = PDATA->user_ctx;
	HSS_newAuthVec(user);

}

void HSS_UpdateLocation(Signal *signal){

    MYSQL_RES *result;
    MYSQL_ROW row;
    uint8_t mmegi[5], apn[100];

    char query[1000];
    struct user_ctx_t *user = PDATA->user_ctx;
    uint16_t mcc;
    uint8_t mnc;

    mcc = user->imsi/1000000000000;
    mnc = (user->imsi/10000000000)%100;
    /*Update Location*/
    sprintf(query, update_location,
            SELF_ON_SIG->servedGUMMEIs->item[0]->servedMMECs->item[0]->s[0],
            bin_to_strhex(SELF_ON_SIG->servedGUMMEIs->item[0]->servedGroupIDs->item[0]->s,2,mmegi),
            0,
            mcc, mnc, user->imsi%10000000000);

    if (mysql_query(HSSConnection, query)){
        log_msg(LOG_ERR, mysql_errno(HSSConnection), "%s", mysql_error(HSSConnection));
        return;
    }

    /*Get Subscriber info*/
    sprintf(query, get_subscriber_profile,
            mcc, mnc, user->imsi%10000000000, 0);

    if (mysql_query(HSSConnection, query)){
        log_msg(LOG_ERR, mysql_errno(HSSConnection), "%s", mysql_error(HSSConnection));
        return;
    }

    result = mysql_store_result(HSSConnection);
    row = mysql_fetch_row(result);


    user->msisdn = strtoull(row[0], NULL, 0);
    user->ue_ambr_ul = strtoull(row[1], NULL, 0);
    user->ue_ambr_dl = strtoull(row[2], NULL, 0);
    user->ebearer[0].qos.qci = atoi(row[13]);
    user->ebearer[0].qos.pl  = atoi(row[14]);
    user->ebearer[0].qos.pci = *row[15];
    user->ebearer[0].qos.pvi = *row[16];
    user->ebearer[0].qos.mbr_ul = 0;
    user->ebearer[0].qos.mbr_dl = 0;
    user->ebearer[0].qos.gbr_ul = 0;
    user->ebearer[0].qos.gbr_dl = 0;


    user->pdn_type = atoi(row[9]);
    sprintf(user->aPname, "%s.mnc%.3u.mcc%.3u.gprs", row[5], mnc, mcc);

    mysql_free_result(result);

}
