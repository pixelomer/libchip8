#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "emulator.h"
#include <pthread.h>

#define eexit(args...) { fprintf(stderr, args); exit(1); }

static int argc;
static char **argv;
static chip8_t *chip8;
static WINDOW *game_window;
static WINDOW *debug_window;
static pthread_t main_thread;
static pthread_t calling_thread;
static chip8_callback_type_t call_reason = 0;

static void handle_chip8_call(chip8_t *chip8, chip8_callback_type_t reason) {
	calling_thread = pthread_self();
	call_reason = reason;
	while (call_reason);
}

static void initialize_debug_window(void) {
	debug_window = newwin(32, 11, 1, 130);
	wclear(debug_window);
	for (unsigned int i=0; i<16; i++) {
		mvwprintw(debug_window, i, 1, "V%X:", i);
	}
	mvwprintw(debug_window, 16, 1, "SP:\n PC:\n  I:\n DT:\n ST:");
}

int main(int _argc, char **_argv) {
	argv = _argv;
	argc = _argc;
	main_thread = pthread_self();
	if (argc < 2) {
		eexit("Usage: %s <rom>\n", *argv);
	}
	if (!(chip8 = chip8_init())) eexit("Failed to initialize the emulator.\n");
	if (!chip8_load_rom(chip8, argv[1])) eexit("Failed to load the ROM file.\n");
	initscr();
	curs_set(0);
	noecho();
	nodelay(stdscr, 1);
	for (uint8_t i=0; i<142; i++) for (uint8_t y=0; y<34; y++) mvwaddch(stdscr, y, i, '#');
	game_window = newwin(32, 128, 1, 1);
	wclear(game_window);
	wrefresh(stdscr);
	wrefresh(game_window);
	initialize_debug_window();
	chip8_set_callback(chip8, CHIP8_CYCLE, &handle_chip8_call);
	chip8_set_callback(chip8, CHIP8_REDRAW, &handle_chip8_call);
	chip8_set_callback(chip8, CHIP8_BEEP, &handle_chip8_call);
	chip8_start(chip8);
	while (1) {
		// UI Loop
		int c;
		while ((c = wgetch(stdscr)) != ERR) {
			if (c <= '9' && c >= '0') chip8_keypress(chip8, (uint8_t)(c - 0x30));
			else if (c <= 'f' && c >= 'a') chip8_keypress(chip8, (uint8_t)(c - 0x61 + 10));
			else if (c <= 'F' && c >= 'A') chip8_keypress(chip8, (uint8_t)(c - 0x41 + 10));
		}
		switch (call_reason) {
			case CHIP8_CYCLE: {
				int i;
				for (i=0; i<16; i++) {
					mvwprintw(debug_window, i, 5, "%02X", (unsigned int)chip8->registers[i]);
				}
				mvwprintw(debug_window, 16, 5, "%04X", (unsigned int)chip8->stack_pt);
				mvwprintw(debug_window, 17, 5, "%04X", (unsigned int)chip8->program_counter);
				mvwprintw(debug_window, 18, 5, "%04X", (unsigned int)chip8->mem_pt);
				mvwprintw(debug_window, 19, 5, "%02X", (unsigned int)chip8->timers.delay);
				mvwprintw(debug_window, 20, 5, "%02X", (unsigned int)chip8->timers.sound);
				uint8_t *pt = &chip8->memory[chip8->program_counter];
				for (i=0; i<=16; i+=2) {
					mvwprintw(debug_window, 22+(i/2), 1, "%04X:%02X%02X", (chip8->program_counter+i), pt[i-3], pt[i-2]);
				}
				wrefresh(debug_window);
				break;
			}
			case CHIP8_REDRAW:
				wclear(game_window);
				wmove(game_window, 0, 0);
				for (uint8_t y=0; y<CHIP8_SCREEN_HEIGHT; y++) {
					for (uint8_t x=0; x<CHIP8_SCREEN_WIDTH; x++) {
						bool pixel = chip8->framebuffer[x][y];
						for (uint8_t i=0; i<2; i++) {
							if (pixel) waddch(game_window, ' ' | A_REVERSE);
							else waddch(game_window, ' ');
						}
					}
				}
				wrefresh(game_window);
				break;
			case CHIP8_BEEP:
				//fprintf(stderr,"%u\n", chip8->timers.sound);
				//beep();
				break;
		}
		call_reason = 0;
	}
	endwin();
	return 0;
}