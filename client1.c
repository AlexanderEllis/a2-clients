/**
 * This is Client 1. It is a valid client and does the following in order:
 * Sends HELLO
 * Confirms it got a HELLO_ACK
 * Confirms it got a CLIENT_LIST
 * Sends LIST_REQ
 * Confirms it got a CLIENT_LIST
 * Sends EXIT
 *
 * (See README for more details on other clients)
 *
 * Takes in the server's port as an arg.
 *
 * To use:
 * ./client 9010
 *
 * To build:
 * gcc -g -o client1 client1.c
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
#define SERVER_ID "Server"
#define CLIENT_ID "Client1"
// Message max size is 2 + 20 + 20 + 4 + 4 + 400
#define MESSAGE_MAX_SIZE 450

struct __attribute__((__packed__)) Message {
  unsigned short type;
  char source[20];
  char destination[20];
  unsigned int length;
  unsigned int message_id;
  char data[400];
};

struct Message get_hello_message() {
  struct Message message;
  message.type = 1;
  bzero(message.source, 20);
  strcpy(message.source, CLIENT_ID);
  bzero(message.destination, 20);
  strcpy(message.destination, SERVER_ID);
  message.length = 0;
  message.message_id = 0;

  return message;
}

struct Message get_list_req_message() {
  struct Message message;
  message.type = 3;
  bzero(message.source, 20);
  strcpy(message.source, CLIENT_ID);
  bzero(message.destination, 20);
  strcpy(message.destination, SERVER_ID);
  message.length = 0;
  message.message_id = 0;

  return message;
}

struct Message get_exit_message(char * client_id) {
  struct Message message;
  message.type = 6;
  bzero(message.source, 20);
  strcpy(message.source, client_id);
  bzero(message.destination, 20);
  strcpy(message.destination, SERVER_ID);
  message.length = 0;
  message.message_id = 0;

  return message;
}

int write_message(int file_descriptor, struct Message * message) {
  int message_byte_size =
      write(file_descriptor, message, sizeof((*message)));
  if (message_byte_size < 0) {
    perror("write");
    exit(EXIT_FAILURE);
  } else {
    printf("MEssage written\n");
    printf("bytes written: %d\n", message_byte_size);
  }

  return message_byte_size;
}

void print_message(struct Message * message) {
  printf("Printing message information.\n");
  printf("type: '%hu'\n", message->type);
  printf("source: '%s'\n", message->source);
  printf("destination: '%s'\n", message->destination);
  printf("length: '%d'\n", message->length);
  printf("message_id: '%d'\n", message->message_id);
  printf("data: '");
  for (int i = 0; i < message->length; i++) {
    printf("%c", message->data[i]);
  }
  printf("'\n");
}

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

  char client_id[20];
  bzero(client_id, 20);
  strcpy(client_id, "Client1");

  // Start with hello.
  struct Message hello_message = get_hello_message();
  printf("Writing hello.\n");
  // Write to server.
  message_byte_size = write_message(socket_file_descriptor, &hello_message);

  // Read from server. We should get a HELLO_ACK.
  char buffer[MESSAGE_MAX_SIZE];
  bzero(buffer, MESSAGE_MAX_SIZE);
  printf("Gonna try to read\n");
  message_byte_size = read(socket_file_descriptor, buffer, MESSAGE_MAX_SIZE);
  if (message_byte_size < 0) {
    perror("read");
    exit(EXIT_FAILURE);
  }
  struct Message hello_ack_message;
  memcpy(&hello_ack_message, buffer, message_byte_size);
  print_message(&hello_ack_message);

  // Read from server. We should get a CLIENT_LIST.
  bzero(buffer, MESSAGE_MAX_SIZE);
  printf("Gonna try to read\n");
  message_byte_size = read(socket_file_descriptor, buffer, MESSAGE_MAX_SIZE);
  if (message_byte_size < 0) {
    perror("read");
    exit(EXIT_FAILURE);
  }
  struct Message client_list_message;
  memcpy(&client_list_message, buffer, message_byte_size);
  print_message(&client_list_message);

  // TODO: send EXIT

  // Close the active socket.
  close(socket_file_descriptor);

  return 0;
}
