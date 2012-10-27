
VERSION = "cookiejar-git"
WM_NAME = "cookiewm"
CL_NAME = "cookie"

CC      = cc
LIBS    = -lxcb -lxcb-icccm -lxcb-ewmh -lxcb-xinerama -lxcb-randr
CFLAGS  = -std=c99 -pedantic -pedantic-errors -Wall -Wextra
CFLAGS += -DWM_NAME=\"$(WM_NAME)\" -DVERSION=\"$(VERSION)\"
LDFLAGS = $(LIBS)

PREFIX   ?= /usr/local
BINPREFIX = $(PREFIX)/bin

WM_SRC = cookiewm.c helpers.c events.c messages.c monitor.c client.c rules.c pointer.c window.c ewmh.c icccm.c tile.c
CL_SRC = cookie.c helpers.c

WM_OBJ = $(WM_SRC:.c=.o)
CL_OBJ = $(CL_SRC:.c=.o)

all: CFLAGS += -Os
all: LDFLAGS += -s
all: options $(WM_NAME) $(CL_NAME)

debug: CFLAGS += -O0 -g -DDEBUG
debug: options $(WM_NAME) $(CL_NAME)

options:
	@echo "$(WM_NAME) build options:"
	@echo "CC      = $(CC)"
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "PREFIX  = $(PREFIX)"

.c.o:
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(WM_NAME): $(WM_OBJ)
	@echo CC -o $@
	@$(CC) -o $@ $(WM_OBJ) $(LDFLAGS)

$(CL_NAME): $(CL_OBJ)
	@echo CC -o $@
	@$(CC) -o $@ $(CL_OBJ) $(LDFLAGS)

clean:
	@echo "cleaning"
	@rm -f $(WM_OBJ) $(CL_OBJ) $(WM_NAME) $(CL_NAME)

install:
	@echo "installing executable files to $(DESTDIR)$(BINPREFIX)"
	@install -D -m 755 $(WM_NAME) $(DESTDIR)$(BINPREFIX)/$(WM_NAME)
	@install -D -m 755 $(CL_NAME) $(DESTDIR)$(BINPREFIX)/$(CL_NAME)

uninstall:
	@echo "removing executable files from $(DESTDIR)$(BINPREFIX)"
	@rm -f $(DESTDIR)$(BINPREFIX)/$(WM_NAME)
	@rm -f $(DESTDIR)$(BINPREFIX)/$(CL_NAME)

.PHONY: all debug options clean install uninstall
