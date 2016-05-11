#ifndef S6B0108EMU_LCDSIM_H
#define S6B0108EMU_LCDSIM_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

// Controller select
#define B_CS2 0x10
#define B_CS1 0x08

// Controller commands
#define LCD_SET_ADD  0x40
#define LCD_SET_PAGE 0xB8

// Function declarations
void lcdsim_init(const uint8_t pixel_size, const uint8_t padding);
void lcdsim_exit();
void lcdsim_keyb(char* c);
void lcdsim_write_command(const uint8_t cmd, const uint8_t controller);
void lcdsim_write_data(const uint8_t data, const uint8_t controller);
uint8_t lcdsim_read(const uint8_t controller);
void lcdsim_delay_milli(int ms);

int keysym_to_arrow_key(KeySym keysym);
void redraw();
void draw_byte(uint8_t byte, uint8_t adr, uint8_t page);
void draw_buffers();

#endif // S6B0108EMU_LCDSIM_H
