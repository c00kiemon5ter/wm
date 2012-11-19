
VERSION = "cookiejar-git"
WM_NAME = "cookiewm"

CC      = cc
LIBS    = -lxcb -lxcb-icccm -lxcb-ewmh -lxcb-xinerama -lxcb-randr
CFLAGS  = -std=c99 -pedantic -pedantic-errors -Wall -Wextra
CFLAGS += -DWM_NAME=\"$(WM_NAME)\" -DVERSION=\"$(VERSION)\"
LDFLAGS = $(LIBS)

PREFIX   ?= /usr/local
BINPREFIX = $(PREFIX)/bin

SRC = wm.c helpers.c events.c messages.c monitor.c client.c rules.c pointer.c window.c ewmh.c icccm.c tile.c
OBJ = $(SRC:.c=.o)

all: CFLAGS += -Os
all: LDFLAGS += -s
all: options $(WM_NAME)

debug: CFLAGS += -O0 -g -DDEBUG
debug: options $(WM_NAME)

options:
	@echo "$(WM_NAME) build options:"
	@echo "CC      = $(CC)"
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "PREFIX  = $(PREFIX)"

.c.o:
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(WM_NAME): $(OBJ)
	@echo CC -o $@
	@$(CC) -o $@ $(OBJ) $(LDFLAGS)

clean:
	@echo "cleaning"
	@rm -f $(OBJ) $(WM_NAME)

install:
	@echo "installing executable files to $(DESTDIR)$(BINPREFIX)"
	@install -D -m 755 $(WM_NAME) $(DESTDIR)$(BINPREFIX)/$(WM_NAME)

uninstall:
	@echo "removing executable files from $(DESTDIR)$(BINPREFIX)"
	@rm -f $(DESTDIR)$(BINPREFIX)/$(WM_NAME)

.PHONY: all debug options clean install uninstall
