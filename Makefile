CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -Werror -fstack-protector-strong -g

.PHONY: all clean test

BUILDDIR = build
EXECUTABLE = $(BUILDDIR)/sokoban

all: $(EXECUTABLE)

$(EXECUTABLE): sokoban.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $< -o $@

clean:
	find tests -name "*.myout" -delete

test: $(EXECUTABLE) test.sh tests
	./test.sh
