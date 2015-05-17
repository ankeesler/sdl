#
# TEST
#

all: test

#
# VARS
#

INC_DIR=inc
SNET_DIR=snet
PHY_DIR=phy
MAC_DIR=mac
TEST_APPS_DIR=nodes
TEST_DIR=test
CAP_DIR=cap

INCLUDES=-I. -I$(INC_DIR) -I$(PHY_DIR) -I$(MAC_DIR) -I$(SNET_DIR)
DEFINES=

CC=gcc
CFLAGS=-g -O0 -Wall -Werror $(INCLUDES) $(DEFINES)
LDFLAGS=-lmcgoo
SHELL=sh

BUILD_DIR=build
BUILD_DIR_CREATED=$(BUILD_DIR)/created

VPATH=$(SNET_DIR) $(PHY_DIR) $(MAC_DIR) $(TEST_APPS_DIR) $(TEST_DIR) $(CAP_DIR)

#
# UTIL
#

run-%: $(BUILD_DIR)/%
	./$<

#
# BUILD
#

clean: clean-cap
	rm -frd ./*.o $(BUILD_DIR) $(SDL_LOG_TEST_FILE)

$(BUILD_DIR_CREATED):
	mkdir -p $(BUILD_DIR) && touch $(BUILD_DIR_CREATED)

$(BUILD_DIR)/%.o: %.c $(BUILD_DIR_CREATED)
	$(CC) $(CFLAGS) -o $@ -c $<

#
# TEST
#

test: run-mac-test run-log-test

#
# PHY
#

PHY_FILES=$(PHY_DIR)/phy.c

#
# MAC
#

MAC_FILES=$(MAC_DIR)/mac.c

MAC_TEST_FILES=$(TEST_DIR)/mac-test.c $(MAC_FILES)
$(BUILD_DIR)/mac-test: $(addprefix $(BUILD_DIR)/,$(notdir $(MAC_TEST_FILES:.c=.o)))
	$(CC) $(LDFLAGS) -o $@ $^

#
# SDL
#

SDL_LOG_FILES=$(CAP_DIR)/sdl-log.c

SDL_FILES=$(PHY_FILES) $(MAC_FILES) $(SDL_LOG_FILES)

SDL_LOG_TEST_FILE=tuna.sdl
LOG_TEST_FILES=$(PHY_FILES) $(SDL_LOG_FILES) test/log-test.c
$(BUILD_DIR)/log-test: DEFINES += -DSDL_LOG -DSDL_LOG_FILE=\"$(SDL_LOG_TEST_FILE)\" -DSNET_TEST
$(BUILD_DIR)/log-test: $(addprefix $(BUILD_DIR)/,$(notdir $(LOG_TEST_FILES:.c=.o)))
	$(CC) $(LDFLAGS) -o $@ $^

#
# SNET
#

SNET_PARENT_FILES=$(SNET_DIR)/snet.c
SNET_CHILD_FILES=$(SDL_FILES)

SNET_TEST_EXES=              \
  $(BUILD_DIR)/snet-test     \
  $(BUILD_DIR)/client/client \
  $(BUILD_DIR)/server/server

$(SNET_TEST_EXES): DEFINES += -DSNET_TEST

SNET_TEST_FILES=$(SNET_PARENT_FILES) $(TEST_DIR)/snet-test.c $(SDL_FILES)
SNET_TEST_OBJ=$(addprefix $(BUILD_DIR)/,$(notdir $(SNET_TEST_FILES:.c=.o)))
$(BUILD_DIR)/snet-test: $(SNET_TEST_OBJ) | $(BUILD_DIR_CREATED)
	$(CC) $(LDFLAGS) -o $@ $^

run-snet-test: $(SNET_TEST_EXES)
	./$<

#
# TEST APPS
#

SERVER_DIR_CREATED=$(BUILD_DIR)/server/created
$(SERVER_DIR_CREATED): $(BUILD_DIR_CREATED)
	mkdir $(@D) && touch $@
$(BUILD_DIR)/server/%.o: %.c | $(SERVER_DIR_CREATED)
	$(CC) $(CFLAGS) -I. -o $@ -c $<
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

.PHONY: $(CAP_DIR)
$(CAP_DIR): grammar
	javac -cp $(ANTLR_JAR):$@:$(SNAKE_YAML_JAR) $@/*.java

.PHONY: cap/Decode.jar
$(CAP_DIR)/Decode.jar: $(CAP_DIR)
	cd $<; jar cfm $(@F) $(MANIFEST_FILE) *.class $(IPV6_YAML_FILE)

decoder: $(CAP_DIR)/Decode.jar
	java -jar $<

clean-cap:
	rm -f $(CAP_DIR)/*.class $(CAP_DIR)/*.jar $(CAP_DIR)/Decode.jar
