//
//  message.c
//  Bomm
//
//  Created by Fränz Friederes on 17/03/2023.
//

#include <criterion/criterion.h>
#include "shared/helpers.h"
#include "../src/message.h"
#include "../src/measure.h"
#include "../src/utility.h"

Test(message, test_alphabet) {
    cr_expect_eq(BOMM_ALPHABET_SIZE % 2, 0,
        "The alphabet must contain an even number of letters");
    cr_expect_eq(bomm_str_unique(BOMM_ALPHABET), true,
        "The alphabet must not contain duplicate letters");
}

Test(message, bomm_letter_t) {
    cr_expect_gt((bomm_letter_t)~0, BOMM_ALPHABET_SIZE,
        "Letters of the alphabet must fit into the bomm_letter_t type");
}

Test(message, bomm_message_init) {
    bomm_message_t* message;
    char* expected_string;

    expected_string = BOMM_ALPHABET;
    message = bomm_message_init(expected_string);
    char string[bomm_message_serialize_size(message)];
    bomm_message_stringify(string, -1, message);
    cr_assert_str_eq(string, expected_string);
    cr_assert_eq(message->length, strlen(expected_string));
    free(message);
}

Test(message, bomm_message_the_quick_brown_fox, BOMM_TEST_DISABLE_FOR_NON_LATIN_ALPHABET) {
    char* input_string = "the quick brown fox jumps over the lazy dog";
    char* expected_string = "thequickbrownfoxjumpsoverthelazydog";
    bomm_message_t* message = bomm_message_init(input_string);
    cr_assert_eq(message->length, strlen(expected_string));
    char string[bomm_message_serialize_size(message)];
    bomm_message_stringify(string, -1, message);
    cr_assert_str_eq(string, expected_string);
    free(message);
}
