/*
 * S6B0108 Emulator
 *
 * Must call keyb(char* c) in the main loop, because this function handles all X11 events, including redraw
 * of windows.
 */

// Define to get substitutions of function names, see main.h for details.
#define SIMULATOR

#include "main.h"

#ifdef SIMULATOR
#include "lcdsim.h"
#endif

// Example main function and loop
int main() {
  // Initialize the display with pixel size, pixel padding (not implemented yet)
  lcdsim_init(3, 0);
  
  // Character array containing the keypresses
  char c[4] = {0,0,0,0};

  int i = 0;
  while(1) {
    // Need to call keyb() even if there is no input because all X11 events i handled there
    keyb(c);

    draw_byte((c[0]) ? 0xFF : 0, 63, 3);
    draw_byte((c[1]) ? 0xFF : 0, 63, 4);
    draw_byte((c[2]) ? 0xFF : 0, 61, 4);
    draw_byte((c[3]) ? 0xFF : 0, 65, 4);

    draw_byte(0, (i % 128), 7);
    draw_byte(0xFF, (++i % 128), 7);
    delay_milli(40);
  }
  return 0;
}

