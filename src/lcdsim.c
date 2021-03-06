
#include "lcdsim.h"

/* Global X vars */
Display *dis;
int screen;
Window win;
GC gc;
unsigned long black, white;

/*LCDSIM vars */
static unsigned int win_width, win_height, lcd_left, lcd_top, lcd_width, lcd_height, lcd_padding, ascii_top;
static uint8_t buf[1024], cur_adr[2], cur_page[2];
static uint8_t lcd_pixel_size;
static char ascii_buf0[20], ascii_buf1[20];
static uint8_t ascii_pos = 0;

void lcdsim_init(const uint8_t pixel_size, const uint8_t padding) {
  // X initialization
  lcd_pixel_size = pixel_size;
  lcd_padding = padding;
  lcd_width = 128 * lcd_pixel_size + 127 * lcd_padding;
  lcd_height = 64 * lcd_pixel_size + 63 * lcd_padding;
  dis = XOpenDisplay((char *)0);
  screen = DefaultScreen(dis);
  black = BlackPixel(dis, screen), white = WhitePixel(dis, screen);
  win = XCreateSimpleWindow(dis, DefaultRootWindow(dis), 0, 0, lcd_width, lcd_height + 10 + lcd_pixel_size * 20, 5, black, white);
  XSetStandardProperties(dis, win, "LCD Simulator", "LCDSim", None, NULL, 0, NULL);
  XSelectInput(dis, win, ExposureMask | KeyPressMask | KeyReleaseMask);
  gc = XCreateGC(dis, win, 0,0);        
  XSetBackground(dis, gc, white);
  XSetForeground(dis, gc, black);
  XClearWindow(dis, win);
  XMapRaised(dis, win);

  // Clear buffer
  int i;
  for (i = 0; i < 1024; i++) {
    buf[i] = 0;
  }

  // Initialize text buffer
  for (i = 0; i < 20; i++) {
    ascii_buf0[i] = ' ';
    ascii_buf1[i] = ' ';
  }
}

void lcdsim_exit() {
  XFreeGC(dis, gc);
  XDestroyWindow(dis, win);
  XCloseDisplay(dis);	
  exit(1);				
}

// Returns pressed keys in character array:
// Up = 0, Down = 1, Left = 2, Right = 3
void lcdsim_keyb (char *c) {
  if (XPending(dis) < 1)
    return;

  XEvent event;
  KeySym key;
  char text[255];
  int arrow_key;

  XNextEvent(dis, &event);
    
  if (event.type == Expose && event.xexpose.count == 0)
    redraw();

  if (event.type == KeyPress) {
    if (XLookupString(&event.xkey,text,255,&key,0) == 1) {
      if (text[0] == 'q')
	lcdsim_exit();
    }
    key = XLookupKeysym(&event.xkey,0);
    arrow_key = keysym_to_arrow_key(key);
    if (arrow_key != -1)
      c[arrow_key] = 1;
  }
  if (event.type == KeyRelease) {
    XEvent next_event;
    key = XLookupKeysym(&event.xkey,0);
    arrow_key = keysym_to_arrow_key(key);
    
    // Check for auto repeat
    if(XPending(dis) > 0
       && XPeekEvent(dis, &next_event)
       && next_event.type == KeyPress
       && next_event.xkey.time == event.xkey.time
       && next_event.xkey.keycode == event.xkey.keycode) {
      XNextEvent(dis, &next_event);
    } else if(c[arrow_key] == 1) {
      c[arrow_key] = 0;
    }
  }
}

void lcdsim_ascii_write_char(unsigned char c) {
  if (ascii_pos < 20)
    ascii_buf0[ascii_pos++] = c;
  else
    ascii_buf1[ascii_pos++ - 20] = c;
  draw_ascii();
}

void lcdsim_ascii_gotoxy(int x, int y) {
  if (x >= 0 && x <= 19 && y >= 0 && y <= 1)
    ascii_pos = x + 20 * y;
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
  for (i = 0; i < 1024; i++) {
    draw_byte(buf[i], i / 8, i % 8);
  }
}

void draw_ascii() {
  XSetForeground(dis, gc, white); // replace with macros
  XFillRectangle(dis, win, gc, lcd_left, ascii_top, lcd_width, lcd_pixel_size * 20 - 1);
  XSetForeground(dis, gc, black); // replace with macros
  int i, x, y, dy;
  for (i = 0; i < 20; i++) {
    x = lcd_left + (i + 1) * lcd_width / 22;
    dy = lcd_pixel_size * 7;
    y = ascii_top + dy;
    XDrawString(dis, win, gc, x, y, ascii_buf0 + i, 1);
    XDrawString(dis, win, gc, x, y + dy, ascii_buf1 +i, 1);
  }
}

int keysym_to_arrow_key(KeySym keysym) {
  switch(keysym) {
  case XK_Up:    return 0;
  case XK_Down:  return 1;
  case XK_Left:  return 2;
  case XK_Right: return 3;
  }
  return -1;
}

void redraw() {
  int x, y; // Get rid of these dummy vars
  unsigned int depth, border_width;
  Window root;
  XGetGeometry(dis, win, &root, &x, &y, &win_width, &win_height, &border_width, &depth);
  lcd_left = (win_width -  lcd_width)  / 2;
  lcd_top =  (win_height - lcd_height) / 2;
  ascii_top = lcd_top + lcd_height + 10;
  draw_buffers();
  draw_ascii();

   
  XSetForeground(dis, gc, black); // replace with macro
  XDrawRectangle(dis, win, gc, lcd_left - 1, lcd_top - 1, lcd_width + 1, lcd_height + 1);
  XDrawRectangle(dis, win, gc, lcd_left - 1, ascii_top -1, lcd_width + 1, lcd_pixel_size * 20);
}

void lcdsim_write_command(const uint8_t cmd, const uint8_t controller) {
  uint8_t cs;
  if (controller & B_CS1)
    cs = 0;
  else if (controller & B_CS2)
    cs = 1;
  else return;
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
  uint8_t data;
  if (controller & B_CS1) {
    data = buf[cur_adr[0] * 8 + cur_page[0]];
  }
  if (controller & B_CS2) {
    data = buf[512 + cur_adr[1] * 8 + cur_page[1]];
  }
  return data;
}

void lcdsim_delay_milli(int ms) {
  unsigned int us = ms * 1000;
  usleep(us);
}

