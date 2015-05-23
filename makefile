#
# TEST
#

all: test

#
# VARS
#

SHELL=sh

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

VPATH=$(SNET_DIR) $(PHY_DIR) $(MAC_DIR) $(TEST_APPS_DIR) $(TEST_DIR) $(CAP_DIR)

ALL_SOURCE=$(shell find . -name "*.[ch]") $(shell find . -name "*.java")

#
# SOURCE
#

SDL_LOG_FILES=$(CAP_DIR)/sdl-log.c

MAC_FILES=$(MAC_DIR)/mac.c

PHY_FILES=$(PHY_DIR)/phy.c

SDL_FILES=$(PHY_FILES) $(MAC_FILES) $(SDL_LOG_FILES)

SNET_CHILD_FILES=$(SDL_FILES)
SNET_DEBUG_FILE=$(SNET_DIR)/snet-debug.c
SNET_PARENT_FILES=$(SNET_DIR)/snet.c $(SNET_DEBUG_FILE)

#
# UTIL
#

run-%: $(BUILD_DIR)/%
	./$<

cscope.files: $(ALL_SOURCE)
	echo $^ > $@

cscope: cscope.files
	cscope -b -q

#
# BUILD
#

clean: clean-cap
	rm -frd ./*.o $(BUILD_DIR) $(SDL_LOG_TEST_FILE)

$(BUILD_DIR):
	mkdir $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<

#
# TEST
#

test: run-snet-test run-phy-test run-mac-test run-log-test

#
# PHY
#

PHY_TEST_FILES=          \
  $(SNET_PARENT_FILES)   \
  $(TEST_DIR)/phy-test.c \
  $(MAC_FILES)           \
  $(SNET_DEBUG_FILE)
PHY_TEST_EXES=               \
  $(BUILD_DIR)/phy-test      \
  $(BUILD_DIR)/client/client \
  $(BUILD_DIR)/server/server

$(BUILD_DIR)/phy-test: $(addprefix $(BUILD_DIR)/,$(notdir $(PHY_TEST_FILES:.c=.o)))
	$(CC) $(LDFLAGS) -o $@ $^

run-phy-test: $(PHY_TEST_EXES)
	./$<

#
# MAC
#

MAC_TEST_FILES=$(TEST_DIR)/mac-test.c $(MAC_FILES)
$(BUILD_DIR)/mac-test: $(addprefix $(BUILD_DIR)/,$(notdir $(MAC_TEST_FILES:.c=.o)))
	$(CC) $(LDFLAGS) -o $@ $^

#
# SDL
#

SDL_LOG_TEST_FILE=tuna.sdl

# So that we can compile in logging for this test.
$(BUILD_DIR)/sdl-log-on.o: $(CAP_DIR)/sdl-log.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<

LOG_TEST_OBJ=$(addprefix $(BUILD_DIR)/, phy.o log-test.o sdl-log-on.o snet-debug.o)

$(BUILD_DIR)/log-test:                                 \
    DEFINES += -DSNET_TEST                             \
               -DSDL_LOG                               \
               -DSDL_LOG_FILE=\"$(SDL_LOG_TEST_FILE)\" \
               -DSDL_LOG_TEST
$(BUILD_DIR)/log-test: $(LOG_TEST_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

#
# SNET
#

SNET_TEST_EXES=              \
  $(BUILD_DIR)/snet-test     \
  $(BUILD_DIR)/server/server

$(SNET_TEST_EXES): DEFINES += -DSNET_TEST

SNET_TEST_FILES=$(SNET_PARENT_FILES) $(TEST_DIR)/snet-test.c $(MAC_FILES)
SNET_TEST_OBJ=$(addprefix $(BUILD_DIR)/,$(notdir $(SNET_TEST_FILES:.c=.o)))
$(BUILD_DIR)/snet-test: $(SNET_TEST_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

run-snet-test: $(SNET_TEST_EXES)
	./$<

#
# TEST APPS
#

$(BUILD_DIR)/client/client $(BUILD_DIR)/server/server: \
    DEFINES += -DSNET_TEST -DSDL_LOG -DSDL_LOG_FILE=\"$(SDL_LOG_TEST_FILE)\"

SERVER_DIR=$(BUILD_DIR)/server
$(SERVER_DIR): | $(BUILD_DIR)
	mkdir $@
$(BUILD_DIR)/server/%.o: %.c | $(SERVER_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<
SERVER_FILES=$(SNET_CHILD_FILES) $(TEST_APPS_DIR)/server.c $(SNET_DEBUG_FILE)
SERVER_OBJ=$(addprefix $(SERVER_DIR)/,$(notdir $(SERVER_FILES:.c=.o)))
$(BUILD_DIR)/server/server: $(SERVER_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^
run-server: $(BUILD_DIR)/server/server
	./$< $(ARGS)

CLIENT_DIR=$(BUILD_DIR)/client
$(CLIENT_DIR): | $(BUILD_DIR)
	mkdir $@
$(BUILD_DIR)/client/%.o: %.c | $(CLIENT_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<
CLIENT_FILES=$(SNET_CHILD_FILES) $(TEST_APPS_DIR)/client.c $(SNET_DEBUG_FILE)
CLIENT_OBJ=$(addprefix $(CLIENT_DIR)/,$(notdir $(CLIENT_FILES:.c=.o)))
$(BUILD_DIR)/client/client: $(CLIENT_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^
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
