#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "emulator.h"
#include <pthread.h>
#include <signal.h>

#define eexit(args...) { fprintf(stderr, args); exit(1); }

static int argc;
static char **argv;
static uint8_t multiplier = 2;
static chip8_t *chip8;
static WINDOW *game_window;

static void handle_chip8_call(chip8_t *chip8, chip8_event_t event) {
	switch (event.type) {
		case CHIP8_REDRAW: {
			static _Bool was_inverted = 0;
			_Bool should_invert = (chip8->timers.sound > 0);
			if (was_inverted != should_invert) {
				was_inverted = should_invert;
				event.redraw_event.height = CHIP8_SCREEN_HEIGHT;
				event.redraw_event.width = CHIP8_SCREEN_WIDTH;
				event.redraw_event.x = 0;
				event.redraw_event.y = 0;
			}
			// FIXME: Drawing out of bounds doesn't wrap around
			for (uint8_t y=0; y<event.redraw_event.height; y++) {
				if ((event.redraw_event.y+y) >= CHIP8_SCREEN_HEIGHT) break;
				for (uint8_t x=0; x<event.redraw_event.width; x++) {
					if ((event.redraw_event.x+x) >= CHIP8_SCREEN_HEIGHT) break;
					wmove(game_window, (event.redraw_event.y+y), (event.redraw_event.x+x)*multiplier);
					bool pixel = chip8->framebuffer[x+event.redraw_event.x][y+event.redraw_event.y];
					if (should_invert) pixel = !pixel;
					for (uint8_t i=0; i<multiplier; i++) {
						if (pixel) waddch(game_window, ' ' | A_REVERSE);
						else waddch(game_window, ' ');
					}
				}
			}
			wrefresh(game_window);
			break;
		}
		default:
			break;
	}
}

int main(int _argc, char **_argv) {
	argv = _argv;
	argc = _argc;
	if (argc < 2) {
		eexit("Usage: %s <rom> [font-flag]\n"
		      "  font-flag: Specify 0 to print 1 character per pixel or\n"
			  "             a different value (default) if you want to\n"
			  "             2 characters per pixel.\n", *argv);
	}
	if ((argc >= 3) && (*(argv[2]) == '0')) multiplier = 1;
	if (!(chip8 = chip8_init())) eexit("Failed to initialize the emulator.\n");
	if (!chip8_load_rom(chip8, argv[1])) eexit("Failed to load the ROM file.\n");
	initscr();
	curs_set(0);
	noecho();
	for (uint8_t i=0; i<((CHIP8_SCREEN_WIDTH * multiplier)+2); i++) for (uint8_t y=0; y<CHIP8_SCREEN_HEIGHT+2; y++) mvwaddch(stdscr, y, i, '#');
	game_window = newwin(CHIP8_SCREEN_HEIGHT, (CHIP8_SCREEN_WIDTH * multiplier), 1, 1);
	wclear(game_window);
	wrefresh(stdscr);
	wrefresh(game_window);
	chip8_set_callback(chip8, CHIP8_REDRAW, &handle_chip8_call);
	chip8_start(chip8);
	while (1) {
		// UI Loop
		nodelay(stdscr, 0);
		int c = wgetch(stdscr);
		nodelay(stdscr, 1);
		do {
			if ((c >= 'a') && (c <= 'z')) c -= 0x20;
			static const char *keymap = "X123QWEASDZC4RFV";
			for (unsigned char i=0; i<0x10; i++) {
				if (keymap[i] == c) {
					chip8_keypress(chip8, i);
					break;
				}
			}
		} while ((c = wgetch(stdscr)) != ERR);
	}
	endwin();
	return 0;
}