#include <stdio.h>	  // For printf
#include <stdlib.h>	  // malloc()
#include <time.h>	  // To get current time and date
#include <ctype.h>	  // tolower()
#include <stdint.h>	  // For uint8_t
#include <termios.h>  // Disabl_ctrl_d (IN) functions.c
#include <signal.h>	  // To remap CTRL+C
#include <string.h>	  // strcmp, strlens
#include <unistd.h>	  // sleep, STDIN_FILENO
#include <dirent.h>	  // for directory listing
#include <inttypes.h> // for string formating (like PRIu64)

#pragma GCC poison int unsigned short strlen size_t pause

#include "functions.c"
#include "linked_lists.h"

// Define some macros for use "in the source code"
#define AGGRESSIVE // If defined, CTRL+D is disabled in the current terminal, and a custom handler for CTRL+C is activated.

#include <stdint.h>

typedef struct _project_name{
	uint16_t x;
	uint16_t y;
	char name[25];
	uint8_t length;
} PROJECT_NAME;
typedef struct _colour{
	uint16_t x_h;
	uint16_t y_h;
	uint16_t x_d;
	uint16_t y_d;
	char hex[3];
	char dec[4];
	uint8_t len_h;
	uint8_t len_d;
} COLOUR;
typedef struct _canvas_pointer{
	uint16_t x;
	uint16_t y;
	uint16_t base_x;
	uint16_t base_y;
} CANVAS_POINTER;
typedef struct _status_bar {
    uint16_t x;
	uint16_t y;
} STATUS_BAR;
typedef struct _project {
    uint16_t width;
    uint16_t height;
    LinkedList* data;  // pointer to 2D array of chars
    char* colours;     // since the colours are 1 byte each, I'll use a char array
} PROJECT;

// Definition of some constants
// TODO: update logging scheme
const char* MAIN_LOG = "main.log";
const uint8_t CURRENT_ACTION_UID = 0000;
static char PALLET_MODE = 00;
const uint16_t EDITOR_WIDTH = 180;
const uint16_t EDITOR_HEIGHT = 58;
const uint16_t MENU_SEPARATOR = 78;

const uint8_t DISPLAY_HEX_COLOURS = 1;
const uint8_t CURSOR_OVERWRITE = 1;

PROJECT_NAME project_name = {17, 20, "", 0};
COLOUR chosen_colour = {72, 2, 29, 3, "0F", "015", 2, 3};
CANVAS_POINTER canvas_pointer = {MENU_SEPARATOR+1, 2, MENU_SEPARATOR+1, 2};
STATUS_BAR status_bar = {11, 22};
PROJECT bp = {0, 0, NULL, NULL};

// Definition of the menu arrays, always in the format len / options
const uint8_t len_main_menu = 3;
const char main_menu[][CABECALHO_LEN] = {
	"ANSI4 (16 Colours)",
	"ANSI8 (256 Colours)",
	"Exit"
};

// Functions and methods
uint8_t log_(const char* log_file_path, const char* msg){
	/*
	Function used to write a system action that took place to a log file in a specific format.
	The format that is logged is:
		[*] year-month-day hour:minute LOG 00000 message

	Arguments:
		const char* log_file_path	-> Pointer to null terminated string to the file where the log message is writen.
		const char* msg				-> Pointer to null terminated string containing the message to be logged.

	Return (uint8_t):
		0 -> Everything went fine.
		1 -> When trying opening the log file, NULL was returned by fopen
	*/
    FILE* file = fopen(log_file_path, "at");
    if (file == NULL) return 1;

    // FIXME: Why date and (date)?? same on times...
    DATE date = get_current_date(date);
    TIME times = get_current_time(times);
    fprintf(file, "[*] %04d-%02d-%02d %02d:%02d LOG 00000 %s\n", date.year, date.month, date.day, times.hour, times.minutes, msg);
    fclose(file);
    return 0;
}
uint8_t logc_(const char* log_file_path, const char* msg){
	/*
	Function used to write a custom action from the current user that took place to a log file in a specific format.
	The format that is logged is:
		[*] year-month-day hour:minute LOG currently_logdin_uid message

	Global variables or macros expected:
		ACCOUNT CURRENT_LOGIN		-> Currently loged-in user. Only the UID (in .uid) field is used.

	Arguments:
		const char* log_file_path	-> Pointer to null terminated string to the file where the log message is writen.
		const char* msg				-> Pointer to null terminated string containing the message to be logged.

	Return (uint8_t):
		0 -> Everything went fine.
		1 -> When trying opening the log file, NULL was returned by fopen
	*/
    FILE* file = fopen(log_file_path, "at");
    if (file == NULL) return 1;

    DATE date = get_current_date(date);
    TIME times = get_current_time(times);
    fprintf(file, "[*] %04d-%02d-%02d %02d:%02d LOG %5u %s\n", date.year, date.month, date.day, times.hour, times.minutes, CURRENT_ACTION_UID, msg);
    fclose(file);
    return 0;
}

void show_cursor(){ printf("\033[?25h"); }
void hide_cursor(){ if (CURSOR_OVERWRITE) printf("\033[?25l"); }
void remove_cursor(){
    // TODO: implement more checks, like char validity check.
    // keep in mind: n_row and n_culm zero indexed!
    uint16_t n_row = canvas_pointer.y - canvas_pointer.base_y;
    uint16_t n_culm = canvas_pointer.x - canvas_pointer.base_x;

    if (bp.data != NULL && n_row < bp.height && n_culm < bp.width) {
        NODE* row = bp.data->head;
        for (uint16_t i = 0; i < n_row; i++) row = row->next;

        char ch = ((char*)row->data)[n_culm];
        uint8_t colour = bp.colours[n_row * bp.width + n_culm];
        printf("\033[38;5;%um%c\033[m", colour, ch);

    } else printf(" ");
    printf("\033[1D");
}
void print_cursor(){
    uint16_t n_row = canvas_pointer.y - canvas_pointer.base_y;
    uint16_t n_culm = canvas_pointer.x - canvas_pointer.base_x;
    char prnt = 0x20;
    if (bp.data != NULL && n_row < bp.height && n_culm < bp.width) {
        NODE* row = bp.data->head;
        for (uint16_t i = 0; i < n_row; i++) row = row->next;
        prnt = ((char*)row->data)[n_culm];
    }
    printf("\033[30;47m%c\033[m\033[1D", prnt);// fg: black, bg: white
}

PROJECT* load_blueprint(const char* file_path, PROJECT* bp) {
    FILE* fp = fopen(file_path, "rb");
    if (!fp) {
        perror("Failed to open blueprint file");
        return NULL;
    }

    // Read width from first line
    int16_t c;
    bp->width = 0;
    while (1){
        c = fgetc(fp);
        if (c == EOF){
            perror("Blueprint file ended too soon (line width discovery failed)\n");
            goto close_return;
        }
        if (c == 0x0A) break;
        if (c >= 0x30 && c <= 0x39) bp->width = bp->width * 10 + (c - 0x30);
        else {
            perror("Invalid width character in blueprint\n");
            goto close_return;
        }
    }
    if (bp->width == 0) {
        perror("Blueprint width cannot be zero!\n");
        goto close_return;
    }

    // Count lines to get height
    bp->height = 0;
    uint64_t pos = ftell(fp);
    while ((c = fgetc(fp)) != EOF) {
        if (c == 0x0A) bp->height++;
    }
    fseek(fp, pos, SEEK_SET);

    // Prepare the linked list to hold the lines
    bp->data = create_linked_list();
    if (!bp->data) {
        perror("Failed to allocate memory for the linked list!\n");
        goto close_return;
    }

    NODE* created_node;
    uint8_t padding;
    // Allocate each line and read bytes [might be less than the bytes, fill with " ". if more cut].
    for (uint16_t line = 0; line < bp->height; line++) {
        created_node = append_data_to_list(bp->data, malloc(bp->width + 1));

        if (!created_node->data) {
            perror("Failed to allocate memory for line. Reverting...\n");
            delete_linked_list(bp->data, free);
            bp->data = NULL;
            goto close_return;
        }

        padding = 0;
        for (uint16_t char_num = 0; char_num < bp->width; char_num++) {
            if (!padding) c = fgetc(fp);
            if (c == EOF || c == 0x0A) {
                ((char*)created_node->data)[char_num] = ' '; // pad with space if line is short
                padding = 1;
            } else {
                ((char*)created_node->data)[char_num] = c;
            }
        }
        // go until newline if line was longer than expected, also jump if already reached the end and padded needed
        if (!padding) while (c != EOF && c != 0x0A) c = fgetc(fp);

        ((char*)created_node->data)[bp->width] = 0x00; // null terminate the line
    }

    fclose(fp);
    return bp;

    close_return:
    fclose(fp);
    return NULL;
}
void save_blueprint(const char* file_path, PROJECT* bp) {
    // TODO: also check for PROJECT being null and other checks
    FILE* fp = fopen(file_path, "wb");
    if (!fp) {
        perror("Failed to open/create blueprint file");
        return;
    }
    // write width on first line
    fprintf(fp, "%u\n", bp->width);
    // write lines
    NODE* current_node = bp->data->head;
    while (current_node!=NULL){
        fprintf(fp, "%s\n", (char*)current_node->data);
        current_node = current_node->next;
    }
    fclose(fp);
    return;
}
PROJECT* load_colours(const char* file_path, PROJECT* bp) {
    FILE* fp = fopen(file_path, "rb");
    if (!fp) {
        perror("Failed to open colours file!\n");
        return NULL;
    }

    int16_t c;
    // width (16b) * height (16b) = full canvas size (32b);
    uint32_t max = (uint32_t)bp->width * (uint32_t)bp->height;
    // TODO: add check for size not being zero, no point in reading nothing
    bp->colours = (char*) malloc(max + 1); // although not necessary, +1 is to prevent runaway
    if (bp->colours == NULL) {
        perror("Failed to allocate memory for the colour array!\n");
        fflush(stderr);
        goto close_return2;
    }
    bp->colours[max] = 0x00;

    for (uint32_t i=0; i < max; i++){
        c = fgetc(fp);
        if (c == EOF){
            if (feof(fp)) perror("Colours file ended too soon (colour loading incomplete)\n");
            else if (ferror(fp)) perror("Read error on colours file!\n");
            free(bp->colours);
            bp->colours = NULL;
            goto close_return2;
        }
        bp->colours[i] = c;
        // TODO: add byte checking for the allow colour spectre [full 0xFF for ANSI8]
        // perror("Invalid character in blueprint\n");
    }

    fclose(fp);
    return bp;

    close_return2:
    fclose(fp);
    return NULL;
}
void save_colours(const char* file_path, PROJECT* bp) {
    // TODO: also check for PROJECT being null and other checks
    FILE* fp = fopen(file_path, "wb");
    if (!fp) {
        perror("Failed to open/create colours file!\n");
        return;
    }
    // width (16b) * height (16b) = full canvas size (32b);
    uint32_t num_colours = (uint32_t)bp->width * (uint32_t)bp->height;
    // bp->colours
    // cannot use fprintf as bp->colours contains 0x00 as valid colour codes.
    fwrite(bp->colours, 1, num_colours, fp);
    fclose(fp);
    return;
}
void free_blueprint(PROJECT *bp) {
    if (!bp || !bp->data) return;
    delete_linked_list(bp->data, free);
    bp->data = NULL;
    bp->height = 0;
    bp->width = 0;
}

void handle_sigint_temp(int32_t sig){
	// simple intermediary for the normal handler, just for login purposes.
	char buffer[35+12];
	snprintf(buffer, sizeof(buffer),"Received control interrupt signal %d.", sig);
	log_(MAIN_LOG, buffer);
	show_cursor();
	handle_sigint(sig);
}

// Interface
void status_msg(char* str, uint8_t color){
    printf("\033[%d;%dH\033[m", status_bar.y, status_bar.x);
    for (uint8_t i = status_bar.x; i < MENU_SEPARATOR; i++) printf(" ");
    printf("\033[%d;%dH", status_bar.y, status_bar.x);
    switch(color){
        case 1: printf("\033[32m"); break; // Green
        case 2: printf("\033[1;33m"); break; // Yellow
        case 3: printf("\033[1;31m"); break; // Red
        case 4: printf("\033[34m"); break; // Debug blue
        default: printf("\033[37m"); // Grey
    }
    printf(str);
    printf("\033[m\033[%d;%dH", canvas_pointer.y, canvas_pointer.x);
    return;
}
void print_base_interface(){ // ┌ ─ ┐ │ ┤ ├ └ ┘ ┬ ┴ ┼
    clear_screen();
    // print upper line
    printf("\033[1;37m┌");
    uint16_t i;
    for (i = 2; i<MENU_SEPARATOR; i++) printf("─");
    printf("┬");
    for (i = MENU_SEPARATOR+1; i<EDITOR_WIDTH; i++) printf("─");
    printf("┐\n");

    // print box layers
    for (i = 2; i < EDITOR_HEIGHT; i++) printf("│\033[%dC│\033[%dC│\n", MENU_SEPARATOR-2, EDITOR_WIDTH-MENU_SEPARATOR-1);

    // print lower line
    printf("└");
    for (i = 2; i<MENU_SEPARATOR; i++) printf("─");
    printf("┴");
    for (i = MENU_SEPARATOR+1; i<EDITOR_WIDTH; i++) printf("─");
    printf("┘");

    // move three lines up and draw the menu line
    printf("\033[2A\033[%dD├", EDITOR_WIDTH);
    for (i=2; i<MENU_SEPARATOR; i++) printf("─");
    printf("┤\n");
    // print the menu
    printf("│\033[32m[\033[36mSPACE\033[32m]\033[1;37m: Paint Character  \033[32m[\033[36mS\033[32m]\033[1;37m: Save  \033[32m[\033[36mR\033[32m]\033[1;37m: Reload  \033[32m[\033[36mQ\033[32m]\033[1;37m: Quit");

    // print the pallet
    printf("\033[2;2H");
    if (PALLET_MODE == 0x08){
        print_ansi8_palette(1, DISPLAY_HEX_COLOURS);
        printf("\033[19;1H├");
        for (i = 0; i < 51; i++) printf("─");
        printf("┴\033[1D\033[18A┬");
        // And finish the bottom line until the MENU_SEPARATOR
        printf("\033[1D\033[18B┴");
        for (i = 54; i < MENU_SEPARATOR; i++) printf("─");
        printf("┤");
    } // TODO: To add the 0x04 mode

    // Current setting display
    printf("\033[2;55H\033[1;31mC\033[36murrent Color: \033[1;37m0x0F \033[1;37m█\033[7D\033[1B\033[90m(015)");

    // print the project name box and stuff
    printf("\033[21;1H\033[1;37m├");
    printf("\033[1A\033[1;31m P\033[36mroject Name:\033[m _________________________ \033[90m (a-z,A-Z,0-9,-,.)  \033[33m[MAX 24 chars]\033[1;37m\n\033[1C");
    for (i=2; i < MENU_SEPARATOR; i++) printf("─");
    printf("┤");

    printf("\033[23;1H\033[1;37m├");
    printf("\033[1A \033[36mStatus:\033[32m Interface loading successful!\033[1;37m\n\033[1C");
    for (i=2; i < MENU_SEPARATOR; i++) printf("─");
    printf("┤");

    // Set cursor to the top corner of the editor!
    printf("\033[%d;%dH", canvas_pointer.y, canvas_pointer.x);
    return;
}

void print_colour_char(char char_to_print, uint8_t colour){
    uint16_t tp = 0x0000;
    ((char*) &tp)[0] = char_to_print;
    printf("\033[38;5;%um%s\033[m", colour, (char*) &tp);
}

// MENUS/Program Loops
void editor_main_loop(){
	uint8_t _escolha_menu;
    print_base_interface();
    char command;
    hide_cursor();
    print_cursor();
	while (1){
	    // TODO: Finish writing the base loops for controlling the characters
	    // TODO: Add a pick colour (like put coursor on top of a char, press key, and it gets the colour
	    fflush(stdout);
	    command = getch();
	    if (command == 'q' || command == 'Q') {
	        show_cursor();
	        clear_screen();
        	printf("\033[1;38m");
            _escolha_menu = confirmation_with_cabecalho("EXITING EDIT MODE ", "Do you want to save you project?", CABECALHO_LEN);
            if (_escolha_menu==0) printf("Saving...\n");
            pause_();
            break;
	    }
	    else if (command == 'p' || command == 'P'){
	        remove_cursor();
	        show_cursor();
	        printf("\033[%d;%dH\033[m", project_name.y, project_name.x + project_name.length);
	        while (1){
                fflush(stdout);
                command = getch();
	            if ((command >= 0x61 && command <= 0x7A) || (command >= 0x41 && command <= 0x5A) || (command >= 0x30 && command <= 0x39) || command == 0x2D ||  command == 0x2E) {
                    // Valid char
                    if (project_name.length < 24){
                        putchar(command);
                        project_name.name[project_name.length] = command;
                        project_name.length++;
                         project_name.name[project_name.length] = 0x00;
                    }
                } else if (command == 0x0A) {
                    // Set cursor to the top corner of the editor
                    printf("\033[%d;%dH", canvas_pointer.y, canvas_pointer.x);
                    break;
                } else if ((command == 0x08 || command == 0x7F)) { // Backspace and Delete
                    if (project_name.length > 0){
                        project_name.length--;
                        project_name.name[project_name.length] = 0x00;
                        printf("\033[1D_\033[1D"); // remove the last char and move back
                    }
                }
	        }
	        hide_cursor();
	        print_cursor();
	    }
	    else if (command == 's' || command == 'S'){
	        if (bp.data != NULL && bp.colours != NULL){
	            status_msg("Saving project...", 2);
                char file_name[project_name.length + 7]; // name+.blprt+0x00
                strcpy(file_name, project_name.name);
                strcpy(&file_name[project_name.length], ".blprt");
                file_name[project_name.length + 6] = 0x00;
                save_blueprint(file_name, &bp);

                strcpy(&file_name[project_name.length], ".clr\0");
                save_colours(file_name, &bp);

                status_msg("Project saved successfully!", 1);
	        } else {
	            status_msg("Cannot save project! Project not loaded or corrupted!", 3);
	        }
	    }
	    else if (command == 'r' || command == 'R'){
            status_msg("Loading project", 2);
            fflush(stdout);

            char file_name[project_name.length + 7]; // name+.blprt+0x00
            strcpy(file_name, project_name.name);
            strcpy(&file_name[project_name.length], ".blprt");
            file_name[project_name.length + 6] = 0x00;

            if (bp.data) free_blueprint(&bp);
            if (load_blueprint(file_name, &bp) == NULL) status_msg("Failed to load project (blueprint)!", 3);
            else if (bp.data) {
                strcpy(&file_name[project_name.length], ".clr\0");
                if (load_colours(file_name, &bp) == NULL) status_msg("Failed to load project (colours)!", 3);
                else {
                    // send pointer to canvas top left
                    printf("\033[%d;%dH", canvas_pointer.base_y, canvas_pointer.base_x);
                    fflush(stdout);
                    // display the blueprint
                    NODE* currnt= bp.data->head;
                    uint16_t char_i;
                    uint16_t column_offset=0;
                    while (currnt!=NULL){
                        // printf("%s", ((char*)currnt->data));
                        for (char_i=0;char_i < bp.width;char_i++){
                            print_colour_char(((char*)currnt->data)[char_i], bp.colours[column_offset+char_i]);
                        }
                        printf("\033[1B\033[%uD", bp.width);
                        fflush(stdout);
                        currnt = currnt->next;
                        column_offset+=bp.width;
                    }

                    status_msg("Loaded blueprint successfully", 1);

                    // reset pointer position
                    printf("\033[%d;%dH", canvas_pointer.y, canvas_pointer.x);
                }
            }
            print_cursor();
            fflush(stdout);
	    }
	    else if (command == 0x20) {
            // keep in mind: n_row and n_culm zero indexed!
            uint16_t n_row = canvas_pointer.y - canvas_pointer.base_y;
            uint16_t n_culm = canvas_pointer.x - canvas_pointer.base_x;
            if (bp.data != NULL && n_row < bp.height && n_culm < bp.width) {
                bp.colours[n_row * bp.width + n_culm] = hexToDec(chosen_colour.hex);
                status_msg("Painted successfully!", 1);
            }
            else status_msg("No char to paint (Outside project boundaries)!", 2);
	    }
        else if (command == 'c' || command == 'C'){
            remove_cursor();
            show_cursor();
            // TODO: Apply color changing (possibly 'c' changes between hex and decimal
            //Or a two cell selector, like L | C, and work from there
            status_msg("Select new color.", 2);
            printf("\033[%d;%dH", chosen_colour.y_h, chosen_colour.x_h + chosen_colour.len_h);
            while (1){
                printf("\033[1;37m");
                fflush(stdout);
                command = getch();
	            if (command == 0x0A) {
	                if (chosen_colour.len_h == 2){
                        sprintf(chosen_colour.dec, "%03u", hexToDec(chosen_colour.hex));
                        printf(" \033[38;5;%sm█", chosen_colour.dec);
                        printf("\033[6D\033[1B\033[90m%s\033[m", chosen_colour.dec); // update on screen decimal
                        status_msg("Color selected successfully!", 1);
                        printf("\033[%d;%dH", canvas_pointer.y, canvas_pointer.x); // Set cursor to the top corner of the editor
                        fflush(stdout);
                        break;
                    } else {
                        status_msg("You must have both characters before selecting!", 2);
                        printf("\033[%d;%dH", chosen_colour.y_h, chosen_colour.x_h + chosen_colour.len_h);
                    }
                }
                else if ((command == 0x08 || command == 0x7F)) { // Backspace and Delete
                    if (chosen_colour.len_h > 0){
                        chosen_colour.len_h--;
                        chosen_colour.hex[chosen_colour.len_h] = 0x00;
                        printf("\033[1D_\033[1D"); // remove the last char and move back
                    }
                }
                else if ((command >= 0x41 && command <= 0x46) || (command >= 0x30 && command <= 0x39) || ((command=command - 0x20) >= 0x41 && command <= 0x46) ) {
                    // Valid char
                    if (chosen_colour.len_h < 2){
                        putchar(command);
                        chosen_colour.hex[chosen_colour.len_h] = command;
                        chosen_colour.len_h++;
                        chosen_colour.hex[chosen_colour.len_h] = 0x00;
                    }
                }
	        }
	        hide_cursor();
	        print_cursor();
        }
        else if ((command == 27) && ((command = getch()) == 91)) {
			command = getch();
			if (command == 65 && canvas_pointer.y > 2) {  // up arrow pressed
			    remove_cursor();
				printf("\033[1A");
				canvas_pointer.y--;
				print_cursor();
			}
			if (command == 68 && canvas_pointer.x > MENU_SEPARATOR + 1) {  // left arrow pressed
			    remove_cursor();
				printf("\033[1D");
				canvas_pointer.x--;
				print_cursor();
			}
			else if (command == 66 && canvas_pointer.y < EDITOR_HEIGHT - 1) { // down arrow
				remove_cursor();
				printf("\033[1B");
                canvas_pointer.y++;
                print_cursor();
			}
			else if (command == 67 && canvas_pointer.x < EDITOR_WIDTH - 1) { // right arrow pressed
				remove_cursor();
				printf("\033[1C");
				canvas_pointer.x++;
				print_cursor();
			}
		}
	}
	return;
}

int32_t main(void){
	#ifdef AGGRESSIVE
		signal(SIGINT, handle_sigint_temp);
		disable_ctrl_d();
	#endif
    fflush(stdin); // apparently useless on linux, but somehow affects things on my pc :man_shrugging:
	uint8_t escolha_menu;
	log_(MAIN_LOG, "Program initiated.");
	printf("\033[1;37m");
	while (1){
		clear_screen();
		// has a trailing space for centering, because menu does not work correctly with non perfect divisions
		escolha_menu = menu("PLEASE CHOOSE ANSI COLOUR DEPTH ", CABECALHO_LEN, main_menu, len_main_menu, 1);
		if(escolha_menu==0) break;
		switch(escolha_menu){
			case 1: PALLET_MODE = 0x04; editor_main_loop(); break;
			case 2: PALLET_MODE = 0x08; editor_main_loop(); break;
			default: printf("\nFunção ainda não implementada!!\n");
		}
	}
	printf("Saindo...\n");
	#ifdef AGGRESSIVE
		enable_ctrl_d();
	#endif
	log_(MAIN_LOG, "Program finished sucsesfully.");
	return 0;
}
