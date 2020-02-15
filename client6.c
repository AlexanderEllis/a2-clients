/**
 * This is Client 6. It is a valid client and does the following in order:
 * Sends HELLO
 * Tries to read a few time and prints messages
 *
 * I use this to test the ERRROR(CLIENT_ALREADY_PRESENT) by running it twice.
 * (See README for more details on other clients)
 *
 * Takes in the server's port as an arg.
 *
 * To use:
 * ./client 9010
 *
 * To build:
 * gcc -g -o client6 client6.c
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
#define CLIENT_ID "Client6"
// Message max size is 2 + 20 + 20 + 4 + 4 + 400
#define MESSAGE_MAX_SIZE 450

// DEBUG macro jacked from https://stackoverflow.com/questions/1941307/debug-print-macro-in-c
// Super helpful because it includes fn name and line number.
#define DEBUG 3
#if defined(DEBUG) && DEBUG > 0
 #define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, \
    __FILE__, __LINE__, __func__, ##args)
#else
 #define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif

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

struct Message get_exit_message() {
  struct Message message;
  message.type = 6;
  bzero(message.source, 20);
  strcpy(message.source, CLIENT_ID);
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
    DEBUG_PRINT("Message written\n");
    DEBUG_PRINT("bytes written: %d\n", message_byte_size);
  }

  return message_byte_size;
}

/**
 * Prints the fields of a message.
 * Note the custom printing for null bytes in the data, where it prints
 * '\0'. I found this was super helpful for visualizing the client list, which
 * ends up looking like 'Client1\0Client2\0'.
 */
void print_message(struct Message * message) {
  DEBUG_PRINT("Printing message information.\n");
  DEBUG_PRINT("type: '%hu'\n", message->type);
  DEBUG_PRINT("source: '%s'\n", message->source);
  DEBUG_PRINT("destination: '%s'\n", message->destination);
  DEBUG_PRINT("length: '%d'\n", message->length);
  DEBUG_PRINT("message_id: '%d'\n", message->message_id);
  DEBUG_PRINT("data: '");
  // Don't use DEBUG_PRINT for the next two because it fucks with the terminal
  // formatting and looks bad. We want it to be continuation from the previous
  // debug print call.
  for (int i = 0; i < message->length; i++) {
    if (message->data[i] == '\0') {
      printf("\\0");
    } else {
      printf("%c", message->data[i]);
    }
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
  DEBUG_PRINT("Writing hello.\n");
  // Write to server.
  message_byte_size = write_message(socket_file_descriptor, &hello_message);
  DEBUG_PRINT("Bytes written: %d\n", message_byte_size);

  // Read from server. We could get a HELLO_ACK or an ERROR.
  char buffer[MESSAGE_MAX_SIZE];
  bzero(buffer, MESSAGE_MAX_SIZE);
  DEBUG_PRINT("Reading first message\n");
  message_byte_size = read(socket_file_descriptor, buffer, MESSAGE_MAX_SIZE);
  DEBUG_PRINT("Bytes read: %d\n", message_byte_size);
  if (message_byte_size < 0) {
    perror("read");
    exit(EXIT_FAILURE);
  }
  struct Message hello_ack_message;
  memcpy(&hello_ack_message, buffer, message_byte_size);
  print_message(&hello_ack_message);

  // Read from server. We should get a CLIENT_LIST if we were first, or nothing
  // if this is the second (in which case this will hang).
  bzero(buffer, MESSAGE_MAX_SIZE);
  DEBUG_PRINT("Reading second message\n");
  message_byte_size = read(socket_file_descriptor, buffer, MESSAGE_MAX_SIZE);
  DEBUG_PRINT("Bytes read: %d\n", message_byte_size);
  if (message_byte_size < 0) {
    perror("read");
    exit(EXIT_FAILURE);
  }
  struct Message client_list_message;
  memcpy(&client_list_message, buffer, message_byte_size);
  print_message(&client_list_message);

  // If this is the first run, we want to just hang and keep the connection.
  DEBUG_PRINT("We're just gonna hang out now\n");
  while(1) {

  }

  // Close the active socket.
  close(socket_file_descriptor);

  return 0;
}
