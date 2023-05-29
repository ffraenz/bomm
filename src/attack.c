//
//  attack.c
//  Bomm
//
//  Created by Fränz Friederes on 20/03/2023.
//

#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE

#include "attack.h"
#include "pass.h"

void* bomm_attack_thread(void* arg) {
    // The argument is assumed to be an attack
    bomm_attack_t* attack = (bomm_attack_t*) arg;
    bomm_attack_key_space(attack);
    return NULL;
}

void bomm_attack_key_space(bomm_attack_t* attack) {
    double score;
    double min_score = -INFINITY;
    unsigned int plugboard[BOMM_ALPHABET_SIZE];
    char hold_preview[BOMM_HOLD_PREVIEW_SIZE];
    unsigned int i;

    // Allocate messages on the stack
    size_t message_size = bomm_message_size_for_length(attack->ciphertext->length);
    bomm_message_t *plaintext = alloca(message_size);
    bomm_message_t *ciphertext = alloca(message_size);
    memcpy(ciphertext, attack->ciphertext, message_size);

    // Allocate scrambler on the stack
    bomm_scrambler_t *scrambler = alloca(bomm_scrambler_size(ciphertext->length));
    scrambler->length = ciphertext->length;

    // Copy passes on the stack
    unsigned int num_passes = attack->num_passes;
    bomm_pass_t passes[BOMM_MAX_NUM_PASSES];
    memcpy(&passes, &attack->passes, num_passes * sizeof(bomm_pass_t));

    double start_timestamp = bomm_timestamp_sec();
    double batch_start_timestamp = start_timestamp;
    double batch_duration_sec;
    bool cancelling = false;
    unsigned long num_keys = bomm_key_space_count(&attack->key_space);
    unsigned int num_batch_keys = 26 * 26 * 8;
    unsigned int num_batch_keys_completed = 0;

    bomm_key_iterator_t key_iterator;
    if (bomm_key_iterator_init(&key_iterator, &attack->key_space) == NULL) {
        // Key space is empty
        return;
    }

    // Initial progress update
    pthread_mutex_lock(&attack->mutex);
    attack->progress.num_batch_units = num_batch_keys;
    attack->progress.num_units = num_keys;
    attack->progress.num_units_completed = 0;
    attack->progress.batch_duration_sec = 0;
    attack->progress.duration_sec = 0;
    pthread_mutex_unlock(&attack->mutex);

    // Iterate over keys in the key space
    do {
        if (key_iterator.scrambler_changed) {
            bomm_enigma_generate_scrambler(scrambler, &key_iterator.key);

            // TODO: Remove debugging code
            // bomm_key_debug(&key_iterator.key);
        }

        // Make a working copy of the plugboard
        memcpy(plugboard, key_iterator.key.plugboard, sizeof(plugboard));

        // Iterate over passes
        score = 0;
        for (i = 0; i < num_passes; i++) {
            score = bomm_pass_run(
                &passes[i],
                plugboard,
                scrambler,
                ciphertext,
                score
            );
            if (score > min_score) {
                bomm_scrambler_encrypt(scrambler, plugboard, ciphertext, plaintext);
                bomm_message_stringify(hold_preview, sizeof(hold_preview), plaintext);

                bomm_key_t key;
                memcpy(&key, &key_iterator.key, sizeof(key));
                memcpy(key.plugboard, plugboard, sizeof(plugboard));
                min_score = bomm_hold_add(attack->query->hold, score, &key, hold_preview);
            }
        }

        // Report the progress every time a batch has been finalized
        if (++num_batch_keys_completed >= num_batch_keys) {
            // Measure time
            batch_duration_sec = batch_start_timestamp;
            batch_start_timestamp = bomm_timestamp_sec();
            batch_duration_sec = batch_start_timestamp - batch_duration_sec;

            // Intermediate progress update
            pthread_mutex_lock(&attack->mutex);
            attack->progress.num_units_completed += num_batch_keys_completed;
            attack->progress.duration_sec = batch_start_timestamp - start_timestamp;
            attack->progress.batch_duration_sec = batch_duration_sec;
            cancelling = attack->state == BOMM_ATTACK_STATE_CANCELLING;
            pthread_mutex_unlock(&attack->mutex);

            // Reset counter
            num_batch_keys_completed = 0;
        }
    } while (!cancelling && !bomm_key_iterator_next(&key_iterator));

    // Final progress update
    pthread_mutex_lock(&attack->mutex);
    attack->progress.num_units_completed += num_batch_keys_completed;
    attack->state = cancelling ? BOMM_ATTACK_STATE_CANCELLED : BOMM_ATTACK_STATE_COMPLETED;
    pthread_mutex_unlock(&attack->mutex);
}
