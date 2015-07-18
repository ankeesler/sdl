#
# TEST
#

all: test

#
# VARS
#

SHELL=sh

INC_DIR=inc
SRC_DIR=src

SNET_DIR=$(SRC_DIR)/snet
PHY_DIR=$(SRC_DIR)/phy
MAC_DIR=$(SRC_DIR)/mac
TEST_APPS_DIR=app
TEST_DIR=test
CAP_DIR=cap

INCLUDES=-I. -I$(INC_DIR) -I$(PHY_DIR) -I$(MAC_DIR) -I$(SNET_DIR)
DEFINES=

CC=gcc
CFLAGS=-g -O0 -Wall -Werror $(INCLUDES) $(DEFINES)
LDFLAGS=-lmcgoo
SHELL=sh

BUILD_DIR=build
BUILD_DIR_CREATED=$(BUILD_DIR)/tuna

VPATH=$(SNET_DIR) $(PHY_DIR) $(MAC_DIR) $(TEST_APPS_DIR) $(TEST_DIR) $(CAP_DIR)

SDL_LOG_TEST_FILE=tuna.sdl

#
# SOURCE
#

SDL_LOG_FILES=$(CAP_DIR)/sdl-log.c

MAC_FILES=$(MAC_DIR)/mac.c $(MAC_DIR)/mac-util.c

PHY_FILES=$(PHY_DIR)/phy.c $(PHY_DIR)/uart.c

SDL_FILES=$(PHY_FILES) $(MAC_FILES) $(SDL_LOG_FILES)

SNET_CHILD_FILES=$(SDL_FILES)
SNET_DEBUG_FILE=$(SNET_DIR)/snet-debug.c
SNET_PARENT_FILES=$(SNET_DIR)/snet.c $(SNET_DEBUG_FILE)

ALL_SOURCE=$(shell find . -name "*.[ch]") $(shell find . -name "*.java")

#
# UTIL
#

COMPILE=$(CC) $(CFLAGS) -o $@ -c $<
LINK=$(CC) $(LDFLAGS) -o $@ $^

cscope.files: $(ALL_SOURCE)
	echo $^ > $@

cscope: cscope.files
	rm cscope*out
	cscope -b -q

#
# BUILD
#

clean: clean-cap
	rm -frd ./*.o $(BUILD_DIR) $(SDL_LOG_TEST_FILE) .child*

$(BUILD_DIR_CREATED):
	mkdir $(@D)
	touch $@

#
# TEST
#

.PHONY: test
test: run-snet-test run-phy-test run-mac-test

#
# PHY
#

PHY_TEST_FILES=          \
  $(TEST_DIR)/phy-test.c \
  $(PHY_FILES)           \
  $(SNET_DEBUG_FILE)     \
  $(SDL_LOG_FILES)

PHY_TEST_IN=phy-test.in
STDIN  = 0
STDERR = 2

PHY_TEST_DIR=$(BUILD_DIR)/phy-test-dir
PHY_TEST_DIR_CREATED=$(PHY_TEST_DIR)/tuna
$(PHY_TEST_DIR_CREATED): $(BUILD_DIR_CREATED)
	mkdir $(@D)
	touch $@

$(PHY_TEST_DIR)/%.o: %.c | $(PHY_TEST_DIR_CREATED)
	$(COMPILE)

$(PHY_TEST_IN):
	touch $@

$(PHY_TEST_DIR)/phy-test:                              \
    DEFINES += -DSNET_TEST                             \
               -DPHY_TEST                              \
               -DSDL_LOG                               \
               -DSDL_LOG_FILE=\"$(SDL_LOG_TEST_FILE)\"

$(PHY_TEST_DIR)/phy-test: $(addprefix $(PHY_TEST_DIR)/,$(notdir $(PHY_TEST_FILES:.c=.o)))
	$(LINK)

.PHONY: run-phy-test
run-phy-test: $(PHY_TEST_DIR)/phy-test $(PHY_TEST_IN)
	./$< phy-test $(STDIN) $(STDERR) < $(PHY_TEST_IN) 2>/dev/null

#
# MAC
#

MAC_TEST_FILES=$(TEST_DIR)/mac-test.c $(MAC_FILES)

MAC_TEST_DIR=$(BUILD_DIR)/mac-test-dir
MAC_TEST_DIR_CREATED=$(MAC_TEST_DIR)/tuna
$(MAC_TEST_DIR_CREATED): $(BUILD_DIR_CREATED)
	mkdir $(@D)
	touch $@

$(MAC_TEST_DIR)/%.o: %.c | $(MAC_TEST_DIR_CREATED)
	$(COMPILE)

$(MAC_TEST_DIR)/mac-test: $(addprefix $(MAC_TEST_DIR)/,$(notdir $(MAC_TEST_FILES:.c=.o)))
	$(LINK)

.PHONY: run-mac-test
run-mac-test: $(MAC_TEST_DIR)/mac-test
	./$<

#
# SDL
#

#
# SNET
#

SNET_TEST_FILES=$(SNET_PARENT_FILES) $(TEST_DIR)/snet-test.c $(MAC_FILES)

SNET_TEST_DIR=$(BUILD_DIR)/snet-test-dir
SNET_TEST_DIR_CREATED=$(SNET_TEST_DIR)/tuna
$(SNET_TEST_DIR_CREATED): $(BUILD_DIR_CREATED)
	mkdir $(@D)
	touch $@

$(SNET_TEST_DIR)/%.o: DEFINES += -DSNET_TEST
$(SNET_TEST_DIR)/%.o: %.c | $(SNET_TEST_DIR_CREATED)
	$(COMPILE)

SNET_TEST_EXES=               \
  $(SNET_TEST_DIR)/snet-test  \
  $(BUILD_DIR)/server/server

$(SNET_TEST_DIR)/snet-test: $(addprefix $(SNET_TEST_DIR)/,$(notdir $(SNET_TEST_FILES:.c=.o)))
	$(LINK)

.PHONY: run-snet-test
run-snet-test: $(SNET_TEST_EXES)
	./$<

#
# TEST APPS
#

ALL_TEST_APPS:                 \
    $(BUILD_DIR)/client/client \
    $(SERVER_DIR)/server

$(ALL_TEST_APPS):                                       \
    DEFINES += -DSNET_TEST                              \
               -DSDL_LOG                                \
               -DSDL_LOG_FILE=\"$(SDL_LOG_TEST_FILE)\"

SERVER_DIR=$(BUILD_DIR)/server
SERVER_DIR_CREATED=$(SERVER_DIR)/tuna
$(SERVER_DIR_CREATED): $(BUILD_DIR_CREATED)
	mkdir $(@D)
	touch $@
$(SERVER_DIR)/%.o: %.c | $(SERVER_DIR_CREATED)
	$(COMPILE)
SERVER_FILES=$(SNET_CHILD_FILES) $(TEST_APPS_DIR)/server.c $(SNET_DEBUG_FILE)
SERVER_OBJ=$(addprefix $(SERVER_DIR)/,$(notdir $(SERVER_FILES:.c=.o)))
$(SERVER_DIR)/server: $(addprefix $(SERVER_DIR)/,$(notdir $(SERVER_FILES:.c=.o)))
	$(LINK)

CLIENT_DIR=$(BUILD_DIR)/client
CLIENT_DIR_CREATED=$(CLIENT_DIR)/tuna
$(CLIENT_DIR_CREATED): $(BUILD_DIR_CREATED)
	mkdir $(@D)
	touch $@
$(CLIENT_DIR)/%.o: %.c | $(CLIENT_DIR_CREATED)
	$(COMPILE)
CLIENT_FILES=$(SNET_CHILD_FILES) $(TEST_APPS_DIR)/client.c $(SNET_DEBUG_FILE)
CLIENT_OBJ=$(addprefix $(CLIENT_DIR)/,$(notdir $(CLIENT_FILES:.c=.o)))
$(CLIENT_DIR)/client: $(addprefix $(CLIENT_DIR)/,$(notdir $(CLIENT_FILES:.c=.o)))
	$(LINK)

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
