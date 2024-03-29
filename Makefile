GCC := gcc
CFLAGS := -Wall -Wextra -Werror -Wpedantic -Wconversion -std=c99 -g -O3
TARGET := $(wildcard src/*.c) 
ELF := $(TARGET:.c=.o)
EXEC := src/main

.PHONY: all clean

all: $(EXEC)
	mv $(EXEC) .
	mkdir -p elf
	mv $(ELF) elf

$(EXEC): $(ELF)
	$(GCC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(GCC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(ELF) $(EXEC)
	rm -rf elf
	rm -rf main
