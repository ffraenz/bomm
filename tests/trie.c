//
//  trie.c
//  Bomm
//
//  Created by Fränz Friederes on 29/05/2023.
//

#include <stdio.h>
#include <criterion/criterion.h>
#include "shared/helpers.h"
#include "../src/trie.h"

Test(trie, bomm_trie_init) {
    bomm_trie_t* expected_children[BOMM_ALPHABET_SIZE];
    memset(expected_children, 0, sizeof(expected_children));

    bomm_trie_t stack_trie;
    cr_assert_eq(bomm_trie_init(&stack_trie), &stack_trie);
    cr_assert_arr_eq(stack_trie.children, expected_children, sizeof(expected_children));
    cr_assert_eq(stack_trie.value, 0.0);

    bomm_trie_t* heap_trie = bomm_trie_init(NULL);
    cr_assert_neq(heap_trie, NULL);
    cr_assert_arr_eq(heap_trie->children, expected_children, sizeof(expected_children));
    cr_assert_eq(heap_trie->value, 0.0);
    free(heap_trie);
}

Test(trie, bomm_trie_insert) {
    bomm_test_skip_if_non_latin_alphabet;
    bomm_trie_t trie;
    bomm_trie_init(&trie);

    bomm_message_t* foo = bomm_message_init("foo");
    bomm_message_t* bar = bomm_message_init("bar");
    bomm_message_t* foobar = bomm_message_init("foobar");

    bomm_letter_t a = bomm_message_letter_from_ascii('a');
    bomm_letter_t b = bomm_message_letter_from_ascii('b');
    bomm_letter_t f = bomm_message_letter_from_ascii('f');
    bomm_letter_t o = bomm_message_letter_from_ascii('o');
    bomm_letter_t r = bomm_message_letter_from_ascii('r');

    bomm_trie_insert(&trie, foo, 0, 1.0);
    bomm_trie_insert(&trie, bar, 0, 2.0);
    bomm_trie_insert(&trie, foobar, 0, 3.0);

    cr_assert_eq(trie.value, 0.0);
    cr_assert_eq(trie.children[a], NULL);
    cr_assert_neq(trie.children[f], NULL);
    cr_assert_eq(trie.children[f]->value, 0.0);
    cr_assert_neq(trie.children[b], NULL);
    cr_assert_eq(trie.children[b]->value, 0.0);
    cr_assert_neq(trie.children[b]->children[a], NULL);
    cr_assert_eq(trie.children[b]->children[a]->value, 0.0);
    cr_assert_eq(trie.children[f]->children[b], NULL);
    cr_assert_neq(trie.children[f]->children[o], NULL);
    cr_assert_eq(trie.children[f]->children[o]->value, 0.0);

    bomm_trie_t* trie_foo = trie.children[f]->children[o]->children[o];
    cr_assert_neq(trie_foo, NULL);
    cr_assert_eq(trie_foo->value, 1.0);
    cr_assert_neq(trie_foo->children[b], NULL);
    cr_assert_neq(trie_foo->children[b]->children[a], NULL);

    bomm_trie_t* trie_bar = trie.children[b]->children[a]->children[r];
    cr_assert_neq(trie_bar, NULL);
    cr_assert_eq(trie_bar->value, 2.0);
    cr_assert_eq(trie_bar->children[b], NULL);

    bomm_trie_t* trie_foobar = trie_foo->children[b]->children[a]->children[r];
    cr_assert_neq(trie_foobar, NULL);
    cr_assert_eq(trie_foobar->value, 3.0);
}

Test(trie, bomm_trie_insert_garbled) {
    bomm_test_skip_if_non_latin_alphabet;
    bomm_trie_t* trie = bomm_trie_init(NULL);

    bomm_message_t* siegfried = bomm_message_init("siegfried");
    bomm_trie_insert(trie, siegfried, 1, 1.0);
    free(siegfried);

    bomm_message_t* toni = bomm_message_init("toni");
    bomm_trie_insert(trie, toni, 2, 100.0);
    free(toni);

    bomm_message_t* message = bomm_message_init(
        "dooy siekfried toni siegfried dony ziegfried doni syegfryed tony");
    free(message);

    bomm_trie_destroy(trie);
    free(trie);
}

Test(trie, bomm_trie_measure_message) {
    bomm_test_skip_if_non_latin_alphabet;
    bomm_message_t* message;
    bomm_trie_t* trie = bomm_trie_init(NULL);

    message = bomm_message_init("foo");
    bomm_trie_insert(trie, message, 0, 1.0);
    free(message);
    message = bomm_message_init("bar");
    bomm_trie_insert(trie, message, 0, 100.0);
    free(message);
    message = bomm_message_init("foobar");
    bomm_trie_insert(trie, message, 0, 10000.0);
    free(message);

    message = bomm_message_init("helloworld");
    cr_assert_eq(bomm_trie_measure_message(trie, message), 0.0);
    free(message);

    message = bomm_message_init("foobarbarfoobarfoofoobarbar");
    cr_assert_eq(bomm_trie_measure_message(trie, message), 30504.0);
    free(message);

    bomm_trie_destroy(trie);
    free(trie);
}
