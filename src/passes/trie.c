//
//  trie.c
//  Bomm
//
//  Created by Fränz Friederes on 29/05/2023.
//

#include "trie.h"

double bomm_pass_trie_climb_run(
    bomm_pass_trie_config_t* config,
    unsigned int* plugboard,
    bomm_scrambler_t* scrambler,
    bomm_message_t* ciphertext,
    double score
) {
    bomm_message_t* plaintext = alloca(bomm_message_size_for_length(ciphertext->length));
    bomm_scrambler_encrypt(scrambler, plugboard, ciphertext, plaintext);
    double trie_score = bomm_trie_measure_message(config->trie, plaintext);
    return score + trie_score;
}

bomm_pass_trie_config_t* bomm_pass_trie_config_init_with_json(
    bomm_pass_trie_config_t* config,
    json_t* config_json
) {
    if (config_json->type != JSON_OBJECT) {
        return NULL;
    }

    json_t* cribs_json = json_object_get(config_json, "words");
    if (cribs_json->type != JSON_ARRAY) {
        return NULL;
    }

    unsigned int num_cribs = (unsigned int) json_array_size(cribs_json);
    if (num_cribs == 0) {
        return NULL;
    }

    bool owning = config == NULL;
    if (!config && !(config = malloc(sizeof(bomm_pass_trie_config_t)))) {
        return NULL;
    }

    config->trie = bomm_trie_init(NULL);

    bool error = config->trie == NULL;
    json_t *crib_json, *word_json, *value_json;
    unsigned int i = 0;
    while (!error && i < num_cribs) {
        crib_json = json_array_get(cribs_json, i);
        if (crib_json->type == JSON_OBJECT) {
            word_json = json_object_get(crib_json, "word");
            value_json = json_object_get(crib_json, "value");
            if (json_is_string(word_json) && json_is_number(value_json)) {
                double value = json_number_value(value_json);
                const char* word_string = json_string_value(word_json);
                bomm_message_t* word = bomm_message_init(word_string);
                if (word != NULL) {
                    bomm_trie_insert(config->trie, word, value);
                    free(word);
                } else {
                    error = true;
                }
            } else {
                error = true;
            }
        } else {
            error = true;
        }
        i++;
    }

    if (error) {
        bomm_trie_destroy(config->trie);
        config->trie = NULL;
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
