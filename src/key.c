//
//  key.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include "key.h"

void bomm_key_serialize(char* str, size_t size, bomm_key_t* key) {
    // Clear string
    str[0] = 0;
    
    // Compose wheel order
    for (unsigned int slot = 0; slot < key->model->slot_count; slot++) {
        char* name = key->model->rotors[key->model->slot_rotor_indices[slot][key->slot_rotor[slot]]].name;
        snprintf(str, size, "%s%s%7s", str, slot == 0 ? "" : ":", name);
    }
    
    // Compose ring settings
    snprintf(str, size, "%s |", str);
    for (unsigned int slot = 0; slot < key->model->slot_count; slot++) {
        snprintf(str, size, "%s %2d", str, key->slot_rings[slot] + 1);
    }
    
    // Compose start positions
    snprintf(str, size, "%s |", str);
    for (unsigned int slot = 0; slot < key->model->slot_count; slot++) {
        snprintf(str, size, "%s %2d", str, key->slot_positions[slot] + 1);
    }
}

void* bomm_key_leaderboard_alloc(unsigned int size) {
    bomm_key_leaderboard_t* leaderboard =
        malloc(sizeof(bomm_key_leaderboard_t) + sizeof(bomm_key_scored_t) * size);
    leaderboard->size = size;
    leaderboard->count = 0;
    return leaderboard;
}

float bomm_key_leaderboard_add(bomm_key_leaderboard_t* leaderboard, bomm_key_t* key, float score) {
    // Bail out if scored key does not fit on the leaderboard
    if (leaderboard->count == leaderboard->size && leaderboard->entries[leaderboard->count - 1].score > score) {
        return leaderboard->entries[leaderboard->count - 1].score;
    }
    
    // Select last position
    int index;
    if (leaderboard->count < leaderboard->size) {
        index = leaderboard->count++;
    } else {
        index = leaderboard->size - 1;
    }
    
    // Move up the leaderboard until finding the right spot
    while (index > 0 && leaderboard->entries[index - 1].score < score) {
        memcpy(&leaderboard->entries[index], &leaderboard->entries[index - 1], sizeof(bomm_key_scored_t));
        index--;
    }

    // Add key and score to the leaderboard
    memcpy(&leaderboard->entries[index].key, key, sizeof(bomm_key_t));
    leaderboard->entries[index].score = score;
    
    printf("LEADERBOARD:\n");
    bomm_key_leaderboard_print(leaderboard);
    
    // Return new minimum score necessary to enter the leaderboard
    if (leaderboard->count == leaderboard->size) {
        return leaderboard->entries[leaderboard->count - 1].score;
    } else {
        return -INFINITY;
    }
}

void bomm_key_leaderboard_print(bomm_key_leaderboard_t* leaderboard) {
    char key_string[128];
    for (unsigned int i = 0; i < leaderboard->count; i++) {
        bomm_key_serialize(key_string, 128, &leaderboard->entries[i].key);
        printf("%3d. %s %f\n", i + 1, key_string, leaderboard->entries[i].score);
    }
}
