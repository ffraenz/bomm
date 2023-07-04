//
//  query.h
//  Bomm
//
//  Created by Fränz Friederes on 10/04/2023.
//

#ifndef query_h
#define query_h

#include <jansson.h>
#include "hold.h"
#include "key.h"
#include "message.h"
#include "progress.h"
#include "attack.h"
#include "pass.h"

#define BOMM_MAX_NUM_PASSES 5

typedef struct _bomm_query bomm_query_t;

/**
 * Attack state
 */
typedef enum {
    /**
     * The attack has not been started, yet.
     */
    BOMM_ATTACK_STATE_IDLE,

    /**
     * The attack is currently being executed.
     */
    BOMM_ATTACK_STATE_PENDING,

    /**
     * The attack has been completed.
     */
    BOMM_ATTACK_STATE_COMPLETED,

    /**
     * A cancellation has been requested and is pending.
     */
    BOMM_ATTACK_STATE_CANCELLING,

    /**
     * The attack has been marked cancelled.
     */
    BOMM_ATTACK_STATE_CANCELLED
} bomm_attack_state_t;

/**
 * Struct representing an attack on Enigma ciphertext that is executed in a
 * single thread
 */
typedef struct _bomm_attack {
    /**
     * Pointer to the query
     */
    bomm_query_t* query;

    /**
     * Number uniquely identifying the slice within an attack
     */
    unsigned int id;

    /**
     * Target key space
     */
    bomm_key_space_t key_space;

    /**
     * Number of passes
     */
    unsigned int num_passes;

    /**
     * Passes
     */
    bomm_pass_t passes[BOMM_MAX_NUM_PASSES];

    /**
     * Target ciphertext
     */
    bomm_message_t* ciphertext;

    /**
     * Pointer to object identifying the thread that is executing this slice
     */
    pthread_t thread;

    /**
     * Current state of the attack.
     * Protected by mutex `progress_mutex`.
     */
    bomm_attack_state_t state;

    /**
     * Current progress of the attack.
     * Protected by mutex `progress_mutex`.
     */
    bomm_progress_t progress;

    /**
     * Mutex for access control on progress related fields across threads
     */
    pthread_mutex_t mutex;
} bomm_attack_t;

/**
 * Struct representing a search query on Enigma ciphertext
 */
typedef struct _bomm_query {
    /**
     * Name of the query (usually the filename)
     */
    char name[80];

    /**
     * Default ciphertext
     */
    bomm_message_t* ciphertext;

    /**
     * Ciphertext score
     */
    double ciphertext_score;

    /**
     * Query measure solutions are compared with
     */
    bomm_measure_t measure;

    /**
     * Shared hold the best results are reported in
     */
    bomm_hold_t* hold;

    /**
     * Whether quiet mode is enabled
     */
    bool quiet;

    /**
     * Whether verbose mode is enabled
     */
    bool verbose;

    /**
     * Joint progress of the embedded attacks;
     * Updated by calling `bomm_query_print`.
     */
    bomm_progress_t joint_progress;

    /**
     * Number of attacks executed for this query
     */
    unsigned int num_attacks;

    /**
     * Array of attacks
     */
    bomm_attack_t attacks[];
} bomm_query_t;

/**
 * Initialize a query from the given program input.
 */
bomm_query_t* bomm_query_init(int argc, char *argv[]);

/**
 * Destroy the given query and free its memory.
 */
void bomm_query_destroy(bomm_query_t* query);

/**
 * Start threaded query evaluation.
 * @return False, if no error occurred.
 */
bool bomm_query_start(bomm_query_t* query);

/**
 * Whether the query is pending, i.e. at least one attack is pending.
 */
bool bomm_query_is_pending(bomm_query_t* query);

/**
 * Mark pending attacks as cancelled.
 * Use `bomm_query_join` to wait for threads to actually terminate.
 */
void bomm_query_cancel(bomm_query_t* query);

/**
 * Join query evaluation threads and wait for them to finish.
 */
void bomm_query_join(bomm_query_t* query);

/**
 * Print the status quo of the given query.
 * @param num_elements The number of hold entries to be included
 */
void bomm_query_print(bomm_query_t* query, unsigned int num_elements);

#endif /* query_h */
