
all:lib

CC=cc
CFLAGS=-g -Wall -Werror -fpic
LIBFLAGS=-shared
SHELL=sh

BUILD_DIR=build
BUILD_DIR_CREATED=$(BUILD_DIR)/created
OBJ_FILES=$(BUILD_DIR)/unit-test.o

$(BUILD_DIR_CREATED):
	mkdir -p $(BUILD_DIR); touch $(BUILD_DIR_CREATED)

clean:
	rm -frd ./*.o $(SDL_LIB) $(BUILD_DIR)

SDL_FILES=sdl-main.c sdl-net.c
SDL_OBJ=$(shell echo $(SDL_FILES) | sed -E -e 's/([a-z\-]+).c/$(BUILD_DIR)\/\1.o/g')

SDL_CALLBACK_STUB_OBJ=$(BUILD_DIR)/sdl-stub.o

$(BUILD_DIR)/%.o: %.c $(BUILD_DIR_CREATED)
	$(CC) -g -Wall -Werror -o $@ -c $<

$(BUILD_DIR)/basic.o: test/basic.c $(BUILD_DIR_CREATED)
	$(CC) -g -I. -o $@ -c $<

$(BUILD_DIR)/sdl: $(SDL_OBJ) $(SDL_CALLBACK_STUB_OBJ)
	$(CC) -g -Wall -Werror -o $@ $^

test: run-cli-test run-basic-test

CLI_TEST=test/cli-test.pl

run-cli-test: $(BUILD_DIR)/sdl
	./test/cli-test.pl

$(BUILD_DIR)/basic-test: $(BUILD_DIR)/basic.o $(SDL_OBJ)
	$(CC) -g -Wall -lmcgoo -o $@ $^

run-basic-test: $(BUILD_DIR)/basic-test
	./$< -n 2