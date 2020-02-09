/**
 * This is a test client for the chat app. On start, it will
 * ask for a clientID from the command line, then initiate a
 * HELLO with the server.
 *
 * It takes two command line args: the ClientID to use and the server port.
 *
 * To use:
 * ./client 9010
 *
 * To build:
 * gcc -g -o client client.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>

#define BUFSIZE 1024
#define SERVER_PORT 9000

int main() {
  // Create socket
  int socket_file_descriptor = socket(AF_INET,
                                      SOCK_STREAM,
                                      0);
  if (socket_file_descriptor == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  // Build proxy address
  struct sockaddr_in server_address;
  bzero((char *) &server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;  // This will be internet address
  server_address.sin_addr.s_addr =
      htonl(INADDR_ANY); // Bind socket to available interfaces.
  server_address.sin_port =
      htons((unsigned short) SERVER_PORT);

  // Connect to server
  if (connect(socket_file_descriptor,
              (struct sockaddr *) &server_address,
              sizeof(server_address)) < 0) {
    perror("connect");
    exit(EXIT_FAILURE);
  }

  int message_byte_size;
  // Start with super basic GET message
  char * message =
      "GET /alex/index.html HTTP/1.1\r\n"
      "Host: 127.0.0.1\r\n";

  // Write to proxy
  message_byte_size = write(socket_file_descriptor, message, strlen(message));
  if (message_byte_size < 0) {
    perror("write");
    exit(EXIT_FAILURE);
  }

  // Read from proxy
  char buffer[BUFSIZE];
  bzero(buffer, BUFSIZE);
  message_byte_size = read(socket_file_descriptor, buffer, BUFSIZE);
  if (message_byte_size < 0) {
    perror("read");
    exit(EXIT_FAILURE);
  }
  printf("We got this from the proxy: %s\n", buffer);

  // Close the active socket.
  close(socket_file_descriptor);

  return 0;
}
