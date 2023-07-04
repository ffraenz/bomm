//
//  trie.h
//  Bomm
//
//  Created by Fränz Friederes on 29/05/2023.
//

#ifndef passes_trie_h
#define passes_trie_h

#include <jansson.h>
#include "../message.h"
#include "../wiring.h"
#include "../trie.h"
#include "../measure.h"

/**
 * Struct representing a set of values configuring a trie pass
 */
typedef struct _bomm_pass_trie_config {
    /**
     * Trie root node
     */
    bomm_trie_t* trie;

    /**
     * Base measure
     */
    bomm_measure_t base_measure;
} bomm_pass_trie_config_t;

/**
 * Run a trie pass on the given plugboard and scrambler
 */
double bomm_pass_trie_climb_run(
    bomm_pass_trie_config_t* config,
    unsigned int* plugboard,
    bomm_scrambler_t* scrambler,
    bomm_message_t* ciphertext,
    double score,
    unsigned int* num_decrypts
);

/**
 * Init a pass config from the given JSON value
 */
bomm_pass_trie_config_t* bomm_pass_trie_config_init_with_json(
    bomm_pass_trie_config_t* config,
    json_t* config_json
);

/**
 * Free a pass config with all dependencies
 */
void bomm_pass_trie_config_destroy(
    bomm_pass_trie_config_t* config
);

#endif /* passes_trie_h */
