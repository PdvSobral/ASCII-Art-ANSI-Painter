#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <stdint.h>
#include <termios.h>    // -> Disabl_ctrl_d
#include <signal.h>		// -> To remap CTRL+C
#include <string.h>		// strcmp, strlens
#include <unistd.h>		// sleep, STDIN_FILENO

#define CABECALHO_LEN 70
#define CLEAR 1
#define BUFFER_LEN 256

// Defenition of the structs
typedef struct _date{
	uint8_t day;
	uint8_t month;
	uint16_t year;
} DATE;
typedef struct _time{
	uint8_t hour;
	uint8_t minutes;
} TIME;

const char* ANSI4[] = {
    "\033[30m",  // 0: Black
    "\033[31m",  // 1: Red
    "\033[32m",  // 2: Green
    "\033[33m",  // 3: Yellow
    "\033[34m",  // 4: Blue
    "\033[35m",  // 5: Magenta
    "\033[36m",  // 6: Cyan
    "\033[37m",  // 7: White
    "\033[90m",  // 8: Bright Black (Gray)
    "\033[91m",  // 9: Bright Red
    "\033[92m",  // 10: Bright Green
    "\033[93m",  // 11: Bright Yellow
    "\033[94m",  // 12: Bright Blue
    "\033[95m",  // 13: Bright Magenta
    "\033[96m",  // 14: Bright Cyan
    "\033[97m"   // 15: Bright White
};

struct termios original_tio;
void disable_ctrl_d(){
    struct termios new_tio;
    tcgetattr(STDIN_FILENO, &original_tio);
    new_tio = original_tio;
    new_tio.c_cc[VEOF] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    return;
}
void enable_ctrl_d(){
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
    return;
}
void handle_sigint(int32_t sig){
    printf("\n\n\nCaught signal %d. Restoring terminal settings and exiting...\n", sig);
    enable_ctrl_d();
    exit(1);
}

void clear_screen(){
	/*
		Clears the screen and sets the cursor at the upper right corner
	*/
	printf("\033[2J\033[H");
}

/*
Strlen with support for UTF-8;
*/
uint32_t strlen2(const char *str) {
        uint32_t count = 0;
        uint8_t *s = (uint8_t *)str;
        while (*s != 0x00) {
            if ((*s & 0x80) == 0) {
                // 1-byte character (ASCII)
                count++;
                s++;
            } else if ((*s & 0xE0) == 0xC0) {
                // 2-byte character
                count++;
                s += 2;
            } else if ((*s & 0xF0) == 0xE0) {
                // 3-byte character
                count++;
                s += 3;
            } else if ((*s & 0xF8) == 0xF0) {
                // 4-byte character
                count++;
                s += 4;
            } else {
                // Invalid UTF-8 byte sequence
                break;
            }
        }
        return count;
    }

void reverse_str(char *str, uint16_t length){
	/*
	Tal como o nome indica, inverte uma string.
	Recebe como argumentos:
		char *str   -> Pointer para a str a inverter.
		int  length -> Tamanho da str a inverter
	*/
    uint16_t start = 0;
    uint16_t end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        end--;
        start++;
    }
}
char* my_itoa(int16_t num, char* str, int16_t base){
	/*
	Our implementation of itoa() as it was not being found by the compiler in stdlib.h
	Arguments:
		int16_t num  -> Integer to convert to string
		char*   str  -> Pointer to buffer where the string is going to be stored
		int16_t base -> Base of the integer to convert, E.x. 10
	Returns:
		char* <- Poiter to where string was stored.
	*/
    int16_t i = 0;
    int16_t isNegative = 0;
    /* Handle 0 explicitly, otherwise empty string is saved for 0 */
    if(num == 0){
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
    // In standard itoa(), negative numbers are handled only with base 10. Numbers are ptherwise considered unsigned.
    if(num < 0 && base == 10){
        isNegative = 1;
        num = -num;
    }
    while (num != 0){
        uint16_t rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
    if(isNegative == 1) str[i++] = '-';
    str[i] = '\0';
    reverse_str(str, i);
    return str;
}

/*
Using fflush(stdin), a loop until \n, scanf and fgets was giving inconsistency performance,
so these functions were created:
    flush_stdin();
    getch();
    read_n_chars();
    get_password();
*/
void flush_stdin(void){
    char c;
    fflush(stdin);
    while((c = getchar()) != '\n' && c != EOF){};
	return;
}
char getch(){
    char ch;
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt); 			// Get the current terminal settings
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);			// Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);	// Set new terminal settings
    read(STDIN_FILENO, &ch, 1);					// Read a single character from stdin
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);	// Restore the old terminal settings
    return ch;
}
void read_n_chars(uint8_t _len_to_read, char* pointer_to_write_to){
	/*
	Implementação de uma função parecida com gets, pois esta estava a dar muitos erros de inconsistência
	Argumentos:
		uint8_t _len_to_read        -> Número de caracteres que serão interpretados pelo programa
		char*   pointer_to_write_to -> Onde os characteres lidos serão wuardados.
	*/
	char command[12] = " %";
	my_itoa(_len_to_read, command+strlen2(command), 10);
	strcpy(command+strlen2(command), "[^\n]");
	scanf(command, pointer_to_write_to);
	flush_stdin();
}
void get_password(char *password_str, uint16_t max_length) {
	fflush(stdout);
    char ch;
    uint16_t current_length = 0;
    while(1) {
        ch = getch();
        if (ch == 0x0A) break;
        else if ((ch == 0x08 || ch == 0x7F)) { // Backspace and Delete
        	if (current_length > 0){
				current_length--;
				printf("\033[1D \033[1D"); // remove the last char and move back
			}
        }
        else if (current_length < max_length) {
        	if (ch >= 0x20 || (int32_t) ch < 0x00){
				putchar('*');
				password_str[current_length++] = ch;
			}
        }
        fflush(stdout);
    }
    password_str[current_length] = 0x00; // Null-terminate the string
    printf("\n"); // Move to the next line after password input
    return;
}

DATE get_current_date(DATE struct_data){
	/*
	Gets current system date and writes it to a passed struct, as well as returning its pointer
	Arguments:
		DATE struct_data -> pointer to first element of struct
	Return:
		DATE -> pointer to first element of struct
	*/
	time_t _time = time(NULL);
	struct tm time_maneger = *localtime(&_time);
	struct_data.year = time_maneger.tm_year + 1900;
	struct_data.month = time_maneger.tm_mon + 1;
	struct_data.day = time_maneger.tm_mday;
	return struct_data;
}
TIME get_current_time(TIME times){
    time_t now = time(NULL);			// Get Current UNIX Timestamp
    struct tm* local = localtime(&now);	// Convert to "normal" time
    times.hour = local->tm_hour;
    times.minutes = local->tm_min;
    return times;
}

int64_t str_to_int64_t(char *pointer_to_str){
	/*
	Função que converte um inteiro representado como str para um inteiro
	Argumentos:
		char *pointer_to_str -> Início da str para converter para inteiro
	Retorno:
		int64_t -> inteiro contido na str
	*/
	int64_t _buffer = -1;
	char char_to_evaluate;
	while(1){
		char_to_evaluate = *(pointer_to_str);
		if(char_to_evaluate == 0x00) break;
		if(char_to_evaluate < 0x30) break;
		if(char_to_evaluate > 0x39) break;
		if(_buffer==-1) _buffer=0;
		_buffer*=10;
		_buffer+=(char_to_evaluate - 0x30);
		pointer_to_str++;
	}
	return _buffer;
}
int64_t str_to_int64_t_flag(char *pointer_to_str, uint8_t *is_numeric){
	/*
	Função que converte um inteiro representado como str para um inteiro
	Argumentos:
		char *pointer_to_str -> Início da str para converter para inteiro
		*uint8_t is_numeric  -> A variável para onde aponta será colocada com 1 se sim, 0 se não
	Retorno:
		int64_t -> inteiro contido na str
	*/
	int64_t _buffer = -1;
	char char_to_evaluate;
	while(1){
		char_to_evaluate = *(pointer_to_str);
		if(char_to_evaluate == 0x00) {*is_numeric=1; break;};
		if(char_to_evaluate < 0x30) {*is_numeric=0; break;};
		if(char_to_evaluate > 0x39) {*is_numeric=0; break;};
		if(_buffer==-1) _buffer=0;
		_buffer*=10;
		_buffer+=(char_to_evaluate - 0x30);
		pointer_to_str++;
	}
	return _buffer;
}
int8_t strIsNum(char *str){
	/*
	Esta função verifica se uma string apenas contém dígitos.
	Argumentos:
		const char *str -> Apontador para o início da string terminada com '\0' a analisar
	Retorno:
		int8_t  0 -> A string passada apenas contém dígitos
		int8_t -1 -> A string passada contém pelo menos um caractere que não é dígito
	*/
    for (uint8_t i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            return -1;
        }
    }
    return 0;
}
uint8_t hexToDec(char* hex) {
    uint8_t decimalValue = 0;
    // Since only uint8_t, 2 chars max.
    for (uint8_t i = 0; i < 2 && hex[i] != '\0'; i++) {
        char c = hex[i];
        if (c >= 0x30 && c <= 0x39) c -= 0x30; // 0-9
        else if (c >= 0x41 && c <= 0x46) c -= (0x41 - 0x0A); // A-F (-A + 10)
        else if (c >= 0x61 && c <= 0x66) c -= (0x61 - 0x0A); // a-f (-a + 10)
        else continue;
        decimalValue = (decimalValue * 0x10) + c;
    }
    return decimalValue; // Return the decimal value as uint8_t
}

void pause_() {
    printf("Press any key to continue . . . ");
    fflush(stdout);
	getch();
}

void cabecalho(const char msg[], uint8_t len_cabecalho){
	/*
	Procedimento que imprime um cabeçalho
	Argumentos:
		char msg[]            -> String a servir de título.
		uint8_t len_cabecalho -> Tamanho do cabeçalho
	Retorno:
		Nenhum
	*/
	uint8_t _len = strlen2(msg);
	printf("┌");
	for(uint8_t _index = 0; _index<len_cabecalho-2; _index++){
		printf("─");
	} printf("┐\n│");
	_len = (len_cabecalho - _len)/2;
	for(uint8_t _index = 0; _index<_len-1; _index++){
		printf(" ");
	} printf("%s", msg);
	for(uint8_t _index = 0; _index<_len-1; _index++){
		printf(" ");
	} printf("│\n");
	printf("└");
	for(uint8_t _index = 0; _index<len_cabecalho-2; _index++){
		printf("─");
	} printf("┘\n");
	return;
}
void reset_line(uint8_t len_cabecalho){
    // Note: [s and [u are a bit inconsistent in some terminals, probably best to replace
	printf("\033[s");
	printf("\033[Am\033[0G");  // up one line and beggining
	printf("├\033[%uC┤\n", len_cabecalho-2);  // replace the line and return to where it was
	printf("\033[u");
	return;
}
int64_t menu(const char tittle[], uint8_t len_cabecalho, const char menu_options[][CABECALHO_LEN], uint8_t menu_size, uint8_t last_zero){
	/*
	Função para apresentar ao utilizador um menu
	Argumentos:
		char tittle[]							 -> Tittle for the menu
		const char menu_options[][CABECALHO_LEN] -> Array de strings a usar como opções do menu
		uint8_t menu_size                        -> Número de opções a ler do array passado
		uint8_t last_zero                        -> Torna a uĺtima opção sempre zero, independentemente das outras
	Retorno:
		int64_t -> Escolha do utilizador
	*/
	uint8_t wrong=0;
	int64_t _option;
	uint8_t numeric_flag = 0;
	while(1){
		cabecalho(tittle, len_cabecalho);
		reset_line(len_cabecalho);
		char buffer[5];
		for(uint8_t _index=0; _index<menu_size; _index++) {
			if(_index+last_zero == menu_size){
				printf("│  0 - %s", menu_options[_index]);
			} else {
				printf("│ %2d - %s", _index + 1,  menu_options[_index]);
			}
			printf("\033[%uC│\n", CABECALHO_LEN - 8 - (uint8_t) strlen2(menu_options[_index]));
		};
		printf("├");
		for(uint8_t _index = 0; _index<len_cabecalho-2; _index++){
			printf("─");
		} printf("┤\n");
		printf("\n└");
		for(uint8_t _index = 0; _index<len_cabecalho-2; _index++){
				printf("─");
			} printf("┘");
		printf("\033[1A\033[%dD", CABECALHO_LEN);
		printf("│ Introduza a sua opção: ");
		for (uint8_t i = 0; i < CABECALHO_LEN - 26; ++i) printf(" ");
		printf("│");

		if(wrong==1){
			printf("\n├\033[%uC┤", CABECALHO_LEN-2);
			printf("\n│ \033[31mInvalid Option!! Please enter a valid option.\033[1;37m");
			printf("\033[%uC│\n└", CABECALHO_LEN - 48);
			for(uint8_t _index = 0; _index<len_cabecalho-2; _index++){
				printf("─");
			} printf("┘");
			printf("\033[3A\033[%dD\033[25C", CABECALHO_LEN);
		} else {
			printf("\033[%dD\033[25C", CABECALHO_LEN);
		}
		read_n_chars(3, buffer);
		_option = str_to_int64_t_flag(buffer, &numeric_flag);
		if((_option<=(menu_size-last_zero)) && ((1-last_zero) <= _option) && numeric_flag) break;
		wrong = 1;
		clear_screen();
	}
	printf("\n");
	return _option;
}

uint8_t confirmation_with_cabecalho(char* cabecalho_msg, char* main_msg, uint8_t len_cabecalho){
	cabecalho(cabecalho_msg, len_cabecalho);
	reset_line(len_cabecalho);
	printf("│ %s", main_msg);
	printf("\033[%uC│\n", len_cabecalho - 3 - strlen2(main_msg));
	for (uint8_t i = 0; i < len_cabecalho; i++) printf("─");
	printf("\n");
	reset_line(len_cabecalho);
	printf("│ [*] Yes     [ ] No");
	printf("\033[%uC│\n", len_cabecalho - 21);
	printf("└");
	for(uint8_t _index = 0; _index<len_cabecalho-2; _index++){
		printf("─");
	} printf("┘");
	printf("\033[1A\033[999D\033[3C");
	fflush(stdout);
	uint8_t state=0; //0 - yes 1 - no
	char ch;
	while (1){
		// chars especiais tipo setas
		if (((ch = getch()) == 27) && ((ch = getch()) == 91)) {
			if (((ch = getch()) == 65 || ch == 68) && state != 0){  // up arrow and left arrow pressed
				printf(" \033[13D*\033[1D");
				state--;
			}
			else if ((ch == 66 || ch == 67) && state != 1){ // down arrow and right arrow pressed
				printf(" \033[11C*\033[1D");
				state++;
			}
		}
		else {
			if (ch == 0x0A){
				printf("\n\n");
				return state;
			}
		}
		fflush(stdout);
	}
}

uint8_t confirmation(char* main_msg, uint8_t len_cabecalho, uint8_t print_line){
	if (print_line==1) for(uint8_t _index = 0; _index<len_cabecalho; _index++) printf("─");
	reset_line(len_cabecalho);
	printf("│ %s", main_msg);
	printf("\033[%uC│\n", len_cabecalho - 3 - strlen2(main_msg));
	for (uint8_t i = 0; i < len_cabecalho; i++) printf("─");
	printf("\n");
	reset_line(len_cabecalho);
	printf("│ [*] Yes     [ ] No");
	printf("\033[%uC│\n", len_cabecalho - 21);
	printf("└");
	for(uint8_t _index = 0; _index<len_cabecalho-2; _index++){
		printf("─");
	} printf("┘");
	printf("\033[1A\033[999D\033[3C");
	fflush(stdout);
	uint8_t state=0; //0 - yes 1 - no
	char ch;
	while (1){
		// chars especiais tipo setas
		if (((ch = getch()) == 27) && ((ch = getch()) == 91)) {
			if (((ch = getch()) == 65 || ch == 68) && state != 0){  // up arrow and left arrow pressed
				printf(" \033[13D*\033[1D");
				state--;
			}
			else if ((ch == 66 || ch == 67) && state != 1){ // down arrow and right arrow pressed
				printf(" \033[11C*\033[1D");
				state++;
			}
		}
		else {
			if (ch == 0x0A){
				printf("\n\n");
				return state;
			}
		}
		fflush(stdout);
	}
}

void print_between(char* str, uint8_t len_cabecalho, uint8_t newline){
	if (len_cabecalho - 4 - (uint8_t) strlen2(str) < 0){
		printf("String to print_between must be at least 4 characters shorter than len_cabecalho.");
		printf("\nPlease increase len_cabecalho.\n");
		exit(5);
	}
	printf("│ %s", str);
	for(uint8_t i=0; i < len_cabecalho-4-strlen2(str); i++) printf(" ");
	printf(" │");
	if (newline == 1) printf("\n");
	return;
}

void print_between_format(char* str, char* format, uint8_t len_cabecalho, uint8_t newline){
	if (len_cabecalho - 4 - (uint8_t) strlen2(str) < 0){
		printf("String to print_between must be at least 4 characters shorter than len_cabecalho.");
		printf("\nPlease increase len_cabecalho.\n");
		exit(5);
	}
	printf("│ %s%s\033[0m", format, str);
	for(uint8_t i=0; i < len_cabecalho-4-strlen2(str); i++) printf(" ");
	printf(" │");
	if (newline == 1) printf("\n");
	return;
}

void print_bottom(uint8_t len_cabecalho, uint8_t newline){
	printf("└");
	for(uint8_t _index = 0; _index<len_cabecalho-2; _index++) printf("─");
	printf("┘");
	if (newline == 1) printf("\n");
	return;
}

void print_upper(uint8_t len_cabecalho, uint8_t newline){
	printf("┌");
	for(uint8_t _index = 0; _index<len_cabecalho-2; _index++) printf("─");
	printf("┐");
	if (newline == 1) printf("\n");
	return;
}

void print_middle(uint8_t len_cabecalho, uint8_t newline){
	printf("├");
	for(uint8_t _index = 0; _index<len_cabecalho-2; _index++) printf("─");
	printf("┤");
	if (newline == 1) printf("\n");
	return;
}

void print_ansi8_palette(char add_border, uint8_t hex_display){
    uint8_t i;
    printf("   ");
    for (i=0; i < 16; i++){
        if (hex_display == 1) printf("%2X ", i);
        else printf("%2d ", i);
    }
    if (add_border==1) printf("│");
    printf("\n");
    for (i=0; i < 16; i++) {
        if (add_border==1) printf("│");
        if (hex_display == 1) printf("%2X ", i);
        else printf("%2d ", i);
        for (uint8_t j=0; j < 16; j++) printf("\033[38;5;%dm██ ", i * 16 + j);
        if (add_border==1) printf("\033[1;37m│");
        printf("\033[1;37m\n");
    }
    return;
}

void print_ansi4_palette(){
    uint8_t i;
    printf("   ");
    for (i=0; i < 4; i++) printf("%2d ", i);
    printf("\n");
    for (i=0; i < 4; i++) {
        printf("%2d ", i);
        for (uint8_t j=0; j < 4; j++) printf("%s██ ", ANSI4[i * 4 + j]);
        printf("\033[1;37m\n");
    }
    return;
}
