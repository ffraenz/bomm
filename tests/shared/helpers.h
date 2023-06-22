//
//  helpers.h
//  Bomm
//
//  Created by Fränz Friederes on 22/06/2023.
//

#ifndef helpers_h
#define helpers_h

// Macro testing if the Latin alphabet is being used. If not, it is reported to
// the console and the function is returned.
#define bomm_test_skip_if_non_latin_alphabet \
    if (strcmp(BOMM_ALPHABET, BOMM_ALPHABET_LATIN) != 0) { \
        printf( \
            "Skipping test %s:%d due to non-Latin alphabet.\n", \
            __FILE__, \
            __LINE__ \
        ); \
        return; \
    } \

#endif /* helpers_h */
