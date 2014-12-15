
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

clean: clean-cap
	rm -frd ./*.o $(SDL_LIB) $(BUILD_DIR) tuna.sdl

SDL_FILES=sdl-main.c sdl-net.c sdl-log.c
SDL_OBJ=$(shell echo $(SDL_FILES) | sed -E -e 's/([a-z\-]+).c/$(BUILD_DIR)\/\1.o/g')

SDL_CALLBACK_STUB_OBJ=$(BUILD_DIR)/sdl-stub.o

$(BUILD_DIR)/%.o: %.c $(BUILD_DIR_CREATED)
	$(CC) -g -Wall -Werror -o $@ -c $<

$(BUILD_DIR)/%.o: test/%.c $(BUILD_DIR_CREATED)
	$(CC) -g -I. -o $@ -c $<

$(BUILD_DIR)/sdl-log-on.o: sdl-log.c $(BUILD_DIR_CREATED)
	$(CC) -g -DSDL_LOG -DSDL_LOG_FILE=\"tuna.sdl\" -I. -o $@ -c $<

$(BUILD_DIR)/sdl: $(SDL_OBJ) $(SDL_CALLBACK_STUB_OBJ)
	$(CC) -g -Wall -Werror -o $@ $^

test: run-cli-test run-basic-test run-full-log-test

CLI_TEST=test/cli-test.pl

run-cli-test: $(BUILD_DIR)/sdl
	./test/cli-test.pl

$(BUILD_DIR)/basic-test: $(BUILD_DIR)/basic.o $(BUILD_DIR)/sdl-log.o $(SDL_OBJ)
	$(CC) -g -Wall -lmcgoo -o $@ $^

run-basic-test: $(BUILD_DIR)/basic-test
	./$< -n 2

$(BUILD_DIR)/log-test: $(BUILD_DIR)/log.o \
											 $(BUILD_DIR)/sdl-log-on.o \
	                     $(BUILD_DIR)/sdl-main.o \
                       $(BUILD_DIR)/sdl-net.o
	$(CC) -g -Wall -lmcgoo -o $@ $^

run-log-test: $(BUILD_DIR)/log-test
	./$< -n 2

run-full-log-test: run-log-test
	./test/log-test.pl

ANTLR_JAR=/usr/local/lib/antlr-4.2-complete.jar

grammar:
	java -jar $(ANTLR_JAR) cap/SdlLog.g4

.PHONY: cap
cap: grammar
	javac -cp $(ANTLR_JAR):$@ $@/*.java

.PHONY: cap/Decode.jar
cap/Decode.jar: cap
	cd $<; jar cfm $(@F) manifest.txt *.class 

decoder: cap/Decode.jar
	java -jar $<

clean-cap:
	rm -f cap/*.class cap/*.jar cap/Decode.jar

