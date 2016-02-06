#
# TEST
#

all: test

#
# VARS
#

SHELL=sh

TEST_DIR=test
APP_DIR=app
SNET_DIR=src/snet

SDL_TEST_LOG_FILE=sdl-test.sdl

INCLUDES=-I.
DEFINES=-DSDL_TEST -DSDL_LOG -DSDL_LOG_FILE=\"$(SDL_TEST_LOG_FILE)\"

CC=gcc
LD=gcc
CFLAGS=-g -O0 -Wall -Werror -MD $(INCLUDES) $(DEFINES)
LDFLAGS=-lmcgoo

BUILD_DIR=build
BUILD_DIR_CREATED=$(BUILD_DIR)/tuna

VPATH += $(APP_DIR) $(TEST_DIR)

#
# SOURCE
#

SNET_ROOT_DIR=$(SNET_DIR)
-include $(SNET_DIR)/snet.mak

SDL_ROOT_DIR=.
-include sdl.mak
INCLUDES += $(SDL_INCLUDES)

#
# UTIL
#

COMPILE=$(CC) $(CFLAGS) -o $@ -c $<
LINK=$(LD) $(LDFLAGS) -o $@ $^

cscope.files:
	echo $(shell find . -name "*.[ch]") > $@

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

$(BUILD_DIR_CREATED):
	mkdir $(@D)
	touch $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR_CREATED)
	$(COMPILE)

#
# TEST
#

TESTS=phy mac plat sensor-sink

.PHONY: snet-test
snet-test:
	make -C $(SNET_DIR) test

.PHONY: test
test: snet-test $(patsubst %, run-%-test, $(TESTS)) zombie-test

zombie-test:
	$(SNET_DIR)/script/zombies.sh -a spin -d
	$(SNET_DIR)/script/zombies.sh -a sensor -d
	$(SNET_DIR)/script/zombies.sh -a sink -d

#
# PHY
#

PHY_TEST_FILES=$(TEST_DIR)/phy-test.c $(SDL_PLAT_FILES) $(SDL_PHY_FILES) $(SDL_CAP_FILES)

$(BUILD_DIR)/phy-test: $(addprefix $(BUILD_DIR)/,$(notdir $(PHY_TEST_FILES:.c=.o)))
	$(LINK)

.PHONY: run-phy-test
run-phy-test: $(BUILD_DIR)/phy-test clean-sdl-test-log-file
	./$<

#
# MAC
#

MAC_TEST_FILES=$(TEST_DIR)/mac-test.c $(SDL_MAC_FILES)

$(BUILD_DIR)/mac-test: $(addprefix $(BUILD_DIR)/,$(notdir $(MAC_TEST_FILES:.c=.o)))
	$(LINK)

.PHONY: run-mac-test
run-mac-test: $(BUILD_DIR)/mac-test
	./$<

#
# PLAT
#

PLAT_TEST_FILES=$(TEST_DIR)/plat-test.c $(SDL_PLAT_FILES)

$(BUILD_DIR)/plat-test: $(addprefix $(BUILD_DIR)/,$(notdir $(PLAT_TEST_FILES:.c=.o)))
	$(LINK)

.PHONY: run-plat-test
run-plat-test: $(BUILD_DIR)/plat-test
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

CAP_SRC_DIR=$(SDL_CAP_DIR)/src
CAP_BIN_DIR=$(SDL_CAP_DIR)/bin

CAP_SRC_FILES=$(CAP_SRC_DIR)/decode/*.java $(CAP_SRC_DIR)/grammar/*.java

DECODER_JAR=$(CAP_BIN_DIR)/Decoder.jar

.PHONY: grammar
grammar:
	cd $(SDL_CAP_DIR) && java -jar $(ANTLR_JAR) -o src/grammar SdlLog.g4

$(CAP_BIN_DIR):
	mkdir $@

.PHONY: $(SDL_CAP_DIR)
$(SDL_CAP_DIR): grammar $(CAP_BIN_DIR)
	javac -cp $(ANTLR_JAR):$(SNAKE_YAML_JAR) -d $(CAP_BIN_DIR) $(CAP_SRC_FILES)

$(DECODER_JAR): $(SDL_CAP_DIR)
	cd $(@D) && jar cfm $(@F) ../manifest.txt ../src/decode/ipv6.yaml *.class

decoder: $(DECODER_JAR)
	java -jar $< $(ARGS)

clean-cap:
	rm -rf $(CAP_BIN_DIR) $(CAP_SRC_DIR)/grammar
