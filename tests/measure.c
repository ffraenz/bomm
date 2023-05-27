//
//  measure.c
//  Bomm
//
//  Created by Fränz Friederes on 24/03/2023.
//

#include <criterion/criterion.h>
#include "../src/measure.h"

#define epsilon 0.00000000000000000001

Test(message, bomm_measure_from_string) {
    cr_assert_eq(bomm_measure_from_string("sinkov_bigram"), BOMM_MEASURE_SINKOV_BIGRAM);
    cr_assert_eq(bomm_measure_from_string("sinkov_trigram"), BOMM_MEASURE_SINKOV_TRIGRAM);
    cr_assert_eq(bomm_measure_from_string("ic"), BOMM_MEASURE_IC);
    cr_assert_eq(bomm_measure_from_string("ic_bigram"), BOMM_MEASURE_IC_BIGRAM);
    cr_assert_eq(bomm_measure_from_string("entropy"), BOMM_MEASURE_ENTROPY);
    cr_assert_eq(bomm_measure_from_string("entropy_bigram"), BOMM_MEASURE_ENTROPY_BIGRAM);
    cr_assert_eq(bomm_measure_from_string("none"), BOMM_MEASURE_NONE);
    cr_assert_eq(bomm_measure_from_string("unknown"), BOMM_MEASURE_NONE);
}

Test(message, bomm_measure_to_string) {
    cr_assert_str_eq(bomm_measure_to_string(BOMM_MEASURE_SINKOV_BIGRAM), "sinkov_bigram");
    cr_assert_str_eq(bomm_measure_to_string(BOMM_MEASURE_SINKOV_TRIGRAM), "sinkov_trigram");
    cr_assert_str_eq(bomm_measure_to_string(BOMM_MEASURE_IC), "ic");
    cr_assert_str_eq(bomm_measure_to_string(BOMM_MEASURE_IC_BIGRAM), "ic_bigram");
    cr_assert_str_eq(bomm_measure_to_string(BOMM_MEASURE_ENTROPY), "entropy");
    cr_assert_str_eq(bomm_measure_to_string(BOMM_MEASURE_ENTROPY_BIGRAM), "entropy_bigram");
    cr_assert_str_eq(bomm_measure_to_string(BOMM_MEASURE_NONE), "none");
}

Test(message, bomm_measure_message_frequency) {
    bomm_message_t *message;
    unsigned int frequencies[BOMM_ALPHABET_SIZE];

    message = bomm_message_init("ff");
    bomm_measure_message_frequency(1, frequencies, message);
    cr_assert_eq(bomm_measure_frequency_ic(1, frequencies), 26.0);
    cr_assert_eq(bomm_measure_message(BOMM_MEASURE_IC, message), 26.0);
    cr_assert_eq(bomm_measure_frequency_entropy(1, frequencies), 0.0);
    cr_assert_eq(bomm_measure_message(BOMM_MEASURE_ENTROPY, message), 0.0);
    free(message);

    message = bomm_message_init("fo");
    bomm_measure_message_frequency(1, frequencies, message);
    cr_assert_eq(bomm_measure_frequency_ic(1, frequencies), 0.0);
    cr_assert_eq(bomm_measure_message(BOMM_MEASURE_IC, message), 0.0);
    cr_assert_eq(bomm_measure_frequency_entropy(1, frequencies), 1.0);
    cr_assert_eq(bomm_measure_message(BOMM_MEASURE_ENTROPY, message), 1.0);
    free(message);

    message = bomm_message_init("the quick brown fox jumps over the lazy dog");
    bomm_measure_message_frequency(1, frequencies, message);
    unsigned int expected_frequencies[BOMM_ALPHABET_SIZE] = {
        1, 1, 1, 1, 3, 1, 1, 2, 1, 1,
        1, 1, 1, 1, 4, 1, 1, 2, 1, 2,
        2, 1, 1, 1, 1, 1
    };
    cr_assert_arr_eq(frequencies, expected_frequencies, sizeof(expected_frequencies));
    cr_assert_leq(
        bomm_measure_frequency_ic(1, frequencies) - 0.5680672526359558105,
        epsilon
    );
    cr_assert_leq(
        bomm_measure_message(BOMM_MEASURE_IC, message) - 0.56806725263595581055,
        epsilon
    );
    cr_assert_leq(
        bomm_measure_frequency_entropy(1, frequencies) - 4.53628635406494140625,
        epsilon
    );
    cr_assert_leq(
        bomm_measure_message(BOMM_MEASURE_ENTROPY, message) - 4.53628635406494140625,
        epsilon
    );
    free(message);
}

Test(measure, bomm_measure_ngram_map_init) {
    bomm_message_t *message;
    bomm_ngram_map_t* trigram_map = bomm_measure_ngram_map_init(3, "./data/frequencies/enigma1941-trigram.txt");

    // Testing a top frequency: EIN (4 8 13)
    cr_assert_leq(
        trigram_map->map[4 * 26 * 26 + 8 * 26 + 13] - 4.51312255859375000000,
        epsilon
    );
    message = bomm_message_init("ein");
    cr_assert_leq(
        bomm_measure_message_sinkov(3, message) - 4.51312255859375000000,
        epsilon
    );
    cr_assert_leq(
        bomm_measure_message(BOMM_MEASURE_SINKOV_TRIGRAM, message) - 4.51312255859375000000,
        epsilon
    );
    free(message);

    // Testing a mid frequency: NIE (13 8 4)
    cr_assert_leq(
        trigram_map->map[13 * 26 * 26 + 8 * 26 + 4] - 6.89060926437377929688,
        epsilon
    );
    message = bomm_message_init("nie");
    cr_assert_leq(
        bomm_measure_message_sinkov(3, message) - 6.89060926437377929688,
        epsilon
    );
    cr_assert_leq(
        bomm_measure_message(BOMM_MEASURE_SINKOV_TRIGRAM, message) - 6.89060926437377929688,
        epsilon
    );
    free(message);

    // Testing a bottom frequency: NAA (13 0 0)
    cr_assert_leq(
        trigram_map->map[13 * 26 * 26] - 9.78098106384277343750,
        epsilon
    );

    // Testing a frequency not listed: XXX (23 23 23)
    cr_assert_leq(
        trigram_map->map[23 * 26 * 26 + 23 * 26 + 23] - 10.47412776947021484375,
        epsilon
    );

    free(trigram_map);

    trigram_map = bomm_measure_ngram_map_init(3, "./data/frequencies/en-trigram.txt");

    // Testing a top frequency: THE (19 7 4)
    cr_assert_leq(
        trigram_map->map[19 * 26 * 26 + 7 * 26 + 4] - 4.00961589813232421875,
        epsilon
    );

    // Testing a mid frequency: DAV (3 0 21)
    cr_assert_leq(
        trigram_map->map[3 * 26 * 26 + 21] - 9.24836921691894531250,
        epsilon
    );

    // Testing a bottom frequency: XXX (23 23 23)
    cr_assert_leq(
        trigram_map->map[23 * 26 * 26 + 23 * 26 + 23] - 13.29916191101074218750,
        epsilon
    );

    free(trigram_map);
}
