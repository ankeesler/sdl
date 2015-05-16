all:test

#
# VARS
#

CC=cc
CFLAGS=-g -Wall -Werror -I. -Iphy -Imac -Isnet
DEFINES=
LIBFLAGS=-shared
SHELL=sh

BUILD_DIR=build
BUILD_DIR_CREATED=$(BUILD_DIR)/created

SDL_LOG_TEST_FILE=tuna.sdl

TEST_APPS_DIR=nodes
TEST_DIR=test

VPATH=$(TEST_DIR) $(TEST_APPS_DIR)

#
# BUILD STUFF
#

clean: clean-cap
	rm -frd ./*.o $(SDL_LIB) $(BUILD_DIR) $(SDL_LOG_TEST_FILE)

$(BUILD_DIR_CREATED):
	mkdir -p $(BUILD_DIR); touch $(BUILD_DIR_CREATED)

$(BUILD_DIR)/%.o: %.c $(BUILD_DIR_CREATED)
	$(CC) $(CFLAGS) $(DEFINES) -I. -o $@ -c $<

$(BUILD_DIR)/sdl-log-on.o: sdl-log.c $(BUILD_DIR_CREATED)
	$(CC) -g -DSDL_LOG -DSDL_LOG_FILE=\"$(SDL_LOG_TEST_FILE)\" -I. -o $@ -c $<

test: run-sdl-test

#
# SDL
#

SDL_FILES=mac/mac.c cap/sdl-log.c
SDL_MAIN_FILE=sdl-main.c

VPATH += $(dir $(SDL_FILES))

BASIC_TEST_FILES=$(TEST_DIR)/basic.c $(SDL_FILES) $(SDL_MAIN_FILE)
$(BUILD_DIR)/basic-test: $(addprefix $(BUILD_DIR)/,$(notdir $(BASIC_TEST_FILES:.c=.o)))
	$(CC) $(CFLAGS) -lmcgoo -o $@ $^

run-basic-test: $(BUILD_DIR)/basic-test
	./$<

SDL_TEST_FILES=$(TEST_DIR)/sdl-test.c $(SDL_FILES)
$(BUILD_DIR)/sdl-test: $(addprefix $(BUILD_DIR)/,$(notdir $(SDL_TEST_FILES:.c=.o)))
	$(CC) -lmcgoo -o $@ $^

run-sdl-test: $(BUILD_DIR)/sdl-test
	./$<

$(BUILD_DIR)/log-test: $(BUILD_DIR)/log.o \
                       $(BUILD_DIR)/sdl-log-on.o \
                       $(BUILD_DIR)/sdl-main.o \
                       $(BUILD_DIR)/sdl-net.o \
                       $(BUILD_DIR)/sdl-id.o
	$(CC) -g -Wall -lmcgoo -o $@ $^

run-log-test: $(BUILD_DIR)/log-test
	./$< -n 2

run-full-log-test: run-log-test
	./test/log-test.pl

#
# SNET
#

SNET_PARENT_FILES=snet/snet.c
SNET_CHILD_FILES=phy/phy.c $(SDL_FILES)

VPATH += $(dir $(SNET_PARENT_FILES)) $(dir $(SNET_CHILD_FILES))

SNET_TEST_EXES=              \
  $(BUILD_DIR)/snet-test     \
  $(BUILD_DIR)/client/client \
  $(BUILD_DIR)/server/server

$(SNET_TEST_EXES): DEFINES += -DSNET_TEST

SNET_TEST_FILES=$(SNET_PARENT_FILES) $(TEST_DIR)/snet-test.c $(SDL_FILES)
SNET_TEST_OBJ=$(addprefix $(BUILD_DIR)/,$(notdir $(SNET_TEST_FILES:.c=.o)))
$(BUILD_DIR)/snet-test: $(SNET_TEST_OBJ) | $(BUILD_DIR_CREATED)
	$(CC) $(CFLAGS) -lmcgoo -o $@ $^

run-snet-test: $(SNET_TEST_EXES)
	./$<

#
# TEST APPS
#

SERVER_DIR_CREATED=$(BUILD_DIR)/server/created
$(SERVER_DIR_CREATED): $(BUILD_DIR_CREATED)
	mkdir $(@D) && touch $@
$(BUILD_DIR)/server/%.o: %.c | $(SERVER_DIR_CREATED)
	$(CC) $(CFLAGS) $(DEFINES) -I. -o $@ -c $<
SERVER_FILES=$(SNET_CHILD_FILES) $(TEST_APPS_DIR)/server.c
SERVER_OBJ=$(addprefix $(BUILD_DIR)/server/,$(notdir $(SERVER_FILES:.c=.o)))
$(BUILD_DIR)/server/server: $(SERVER_OBJ) | $(BUILD_DIR_CREATED)
	$(CC) $(CFLAGS) -o $@ $^
run-server: $(BUILD_DIR)/server/server
	./$< $(ARGS)

CLIENT_DIR_CREATED=$(BUILD_DIR)/client/created
$(CLIENT_DIR_CREATED): $(BUILD_DIR_CREATED)
	mkdir $(@D) && touch $@
$(BUILD_DIR)/client/%.o: %.c | $(CLIENT_DIR_CREATED)
	$(CC) $(CFLAGS) $(DEFINES) -I. -o $@ -c $<
CLIENT_FILES=$(SNET_CHILD_FILES) $(TEST_APPS_DIR)/client.c
CLIENT_OBJ=$(addprefix $(BUILD_DIR)/client/,$(notdir $(CLIENT_FILES:.c=.o)))
$(BUILD_DIR)/client/client: $(CLIENT_OBJ) | $(BUILD_DIR_CREATED)
	$(CC) $(CFLAGS) -o $@ $^
run-client: $(BUILD_DIR)/client/client
	./$< $(ARGS)

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
