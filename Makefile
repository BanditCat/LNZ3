################################################################################
## Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         ##
################################################################################


.PHONY: all
all: depend debug



# Toolchain.
CC=gcc
CCFLAGS=-std=c11 -Wall -fno-exceptions -pedantic -Wextra -Werror -c 
CCINCFLAG=-I/include/SDL2 -Iglew
LD=gcc
LDFLAGS=-Lglew -lglew32 -lSDL2_test -lmingw32 -lSDL2main -lSDL2 -lopengl32 -mwindows -L/lib -lmingw32 -lSDL2main -lSDL2 -mwindows -Wl,--no-undefined -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -static-libgcc


TARGET=lnz.exe
TARGETDEFINE=-DWINDOWS
OBJS:=$(OBJS) windowsResource.o

# Actual build rules.
# These are supposed everything that might be edited.
TXTS:=$(TXTS) $(wildcard ./*.txt) ./Makefile ./README.md ./windowsResource.rc
SRCS:=$(SRCS) $(wildcard ./*.h) $(wildcard ./*.c)
SHDRS:=$(SHDRS) $(wildcard ./*.glsl) $(wildcard ./*.vert) $(wildcard ./*.frag)
CS:=$(CS) $(wildcard ./*.c)
OBJS:=$(OBJS) $(CS:.c=.o)
$(OBJS): Makefile
include deps

# windres
windowsResource.o: windowsResource.rc bcj.ico $(SHDRS)
	windres $< -o $@

# Override defaults
%.o: %.c
	$(CC) $(CCINCFLAG) $(CCFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(LD) $^ -o $@ $(LDFLAGS)
	$(STRIP)
	$(PACK)

TAGS: $(SRCS)
	etags --declarations --ignore-indentation $^


.PHONY: release 
release: $(TARGET)
release: CCFLAGS:=-O4 $(TARGETDEFINE) -flto $(CCFLAGS)
release: LDFLAGS:=-O4 -flto $(LDFLAGS)
release: STRIP:=strip -p $(TARGET)
release: PACK:=upx --best $(TARGET)

.PHONY: debug 
debug: $(TARGET)
debug: CCFLAGS:=$(TARGETDEFINE) -ggdb -DDEBUG $(CCFLAGS)


.PHONY: clean
clean:
	rm -f ./*.o ./$(TARGET)

.PHONY: backup
backup: clean release
	git add -A
	git commit -a -m "$(shell cat ~/lnz3/workingon.txt)" || true

.PHONY: depend
depend:
	gcc $(CCINCFLAG) $(TARGETDEFINE) -MM $(CS) > ./deps

.PHONY: run
run: all
	./$(TARGET)

.PHONY: unixify
unixify:
	dos2unix -U $(TXTS) $(SRCS)

