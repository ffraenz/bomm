//
//  measure.c
//  Bomm
//
//  Created by Fränz Friederes on 24/03/2023.
//

#include <criterion/criterion.h>
#include "../src/measure.h"

Test(message, bomm_measure_ic) {
    bomm_message_t *message;

    message = bomm_message_alloc("ff");
    cr_assert_eq(bomm_measure_ic(message), 26.0);
    free(message);

    message = bomm_message_alloc("fo");
    cr_assert_eq(bomm_measure_ic(message), 0.0);
    free(message);

    message = bomm_message_alloc("the quick brown fox jumps over the lazy dog");
    cr_assert_eq(bomm_measure_ic(message), 0.56806725263595581055);
    free(message);
}

Test(measure, bomm_measure_trigram_map_alloc) {
    unsigned char* trigram_map = bomm_measure_ngram_map_alloc(3, "/Users/ff/Projects/Bachelor/bomm/data/enigma1941-trigram.txt");

    // Testing a top frequency: EIN (4 8 13)
    cr_assert_eq(trigram_map[4 * 26 * 26 + 8 * 26 + 13], 194);

    // Testing a mid frequency: NIE (13 8 4)
    cr_assert_eq(trigram_map[13 * 26 * 26 + 8 * 26 + 4], 18);

    // Testing a bottom frequency: NAA (13 0 0)
    cr_assert_eq(trigram_map[13 * 26 * 26], 1);

    // Testing a frequency not listed: XXX (23 23 23)
    cr_assert_eq(trigram_map[23 * 26 * 26 + 23 * 26 + 23], 0);

    free(trigram_map);

    trigram_map = bomm_measure_ngram_map_alloc(3, "/Users/ff/Projects/Bachelor/bomm/data/en-trigram.txt");

    // Testing a top frequency: THE (19 7 4)
    cr_assert_eq(trigram_map[19 * 26 * 26 + 7 * 26 + 4], 255);

    // Testing a mid frequency: DAV (3 0 21)
    cr_assert_eq(trigram_map[3 * 26 * 26 + 21], 1);

    // Testing a bottom frequency: XXX (23 23 23)
    cr_assert_eq(trigram_map[23 * 26 * 26 + 23 * 26 + 23], 0);

    free(trigram_map);
}
