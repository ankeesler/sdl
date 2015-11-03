//
// sdl-test.c
//
// Andrew Keesler
//
// May 2, 2015
//
// SDL test.
//

#include <unit-test.h>

#include "mac.h"
#include "phy.h"

#include "mac-internal.h"
#include "phy-internal.h"

#include <unistd.h>  // getpid()
#include <stdbool.h> // bool

static SdlAddress source;
static SdlAddress destination = 0x00000000;

#define dataBufferLength (10)
static uint8_t data[dataBufferLength];

static SdlPacket packet;

// Stub.
static bool transmitCalled = false;
SdlStatus sdlPhyTransmit(uint8_t *data, uint8_t length)
{
  transmitCalled = true;
  return SDL_SUCCESS;
}
#define expectTransmitCalled() (expect(transmitCalled))
#define resetTransmitCalled()  (transmitCalled = false)

int sanityCheck(void)
{
  // We shouldn't be able to check out ID, transmit, or receive
  // if we have not initialized.
  expect(sdlMacAddress(&source)
         == SDL_MAC_UNINITIALIZED);
  expect(sdlMacTransmit(SDL_PACKET_TYPE_DATA, destination, data, dataBufferLength)
         == SDL_MAC_UNINITIALIZED);
  expect(sdlMacReceive(&packet)
         == SDL_MAC_UNINITIALIZED);

  // Initialization should go fine.
  expect(sdlMacInit(getpid())
         == SDL_SUCCESS);

  // Now we can get our address.
  expect(sdlMacAddress(&source)
         == SDL_SUCCESS);
  expect(source == getpid());
  
  // Should be able to transmit.
  expect(sdlMacTransmit(SDL_PACKET_TYPE_DATA, destination, data, dataBufferLength)
         == SDL_SUCCESS);

  // Receiving something right now should return SDL_EMPTY.
  expect(sdlMacReceive(&packet)
         == SDL_MAC_EMPTY);

  return 0;
}

int loopbackTest(void)
{
  expect(sdlMacAddress(&source)
         == SDL_SUCCESS);

  // If we send something to ourself...
  expect(sdlMacTransmit(SDL_PACKET_TYPE_DATA, source, data, dataBufferLength)
         == SDL_SUCCESS);

  // ...then we should be able to receive it...right?
  expect(sdlMacReceive(&packet)
         == SDL_SUCCESS);
  expect(packet.type == SDL_PACKET_TYPE_DATA);
  expect(packet.source == source);
  expect(packet.destination == source);
  expect(!memcmp(packet.data, data, dataBufferLength));
  expect(packet.dataLength == dataBufferLength);

  // After, the SDL should be empty.
  expect(sdlMacReceive(&packet)
         == SDL_MAC_EMPTY);

  return 0;
}

int broadcastTest(void)
{
  uint8_t flatPacket[255];

  // Destination address of SDL_MAC_ADDRESS_BROADCAST.
  flatPacket[8] = 0xFF;
  flatPacket[9] = 0xFF;
  flatPacket[10] = 0xFF;
  flatPacket[11] = 0xFF;
  
  // If we receive something broadcasted...
  phyReceiveIsr(flatPacket, SDL_MAC_PDU_LEN);

  // ...then we should be able to receive it.
  expect(sdlMacReceive(&packet)
         == SDL_SUCCESS);

  // After, the SDL should be empty.
  expect(sdlMacReceive(&packet)
         == SDL_MAC_EMPTY);

  // Destination address of almost SDL_MAC_ADDRESS_BROADCAST.
  flatPacket[8] = 0xF1;
  flatPacket[9] = 0xF2;
  flatPacket[10] = 0xF3;
  flatPacket[11] = 0xF4;
  phyReceiveIsr(flatPacket, SDL_MAC_PDU_LEN);
  expect(sdlMacReceive(&packet)
         == SDL_MAC_EMPTY);

  // If we send something as a total broadcast, we should make sure
  // it both goes out the radio...
  resetTransmitCalled();
  expect(sdlMacTransmit(SDL_PACKET_TYPE_DATA,
                        SDL_MAC_ADDRESS_BROADCAST,
                        data,
                        dataBufferLength)
         == SDL_SUCCESS);
  expectTransmitCalled();
  // ...as well as back and to ourselves.
  expect(sdlMacReceive(&packet)
         == SDL_SUCCESS);

  return 0;
}

int utilitiesTest(void)
{
  uint8_t flatPacket[SDL_MAC_SDU_MAX];

  packet.type = SDL_PACKET_TYPE_DATA;
  packet.sequence = 0xABCD;
  packet.source = 0x01234567;
  packet.destination = 0x89ABCDEF;
  memcpy(packet.data, data, dataBufferLength);
  packet.dataLength = dataBufferLength;

  sdlPacketToFlatBuffer(&packet, flatPacket);

  expect(flatPacket[0] == SDL_PACKET_TYPE_DATA);
  expect(flatPacket[2] == 0xAB);
  expect(flatPacket[3] == 0xCD);

  expect(flatPacket[4] == 0x01);
  expect(flatPacket[5] == 0x23);
  expect(flatPacket[6] == 0x45);
  expect(flatPacket[7] == 0x67);

  expect(flatPacket[8] == 0x89);
  expect(flatPacket[9] == 0xAB);
  expect(flatPacket[10] == 0xCD);
  expect(flatPacket[11] == 0xEF);

  expect(!memcmp(flatPacket + 12, data, dataBufferLength));

  return 0;
}

static int rxOverflowTest(void)
{
  uint8_t i, packet[64], macQueueSize;
  uint32_t value = 0xFFFFFFFF;
  SdlPacket reallyPacket;

  // Destination address of SDL_MAC_ADDRESS_BROADCAST so we will
  // put it in our queue.
  packet[8] = 0xFF;
  packet[9] = 0xFF;
  packet[10] = 0xFF;
  packet[11] = 0xFF;

  // MAC RX Overflow should be 0 at this point.
  expect(sdlCounterValue(SDL_COUNTER_MAC_RX_OVERFLOW, &value));
  expectEquals(value, 0);

  // If we receive more than the mac queue can handle, then we should
  // get a SDL_COUNTER_MAC_RX_OVERFLOW counter increment.
  // Keep this macQueueSize value inline with the same one in mac.c!
  macQueueSize = 5;
  for (i = 0; i < macQueueSize; i ++) {
    expect(sdlCounterValue(SDL_COUNTER_MAC_RX_OVERFLOW, &value));
    expectEquals(value, 0);
    phyReceiveIsr(packet, SDL_MAC_PDU_LEN);
  }
  for (i = 1; i < 5; i ++) {
    expect(sdlCounterValue(SDL_COUNTER_MAC_RX_OVERFLOW, &value));
    expectEquals(value, i);
    phyReceiveIsr(packet, SDL_MAC_PDU_LEN);
  }

  // Clear the counter.
  expect(sdlCounterClear(SDL_COUNTER_MAC_RX_OVERFLOW));
  expect(sdlCounterValue(SDL_COUNTER_MAC_RX_OVERFLOW, &value));
  expectEquals(value, 0);

  // If we take a packet off the queue...
  expectEquals(sdlMacReceive(&reallyPacket), SDL_SUCCESS);
  // ...then we should have room to add one...
  phyReceiveIsr(packet, SDL_MAC_PDU_LEN);
  expect(sdlCounterValue(SDL_COUNTER_MAC_RX_OVERFLOW, &value));
  expectEquals(value, 0);
  // ...but then go back to our overflow stuff...
  phyReceiveIsr(packet, SDL_MAC_PDU_LEN);
  expect(sdlCounterValue(SDL_COUNTER_MAC_RX_OVERFLOW, &value));
  expectEquals(value, 1);

  return 0;
}

int main(void)
{
  announce();

  run(sanityCheck);

  run(loopbackTest);
  run(broadcastTest);

  run(utilitiesTest);

  run(rxOverflowTest);

  return 0;
}

