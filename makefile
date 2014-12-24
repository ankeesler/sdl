
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
	rm -frd ./*.o $(SDL_LIB) $(BUILD_DIR) $(SDL_LOG_TEST_FILE)

SDL_FILES=sdl-main.c sdl-net.c sdl-log.c
SDL_OBJ=$(shell echo $(SDL_FILES) | sed -E -e 's/([a-z\-]+).c/$(BUILD_DIR)\/\1.o/g')

SDL_LOG_TEST_FILE=tuna.sdl

SDL_CALLBACK_STUB_OBJ=$(BUILD_DIR)/sdl-stub.o

$(BUILD_DIR)/%.o: %.c $(BUILD_DIR_CREATED)
	$(CC) -g -Wall -Werror -o $@ -c $<

$(BUILD_DIR)/%.o: test/%.c $(BUILD_DIR_CREATED)
	$(CC) -g -I. -o $@ -c $<

$(BUILD_DIR)/sdl-log-on.o: sdl-log.c $(BUILD_DIR_CREATED)
	$(CC) -g -DSDL_LOG -DSDL_LOG_FILE=\"$(SDL_LOG_TEST_FILE)\" -I. -o $@ -c $<

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

#
# CAPTURE FRAMEWORK
#

ANTLR_JAR=/usr/local/lib/antlr-4.2-complete.jar
SNAKE_YAML_JAR=/usr/local/lib/snakeyaml-1.14.jar
IPV6_YAML_FILE=ipv6.yaml
MANIFEST_FILE=manifest.txt

grammar:
	java -jar $(ANTLR_JAR) cap/SdlLog.g4

.PHONY: cap
cap: grammar
	javac -cp $(ANTLR_JAR):$@:$(SNAKE_YAML_JAR) $@/*.java

.PHONY: cap/Decode.jar
cap/Decode.jar: cap
	cd $<; jar cfm $(@F) $(MANIFEST_FILE) *.class $(IPV6_YAML_FILE)

decoder: cap/Decode.jar
	java -jar $<

clean-cap:
	rm -f cap/*.class cap/*.jar cap/Decode.jar
