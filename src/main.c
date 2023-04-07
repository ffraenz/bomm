//
//  main.c
//  Bomm
//
//  Created by Fränz Friederes on 13/03/2023.
//

#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include "attack.h"
#include "message.h"
#include "measure.h"

void* bomm_report_thread_run(void* arg) {
    bomm_attack_t* attack = (bomm_attack_t*) arg;
    time_t start_time = clock();
    
    while (true) {
        sleep(2);
        time_t time = clock();
        printf("\n");
        printf("Hold after %.1fs:\n", (double) (time - start_time) / CLOCKS_PER_SEC);
        bomm_key_hold_print(attack->hold);
    }
    
    return NULL;
}

int main(void) {
    char* ciphertext_string = "gdbyy sgjxd xmalb myoye qchrn wefia jtzkn hwuwb zzexw njuuc riyhf covsa cueqa xowgk qnnae seojx ngeze mdefd jgkvp rnjbq waemb qskqs xqivk ogohm ferqr pimua qcqvk pvaka qzzph rbljn ajcaw vxhaj sxgtx xqfwe tytrx itcpn hbci";
    
    unsigned int hold_size = 40;
    unsigned int thread_count = 5;
    
    bomm_message_t* ciphertext = bomm_message_init(ciphertext_string);
    
    // Prepare attack
    bomm_attack_t* attack = bomm_attack_init(thread_count);
    attack->hold = bomm_hold_init(sizeof(bomm_key_t), hold_size);
    
    for (unsigned int i = 0; i < thread_count; i++) {
        bomm_attack_slice_t* slice = &attack->slices[i];
        
        slice->ciphertext = ciphertext;
        slice->key_space = bomm_key_space_enigma_i_init();
        
        // Limit reflector wheel set to UKW-B
        slice->key_space->wheel_sets[0][1] = NULL;
        
        // Limit left wheel set to wheel i only
        slice->key_space->wheel_sets[1][0] = slice->key_space->wheel_sets[1][i];
        slice->key_space->wheel_sets[1][1] = NULL;
        
        // Run thread
        int result = pthread_create(&slice->thread, NULL, bomm_attack_slice_run, slice);
        assert(!result);
    }
    
    // Start report thread
    pthread_t report_thread;
    int result = pthread_create(&report_thread, NULL, bomm_report_thread_run, attack);
    assert(!result);
    
    // Wait for all threads to finish
    for (unsigned int i = 0; i < thread_count; i++) {
        bomm_attack_slice_t* slice = &attack->slices[i];
        pthread_join(slice->thread, NULL);
        printf("Thread %d is done.\n", i);
    }
    
    // Kill report thread
    pthread_kill(report_thread, 0);
    
    // TODO: Destroy key spaces
    
    bomm_attack_destroy(attack);
    free(ciphertext);
    
    printf("Properly exited.\n");
    
    return 0;
}
