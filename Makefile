# Compiler and flags #
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I./src

# Dir #
SOURCE_DIR = src
OUTPUT_DIR = bin

# Src files #
MAIN_SOURCE = $(SOURCE_DIR)/main.c
LIBRARY_SOURCES = $(wildcard $(SOURCE_DIR)/motivesyz/core/*.c)

# Targets #
all: build

build:
	@mkdir -p $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -o $(OUTPUT_DIR)/motivesyz $(MAIN_SOURCE) $(LIBRARY_SOURCES)

clean:
	rm -rf $(OUTPUT_DIR)

run: build
	./$(OUTPUT_DIR)/motivesyz

.PHONY: all build clean run
