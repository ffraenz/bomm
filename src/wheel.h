//
//  wheel.h
//  Bomm
//
//  Created by Fränz Friederes on 11/04/2023.
//

#ifndef wheel_h
#define wheel_h

#include <jansson.h>
#include "lettermask.h"
#include "utility.h"
#include "wiring.h"

#define BOMM_WHEEL_NAME_MAX_LENGTH 16

/**
 * Struct representing a wheel without its state (e.g. ring setting, position).
 */
typedef struct _bomm_wheel {
    /**
     * Wheel name (null terminated string of max. 15 chars)
     * The wheel name is assumed to be at least one character long as an
     * immediate null character is interpreted as the end of a wheel set.
     */
    char name[BOMM_WHEEL_NAME_MAX_LENGTH];

    /**
     * Wheel wiring
     */
    bomm_wiring_t wiring;

    /**
     * Wheel turnovers
     */
    bomm_lettermask_t turnovers;
} bomm_wheel_t;

/**
 * Struct describing a wheel with strings used to statically store known wheels
 */
typedef struct _bomm_wheel_spec {
    /**
     * Wheel name
     */
    const char* name;

    /**
     * Wheel wiring string
     */
    const char* wiring;

    /**
     * Turnover lettermask string
     */
    const char* turnovers;
} bomm_wheel_spec_t;

/**
 * Set of known wheels that can be referenced by name; Known wheels include the
 * ones used in Enigma models I, M1, M2, M3, and M4.
 */
static const bomm_wheel_spec_t bomm_known_wheels[] = {
    { "ETW-ABC",    "abcdefghijklmnopqrstuvwxyz", ""   },
    { "ETW-QWE",    "jwulcmnohpqzyxiradkegvbtsf", ""   },
    { "I",          "ekmflgdqvzntowyhxuspaibrcj", "q"  },
    { "II",         "ajdksiruxblhwtmcqgznpyfvoe", "e"  },
    { "III",        "bdfhjlcprtxvznyeiwgakmusqo", "v"  },
    { "IV",         "esovpzjayquirhxlnftgkdcmwb", "j"  },
    { "V",          "vzbrgityupsdnhlxawmjqofeck", "z"  },
    { "VI",         "jpgvoumfyqbenhzrdkasxlictw", "mz" },
    { "VII",        "nzjhgrcxmyswboufaivlpekqdt", "mz" },
    { "VIII",       "fkqhtlxocbjspdzramewniuygv", "mz" },
    { "beta",       "leyjvcnixwpbqmdrtakzgfuhos", ""   },
    { "gamma",      "fsokanuerhmbtiycwlqpzxvgjd", ""   },
    { "UKW-A",      "ejmzalyxvbwfcrquontspikhgd", ""   },
    { "UKW-B",      "yruhqsldpxngokmiebfzcwvjat", ""   },
    { "UKW-C",      "fvpjiaoyedrzxwgctkuqsbnmhl", ""   },
    { "UKW-B-thin", "enkqauywjicopblmdxzvfthrgs", ""   },
    { "UKW-C-thin", "rdobjntkvehmlfcwzaxgyipsuq", ""   }
};

/**
 * Initialize a wheel with the given name, wiring, and turnovers.
 */
bomm_wheel_t* bomm_wheel_init(
    bomm_wheel_t* wheel,
    const char* name,
    const char* wiring_string,
    const char* turnovers_string
);

/**
 * Initialize a known wheel from its name.
 */
bomm_wheel_t* bomm_wheel_init_with_name(bomm_wheel_t* wheel, const char* name);

/**
 * Initialize a wheel from the given JSON object.
 */
bomm_wheel_t* bomm_wheel_init_with_json(bomm_wheel_t* wheel, json_t* wheel_json);

/**
 * Initialize a wheel set from the given JSON array.
 */
bool bomm_wheel_set_init_with_json(
    bomm_wheel_t wheel_set[],
    unsigned int wheel_set_size,
    json_t* wheel_set_json,
    bomm_wheel_t wheels[],
    unsigned int num_wheels
);

#endif /* wheel_h */
