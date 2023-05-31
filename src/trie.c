//
//  trie.c
//  Bomm
//
//  Created by Fränz Friederes on 29/05/2023.
//

#include "trie.h"

bool _bomm_trie_insert(
    bomm_trie_t* trie,
    bomm_message_t* word,
    unsigned int offset,
    unsigned int num_garbles,
    double value
) {
    if (offset == word->length) {
        // Base case
        trie->value += value;
        return true;
    }

    unsigned int letter = word->letters[offset];

    bomm_trie_t* garble_node;
    for (unsigned int i = 0; i < BOMM_ALPHABET_SIZE; i++) {
        if (letter == i || num_garbles > 0) {
            if (
                !(garble_node = trie->children[i]) &&
                !(garble_node = trie->children[i] = bomm_trie_init(NULL))
            ) {
                return false;
            }

            if (!_bomm_trie_insert(
                garble_node,
                word,
                offset + 1,
                num_garbles + (letter == i ? 0 : -1),
                value
            )) {
                return false;
            }
        }
    }

    return true;
}

bool bomm_trie_insert(
    bomm_trie_t* trie,
    bomm_message_t* word,
    unsigned int num_garbles,
    double value
) {
    return _bomm_trie_insert(trie, word, 0, num_garbles, value);
}

void _bomm_trie_debug_prefixed(bomm_trie_t* trie, const char* prefix) {
    char child_prefix[strlen(prefix) + 16];
    snprintf(child_prefix, sizeof(child_prefix), "%s│  ", prefix);
    printf("trie %p (value: %f)\n", (void*) trie, trie->value);
    for (unsigned int i = 0; i < BOMM_ALPHABET_SIZE; i++) {
        if (trie->children[i]) {
            printf("%s├─ %c: ", prefix, bomm_message_letter_to_ascii(i));
            _bomm_trie_debug_prefixed(trie->children[i], child_prefix);
        }
    }
}

void bomm_trie_debug(bomm_trie_t* trie) {
    _bomm_trie_debug_prefixed(trie, "");
}

void bomm_trie_destroy(bomm_trie_t* node) {
    bomm_trie_t* child;
    for (unsigned int i = 0; i < BOMM_ALPHABET_SIZE; i++) {
        if ((child = node->children[i]) != NULL) {
            bomm_trie_destroy(child);
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
            json_t* garbles_json = json_object_get(entry_json, "garbles");
            error =
                !json_is_string(word_json) ||
                !json_is_number(value_json) ||
                !(!garbles_json || json_is_integer(garbles_json));
            if (!error) {
                const char* word_string = json_string_value(word_json);
                bomm_message_t* word = bomm_message_init(word_string);
                double value = json_number_value(value_json);
                unsigned int garbles = garbles_json
                    ? (unsigned int) json_integer_value(garbles_json)
                    : 0;
                error = (!word || !bomm_trie_insert(trie, word, garbles, value));
                free(word);
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
