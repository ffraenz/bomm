//
//  query.c
//  Bomm
//
//  Created by Fränz Friederes on 10/04/2023.
//

#include <getopt.h>
#include <limits.h>
#include "query.h"
#include "utility.h"
#include "measure.h"

static struct option _input_options[] = {
    {"help", no_argument, 0, 'h'},
    {"num-hold", no_argument, 0, 'n'},
    {"num-threads", no_argument, 0, 't'},
    {"quiet", no_argument, 0, 'q'},
    {"verbose", no_argument, 0, 'v'},
    {0, 0, 0, 0}
};

static char* _frequencies_keys[8] = {
    "monogram",
    "bigram",
    "trigram",
    "quadgram",
    "pentagram",
    "hexagram"
};

bomm_query_t* bomm_query_init(int argc, char *argv[]) {
    bool verbose = false;
    bool quiet = false;
    unsigned int hold_size = 0;
    unsigned int thread_count = 0;

    // Read options
    int option;
    int option_index = 0;
    while ((option = getopt_long(argc, argv, "hn:t:qv", _input_options, &option_index)) != -1) {
        switch (option) {
            case 'h': {
                printf("Usage: %s [-v] query_filename\n", argv[0]);
                printf("Options:\n");
                printf("  -h, --help        display this help message\n");
                printf("  -n, --num-hold    number of hold elements to collect\n");
                printf("  -t, --num-threads number of concurrent threads to use\n");
                printf("  -q, --quiet       quiet mode\n");
                printf("  -v, --verbose     verbose mode\n");
                return NULL;
            }
            case 'n': {
                unsigned long int number = strtoul(optarg, NULL, 0);
                if (number >= INT_MAX) {
                    fprintf(
                        stderr,
                        "The number of hold elements must be less than %d\n",
                        INT_MAX
                    );
                    return NULL;
                }
                hold_size = (unsigned int) number;
                break;
            }
            case 't': {
                unsigned long int number = strtoul(optarg, NULL, 0);
                if (number >= INT_MAX) {
                    fprintf(
                        stderr,
                        "The number of concurrent threads must be less than %d\n",
                        INT_MAX
                    );
                    return NULL;
                }
                thread_count = (unsigned int) number;
                break;
            }
            case 'q': {
                quiet = true;
                break;
            }
            case 'v': {
                verbose = true;
                break;
            }
        }
    }

    // Make sure the query filename is given
    if (optind != argc - 1) {
        fprintf(stderr, "Error: A single argument with the query filename is expected\n");
        return NULL;
    }

    // Apply defaults
    if (hold_size == 0) {
        hold_size = 100;
    }
    if (thread_count == 0) {
        thread_count = bomm_hardware_concurrency();
    }

    // Read the query
    char* query_filename = argv[optind];
    FILE* query_file = fopen(query_filename, "r");
    if (query_file == NULL) {
        fprintf(stderr, "Error: The query file %s cannot be read\n", query_filename);
        return NULL;
    }

    // Parse the query
    json_error_t error;
    json_t* query_json = json_loadf(query_file, 0, &error);
    fclose(query_file);
    if (query_json == NULL) {
        fprintf(stderr, "Error: JSON parsing error on line %d: %s\n", error.line, error.text);
        return NULL;
    }
    if (query_json->type != JSON_OBJECT) {
        json_decref(query_json);
        fprintf(stderr, "Error: The query is expected to be an object\n");
        return NULL;
    }

    // Read frequencies
    json_t* frequencies_json = json_object_get(query_json, "frequencies");
    if (frequencies_json != NULL) {
        if (frequencies_json->type != JSON_OBJECT) {
            json_decref(query_json);
            fprintf(stderr, "Error: The query field 'frequencies' is expected to be an object\n");
            return NULL;
        }
        for (unsigned int i = 0; i < 8; i++) {
            json_t* frequencies_filename_json = json_object_get(frequencies_json, _frequencies_keys[i]);
            if (frequencies_filename_json != NULL) {
                if (frequencies_filename_json->type != JSON_STRING) {
                    json_decref(query_json);
                    fprintf(stderr, "Error: The frequencies object is expected to hold string values\n");
                    return NULL;
                }
                if (bomm_measure_ngram_map_init(i + 1, json_string_value(frequencies_filename_json)) == NULL) {
                    json_decref(query_json);
                    return NULL;
                }
            }
        }
    }

    // Read passes
    unsigned int num_passes = 0;
    bomm_pass_t passes[BOMM_MAX_NUM_PASSES];
    json_t* passes_json = json_object_get(query_json, "passes");
    if (passes_json == NULL) {
        // Use a single default pass
        bomm_pass_init(&passes[num_passes++]);
    } else {
        if (passes_json->type != JSON_ARRAY) {
            json_decref(query_json);
            fprintf(stderr, "Error: The query field 'passes' is expected to be an array\n");
            return NULL;
        }
        unsigned long num_elements = json_array_size(passes_json);
        if (num_elements == 0 || num_elements > BOMM_MAX_NUM_PASSES) {
            json_decref(query_json);
            fprintf(stderr, "Error: The query field 'passes' must have between 1 and %d elements\n", BOMM_MAX_NUM_PASSES);
            return NULL;
        }
        num_passes = (unsigned int) num_elements;
        for (unsigned int i = 0; i < num_passes; i++) {
            if (!bomm_pass_init_json(&passes[i], json_array_get(passes_json, i))) {
                json_decref(query_json);
                fprintf(stderr, "Error: The query field 'passes' contains an invalid pass object\n");
                return NULL;
            }
        }
    }

    // Alloc query
    size_t query_size = sizeof(bomm_query_t) + thread_count * sizeof(bomm_attack_t);
    bomm_query_t* query = malloc(query_size);
    if (query == NULL) {
        json_decref(query_json);
        fprintf(stderr, "Error: Out of memory\n");
        return NULL;
    }

    // Set defaults
    bomm_strncpy(query->name, query_filename, 80);
    query->ciphertext = NULL;
    query->hold = NULL;
    query->quiet = quiet;
    query->verbose = verbose;
    query->attack_count = thread_count;

    // Use the measure of the last pass as the query measure
    query->measure = bomm_pass_result_measure(&passes[num_passes - 1]);

    // Read ciphertext
    json_t* ciphertext_json = json_object_get(query_json, "ciphertext");
    if (ciphertext_json == NULL || ciphertext_json->type != JSON_STRING) {
        bomm_query_destroy(query);
        json_decref(query_json);
        fprintf(stderr, "Error: The query field 'ciphertext' is expected to be a string\n");
        return NULL;
    }
    query->ciphertext = bomm_message_init(json_string_value(ciphertext_json));
    query->ciphertext_score = bomm_measure_message(query->measure, query->ciphertext);

    // Read wheels
    unsigned int wheel_count = 0;
    json_t* wheels_json = json_object_get(query_json, "wheels");
    if (wheels_json != NULL) {
        if (wheels_json->type != JSON_ARRAY) {
            bomm_query_destroy(query);
            json_decref(query_json);
            fprintf(stderr, "Error: The query field 'wheels' is expected to be an array\n");
            return NULL;
        }
        wheel_count = (unsigned int) json_array_size(wheels_json);
    }
    bomm_wheel_t wheels[wheel_count];
    for (unsigned int i = 0; i < wheel_count; i++) {
        if (bomm_wheel_init_with_json(
            &wheels[i],
            json_array_get(wheels_json, i)
        ) == NULL) {
            bomm_query_destroy(query);
            json_decref(query_json);
            return NULL;
        }
    }

    // Read key space
    bomm_key_space_t key_space;
    json_t* key_space_json = json_object_get(query_json, "space");
    if (bomm_key_space_init_with_json(
        &key_space,
        key_space_json,
        wheels,
        wheel_count
    ) == NULL) {
        bomm_query_destroy(query);
        json_decref(query_json);
        return NULL;
    }

    // Split the key space into the requested number of concurrent threads
    bomm_key_space_t key_space_slices[thread_count];
    unsigned int attack_count = bomm_key_space_slice(
        &key_space,
        thread_count,
        key_space_slices
    );

    if (attack_count == 0) {
        bomm_query_destroy(query);
        json_decref(query_json);
        fprintf(
            stderr,
            "Error: The key space is empty. If you have configured a single " \
            "key it may not be considered to be relevant.\n"
        );
        return NULL;
    }

    // Reduce the size of the query if necessary
    if (attack_count != thread_count) {
        query->attack_count = attack_count;
        query_size = sizeof(bomm_query_t) + attack_count * sizeof(bomm_attack_t);
        query = realloc(query, query_size);
    }

    // Initialize parallel attacks
    for (unsigned int i = 0; i < attack_count; i++) {
        bomm_attack_t* attack = &query->attacks[i];
        attack->query = query;
        attack->id = i + 1;
        memcpy(&attack->key_space, &key_space_slices[i], sizeof(bomm_key_space_t));
        attack->num_passes = num_passes;
        memcpy(&attack->passes, &passes, num_passes * sizeof(bomm_pass_t));
        attack->ciphertext = query->ciphertext;
        attack->thread = 0;
        attack->state = BOMM_ATTACK_STATE_IDLE;

        attack->progress.batch_unit_size = 1;
        attack->progress.completed_unit_count = 0;
        attack->progress.unit_count = 0;
        attack->progress.duration_sec = 0;
        attack->progress.batch_duration_sec = 0;
        pthread_mutex_init(&attack->mutex, NULL);
    }

    // Prepare hold
    query->hold = bomm_hold_init(NULL, sizeof(bomm_key_t), hold_size);

    json_decref(query_json);
    return query;
}

void bomm_query_destroy(bomm_query_t* query) {
    for (unsigned int i = 0; i < query->attack_count; i++) {
        if (query->attacks[i].ciphertext != query->ciphertext) {
            free(query->attacks[i].ciphertext);
        }
    }

    free(query->ciphertext);
    free(query->hold);
    free(query);
}

bool bomm_query_start(bomm_query_t* query) {
    // Create attack threads that work in parallel
    bool error = false;
    for (unsigned int i = 0; i < query->attack_count; i++) {
        bomm_attack_t* attack = &query->attacks[i];
        pthread_mutex_lock(&attack->mutex);
        if (attack->state == BOMM_ATTACK_STATE_IDLE || attack->state == BOMM_ATTACK_STATE_CANCELLED) {
            attack->state = BOMM_ATTACK_STATE_PENDING;
            if (pthread_create(&attack->thread, NULL, bomm_attack_thread, attack)) {
                error = true;
                attack->thread = 0;
            }
        }
        pthread_mutex_unlock(&attack->mutex);
    }

    if (error) {
        bomm_query_cancel(query);
        bomm_query_join(query);
        return true;
    }

    return false;
}

bool bomm_query_is_pending(bomm_query_t* query) {
    bool pending = false;
    unsigned int i = 0;
    while (!pending && i < query->attack_count) {
        bomm_attack_t* attack = &query->attacks[i];
        pthread_mutex_lock(&attack->mutex);
        pending = pending || attack->state == BOMM_ATTACK_STATE_PENDING;
        pthread_mutex_unlock(&attack->mutex);
        i++;
    }
    return pending;
}

void bomm_query_cancel(bomm_query_t* query) {
    // Send cancellation request to query threads
    for (unsigned int i = 0; i < query->attack_count; i++) {
        bomm_attack_t* attack = &query->attacks[i];
        pthread_mutex_lock(&attack->mutex);
        if (attack->state == BOMM_ATTACK_STATE_PENDING) {
            attack->state = BOMM_ATTACK_STATE_CANCELLING;
        }
        pthread_mutex_unlock(&attack->mutex);
    }
}

void bomm_query_join(bomm_query_t* query) {
    for (unsigned int i = 0; i < query->attack_count; i++) {
        if (query->attacks[i].thread != 0) {
            pthread_join(query->attacks[i].thread, NULL);
            query->attacks[i].thread = 0;
        }
    }
}

void bomm_query_print(bomm_query_t* query, unsigned int count) {
    bomm_progress_t joint_progress;
    joint_progress.batch_unit_size = 26;
    bomm_progress_t* attack_progress[query->attack_count];
    for (unsigned int i = 0; i < query->attack_count; i++) {
        attack_progress[i] = &query->attacks[i].progress;
    }

    char duration_string[16];
    char time_remaining_string[16];
    char message_string[80];
    char detail_string[80];
    char score_string[512];

    // Lock progress updates
    for (unsigned int i = 0; i < query->attack_count; i++) {
        pthread_mutex_lock(&query->attacks[i].mutex);
    }

    // Calculate joint progress
    bomm_progress_parallel(&joint_progress, attack_progress, query->attack_count);

    // Unlock progress updates
    for (unsigned int i = 0; i < query->attack_count; i++) {
        pthread_mutex_unlock(&query->attacks[i].mutex);
    }

    double percentage = bomm_progress_percentage(&joint_progress);
    bomm_duration_stringify(duration_string, 16, joint_progress.duration_sec);
    double time_remaining_sec = bomm_progress_time_remaining_sec(&joint_progress);
    bomm_duration_stringify(time_remaining_string, 16, time_remaining_sec);

    // Lock hold mutex while printing
    // Also makes sure multiple threads don't print at the same time
    pthread_mutex_lock(&query->hold->mutex);

    // Print header
    printf("┌──────┬───────────────────────────────────────────────────────────────────────┐\n");
    printf(
        "│ Bomm │ Progress \x1b[32m%10.3f %%\x1b[37m │ " \
        "Elapsed \x1b[32m%13.13s\x1b[37m │ " \
        "Remaining \x1b[32m%11.11s\x1b[37m │\n",
        percentage * 100,
        duration_string,
        time_remaining_string
    );
    printf("├──────┴───────────────────────────────────────────────────────────┬───────────┤\n");

    // Print hold
    for (unsigned int i = 0; i < count; i++) {
        if (i < query->hold->count) {
            bomm_hold_element_t* element = bomm_hold_at(query->hold, i);
            bomm_key_stringify(detail_string, sizeof(detail_string), (bomm_key_t*) element->data);
            snprintf(score_string, sizeof(score_string), "%+10.10f", element->score);
            printf(
                "│ \x1b[32m%-64.64s\x1b[37m   %9.9s │\n",
                element->preview,
                score_string
            );
            printf("│ %-76.76s │\n", detail_string);
        } else {
            printf("│                                                                              │\n");
            printf("│                                                                              │\n");
        }
        if (i < count - 1) {
            printf("├──────────────────────────────────────────────────────────────────┬───────────┤\n");
        }
    }

    // Print footer
    bomm_message_stringify(message_string, sizeof(message_string), query->ciphertext);
    snprintf(score_string, sizeof(score_string), "%+10.10f", query->ciphertext_score);
    snprintf(
        detail_string,
        sizeof(detail_string),
        "Unchanged ciphertext (%d letters)",
        query->ciphertext->length
    );
    printf("╞══════════════════════════════════════════════════════════════════╤═══════════╡\n");
    printf(
        "│ \x1b[32m%-64.64s\x1b[37m   %9.9s │\n",
        message_string,
        score_string
    );
    printf("│ %-76.76s │\n", detail_string);
    printf("└──────────────────────────────────────────────────────────────────────────────┘\n");

    // Unlock hold mutex
    pthread_mutex_unlock(&query->hold->mutex);
}
