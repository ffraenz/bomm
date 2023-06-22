//
//  helpers.h
//  Bomm
//
//  Created by Fränz Friederes on 22/06/2023.
//

#ifndef helpers_h
#define helpers_h

#define BOMM_TEST_DISABLE_FOR_NON_LATIN_ALPHABET \
    .disabled = (strcmp(BOMM_ALPHABET, BOMM_ALPHABET_LATIN) != 0)

#endif /* helpers_h */
