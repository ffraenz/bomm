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
