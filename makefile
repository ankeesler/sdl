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

SNET_DIR=snet
PLAT_DIR=$(SRC_DIR)/plat
PHY_DIR=$(SRC_DIR)/phy
MAC_DIR=$(SRC_DIR)/mac
TEST_DIR=test
APP_DIR=app
CAP_DIR=cap

SDL_TEST_LOG_FILE=sdl-test.sdl

INCLUDES=-I. -I$(INC_DIR) -I$(SRC_DIR)
DEFINES=-DSDL_TEST -DSDL_LOG -DSDL_LOG_FILE=\"$(SDL_TEST_LOG_FILE)\"

CC=gcc
CFLAGS=-g -O0 -Wall -Werror -MD $(INCLUDES) $(DEFINES)
LDFLAGS=-lmcgoo

BUILD_DIR=build
BUILD_DIR_CREATED=$(BUILD_DIR)/tuna

VPATH=$(PLAT_DIR) $(PHY_DIR) $(MAC_DIR) $(APP_DIR) $(TEST_DIR) $(CAP_DIR)

#
# SOURCE
#

SDL_LOG_FILES=$(CAP_DIR)/sdl-log.c

MAC_FILES=$(MAC_DIR)/mac.c $(MAC_DIR)/mac-util.c

PHY_FILES=$(PHY_DIR)/phy.c

PLAT_FILES=$(PLAT_DIR)/nvic.c $(PLAT_DIR)/led.c $(PLAT_DIR)/assert.c

SDL_FILES=$(PLAT_FILES) $(PHY_FILES) $(MAC_FILES) $(SDL_LOG_FILES)

SNET_ROOT_DIR=$(SNET_DIR)
-include snet/snet.mak
INCLUDES += -I$(SNET_SRC_DIR)

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

clean-sdl-test-log-file:
	rm -f $(SDL_TEST_LOG_FILE)

#
# BUILD
#

-include $(patsubst %.c, $(BUILD_DIR)/%.d, $(notdir $(shell find . -name "*.c")))

clean: clean-cap clean-cscope
	rm -rf $(BUILD_DIR)
	rm -rf $(shell find . -name "*.sdl")
	rm -rf $(shell find . -name "*.snet")
	rm -rf $(shell find . -name "*.log")

$(BUILD_DIR_CREATED):
	mkdir $(@D)
	touch $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR_CREATED)
	$(COMPILE)

#
# TEST
#

TESTS=phy mac sensor-sink

.PHONY: test
test: $(patsubst %, run-%-test, $(TESTS))

#
# PHY
#

PHY_TEST_FILES=$(TEST_DIR)/phy-test.c $(PLAT_FILES) $(PHY_FILES) $(SDL_LOG_FILES)

$(BUILD_DIR)/phy-test: $(addprefix $(BUILD_DIR)/,$(notdir $(PHY_TEST_FILES:.c=.o)))
	$(LINK)

.PHONY: run-phy-test
run-phy-test: $(BUILD_DIR)/phy-test clean-sdl-test-log-file
	./$<

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
# APP
#


SENSOR_DIR=$(BUILD_DIR)/sensor
SENSOR_DIR_CREATED=$(SENSOR_DIR)/tuna
$(SENSOR_DIR_CREATED): $(BUILD_DIR_CREATED)
	mkdir $(@D)
	touch $@
SENSOR_DEFINES=-DSDL_LOG -DSDL_LOG_FILE=\"sensor.sdl\" -DSNET_APP
$(SENSOR_DIR)/%.o: %.c | $(SENSOR_DIR_CREATED)
	$(CC) -g -O0 -Wall -Werror -MD $(INCLUDES) $(SENSOR_DEFINES) -o $@ -c $<
SENSOR_FILES=$(SNET_CHILD_FILES) $(SDL_FILES) $(APP_DIR)/sensor.c  $(APP_DIR)/sensor-sink-common.c
SENSOR_OBJ=$(addprefix $(SENSOR_DIR)/,$(notdir $(SENSOR_FILES:.c=.o)))
$(SENSOR_DIR)/sensor: $(addprefix $(SENSOR_DIR)/,$(notdir $(SENSOR_FILES:.c=.o)))
	$(LINK)

.PHONY: sensor
sensor:
	make $(SENSOR_DIR)/sensor

SINK_DIR=$(BUILD_DIR)/sink
SINK_DIR_CREATED=$(SINK_DIR)/tuna
$(SINK_DIR_CREATED): $(BUILD_DIR_CREATED)
	mkdir $(@D)
	touch $@
SINK_DEFINES=-DSDL_LOG -DSDL_LOG_FILE=\"sink.sdl\" -DSNET_APP
$(SINK_DIR)/%.o: %.c | $(SINK_DIR_CREATED)
	$(CC) -g -O0 -Wall -Werror -MD $(INCLUDES) $(SINK_DEFINES) -o $@ -c $<
SINK_FILES=$(SNET_CHILD_FILES) $(SDL_FILES) $(APP_DIR)/sink.c $(APP_DIR)/sensor-sink-common.c
SINK_OBJ=$(addprefix $(SINK_DIR)/,$(notdir $(SINK_FILES:.c=.o)))
$(SINK_DIR)/sink: $(addprefix $(SINK_DIR)/,$(notdir $(SINK_FILES:.c=.o)))
	$(LINK)

.PHONY: sink
sink:
	make $(SINK_DIR)/sink

SENSOR_SINK_TEST_FILES=$(SNET_PARENT_FILES) $(TEST_DIR)/sensor-sink-test.c

$(BUILD_DIR)/sensor-sink-test: $(addprefix $(BUILD_DIR)/,$(notdir $(SENSOR_SINK_TEST_FILES:.c=.o)))
	$(LINK)

run-sensor-sink-test: $(BUILD_DIR)/sensor-sink-test sensor sink
	./$<

#
# CAPTURE FRAMEWORK
#

ANTLR_JAR=/usr/local/lib/antlr-4.2-complete.jar
SNAKE_YAML_JAR=/usr/local/lib/snakeyaml-1.14.jar

CAP_SRC_DIR=$(CAP_DIR)/src
CAP_BIN_DIR=$(CAP_DIR)/bin

CAP_SRC_FILES=$(shell find $(CAP_SRC_DIR) -name "*.java")

DECODER_JAR=$(CAP_BIN_DIR)/Decoder.jar

.PHONY: grammar
grammar:
	cd $(CAP_SRC_DIR)/grammar && java -jar $(ANTLR_JAR) SdlLog.g4

$(CAP_BIN_DIR):
	mkdir $@

.PHONY: $(CAP_DIR)
$(CAP_DIR): $(CAP_SRC_FILES) | grammar $(CAP_BIN_DIR)
	javac -cp $(ANTLR_JAR):$(SNAKE_YAML_JAR) -d $(CAP_BIN_DIR) $^

$(DECODER_JAR): $(CAP_DIR)
	cd $(@D) && jar cfm $(@F) ../manifest.txt ../src/decode/ipv6.yaml *.class

decoder: $(DECODER_JAR)
	java -jar $< $(ARGS)

clean-cap:
	rm -rf $(CAP_BIN_DIR)
