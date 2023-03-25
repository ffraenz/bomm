//
//  message.c
//  Bomm
//
//  Created by Fränz Friederes on 17/03/2023.
//

#include <criterion/criterion.h>
#include "../src/message.h"
#include "../src/measure.h"

Test(message, bomm_letter_t) {
    cr_assert_geq((bomm_letter_t)~0, BOMM_ALPHABET_SIZE,
        "The alphabet is expected to fit into a single letter");
}

Test(message, bomm_message_alloc) {
    bomm_message_t* message;
    char* expected_string;
    
    expected_string = BOMM_ALPHABET;
    message = bomm_message_alloc(expected_string);
    char string[bomm_message_serialize_size(message)];
    bomm_message_serialize(string, -1, message);
    cr_assert_str_eq(string, expected_string);
    cr_assert_eq(message->length, strlen(expected_string));
    free(message);
}

Test(message, bomm_message_the_quick_brown_fox) {
    char* input_string = "the quick brown fox jumps over the lazy dog";
    char* expected_string = "thequickbrownfoxjumpsoverthelazydog";
    bomm_message_t* message = bomm_message_alloc(input_string);
    cr_assert_eq(message->length, strlen(expected_string));
    char string[bomm_message_serialize_size(message)];
    bomm_message_serialize(string, -1, message);
    cr_assert_str_eq(string, expected_string);
    unsigned int expected_frequency[BOMM_ALPHABET_SIZE] = {
        1, 1, 1, 1, 3, 1, 1, 2, 1, 1,
        1, 1, 1, 1, 4, 1, 1, 2, 1, 2,
        2, 1, 1, 1, 1, 1
    };
    cr_assert_arr_eq(message->frequency, expected_frequency, BOMM_ALPHABET_SIZE * sizeof(unsigned int));
    free(message);
}
