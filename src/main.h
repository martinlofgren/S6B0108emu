#ifndef S6B0108EMU_MAIN_H
#define S6B0108EMU_MAIN_H

// Substitutions of function names. The purpose of this is to be able to quickly
// switch between simulator and hardware.
#ifdef SIMULATOR
#define keyb(c) lcdsim_keyb(c)
#define graphic_write_command(cmd, controller) lcdsim_write_command(cmd, controller)
#define graphic_write_data(data, controller) lcdsim_write_data(cmd, controller)
#define graphic_read(controller) lcdsim_read(controller) 
#define delay_milli(ms) lcdsim_delay_milli(ms)
#define ascii_write_char(c) lcdsim_ascii_write_char(c)
#define ascii_gotoxy(x, y) lcdsim_ascii_gotoxy(x, y)
#endif

#endif // S6B0108EMU_MAIN_H
