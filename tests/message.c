//
//  message.c
//  Bomm
//
//  Created by Fränz Friederes on 17/03/2023.
//

#include <criterion/criterion.h>
#include "../src/message.h"

Test(message, bomm_letter_t) {
    cr_assert_geq((bomm_letter_t)~0, BOMM_ALPHABET_SIZE,
        "The alphabet is expected to fit into a single letter");
}

Test(message, bomm_message_alloc) {
    bomm_message_t* message;
    char* string;
    char* expected_string;
    
    expected_string = "abcdefghijklmnopqrstuvwxyz";
    message = bomm_alloc_message(expected_string);
    string = bomm_describe_message(message);
    cr_assert_str_eq(string, expected_string);
    cr_assert_eq(message->length, strlen(expected_string));
    free(string);
    free(message);
}

Test(message, bomm_message_the_quick_brown_fox) {
    char* input_string = "the quick brown fox jumps over the lazy dog";
    char* expected_string = "thequickbrownfoxjumpsoverthelazydog";
    bomm_message_t* message = bomm_alloc_message(input_string);
    cr_assert_eq(message->length, strlen(expected_string));
    char* string = bomm_describe_message(message);
    cr_assert_str_eq(string, expected_string);
    free(string);
    unsigned int expected_frequency[BOMM_ALPHABET_SIZE] = {
        1, 1, 1, 1, 3, 1, 1, 2, 1, 1,
        1, 1, 1, 1, 4, 1, 1, 2, 1, 2,
        2, 1, 1, 1, 1, 1
    };
    cr_assert_arr_eq(message->frequency, expected_frequency, BOMM_ALPHABET_SIZE * sizeof(unsigned int));
    free(message);
}

Test(message, bomm_message_calc_ic) {
    bomm_message_t *message;
    
    message = bomm_alloc_message("ff");
    cr_assert_eq(bomm_message_calc_ic(message), 1.0);
    free(message);
    
    message = bomm_alloc_message("fo");
    cr_assert_eq(bomm_message_calc_ic(message), 0.0);
    free(message);
    
    message = bomm_alloc_message("the quick brown fox jumps over the lazy dog");
    cr_assert_eq(bomm_message_calc_ic(message), 0.02184874005615711212);
    free(message);
}
