//
//  config.h
//  Bomm
//
//  Created by Fränz Friederes on 22/06/2023.
//

#ifndef config_h
#define config_h

#define BOMM_ALPHABET_LATIN "abcdefghijklmnopqrstuvwxyz"

#ifndef BOMM_ALPHABET
    #define BOMM_ALPHABET BOMM_ALPHABET_LATIN
#endif

#define BOMM_ALPHABET_SIZE ((unsigned int) strlen(BOMM_ALPHABET))

#endif /* config_h */
