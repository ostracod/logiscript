
CC = gcc
CFLAGS = -Wall -c -D _GNU_SOURCE
LDFLAGS = -lm
DIRBUILD = build
DIRSRC = src
SOURCES := $(shell ls $(DIRSRC)/*.c)
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = $(DIRBUILD)/logiscript

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	mkdir -p $(DIRBUILD)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

install:
	mv $(EXECUTABLE) /usr/local/bin/logiscript

clean:
	rm $(OBJECTS) $(EXECUTABLE)


