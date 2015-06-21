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
BUILD_DIR_CREATED=$(BUILD_DIR)/tuna

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

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR_CREATED)
	$(CC) $(CFLAGS) -o $@ -c $<

#
# TEST
#

.PHONY: test
test: run-snet-test run-phy-test run-mac-test #run-log-test

#
# PHY
#

PHY_TEST_FILES=          \
  $(TEST_DIR)/phy-test.c \
  $(PHY_FILES)           \
  $(SNET_DEBUG_FILE)     \

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

$(PHY_TEST_DIR)/phy-test: DEFINES += -DSNET_TEST -DPHY_TEST
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

LOG_TEST_FILES=              \
    $(PHY_DIR)/phy.c         \
    $(TEST_DIR)/log-test.c   \
    $(CAP_DIR)/sdl-log.c     \
    $(SNET_DIR)/snet-debug.c
SDL_LOG_TEST_FILE=tuna.sdl

LOG_TEST_DIR=$(BUILD_DIR)/log-test-dir
LOG_TEST_DIR_CREATED=$(LOG_TEST_DIR)/tuna
$(LOG_TEST_DIR_CREATED): $(BUILD_DIR_CREATED)
	mkdir $(@D)
	touch $@

$(LOG_TEST_DIR)/%.o:                                   \
    DEFINES += -DSNET_TEST                             \
               -DSDL_LOG                               \
               -DSDL_LOG_FILE=\"$(SDL_LOG_TEST_FILE)\" \
               -DSDL_LOG_TEST
$(LOG_TEST_DIR)/%.o: %.c | $(LOG_TEST_DIR_CREATED)
	$(COMPILE)

$(LOG_TEST_DIR)/log-test: $(addprefix $(LOG_TEST_DIR)/, $(notdir $(LOG_TEST_FILES:.c=.o)))
	$(LINK)

.PHONY: run-log-test
run-log-test: $(LOG_TEST_DIR)/log-test
	./$<

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

$(BUILD_DIR)/client/client $(BUILD_DIR)/server/server: \
    DEFINES += -DSNET_TEST -DSDL_LOG -DSDL_LOG_FILE=\"$(SDL_LOG_TEST_FILE)\"

SERVER_DIR=$(BUILD_DIR)/server
$(SERVER_DIR): | $(BUILD_DIR)
	mkdir $@
$(BUILD_DIR)/server/%.o: %.c | $(SERVER_DIR)
	$(COMPILE)
SERVER_FILES=$(SNET_CHILD_FILES) $(TEST_APPS_DIR)/server.c $(SNET_DEBUG_FILE)
SERVER_OBJ=$(addprefix $(SERVER_DIR)/,$(notdir $(SERVER_FILES:.c=.o)))
$(BUILD_DIR)/server/server: $(SERVER_OBJ)
	$(LINK)

CLIENT_DIR=$(BUILD_DIR)/client
$(CLIENT_DIR): | $(BUILD_DIR)
	mkdir $@
$(BUILD_DIR)/client/%.o: %.c | $(CLIENT_DIR)
	$(COMPILE)
CLIENT_FILES=$(SNET_CHILD_FILES) $(TEST_APPS_DIR)/client.c $(SNET_DEBUG_FILE)
CLIENT_OBJ=$(addprefix $(CLIENT_DIR)/,$(notdir $(CLIENT_FILES:.c=.o)))
$(BUILD_DIR)/client/client: $(CLIENT_OBJ)
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
