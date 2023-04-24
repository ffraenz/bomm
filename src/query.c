//
//  query.c
//  Bomm
//
//  Created by Fränz Friederes on 10/04/2023.
//

#include <getopt.h>
#include "query.h"
#include "utility.h"
#include "measure.h"

static struct option _input_options[] = {
    {"verbose", no_argument, 0, 'v'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

static char* _frequencies_keys[8] = {
    "monogram",
    "bigram",
    "trigram",
    "quadgram",
    "pentagram",
    "hexagram",
    "heptagram",
    "octagram"
};

bomm_query_t* bomm_query_init(int argc, char *argv[]) {
    bool verbose = false;
    unsigned int hold_size = 40;

    // Read options
    int option;
    int option_index = 0;
    while ((option = getopt_long(argc, argv, "hv", _input_options, &option_index)) != -1) {
        switch (option) {
            case 'h': {
                printf("Usage: %s [-h] filename\n", argv[0]);
                printf("Options:\n");
                printf("  -h, --help        display this help message\n");
                printf("  -v, --verbose     verbose mode\n");
                return NULL;
            }
            case 'v': {
                verbose = true;
                printf("Verbose mode enabled\n");
                break;
            }
        }
    }

    // Make sure the query filename is given
    if (optind != argc - 1) {
        fprintf(stderr, "Error: A single argument with the query filename is expected\n");
        return NULL;
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

    // Read attack count
    json_t* attacks_json = json_object_get(query_json, "attacks");
    if (attacks_json->type != JSON_ARRAY) {
        json_decref(query_json);
        fprintf(stderr, "Error: Query field 'attacks' is expected to be an array\n");
        return NULL;
    }
    unsigned int attack_count = (unsigned int) json_array_size(attacks_json);

    // Alloc query
    size_t query_size = sizeof(bomm_query_t) + attack_count * sizeof(bomm_attack_t);
    bomm_query_t* query = malloc(query_size);
    if (query == NULL) {
        json_decref(query_json);
        fprintf(stderr, "Error: Out of memory\n");
        return NULL;
    }

    // Set defaults
    query->ciphertext = NULL;
    query->hold = NULL;
    query->verbose = verbose;
    query->attack_count = attack_count;

    // Read ciphertext
    json_t* ciphertext_json = json_object_get(query_json, "ciphertext");
    if (ciphertext_json != NULL) {
        if (ciphertext_json->type != JSON_STRING) {
            bomm_query_destroy(query);
            json_decref(query_json);
            fprintf(stderr, "Error: The query field 'ciphertext' is expected to be a string\n");
            return NULL;
        }
        query->ciphertext = bomm_message_init(json_string_value(ciphertext_json));
    }

    // Read wheels
    json_t* wheels_json = json_object_get(query_json, "wheels");
    if (wheels_json != NULL) {
        if (wheels_json->type != JSON_ARRAY) {
            bomm_query_destroy(query);
            json_decref(query_json);
            fprintf(stderr, "Error: The query field 'wheels' is expected to be an array\n");
            return NULL;
        }
        query->wheel_count = (unsigned int) json_array_size(wheels_json);
        if (query->wheel_count > BOMM_QUERY_MAX_WHEEL_COUNT) {
            bomm_query_destroy(query);
            json_decref(query_json);
            fprintf(stderr, "Error: The query field 'wheels' is limited to %d elements\n", BOMM_QUERY_MAX_WHEEL_COUNT);
            return NULL;
        }
    }
    for (unsigned int i = 0; i < query->wheel_count; i++) {
        if (bomm_wheel_init_with_json(&query->wheels[i], json_array_get(wheels_json, i)) == NULL) {
            bomm_query_destroy(query);
            json_decref(query_json);
            return NULL;
        }
    }

    // Read attacks
    json_t* attack_json;
    bomm_attack_t* attack;
    for (unsigned int i = 0; i < attack_count; i++) {
        attack = &query->attacks[i];
        attack->id = i + 1;
        attack->query = query;
        attack->thread = NULL;

        attack_json = json_array_get(attacks_json, i);
        if (attack_json->type != JSON_OBJECT) {
            bomm_query_destroy(query);
            json_decref(query_json);
            fprintf(stderr, "Error: An attack is expected to be an object\n");
            return NULL;
        }

        // Read attack ciphertext
        json_t* ciphertext_json = json_object_get(attack_json, "ciphertext");
        if (ciphertext_json != NULL) {
            if (ciphertext_json->type != JSON_STRING) {
                bomm_query_destroy(query);
                json_decref(query_json);
                fprintf(stderr, "Error: The optional attack field 'ciphertext' is expected to be a string\n");
                return NULL;
            }
            attack->ciphertext = bomm_message_init(json_string_value(ciphertext_json));
        } else {
            attack->ciphertext = query->ciphertext;
        }

        // Read attack key space
        json_t* key_space_json = json_object_get(attack_json, "space");
        if (bomm_key_space_init_with_json(&attack->key_space, key_space_json, query->wheels, query->wheel_count) == NULL) {
            bomm_query_destroy(query);
            json_decref(query_json);
            return NULL;
        }
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
