//
//  measure.c
//  Bomm
//
//  Created by Fränz Friederes on 24/03/2023.
//

#include <criterion/criterion.h>
#include "../src/measure.h"

Test(message, bomm_measure_frequency) {
    bomm_message_t *message;
    unsigned int frequencies[BOMM_ALPHABET_SIZE];
    
    message = bomm_message_init("ff");
    bomm_measure_message_frequency(1, frequencies, message);
    cr_assert_eq(bomm_measure_frequency_ic(1, frequencies), 26.0);
    cr_assert_eq(bomm_measure_frequency_entropy(1, frequencies), 0.0);
    free(message);

    message = bomm_message_init("fo");
    bomm_measure_message_frequency(1, frequencies, message);
    cr_assert_eq(bomm_measure_frequency_ic(1, frequencies), 0.0);
    cr_assert_eq(bomm_measure_frequency_entropy(1, frequencies), 1.0);
    free(message);

    message = bomm_message_init("the quick brown fox jumps over the lazy dog");
    bomm_measure_message_frequency(1, frequencies, message);
    unsigned int expected_frequencies[BOMM_ALPHABET_SIZE] = {
        1, 1, 1, 1, 3, 1, 1, 2, 1, 1,
        1, 1, 1, 1, 4, 1, 1, 2, 1, 2,
        2, 1, 1, 1, 1, 1
    };
    cr_assert_arr_eq(frequencies, expected_frequencies, sizeof(expected_frequencies));
    cr_assert_eq(bomm_measure_frequency_ic(1, frequencies), 0.56806725263595581055);
    cr_assert_eq(bomm_measure_frequency_entropy(1, frequencies), 4.53628635406494140625);
    free(message);
}

Test(measure, bomm_measure_ngram_map_init) {
    bomm_ngram_map_t* trigram_map = bomm_measure_ngram_map_init(3, "./data/frequencies/enigma1941-trigram.txt");

    // Testing a top frequency: EIN (4 8 13)
    cr_assert_eq(trigram_map->map[4 * 26 * 26 + 8 * 26 + 13], -4.51312255859375000000);

    // Testing a mid frequency: NIE (13 8 4)
    cr_assert_eq(trigram_map->map[13 * 26 * 26 + 8 * 26 + 4], -6.89060926437377929688);

    // Testing a bottom frequency: NAA (13 0 0)
    cr_assert_eq(trigram_map->map[13 * 26 * 26], -9.78098106384277343750);

    // Testing a frequency not listed: XXX (23 23 23)
    cr_assert_eq(trigram_map->map[23 * 26 * 26 + 23 * 26 + 23], -10.47412776947021484375);

    free(trigram_map);

    trigram_map = bomm_measure_ngram_map_init(3, "./data/frequencies/en-trigram.txt");

    // Testing a top frequency: THE (19 7 4)
    cr_assert_eq(trigram_map->map[19 * 26 * 26 + 7 * 26 + 4], -4.00961589813232421875);

    // Testing a mid frequency: DAV (3 0 21)
    cr_assert_eq(trigram_map->map[3 * 26 * 26 + 21], -9.24836921691894531250);

    // Testing a bottom frequency: XXX (23 23 23)
    cr_assert_eq(trigram_map->map[23 * 26 * 26 + 23 * 26 + 23], -13.29916191101074218750);

    free(trigram_map);
}
