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

$(BUILD_DIR):
	mkdir $@

$(BUILD_DIR)/%.o: %.c $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<

#
# TEST
#

test: run-mac-test run-log-test run-snet-test

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

# So that we can compile in logging for this test.
$(BUILD_DIR)/sdl-log-on.o: $(CAP_DIR)/sdl-log.c $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<

LOG_TEST_OBJ=$(addprefix $(BUILD_DIR)/, phy.o log-test.o sdl-log-on.o)

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

SNET_PARENT_FILES=$(SNET_DIR)/snet.c
SNET_CHILD_FILES=$(SDL_FILES)

SNET_TEST_EXES=              \
  $(BUILD_DIR)/snet-test     \
  $(BUILD_DIR)/client/client \
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
    DEFINES += -DSDL_LOG -DSDL_LOG_FILE=\"$(SDL_LOG_TEST_FILE)\"

SERVER_DIR=$(BUILD_DIR)/server
$(SERVER_DIR): $(BUILD_DIR) # FIXME:
	if [ ! -d $(SERVER_DIR) ]; then mkdir $@; fi
$(BUILD_DIR)/server/%.o: %.c $(SERVER_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<
SERVER_FILES=$(SNET_CHILD_FILES) $(TEST_APPS_DIR)/server.c
SERVER_OBJ=$(addprefix $(SERVER_DIR)/,$(notdir $(SERVER_FILES:.c=.o)))
$(BUILD_DIR)/server/server: $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^
run-server: $(BUILD_DIR)/server/server
	./$< $(ARGS)

CLIENT_DIR=$(BUILD_DIR)/client
$(CLIENT_DIR): $(BUILD_DIR) # FIXME:
	if [ ! -d $(CLIENT_DIR) ]; then mkdir $@; fi
$(BUILD_DIR)/client/%.o: %.c $(CLIENT_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<
CLIENT_FILES=$(SNET_CHILD_FILES) $(TEST_APPS_DIR)/client.c
CLIENT_OBJ=$(addprefix $(CLIENT_DIR)/,$(notdir $(CLIENT_FILES:.c=.o)))
$(BUILD_DIR)/client/client: $(CLIENT_OBJ)
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
