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
TEST_DIR=test
TEST_APPS_DIR=$(TEST_DIR)/app
CAP_DIR=cap

INCLUDES=-I. -I$(INC_DIR) -I$(PHY_DIR) -I$(MAC_DIR) -I$(SNET_DIR)
DEFINES=-DSDL_TEST -DSDL_LOG -DSDL_LOG_FILE=\"sdl-test.sdl\"

CC=gcc
CFLAGS=-g -O0 -Wall -Werror -MD $(INCLUDES) $(DEFINES)
LDFLAGS=-lmcgoo

BUILD_DIR=build
BUILD_DIR_CREATED=$(BUILD_DIR)/tuna

VPATH=$(PHY_DIR) $(MAC_DIR) $(TEST_APPS_DIR) $(TEST_DIR) $(CAP_DIR)

#
# SOURCE
#

SDL_LOG_FILES=$(CAP_DIR)/sdl-log.c

MAC_FILES=$(MAC_DIR)/mac.c $(MAC_DIR)/mac-util.c

PHY_FILES=$(PHY_DIR)/phy.c

SDL_FILES=$(PHY_FILES) $(MAC_FILES) $(SDL_LOG_FILES)

SNET_ROOT_DIR=snet
-include snet/snet.mak

ALL_SOURCE=$(shell find . -name "*.[ch]") $(shell find . -name "*.java")

#
# UTIL
#

COMPILE=$(CC) $(CFLAGS) -o $@ -c $<
LINK=$(CC) $(LDFLAGS) -o $@ $^

cscope.files: $(ALL_SOURCE)
	echo $^ > $@

cscope: clean-cscope cscope.files
	cscope -b -q

clean-cscope:
	rm -f cscope*

#
# BUILD
#

-include $(patsubst %.c, $(BUILD_DIR)/%.d, $(notdir $(shell find $(SRC_DIR) -name "*.c")))

clean: clean-cap clean-cscope
	rm -rf $(BUILD_DIR)
	rm -rf $(shell find . -name "*.sdl")
	rm -rf $(shell find . -name "*.log")

$(BUILD_DIR_CREATED):
	mkdir $(@D)
	touch $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR_CREATED)
	$(COMPILE)

#
# TEST
#

TESTS=phy mac

.PHONY: test
test: $(patsubst %, run-%-test, $(TESTS))

#
# PHY
#

PHY_TEST_FILES=$(TEST_DIR)/phy-test.c $(PHY_FILES) $(SDL_LOG_FILES)

$(BUILD_DIR)/phy-test: $(addprefix $(BUILD_DIR)/,$(notdir $(PHY_TEST_FILES:.c=.o)))
	$(LINK)

.PHONY: run-phy-test
run-phy-test: $(BUILD_DIR)/phy-test
	./$< phy-test

#
# MAC
#

MAC_TEST_FILES=$(TEST_DIR)/mac-test.c $(MAC_FILES)

$(BUILD_DIR)/mac-test: $(addprefix $(BUILD_DIR)/,$(notdir $(MAC_TEST_FILES:.c=.o)))
	$(LINK)

.PHONY: run-mac-test
run-mac-test: $(BUILD_DIR)/mac-test
	./$<

#
# TEST APPS
#

SENSOR_SINK_TEST_FILES=          \
  $(SNET_CHILD_FILES)            \
  $(TEST_DIR)/sensor-sink-test.c

SENSOR_SINK_TEST_DIR=$(BUILD_DIR)/sensor-sink-test-dir
SENSOR_SINK_TEST_DIR_CREATED=$(SENSOR_SINK_TEST_DIR)/tuna
$(SENSOR_SINK_TEST_DIR_CREATED): $(BUILD_DIR_CREATED)
	mkdir $(@D)
	touch $@

$(SENSOR_SINK_TEST_DIR)/%.o: DEFINES += -DSNET_TEST
$(SENSOR_SINK_TEST_DIR)/%.o: %.c | $(SENSOR_SINK_TEST_DIR_CREATED)
	$(COMPILE)

SENSOR_SINK_TEST_EXES=                      \
  $(SENSOR_SINK_TEST_DIR)/sensor-sink-test  \
  $(BUILD_DIR)/sensor/sensor                \
  $(BUILD_DIR)/sink/sink

$(SENSOR_SINK_TEST_DIR)/sensor-sink-test: $(addprefix $(SENSOR_SINK_TEST_DIR)/,$(notdir $(SENSOR_SINK_TEST_FILES:.c=.o)))
	$(LINK)

.PHONY: run-expect-snet-test
run-sensor-sink-test: $(SENSOR_SINK_TEST_EXES)
	./$< $(ARGS)

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

SENSOR_DIR=$(BUILD_DIR)/sensor
SENSOR_DIR_CREATED=$(SENSOR_DIR)/tuna
$(SENSOR_DIR_CREATED): $(BUILD_DIR_CREATED)
	mkdir $(@D)
	touch $@
$(SENSOR_DIR)/%.o: DEFINES += -DSDL_LOG -DSDL_LOG_FILE=\"sensor.sdl\"
$(SENSOR_DIR)/%.o: %.c | $(SENSOR_DIR_CREATED)
	$(COMPILE)
SENSOR_FILES=$(SNET_CHILD_FILES) $(TEST_APPS_DIR)/sensor.c  $(TEST_APPS_DIR)/sensor-sink-common.c
SENSOR_OBJ=$(addprefix $(SENSOR_DIR)/,$(notdir $(SENSOR_FILES:.c=.o)))
$(SENSOR_DIR)/sensor: $(addprefix $(SENSOR_DIR)/,$(notdir $(SENSOR_FILES:.c=.o)))
	$(LINK)

SINK_DIR=$(BUILD_DIR)/sink
SINK_DIR_CREATED=$(SINK_DIR)/tuna
$(SINK_DIR_CREATED): $(BUILD_DIR_CREATED)
	mkdir $(@D)
	touch $@
$(SINK_DIR)/%.o: DEFINES += -DSDL_LOG -DSDL_LOG_FILE=\"sink.sdl\"
$(SINK_DIR)/%.o: %.c | $(SINK_DIR_CREATED)
	$(COMPILE)
SINK_FILES=$(SNET_CHILD_FILES) $(TEST_APPS_DIR)/sink.c $(TEST_APPS_DIR)/sensor-sink-common.c
SINK_OBJ=$(addprefix $(SINK_DIR)/,$(notdir $(SINK_FILES:.c=.o)))
$(SINK_DIR)/sink: $(addprefix $(SINK_DIR)/,$(notdir $(SINK_FILES:.c=.o)))
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
	java -jar $< $(ARGS)

clean-cap:
	rm -f $(CAP_DIR)/*.class $(CAP_DIR)/*.jar $(CAP_DIR)/Decode.jar
