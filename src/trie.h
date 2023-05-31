//
//  trie.h
//  Bomm
//
//  Created by Fränz Friederes on 29/05/2023.
//

#ifndef trie_h
#define trie_h

#include <jansson.h>
#include <stdbool.h>
#include "message.h"

/**
 * Struct representing a node in a trie (tree data structure)
 */
typedef struct _bomm_trie {
    struct _bomm_trie* children[BOMM_ALPHABET_SIZE];
    double value;
} bomm_trie_t;

/**
 * Initialize a trie node
 */
static inline bomm_trie_t* bomm_trie_init(bomm_trie_t* node) {
    if (!node && !(node = malloc(sizeof(bomm_trie_t)))) {
        return NULL;
    }
    memset(node, 0, sizeof(bomm_trie_t));
    return node;
}

/**
 * Insert a word in the given trie
 */
bool bomm_trie_insert(
    bomm_trie_t* trie,
    bomm_message_t* word,
    unsigned int num_garbles,
    double value
);

/**
 * Init a trie from the given JSON value
 */
bomm_trie_t* bomm_trie_init_with_json(bomm_trie_t* trie, json_t* trie_json);

/**
 * Score a message using the given trie.
 */
static inline double bomm_trie_measure_message(
    bomm_trie_t* trie,
    bomm_message_t* message
) {
    unsigned int length = message->length;
    double score = 0;
    unsigned int i, j;
    bomm_trie_t* node;
    for (i = 0; i < length; i++) {
        node = trie;
        j = 0;
        while (node && i + j < length) {
            score += node->value;
            node = node->children[message->letters[i + j++]];
        }
        if (node) {
            score += node->value;
        }
    }
    return score;
}

/**
 * Print the contents of the given trie to stdout. Useful for debugging.
 */
void bomm_trie_debug(bomm_trie_t* trie);

/**
 * Destroy the given node by freeing its dependencies.
 * The node itself is not freed.
 */
void bomm_trie_destroy(bomm_trie_t* node);

#endif /* trie_h */
