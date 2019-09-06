#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../logging/logging.h"

#include "server.h"

static struct sockaddr_in hostAddress;
static int initialSocketId, socketId;
char buffer[1024] = {0};

void serverListen(int port) {

  initialSocketId = socket(AF_INET, SOCK_STREAM, 0);
  // Test to see if socket is created
  if (initialSocketId < 0) {
    printf("ERROR in creating socket\n");
  } else {
    printf("Successfully created socket\n");
  }

  // Host address info
  // ipv4
  hostAddress.sin_family = AF_INET;
  // binder till alla interfaces även localhost
  hostAddress.sin_addr.s_addr = INADDR_ANY;
  // Porten
  hostAddress.sin_port = htons(port);

  // Bind socket to PORT
  if (bind(initialSocketId, (struct sockaddr *)&hostAddress,
           sizeof(hostAddress)) < 0) {
    printf("ERROR in binding socket\n");
  } else {
    printf("Successfully bound socket\n");
  }

  // Listen to the socket
  if (listen(initialSocketId, 10) < 0) {
    printf("ERROR its quiet\n");
  } else {
    printf("Successfully listened\n");
  }
}

void acceptConnection()
{
  socklen_t addressLength = sizeof(hostAddress);;
  // Waiting for the client to send ous a request
  socketId =
      accept(initialSocketId, (struct sockaddr *)&hostAddress, &addressLength);
  if (socketId < 0) {
    printf("ERROR did not connect\n");
  } else {
    printf("Successfully connected to %s : %i \n", inet_ntoa(hostAddress.sin_addr),
           ntohs(hostAddress.sin_port));
  }
}

void request()
{
  int responslength = recv(socketId, buffer, sizeof(buffer), 0);

  if (responslength < 0) {
    printf("ERROR did not resieve request\n");
  } else {
    printf("Successfully resieved request\n");
    printf("Request from client: %s \n", buffer);
  }
}

void respons()
{
  char *header = "HTTP/1.0 200 OK\nDate:\nServer: WSIC\nContent-Type: text/raw; charset=UTF-8\nContent-Length: 12\n\n";
  char *body = "Hello world!\n";
  // Send respons to client
  if (send(socketId, header, strlen(header), 0) < 0) {
    printf("ERROR did not send header\n");
  } else {
    printf("Successfully sent header\n");
  }
  if (send(socketId, body, strlen(body), 0) < 0) {
    printf("ERROR did not send body\n");
  } else {
    printf("Successfully sent body\n");
  }
}

void closeSocket()
{
  close(socketId);
  printf("Closed socket with id: %d\n\n\n", socketId);
}
