//
//  trie.c
//  Bomm
//
//  Created by Fränz Friederes on 29/05/2023.
//

#include "trie.h"

void bomm_trie_destroy(bomm_trie_t* node) {
    for (unsigned int i = 0; i < BOMM_ALPHABET_SIZE; i++) {
        if (node->children[i] != NULL) {
            bomm_trie_destroy(node->children[i]);
        }
    }
}

bomm_trie_t* bomm_trie_init_with_json(bomm_trie_t* trie, json_t* trie_json) {
    if (!json_is_object(trie_json)) {
        return NULL;
    }

    json_t* words_json = json_object_get(trie_json, "words");
    if (!json_is_array(words_json)) {
        return NULL;
    }

    size_t num_words = json_array_size(words_json);
    if (num_words == 0) {
        return NULL;
    }

    bool owning = trie == NULL;
    if (!(trie = bomm_trie_init(trie))) {
        return NULL;
    }

    bool error = false;
    unsigned int i = 0;
    while (!error && i < num_words) {
        json_t* entry_json = json_array_get(words_json, i++);
        if (!(error = !json_is_object(entry_json))) {
            json_t* word_json = json_object_get(entry_json, "word");
            json_t* value_json = json_object_get(entry_json, "value");
            error = !json_is_string(word_json) || !json_is_number(value_json);
            if (!error) {
                double value = json_number_value(value_json);
                const char* word_string = json_string_value(word_json);
                bomm_message_t* word = bomm_message_init(word_string);
                if (!(error = (word == NULL))) {
                    error = bomm_trie_insert(trie, word, value) == NULL;
                    free(word);
                }
            }
        }
    }

    if (error) {
        bomm_trie_destroy(trie);
        if (owning) {
            free(trie);
        }
        return NULL;
    }

    return trie;
}
