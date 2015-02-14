//
// basic.c
//
// Andrew Keesler
//
// December 5, 2014
//
// A basic test for SDL.

#include <unit-test.h>
#include <unistd.h>

#include <pthread.h>

#include "sdl.h"
#include "sdl-log.h"

#define SHORT_PACKET {0x01, 0x02, 0x03, 0x04}
#define SHORT_PACKET_LENGTH 4
#define LONG_PACKET {0,1,2,4,8,16,32,64,128}
#define LONG_PACKET_LENGTH 9

static unsigned char shortPacket[] = SHORT_PACKET;
static unsigned char longPacket[] = LONG_PACKET;
static unsigned char shortBuffer[SHORT_PACKET_LENGTH];
static unsigned char longBuffer[LONG_PACKET_LENGTH];

#define expectSdlEmpty() expect(!sdlActivity())
#define expectSdlNotEmpty() expect(sdlActivity())

int synchronusRawTest(void)
{
  expectSdlEmpty();
  
  // One node sends something.
  expect(!sdlTransmit(shortPacket, SHORT_PACKET_LENGTH));
  expectSdlNotEmpty();

  // The other node receives it.
  expect(!sdlReceive(shortBuffer, SHORT_PACKET_LENGTH));
  expect(!memcmp(shortBuffer, shortPacket, SHORT_PACKET_LENGTH));

  // The other node sends something.
  expect(!sdlTransmit(longPacket, LONG_PACKET_LENGTH));
  expectSdlNotEmpty();

  // The other node receives it.
  expect(!sdlReceive(longBuffer, LONG_PACKET_LENGTH));
  expect(!memcmp(longBuffer, longPacket, LONG_PACKET_LENGTH));

  // One node sends two things.
  expect(!sdlTransmit(shortPacket, SHORT_PACKET_LENGTH));
  expect(!sdlTransmit(longPacket, LONG_PACKET_LENGTH));

  // The other node receives both.
  expect(!sdlReceive(shortBuffer, SHORT_PACKET_LENGTH));
  expect(!memcmp(shortBuffer, shortPacket, SHORT_PACKET_LENGTH));
  expect(!sdlReceive(longBuffer, LONG_PACKET_LENGTH));
  expect(!memcmp(longBuffer, longPacket, LONG_PACKET_LENGTH));

  // One node sends two things.
  expect(!sdlTransmit(longPacket, LONG_PACKET_LENGTH));
  expect(!sdlTransmit(shortPacket, SHORT_PACKET_LENGTH));
  expectSdlNotEmpty();
  
  // The other node receives the first of them.
  expect(!sdlReceive(longBuffer, LONG_PACKET_LENGTH));
  expect(!memcmp(longBuffer, longPacket, LONG_PACKET_LENGTH));

  // Then the first node sends again.
  expect(!sdlTransmit(longPacket, LONG_PACKET_LENGTH));

  // The second node receives the former packet, then the latter.
  expect(!sdlReceive(shortBuffer, SHORT_PACKET_LENGTH));
  expect(!memcmp(shortBuffer, shortPacket, SHORT_PACKET_LENGTH));
  expect(!sdlReceive(longBuffer, LONG_PACKET_LENGTH));
  expect(!memcmp(longBuffer, longPacket, LONG_PACKET_LENGTH));

  // Log should be off.
  expect(!sdlLogOn());

  return 0;
}

static void *node1BasicTask(void *data)
{
  // tx...
  expect(!sdlTransmit(shortBuffer, SHORT_PACKET_LENGTH));
  
  pthread_exit(NULL);
}

static void *node2BasicTask(void *data)
{
  // ...rx.
  while (sdlReceive(shortBuffer, SHORT_PACKET_LENGTH))
    ; // wait
  
  expect(!memcmp(shortBuffer, shortPacket, SHORT_PACKET_LENGTH));

  pthread_exit(NULL);
}

int asynchronusRawBasicTest(void)
{
  // One node, node1, will send another node, node2, 1 packet
  // of length SHORT_PACKET_LENGTH.
  //   node1 ---> node2
  // Pretty simple.

  // Create the nodes. The tx thread should go first so that the
  // rx thread does not steal the mutex before the tx can transmit.
  pthread_t node1Thread, node2Thread;
  pthread_create(&node1Thread, NULL, node1BasicTask, NULL);
  pthread_create(&node2Thread, NULL, node2BasicTask, NULL);

  // Wait for node1 (rx) to finish first, then wait for node2 (tx).
  pthread_join(node1Thread, NULL);
  pthread_join(node2Thread, NULL);
  
  // Network should be empty.
  expectSdlEmpty();

  return 0;
}

#define NODE1_TX_COUNT 5
#define NODE1_TX_INTERVAL_S 1

static void *node1Tx(void *data)
{
  unsigned char i;
  for (i = 1; i <= NODE1_TX_COUNT; i ++) {
    sleep(NODE1_TX_INTERVAL_S);
    (void)(i & 0x01
           ? expect(!sdlTransmit(shortPacket, SHORT_PACKET_LENGTH))
           : expect(!sdlTransmit(longPacket, LONG_PACKET_LENGTH)));
  }

  pthread_exit(NULL);
}

static void *node2Rx(void *data)
{
  int i, shouldReceiveShort = 1;  
  for (i = 0; i < NODE1_TX_COUNT; i ++) {
    if (shouldReceiveShort) {
      while (sdlReceive(shortBuffer, SHORT_PACKET_LENGTH))
        ; // wait
      expect(!memcmp(shortBuffer, shortPacket, SHORT_PACKET_LENGTH));
    } else {
      while (sdlReceive(longBuffer, LONG_PACKET_LENGTH))
        ; // wait
      expect(!memcmp(longBuffer, longPacket, LONG_PACKET_LENGTH));
    }
    shouldReceiveShort = !shouldReceiveShort;
  }

  pthread_exit(NULL);
}

int asynchronusRawOneWayTest(void)
{
  // Two nodes, node1 and node2, are talking to each other.
  //   node1 ---> node2
  //   ... (x NODE1_TX_COUNT-2)
  //   node1 ---> node2
  // node1 sends every NODE1_TX_INTERVAL_S seconds.
  // In total, node1 sends NODE1_TX_COUNT packets.
  // First, node1 will send shortPacket. Then, it will proceed
  // alternating between longPacket and shortPacket.
  
  // Create the nodes. Once again, create tx first so that
  // it can start sending stuff first.
  pthread_t node1Thread, node2Thread;
  pthread_create(&node1Thread, NULL, node1Tx, NULL);
  pthread_create(&node2Thread, NULL, node2Rx, NULL);
  
  // Wait for node1 to finish first, then node2.
  pthread_join(node1Thread, NULL);
  pthread_join(node2Thread, NULL);

  return 0;
}

static void *clientTask(void *data)
{
  // First, we ask the server if he can connect.
  expect(!sdlTransmit(shortPacket, SHORT_PACKET_LENGTH));
  
  // Wait for the server to receive.
  while (sdlActivity()) ;

  // Then, the server will respond.
  while (sdlReceive(longBuffer, LONG_PACKET_LENGTH))
    ; // wait
  expect(!memcmp(longBuffer, longPacket, LONG_PACKET_LENGTH));

  // Finally, we respond to complete the handshake.
  expect(!sdlTransmit(shortPacket, SHORT_PACKET_LENGTH));
  
  pthread_exit(NULL);
}

static void *serverTask(void *data)
{
  // The client will first ask the server if he can connect.
  while (sdlReceive(shortBuffer, SHORT_PACKET_LENGTH))
    ; // wait
  expect(!memcmp(shortBuffer, shortPacket, SHORT_PACKET_LENGTH));

  // Then, the server will respond.
  expect(!sdlTransmit(longBuffer, LONG_PACKET_LENGTH));
  
  // Wait for the client to receive.
  while (sdlActivity()) ;
  
  // Then, the client will respond back for the last time.
  while (sdlReceive(shortBuffer, SHORT_PACKET_LENGTH))
    ; // wait
  expect(!memcmp(shortBuffer, shortPacket, SHORT_PACKET_LENGTH));

  pthread_exit(NULL);
}

int asynchronusRawTwoWayTest(void)
{
  // Two nodes, client and server, are talking to each other.
  //   client ---> server
  //   client <--- server
  //   client ---> server
  // The client wants to establish a stream connection with the server
  // using TCP.

  // Create the nodes.
  pthread_t clientThread, serverThread;
  pthread_create(&serverThread, NULL, serverTask, NULL);
  pthread_create(&clientThread, NULL, clientTask, NULL);
  
  // Wait for client to finish first, then server.
  pthread_join(clientThread, NULL);
  pthread_join(serverThread, NULL);
 
  return 0;
}

int idTest(void)
{
  // Get an id. It should be positive.
  int id1 = sdlInterfaceId(1);
  expect(id1 > 0);
  
  // Should be able to get the same id with the same key.
  expect(sdlInterfaceId(1) == id1);
  
  // Should get a different id with a different key.
  int id2 = sdlInterfaceId(2);
  expect(id1 != id2);
  
  // Should still be able to get both ids with the same keys.
  expect(sdlInterfaceId(1) == id1);
  expect(sdlInterfaceId(2) == id2);
  
  // Should be able to get a new id with a weird number.
  int id3 = sdlInterfaceId(123123123);
  expect(id3 != id1);
  expect(id3 != id2);

  return 0;
}

int deadPacketTest(void)
{
  // Send something.
  unsigned char packet[5];
  expect(!sdlTransmit(packet, 5));
  
  // Wait over the packet lifetime.
  usleep(SDL_PACKET_LIFETIME_US);
  
  // There should be nothing in the network.
  expect(sdlReceive(packet, 5) == SDL_ERROR_NETWORK_EMPTY);
  
  return 0;
}

int main(void)
{
  announce();
  run(synchronusRawTest);
  run(asynchronusRawBasicTest);
  run(asynchronusRawOneWayTest);
  run(asynchronusRawTwoWayTest);
  run(idTest);
  run(deadPacketTest);
  return 0;
}

