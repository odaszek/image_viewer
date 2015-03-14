CC = gcc
PKGCONFIG = $(shell which pkg-config)
CFLAGS = `$(PKGCONFIG) --cflags gtk+-3.0`
LIBS = `$(PKGCONFIG) --libs gtk+-3.0`
GLIB_COMPILE_RESOURCES = `$(PKGCONFIG) --variable=glib_compile_resources gio-2.0`

SRC = resources.c imageview.c controlview.c appwindow.c application.c main.c

OBJS = $(SRC:.c=.o)

all: application

resources.c: resource.xml window.ui
	$(GLIB_COMPILE_RESOURCES) resource.xml --target=$@ --sourcedir=. --generate-source

%.o: %.c
	$(CC) -c -o $(@F) $(CFLAGS) $<

application: $(OBJS)
	$(CC) -o $(@F) $(LIBS) $(OBJS) -lm

clean:
	rm -f resources.c
	rm -f $(OBJS)
	rm -f application
