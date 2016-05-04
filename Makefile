
SRCS   	= $(wildcard src/*.c)
OUT     = build

CFLAGS 	= -Wall -lX11 

### Compile, link and create load file
$(OUT)/S6B0108 : $(SRCS)
	cc $(CFLAGS) $(SRCS) -o $(OUT)/S6B0108


### Cleanup
.PHONY  : clean
clean   :
	-rm $(OUT)/S6B0108

