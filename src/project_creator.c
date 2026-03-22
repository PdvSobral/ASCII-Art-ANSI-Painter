#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "linked_lists.h"

#define MODE_SELECTOR "create"

char DEFAULT_OUTPUT_NAME[4] = "out";
char NULL_STRING[5] = "null";
char* OUTPUT_NAME = DEFAULT_OUTPUT_NAME;
char* INPUT_NAME = NULL;

void print_help(char** argv, char* mode){ //mode = "create"
    printf("Usage: %s %s [OPTIONS...] input_file\n", argv[0], mode);
    printf("input_file : File to turn into project\n");
    printf("\nOPTIONS:\n");
    printf("\t<-h, --help> : Show the program help/usage message.\n");
    printf("\t<-o, --output> output_name : Define custom output project name. All chars must be respect the [azAZ.-] regex.\n");
    printf("\nNOTE:\n");
    printf("\tProvided Options are processed in order. Consider using --help either alone or as first provided option.");
    fflush(stdout);
    return;
}


int32_t main(int32_t argc, char** argv){
    // we will assume mode already taken care of.
    if (argc <= 2) {
        fprintf(stderr, "No input file provided!");
        return 1;
    }

    // iterate over arguments, skipping name and mode
    for (int32_t i = 2; i < argc; i++){
        if ((strcmp(argv[i], "--help") == 0) || strcmp(argv[i], "-h") == 0) {
            print_help(argv, MODE_SELECTOR);
            return 0;
        }

        if ((strcmp(argv[i], "--output") == 0) || strcmp(argv[i], "-o") == 0) {
            if (++i < argc) {
                OUTPUT_NAME = argv[i];

                uint64_t l_n = strlen(OUTPUT_NAME); //byte-size of str, no 0x00
                for (uint64_t j = 0; j < l_n; j++) {
                    char c = OUTPUT_NAME[j];

                    if (!( (c >= 0x61 && c <= 0x7A) || (c >= 0x41 && c <= 0x5A) || (c >= 0x30 && c <= 0x39) || c == 0x2D || c == 0x2E )) {
                        void* char_ = (void*) (OUTPUT_NAME+j); // get absolute control on memory

                        if ((c & 0x80) == 0) *( ( (uint8_t*) char_) + 1) = 0x00; // 1-byte character (ASCII)
                        // Now for the rest I need also to check for actual string size
                        // TODO: add check for valid continuation byte
                        else if (((c & 0xE0) == 0xC0) && (j+2 <= l_n) ) *( ( (uint8_t*) char_) + 2) = 0x00; // 2-byte character
                        else if (((c & 0xF0) == 0xE0) && (j+3 <= l_n) ) *( ( (uint8_t*) char_) + 3) = 0x00; // 3-byte character
                        else if (((c & 0xF8) == 0xF0) && (j+4 <= l_n) ) *( ( (uint8_t*) char_) + 4) = 0x00; // 4-byte character
                        // invalid sequence, maybe another encoding. simply print null
                        else char_ = (void*) NULL_STRING;

                        fprintf(stderr, "Invalid character '%s' found in output name!\n", ((char*) char_));
                        return 1;
                    }
                }
            } else {
                fprintf(stderr, "No output project name provided!");
                return 1;
            }
        } else if ((i+1) == argc){
            INPUT_NAME = argv[i];
        } else {
            fprintf(stderr, "Unrecognized option '%s'!\nUse '--help' to see usage.", argv[i]);
            return 1;
        }
    }

    if (INPUT_NAME == NULL) {
        fprintf(stderr, "No input file provided!");
        return 1;
    }

    // TODO: Add the rest of the translation here,
    //  as of now, this is the GPT translation
    /*
    FILE* input_file = fopen(INPUT_NAME, "rt");
    if (!input_file) {
        fprintf(stderr, "Failed to open input file '%s'\n", INPUT_NAME);
        return 1;
    }

    size_t lines_alloc = 128;
    size_t line_count = 0;
    char** blueprint_lines = malloc(lines_alloc * sizeof(char*));
    if (!blueprint_lines) return 1;

    char buffer[4096];
    int32_t max_len = 0;

    while (fgets(buffer, sizeof(buffer), input_file)) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') buffer[--len] = '\0';
        if (len > max_len) max_len = len;

        if (line_count >= lines_alloc) {
            lines_alloc *= 2;
            blueprint_lines = realloc(blueprint_lines, lines_alloc * sizeof(char*));
            if (!blueprint_lines) return 1;
        }

        blueprint_lines[line_count] = strdup(buffer);
        if (!blueprint_lines[line_count]) return 1;
        line_count++;
    }
    fclose(input_file);

    // Trim empty lines at start and end
    size_t start = 0, end = line_count;
    while (start < end && blueprint_lines[start][0] == '\0') start++;
    while (end > start && blueprint_lines[end-1][0] == '\0') end--;

    // Write .blprt
    char blprt_filename[512];
    snprintf(blprt_filename, sizeof(blprt_filename), "%s.blprt", OUTPUT_NAME);
    FILE* blprt_file = fopen(blprt_filename, "wt");
    if (!blprt_file) return 1;

    fprintf(blprt_file, "%d\n", max_len);
    for (size_t i = start; i < end; i++) {
        fprintf(blprt_file, "%-*s\n", max_len, blueprint_lines[i]);
    }
    fclose(blprt_file);

    // Write .clr
    char clr_filename[512];
    snprintf(clr_filename, sizeof(clr_filename), "%s.clr", OUTPUT_NAME);
    FILE* clr_file = fopen(clr_filename, "wb");
    if (!clr_file) return 1;

    uint8_t default_color = 0x07;
    for (size_t i = start; i < end; i++)
        for (int j = 0; j < max_len; j++)
            fputc(default_color, clr_file);
    fclose(clr_file);

    // Cleanup
    for (size_t i = 0; i < line_count; i++) free(blueprint_lines[i]);
    free(blueprint_lines);
    */
    return 0;
}
