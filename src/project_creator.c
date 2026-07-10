#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "linked_lists.h"

#define MODE_SELECTOR "create"
#define BUFFER_SIZE 20 // if it is big, it will use more memory but will hit the disk less frequently limited to uint16_t

char DEFAULT_OUTPUT_NAME[4] = "out";
char NULL_STRING[5] = "null";
char* OUTPUT_NAME = NULL;
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

int32_t project_creator_main(int32_t argc, char** argv){
    printf("Starting with mode 'create'...\n");
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
                OUTPUT_NAME = strdup(argv[i]); // duplicate string
                if (OUTPUT_NAME==NULL) {
                    free(INPUT_NAME);
                    fprintf(stderr, "Failed to allocate memory for the project output name!");
                    return 1;
                }
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
                        free(OUTPUT_NAME);
                        free(INPUT_NAME);
                        return 1;
                    }
                }
            } else {
                free(OUTPUT_NAME);
                free(INPUT_NAME);
                fprintf(stderr, "No output project name provided!");
                return 1;
            }
        } else if ((i+1) == argc){
            INPUT_NAME = strdup(argv[i]); // duplicate string
            if (INPUT_NAME==NULL) {
                free(OUTPUT_NAME);
                fprintf(stderr, "Failed to allocate memory for the input file name!");
                return 1;
            }
        } else {
            free(OUTPUT_NAME);
            free(INPUT_NAME);
            fprintf(stderr, "Unrecognized option '%s'!\nUse '--help' to see usage.", argv[i]);
            return 1;
        }
    }

    if (INPUT_NAME == NULL) {
        fprintf(stderr, "No input file provided!");
        return 1;
    }

    printf("Opening input file '%s'...\n", INPUT_NAME);
    FILE* input_file = fopen(INPUT_NAME, "rt");
    if (!input_file) { // maybe use printerr (if I recall it also printed stack)
        free(OUTPUT_NAME);
        free(INPUT_NAME);
        fprintf(stderr, "Failed to open input file '%s'!\n", INPUT_NAME);
        return 1;
    }

    LinkedList* blueprint_lines = create_linked_list();
    if (blueprint_lines==NULL) {
        free(OUTPUT_NAME);
        free(INPUT_NAME);
        fprintf(stderr, "Failed to allocate memory for the linked list to hold the lines!");
        return 1;
    }

    // TODO: I will need to check for overflow possibility and stop saying too big!, probably a simple & 0xFFFF
    uint16_t max_len = 0;// PROJECT can only contain uint16_t on width/length


	uint8_t buffer[BUFFER_SIZE]; // FIXME: maybe use char and/or malloc the buffer
	uint16_t current_line_length = 0; //uint16_T it's the maximum size allowed for the lines in a project
	uint16_t bytesRead;
	// will contain the result of an ftell, so since the file to read is at most uint16_t*uint16_t, uint32_t is enough to handle
	uint32_t start_of_line = ftell(input_file);
    char* pointer_holder;

    printf("Reading lines from input file...\n");
	while (1) {
		bytesRead = fread(buffer, 1, BUFFER_SIZE-1, input_file);
		if (bytesRead == 0) {
		    if (current_line_length != 0){
		        buffer[0] = 0x0A;
		        bytesRead = 1;
		    } else break;
		};

		for (uint16_t i=0; i < bytesRead; i++){
			if (buffer[i] == '\n') {
			    fseek(input_file, start_of_line, SEEK_SET); // reset to beginning of line
                if (current_line_length != 0){
                    // Allocate memory for the line (+1 for null terminator)
                    // TODO: check for null
                    pointer_holder = (char*)malloc(current_line_length + 1);

                    // Read the exact number of bytes for the line
                    // TODO: check it was read correctly
                    fread(pointer_holder, 1, current_line_length, input_file);

                    pointer_holder[current_line_length] = 0x00; // null-terminate

                    append_data_to_list(blueprint_lines, pointer_holder);
                } else append_data_to_list(blueprint_lines, NULL); // if length 0 simply add NULL

				fseek(input_file, 1, SEEK_CUR); // move one byte over (the /n)

                start_of_line = ftell(input_file); // point to new linestart
                // update max_len
                if (current_line_length > max_len) max_len = current_line_length;
				current_line_length = 0;
				break; // exit the forloop, as buffer needs to be recreated before proceeding
			} else current_line_length++; // check for overflow. currently not doing so
		}
	}
	fclose(input_file);

    // Trim empty lines at start and end (assuming the bluprint always has at least one valid line)
    if (blueprint_lines->size == 0) {
        free(OUTPUT_NAME);
        free(INPUT_NAME);
        delete_linked_list(blueprint_lines, free);
        fprintf(stderr, "No lines read in input file!\n");
        return 1;
    } else printf("Read %d lines from input file.\n", blueprint_lines->size);
    fflush(stdout);

    printf("Removing trailing empty lines...\n"); fflush(stdout);
    while (blueprint_lines->size > 0 && blueprint_lines->head->data == NULL) remove_node_at_index(blueprint_lines, 0, NULL);
    printf("Removing empty lines at end...\n"); fflush(stdout);
    while (blueprint_lines->size > 0 && blueprint_lines->tail->data == NULL) remove_node_at_index(blueprint_lines, blueprint_lines->size-1, NULL);

    if (blueprint_lines->size == 0) {
        free(OUTPUT_NAME);
        free(INPUT_NAME);
        delete_linked_list(blueprint_lines, free);
        fprintf(stderr, "No valid lines read in input file!\n");
        return 1;
    } else printf("Read %d valid lines from input file.\n", blueprint_lines->size);
    fflush(stdout);


    if (OUTPUT_NAME == NULL) { // if not defined by user, use default
        OUTPUT_NAME = strdup(DEFAULT_OUTPUT_NAME);
    }
    printf("Preparing to save with project name '%s'.\n", OUTPUT_NAME);

    pointer_holder = (char*)malloc(strlen(OUTPUT_NAME) + 7); // TODO: Chwck for NULL
    strcpy(pointer_holder, OUTPUT_NAME);
    strcpy(pointer_holder+strlen(OUTPUT_NAME), ".blprt"); // assume 0x00 is placed at end

    printf("Creating file '%s'...\n", pointer_holder);

    input_file = fopen(pointer_holder, "wt");
    if (!input_file) {
        fprintf(stderr, "Failed to create file '%s'!\n", pointer_holder);
        free(OUTPUT_NAME);
        free(INPUT_NAME);
        free(pointer_holder);
        delete_linked_list(blueprint_lines, free);
        return 1;
    }
    fprintf(input_file, "%u\n", max_len);

    NODE* current = blueprint_lines->head;
    while (current != NULL) {
        // TODO: Maybe later paddout the line as the python script did
        if (current->data == NULL)  fprintf(input_file, "\n");
        else fprintf(input_file, "%s\n", ((char*)current->data));
        current = current->next;
    }
    free(pointer_holder);
    fclose(input_file);


    pointer_holder = (char*)malloc(strlen(OUTPUT_NAME) + 5); // TODO: Chwck for NULL
    strcpy(pointer_holder, OUTPUT_NAME);
    strcpy(pointer_holder+strlen(OUTPUT_NAME), ".clr"); // assume 0x00 is placed at end
    printf("Creating file '%s'...\n", pointer_holder);
    input_file = fopen(pointer_holder, "wb");
    if (!input_file) {
        fprintf(stderr, "Failed to create file '%s'!\n", pointer_holder);
        free(OUTPUT_NAME);
        free(INPUT_NAME);
        free(pointer_holder);
        delete_linked_list(blueprint_lines, free);
        return 1;
    }

    // Default color 0x07 (normal terminal grey), one byte per character
    uint32_t total_bytes = (uint32_t) max_len * (uint32_t) blueprint_lines->size;
    uint8_t color_byte = 0x07;
    for (; total_bytes > 0; total_bytes--) fwrite(&color_byte, 1, 1, input_file); // TODO: add write check successful
    fclose(input_file);
    free(pointer_holder);

    printf("Finished. Cleaning up...\n");
    free(OUTPUT_NAME);
    free(INPUT_NAME);
    delete_linked_list(blueprint_lines, free);
    return 0;
}
