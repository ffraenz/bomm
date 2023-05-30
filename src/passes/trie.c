//
//  trie.c
//  Bomm
//
//  Created by Fränz Friederes on 29/05/2023.
//

#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE

#include "trie.h"

double bomm_pass_trie_climb_run(
    bomm_pass_trie_config_t* config,
    unsigned int* plugboard,
    bomm_scrambler_t* scrambler,
    bomm_message_t* ciphertext,
    double score
) {
    size_t message_size = bomm_message_size_for_length(ciphertext->length);
    bomm_message_t* plaintext = malloc(message_size);
    if (plaintext) {
        bomm_scrambler_encrypt(scrambler, plugboard, ciphertext, plaintext);
        if (config->base_measure != BOMM_MEASURE_NONE) {
            score = bomm_measure_message(config->base_measure, plaintext);
        }
        score += bomm_trie_measure_message(config->trie, plaintext);
    }
    return score;
}

bomm_pass_trie_config_t* bomm_pass_trie_config_init_with_json(
    bomm_pass_trie_config_t* config,
    json_t* config_json
) {
    if (config_json->type != JSON_OBJECT) {
        return NULL;
    }

    bool owning = config == NULL;
    if (!config && !(config = malloc(sizeof(bomm_pass_trie_config_t)))) {
        return NULL;
    }

    json_t* base_measure_json = json_object_get(config_json, "baseMeasure");
    config->base_measure = bomm_measure_from_json(base_measure_json);

    json_t* trie_json = json_object_get(config_json, "trie");
    config->trie = bomm_trie_init_with_json(NULL, trie_json);

    if (config->trie == NULL) {
        if (owning) {
            free(config);
        }
        return NULL;
    }

    return config;
}

void bomm_pass_trie_config_destroy(
    bomm_pass_trie_config_t* config
) {
    bomm_trie_destroy(config->trie);
}
