/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   storagesys.h
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  User Context storage system.
 *
 * Storage system headers.
 * It stores the user context in a hash table implemented using the Troy D. Hanson's
 * uthash library:  https://github.com/troydhanson/uthash
 * User Guide:      http://troydhanson.github.com/uthash/userguide.html
 */

#ifndef STORAGE_SYS
#define STORAGE_SYS

#include <stdint.h>
#include <MME.h>

extern int ht_proc_time_usec;

/**@brief Function to init the storage system.
 *
 * The hash table doesn't need any initialization.
 * Currently does nothing, but its purpose is to initialize any structures for persistent storage in DB */
extern void init_storage_system();

/**@brief Delete all user contexts stored on the hash table
 *
 * In the future may manage any DB operations*/
extern void free_storage_system();

/**@brief Count the total amount of users stored currently
 * @return number of user context */
extern uint32_t count_users();

/**@brief Save user context on the hash table
 * @param [in] user user context */
extern void store_user_ctx(struct user_ctx_t *user);

/**@brief Search and return the user context corresponding to the actual imsi
 * @param [in] imsi network ordered imsi (8bytes, 4 bits per digit, max 15 digits)
 * @return user context structure */
extern struct user_ctx_t *get_user_ctx_imsi( const  uint64_t imsi);

/**@brief Search and return the user context corresponding to the actual teid
 * @param [in] teid network ordered teid (4bytes)
 * @return user context structure */
extern struct user_ctx_t *get_user_ctx_teid( const uint32_t teid);

/**@brief Find and delete the user context corresponding to the actual imsi
 * @param [in] imsi network ordered imsi (8bytes, 4 bits per digit, max 15 digits) */
extern void delete_user_ctx_imsi( const uint64_t imsi);

/**@brief Find and delete the user context corresponding to the actual teid
 * @param [in] teid network ordered teid (4bytes) */
extern void delete_user_ctx_teid( const uint32_t teid);

#endif /* STORAGE_SYS */
