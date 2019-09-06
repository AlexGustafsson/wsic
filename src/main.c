#include "logging/logging.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT 8080

int main() {
  printf("Hello World\n");
  printf("Small test too see if i can get sockets upp and listening\n");

  int initialSocket, newSocket;
  socklen_t addressLength = 0;
  char buffer[1024] = {0};
  struct sockaddr_in hostAddress;

  // Creats a socket for us to use
  // socket(ipv4/ipv6, TCP/UDP, protocol)
  initialSocket = socket(AF_INET, SOCK_STREAM, 0);
  // Test to see if socket is created
  if (initialSocket < 0) {
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
  hostAddress.sin_port = htons(PORT);

  // Bind socket to PORT
  if (bind(initialSocket, (struct sockaddr *)&hostAddress,
           sizeof(hostAddress)) < 0) {
    printf("ERROR in binding socket\n");
  } else {
    printf("Successfully bound socket\n");
  }

  // Listen to the socket
  if (listen(initialSocket, 10) < 0) {
    printf("ERROR its quiet\n");
  } else {
    printf("Successfully listened\n");
  }

  // Waiting for the client to send ous a request
  addressLength = sizeof(hostAddress);
  newSocket =
      accept(initialSocket, (struct sockaddr *)&hostAddress, &addressLength);
  if (newSocket < 0) {
    printf("ERROR in waiting\n");
  } else {
    printf("Successfully waited\n");
  }

  // Recieving messages from client
  if (recv(newSocket, buffer, sizeof(buffer), 0) < 0) {
    printf("ERROR did not resieve message\n");
  } else {
    printf("Successfully resieved message\n");
    printf("Message from client: %s \n", buffer);
  }

  // Show information about the client
  printf("Request from %s : %i \n", inet_ntoa(hostAddress.sin_addr),
         ntohs(hostAddress.sin_port));

  // Send respons to client
  fgets(buffer, 1024, stdin);
  if (send(newSocket, buffer, strlen(buffer) + 1, 0) < 0) {
    printf("ERROR did not send message\n");
  } else {
    printf("Successfully sent message\n");
  }

  // Close the socket
  close(newSocket);
  close(initialSocket);

  return 0;
}
