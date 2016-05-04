/* Reference: http://math.msu.su/~vvb/2course/Borisenko/CppProjects/GWindow/xintro.html
   Compile:   gcc src/main.c -o lcdsim -lX11 -Wall */

#define DEBUG

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#define B_CS2    0x10 /* Controller select 2 */
#define B_CS1    0x08 /* Controller select 1 */

#define LCD_SET_ADD    0x40  // 0100 0000
#define LCD_SET_PAGE   0xB8  // 1100 1000

/* Global X vars */
Display *dis;
int screen;
Window win;
GC gc;
unsigned long black, white; // moved here from init

unsigned int win_width, win_height, lcd_left, lcd_top, lcd_width, lcd_height, lcd_padding;

/*LCDSIM vars */
static uint8_t buf[1024], cur_adr[2], cur_page[2];
static uint8_t lcd_pixel_size;

/* Function declarations */
void main_loop();
void lcdsim_init(const uint8_t pixel_size, const uint8_t padding);
void lcdsim_exit();
void redraw();
void draw_buffers();
void lcdsim_write_command(const uint8_t cmd, const uint8_t controller);
void lcdsim_handle_command(const uint8_t cmd, uint8_t cs);
void lcdsim_write_data(const uint8_t data, const uint8_t controller);
uint8_t lcdsim_read(const uint8_t controller);
void delay_milli(int ms);

int main() {
  lcdsim_init(3, 0);
  main_loop();
  return 0;
}

void main_loop () {
  XEvent event;
  KeySym key;
  char text[255];

  int i, j;

  XNextEvent(dis, &event);
    
  if (event.type == Expose && event.xexpose.count == 0)
    redraw();

  for (j = 0; j < 64; j++) {
    for (i = 0; i < 64; i++) {
      lcdsim_write_command(LCD_SET_PAGE | i % 8, B_CS1 | B_CS2);
      lcdsim_write_command(LCD_SET_ADD  | (i / 8 + j), B_CS1 | B_CS2);
      lcdsim_write_data(0x0F, B_CS2);
      lcdsim_write_data(0xF0, B_CS1);
    }
    delay_milli(40);
  }

  printf("Klar\n");

  while(1) {		
    XNextEvent(dis, &event);
    
    if (event.type == Expose && event.xexpose.count == 0)
      redraw();
    if (event.type == KeyPress && XLookupString(&event.xkey,text,255,&key,0) == 1) {
      /* use the XLookupString routine to convert the invent KeyPress data into regular text. */
      if (text[0] == 'q')
	lcdsim_exit();
    }
  }
}

void lcdsim_init(const uint8_t pixel_size, const uint8_t padding) {
  /* X initialization */
  lcd_pixel_size = pixel_size;
  lcd_padding = padding;
  lcd_width = 128 * lcd_pixel_size + 127 * lcd_padding;
  lcd_height = 64 * lcd_pixel_size + 63 * lcd_padding;
#ifdef DEBUG
  printf("lcd_width: %d\nlcd_height: %d\n", lcd_width, lcd_height);
#endif

  dis = XOpenDisplay((char *)0);
  screen = DefaultScreen(dis);
  black = BlackPixel(dis, screen), white = WhitePixel(dis, screen);
  win = XCreateSimpleWindow(dis, DefaultRootWindow(dis), 0, 0, lcd_width, lcd_height, 5, black, white);
  XSetStandardProperties(dis, win, "LCD Simulator", "LCDSim", None, NULL, 0, NULL);
  XSelectInput(dis, win, ExposureMask | KeyPressMask);
  gc = XCreateGC(dis, win, 0,0);        
  XSetBackground(dis, gc, white);
  XSetForeground(dis, gc, black);
  XClearWindow(dis, win);
  XMapRaised(dis, win);

  int i;
  for (i = 0; i < 1024; i++) {
    buf[i] = 0;
  }
}

void lcdsim_exit() {
  XFreeGC(dis, gc);
  XDestroyWindow(dis, win);
  XCloseDisplay(dis);	
  exit(1);				
}

void draw_byte(uint8_t byte, uint8_t adr, uint8_t page) {
  int i, j, k, x, y;
  for (i = 0; i < 8; i++) {
    x = lcd_left + adr * lcd_pixel_size;
    y = lcd_top + (page * 8 + i) * lcd_pixel_size;
    XSetForeground(dis, gc, (byte & 1 << i) ? black : white); // replace with macros
    for (j = 0; j < lcd_pixel_size ; j++) {
      for (k = 0; k < lcd_pixel_size; k++) {
	XDrawPoint(dis, win, gc, x + j, y + k);
      }
    }
  } 
}

void draw_buffers() {
  int i;
  
  //XClearWindow(dis, win);
  
  for (i = 0; i < 1024; i++) {
    draw_byte(buf[i], i / 8, i % 8);
  }
}

void redraw() {
  int x, y; // Get rid of these dummy vars
  unsigned int depth, border_width;
  Window root;
  XGetGeometry(dis, win, &root, &x, &y, &win_width, &win_height, &border_width, &depth);
#ifdef DEBUG
  printf("win_width: %d\nwin_height: %d\n", win_width, win_height);
#endif
  lcd_left = (win_width -  lcd_width)  / 2;
  lcd_top =  (win_height - lcd_height) / 2;
#ifdef DEBUG
  printf("lcd_left: %d\nlcd_top: %d\n", lcd_left, lcd_top);
#endif

  draw_buffers();

  // Draw box around display
  XSetForeground(dis, gc, black); // replace with macro
  XDrawLine(dis, win, gc, lcd_left - 1, lcd_top - 1, lcd_left + lcd_width + 1, lcd_top - 1);
  XDrawLine(dis, win, gc, lcd_left - 1, lcd_top + lcd_height, lcd_left + lcd_width + 1, lcd_top + lcd_height);
  XDrawLine(dis, win, gc, lcd_left - 1, lcd_top - 1, lcd_left - 1, lcd_top + lcd_height);
  XDrawLine(dis, win, gc, lcd_left + lcd_width, lcd_top - 1, lcd_left + lcd_width, lcd_top + lcd_height);
}

void lcdsim_write_command(const uint8_t cmd, const uint8_t controller) {
  if (controller & B_CS1)
    lcdsim_handle_command(cmd, 0);
  if (controller & B_CS2)
    lcdsim_handle_command(cmd, 1);
}

void lcdsim_handle_command(const uint8_t cmd, uint8_t cs) {
  if (cmd & LCD_SET_ADD)
    cur_adr[cs] = cmd & 0x3F;
  else if (cmd & LCD_SET_PAGE)
    cur_page[cs] = cmd & 0x7;
}

void lcdsim_write_data(const uint8_t data, const uint8_t controller) {
  /* Replace index with pointer */
  if (controller & B_CS1) {
    buf[cur_adr[0] * 8 + cur_page[0]] = data;
    draw_byte(data, cur_adr[0], cur_page[0]);
  }
  if (controller & B_CS2) {
    buf[512 + cur_adr[1] * 8 + cur_page[1]] = data;
    draw_byte(data, 64 + cur_adr[1], cur_page[1]);
  }
}

uint8_t lcdsim_read(const uint8_t controller) {
  return 0;
}

void delay_milli(int ms) {
  unsigned int us = ms * 1000;
  usleep(us);
}
