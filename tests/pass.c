//
//  pass.c
//  Bomm
//
//  Created by Fränz Friederes on 24/05/2023.
//

#include <criterion/criterion.h>
#include <jansson.h>
#include "shared/helpers.h"
#include "../src/pass.h"

Test(wiring, bomm_pass_init_json_hill_climb_1) {
    const char* pass_json_string =
        "{ " \
        "\"type\": \"hill_climb\", " \
        "\"measure\": \"ic\", " \
        "\"finalMeasure\": \"sinkov_trigram\", " \
        "\"finalMeasureMinNumPlugs\": 1 " \
        "}";

    json_error_t error;
    json_t* pass_json = json_loads(pass_json_string, 0, &error);

    bomm_pass_t pass;
    cr_assert_eq(bomm_pass_init_with_json(&pass, pass_json), &pass);

    cr_assert_eq(pass.type, BOMM_PASS_HILL_CLIMB);
    cr_assert_eq(pass.config.hill_climb.measure, BOMM_MEASURE_IC);
    cr_assert_eq(pass.config.hill_climb.final_measure, BOMM_MEASURE_SINKOV_TRIGRAM);
    cr_assert_eq(pass.config.hill_climb.final_measure_min_num_plugs, 1);

    if (BOMM_ALPHABET_SIZE == 26) {
        cr_assert_eq(pass.config.hill_climb.backtracking_min_num_plugs, 5);
    }
}

Test(wiring, bomm_pass_init_json_hill_climb_2) {
    const char* pass_json_string =
        "{ " \
        "\"type\": \"hill_climb\", " \
        "\"measure\": \"sinkov_trigram\", " \
        "\"backtrackingMinNumPlugs\": 1 " \
        "}";

    json_error_t error;
    json_t* pass_json = json_loads(pass_json_string, 0, &error);

    bomm_pass_t pass;
    cr_assert_eq(bomm_pass_init_with_json(&pass, pass_json), &pass);

    cr_assert_eq(pass.type, BOMM_PASS_HILL_CLIMB);
    cr_assert_eq(pass.config.hill_climb.measure, BOMM_MEASURE_SINKOV_TRIGRAM);
    cr_assert_eq(pass.config.hill_climb.final_measure, BOMM_MEASURE_SINKOV_TRIGRAM);
    cr_assert_eq(pass.config.hill_climb.backtracking_min_num_plugs, 1);

    if (BOMM_ALPHABET_SIZE == 26) {
        cr_assert_eq(pass.config.hill_climb.final_measure_min_num_plugs, 7);
    }
}

Test(wiring, bomm_pass_init_json_reswapping) {
    const char* pass_json_string =
        "{ " \
        "\"type\": \"reswapping\", " \
        "\"measure\": \"entropy\" " \
        "}";

    json_error_t error;
    json_t* pass_json = json_loads(pass_json_string, 0, &error);

    bomm_pass_t pass;
    cr_assert_eq(bomm_pass_init_with_json(&pass, pass_json), &pass);

    cr_assert_eq(pass.type, BOMM_PASS_RESWAPPING);
    cr_assert_eq(pass.config.reswapping.measure, BOMM_MEASURE_ENTROPY);
}

Test(wiring, bomm_pass_init_json_trie) {
    bomm_test_skip_if_non_latin_alphabet;
    const char* pass_json_string =
        "{ " \
        "\"type\": \"trie\", " \
        "\"trie\": { " \
        "\"words\": [" \
        "{ \"word\": \"foo\", \"value\": 1.0 }, " \
        "{ \"word\": \"bar\", \"value\": 100.0 }, " \
        "{ \"word\": \"foobar\", \"value\": 10000.0 }" \
        "] " \
        "} " \
        "}";

    json_error_t error;
    json_t* pass_json = json_loads(pass_json_string, 0, &error);

    bomm_letter_t f = bomm_message_letter_from_ascii('f');
    bomm_letter_t o = bomm_message_letter_from_ascii('o');

    bomm_pass_t pass;
    cr_assert_eq(bomm_pass_init_with_json(&pass, pass_json), &pass);
    cr_assert_eq(pass.type, BOMM_PASS_TRIE);
    cr_assert_neq(pass.config.trie.trie, NULL);

    cr_assert_neq(pass.config.trie.trie->children[f], NULL);
    cr_assert_neq(pass.config.trie.trie->children[f]->children[o], NULL);

    bomm_trie_t* trie_foo = pass.config.trie.trie->children[f]->children[o]->children[o];
    cr_assert_neq(trie_foo, NULL);
    cr_assert_eq(trie_foo->value, 1.0);

    bomm_pass_destroy(&pass);
}

Test(wiring, bomm_pass_init_json_unknown) {
    const char* pass_json_string =
        "{ " \
        "\"type\": \"unknown\", " \
        "\"foo\": \"bar\" " \
        "}";

    json_error_t error;
    json_t* pass_json = json_loads(pass_json_string, 0, &error);
    cr_assert_eq(bomm_pass_init_with_json(NULL, pass_json), NULL);
}
