/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   storagesys.c
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  User Context storage system.
 *
 * Storage system implementation.
 */

#include "MME.h"
#include "uthash.h"
#include "logmgr.h"
#include <stdlib.h>
#include "storagesys.h"

/******************* Linux dependent ************************/
#include <sys/time.h>

/*Process time macros*/
#define STARTTIME \
    struct timeval stop, start; \
    gettimeofday(&start, NULL);
#define ENDTIME \
    gettimeofday(&stop, NULL);\
    ht_proc_time_usec=(stop.tv_usec - start.tv_usec) + (stop.tv_sec - start.tv_sec)*1000000;
/******************* Linux dependent ************************/



/** Struct to store the user context. This structure is only a wrapper to add
 *  the hash table handler to the user context structure*/
struct hash_elem{
    struct user_ctx_t     *user;
    UT_hash_handle        hh1;  /* makes this structure hashable */ /* user->imsi is the key*/
    UT_hash_handle        hh2;  /* makes this structure hashable */ /* user->S11MMETeid is the key*/
};

/*Hash table*/
static struct hash_elem *usersht_imsi = NULL;
static struct hash_elem *usersht_teid = NULL;


/*Instance of global variable*/
int ht_proc_time_usec;


void init_storage_system(){
    STARTTIME
    ht_proc_time_usec=0;
    ENDTIME
}

void free_storage_system(){
    struct hash_elem *current_user, *tmp;
    STARTTIME

    HASH_ITER(hh1, usersht_imsi, current_user, tmp) {
        HASH_DELETE(hh1, usersht_imsi, current_user);  /* delete; users advances to next */
        HASH_DELETE(hh2, usersht_teid, current_user);  /* delete; users advances to next */
        free(current_user);
    }
    ENDTIME
}

uint32_t count_users(){
    STARTTIME
    uint32_t num_users;
    num_users = HASH_CNT(hh1, usersht_imsi);
    ENDTIME
    return num_users;
}

void store_user_ctx(struct user_ctx_t *user){

    struct hash_elem *new, *stored, *stored1;

    STARTTIME
    if(user){
        HASH_FIND(hh1, usersht_imsi, &(user->imsi), sizeof(uint64_t), stored);
        HASH_FIND(hh2, usersht_teid, &(user->S11MMETeid), sizeof(uint32_t), stored1);
        if(stored == NULL && stored1 == NULL){
            new = malloc(sizeof(struct hash_elem));
            new->user = user;
            if(user->imsi == 0){
                log_msg(LOG_WARNING, 0, "store_user_ctx(): user->imsi = 0");
            }
            if(user->S11MMETeid == 0){
                log_msg(LOG_WARNING, 0, "store_user_ctx(): user->S11MMETeid = 0");
            }
            HASH_ADD(hh1, usersht_imsi, user->imsi, sizeof(uint64_t), new);
            HASH_ADD(hh2, usersht_teid, user->S11MMETeid, sizeof(uint32_t), new);
            log_msg(LOG_INFO, 0, "store_user_ctx(): User Context Stored (imsi = %llu, teid = %d), count %d", new->user->imsi, new->user->S11MMETeid, count_users());

        }else{
            if(stored != NULL){ /*Check which one is already on the hash table*/
                stored1 = stored;
                log_msg(LOG_ERR, 0,"imsi already on ht");
            }
            log_msg(LOG_ERR, 0, "store_user_ctx(): User Context is already stored (imsi = %llu , teid = %d)", stored1->user->imsi, stored1->user->S11MMETeid);
        }
    }
    else{
        log_msg(LOG_WARNING, 0, "store_user_ctx(): User Context parameter not valid");
    }
    ENDTIME
}


struct user_ctx_t *get_user_ctx_imsi( const uint64_t imsi){

    struct user_ctx_t *res = NULL;
    struct hash_elem *stored = NULL;
    uint64_t key;
    STARTTIME
    key = imsi;
    HASH_FIND(hh1, usersht_imsi, &key, sizeof(uint64_t), stored );       /* stored: output pointer */
    if(stored){
        res = stored->user;
        log_msg(LOG_DEBUG, 0, "get_user_ctx_imsi(): User Context found on storage imsi=%llu, teid = %d, ht size = %d", imsi, stored->user->S11MMETeid, count_users());
    }
    else{
        log_msg(LOG_DEBUG, 0, "get_user_ctx_imsi(): User Context not found on storage imsi=%llu", imsi);
        res = NULL;
    }
    ENDTIME
    return res;
}

struct user_ctx_t *get_user_ctx_teid( const uint32_t teid){

    struct user_ctx_t *res = NULL;
    struct hash_elem *stored = NULL;
    uint32_t key;
    STARTTIME
    key = teid;
    HASH_FIND(hh2, usersht_teid, &key, sizeof(uint32_t), stored );       /* stored: output pointer */
    if(stored){
        res = stored->user;
    }
    else{
        log_msg(LOG_DEBUG, 0, "get_user_ctx_teid(): User Context not found on storage teid=%8x", teid);
        res = NULL;
    }
    ENDTIME
    return res;
}

void delete_user_ctx_imsi( const uint64_t imsi) {

    struct hash_elem *stored;
    STARTTIME
    HASH_FIND(hh1, usersht_imsi, &imsi, sizeof(uint64_t), stored );       /* stored: output pointer */
    if(stored){
        HASH_DELETE(hh1, usersht_imsi, stored);                                     /* stored: pointer to delete */
        HASH_DELETE(hh2, usersht_teid, stored);                                     /* stored: pointer to delete */

        free(stored);
    }
    else{
        log_msg(LOG_WARNING, 0, "delete_user_ctx_imsi(): User Context not found on storage imsi=%16x", imsi);
    }
    ENDTIME
}

void delete_user_ctx_teid( const uint32_t teid) {

    struct hash_elem *stored;
    STARTTIME
    HASH_FIND(hh2, usersht_teid, &teid, sizeof(uint32_t), stored );       /* stored: output pointer */
    if(stored){
        HASH_DELETE(hh1, usersht_imsi, stored);                                     /* stored: pointer to delete */
        HASH_DELETE(hh2, usersht_teid, stored);                                     /* stored: pointer to delete */
        free(stored);
    }
    else{
        log_msg(LOG_WARNING, 0, "delete_user_ctx_teid(): User Context not found on storage teid=%8x", teid);
    }
    ENDTIME
}


