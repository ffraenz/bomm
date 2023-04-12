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

#define BOMM_WHEEL_NAME_MAX_LENGTH 16

/**
 * Struct representing a wheel without its state (e.g. ring setting, position).
 */
typedef struct _bomm_wheel {
    /**
     * Wheel name (null terminated string of max. 15 chars)
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
 * Initialize a wheel with the given name, wiring, and turnovers.
 */
bomm_wheel_t* bomm_wheel_init(
    bomm_wheel_t* wheel,
    const char* name,
    const char* wiring_string,
    const char* turnovers_string
);

/**
 * Extract a wheel from the given JSON object.
 */
bomm_wheel_t* bomm_wheel_extract_json(bomm_wheel_t* wheel, json_t* wheel_json);

/**
 * Extract a set of wheels from the given JSON array.
 */
bool bomm_wheel_set_extract_json(
    bomm_wheel_t* wheel_set[],
    unsigned int wheel_set_size,
    json_t* wheel_set_json,
    bomm_wheel_t wheels[],
    unsigned int wheel_count
);

#endif /* wheel_h */
