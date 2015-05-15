//
// sdl-id.c
//
// Andrew Keesler
//
// Saturday January 17, 2015
//
// SDL interface ID management.

#include "mac.h"

typedef struct {
  int key;
  int id;
} InterfaceIdData;

static InterfaceIdData idDatas[SDL_MAX_HOSTS];
static int idDataIndex = 0;

int sdlInterfaceId(int key)
{
  int i = 0;
  
  if (idDataIndex == SDL_MAX_HOSTS || key < 0)
    return -1;
  
  while (i < idDataIndex) {
    if (idDatas[i].key == key) {
      return idDatas[i].id;
    }
    i ++;
  }
  
  idDatas[idDataIndex] = (InterfaceIdData){key, idDataIndex+1};
  ++ idDataIndex;
  return idDataIndex;
}

// Internal API.

void sdlResetInterfaceIds(void) {
  int i;
  for (i = 0; i < SDL_MAX_HOSTS; i ++)
    idDatas[i].key = -1;
}
