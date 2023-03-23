//
//  utility.h
//  Bomm
//
//  Created by Fränz Friederes on 23/03/2023.
//

#ifndef utility_h
#define utility_h

/**
 * Calculate the modulo operation on the given numbers. Assumes `b > 0`.
 */
inline static int bomm_mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

#endif /* utility_h */
