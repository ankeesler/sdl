##
## sdl.mak
##
## Andrew Keesler
##
## Sunday, December 20, 2015
##
## SDL files and build configurations.
## Meant to be included in a larger makefile for project integration.
##

ifndef SDL_ROOT_DIR
  SDL_ROOT_DIR=.
endif

SDL_INC_DIR=$(SDL_ROOT_DIR)/inc
SDL_SRC_DIR=$(SDL_ROOT_DIR)/src

SDL_CAP_DIR=$(SDL_ROOT_DIR)/cap
SDL_PLAT_DIR=$(SDL_SRC_DIR)/plat
SDL_PHY_DIR=$(SDL_SRC_DIR)/phy
SDL_MAC_DIR=$(SDL_SRC_DIR)/mac

SDL_CAP_FILES=               \
  $(SDL_CAP_DIR)/sdl-log.c
SDL_PLAT_FILES=              \
  $(SDL_PLAT_DIR)/nvic.c     \
  $(SDL_PLAT_DIR)/led.c      \
  $(SDL_PLAT_DIR)/assert.c   \
  $(SDL_PLAT_DIR)/serial.c
SDL_PHY_FILES=               \
  $(SDL_PHY_DIR)/phy.c
SDL_MAC_FILES=               \
  $(SDL_MAC_DIR)/mac.c       \
  $(SDL_MAC_DIR)/mac-util.c

SDL_FILES=                   \
  $(SDL_CAP_FILES)           \
  $(SDL_PLAT_FILES)          \
  $(SDL_PHY_FILES)           \
  $(SDL_MAC_FILES)           \

VPATH +=                     \
  $(SDL_CAP_DIR)             \
  $(SDL_PLAT_DIR)            \
  $(SDL_PHY_DIR)             \
  $(SDL_MAC_DIR)

echo-sdl-files:
	@echo $(SDL_FILES)
