//
//  turnovers.h
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#ifndef turnovers_h
#define turnovers_h

#include <stdio.h>
#include <stdbool.h>
#include "message.h"
#include "wiring.h"

#define bomm_is_turnover(turnovers, x) ((turnovers >> x) & 0x1)

/**
 * Turnovers (bit encoded)
 * The size of the alphabet in bits need to fit into this value.
 */
typedef unsigned long bomm_turnovers_t;

/**
 * Load the given turnovers string into memory at the specified pointer.
 */
void bomm_load_turnovers(bomm_turnovers_t* ptr, char* turnovers_string);

/**
 * Export the given turnovers to a string.
 */
char* bomm_describe_turnovers(bomm_turnovers_t* turnovers);

#endif /* turnovers_h */
