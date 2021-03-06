/*
 * S6B0108 Emulator
 *
 * Must call keyb(char* c) in the main loop, because this function handles all
 * X11 events, including redraw of windows.
 */

// Define this to get substitutions of function names, see main.h for details.
#define SIMULATOR

#include "main.h"

#ifdef SIMULATOR
#include "lcdsim.h"
#endif

// Character array containing the keypresses
char c[4] = {0,0,0,0};

// Example main function and loop
int main() {
  // Initialize the display with (pixel size, pixel padding) (pixel padding not
  // implemented yet)
  lcdsim_init(3, 0);

  // Ascii display example
  char *cp, test0[] = "Static test string", test1[] = "Another string";
  cp = test0;
  ascii_gotoxy(0,0);
  while (*cp)
    ascii_write_char(*cp++);
  cp = test1;
  ascii_gotoxy(0,1);
  while (*cp)
    ascii_write_char(*cp++);
  
  int i = 0;
  while(1) {
    keyb(c);

    // Show pressed down key
    draw_byte((c[0]) ? 0xFF : 0, 63, 3);
    draw_byte((c[1]) ? 0xFF : 0, 63, 5);
    draw_byte((c[2]) ? 0xFF : 0, 58, 4);
    draw_byte((c[3]) ? 0xFF : 0, 68, 4);

    // Moving byte
    draw_byte(0, (i % 128), 7);
    draw_byte(0xFF, (++i % 128), 7);

    delay_milli(40);    
  }
  return 0;
}

