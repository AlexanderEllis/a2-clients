#include <assert.h>
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

/**
 * Connects to server. Returns socket file descriptor for connection.
 */
int connect_to_server() {
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

  return socket_file_descriptor;
}

struct Message get_hello_message(char * client_id) {
  struct Message message;
  message.type = 1;
  bzero(message.source, 20);
  strcpy(message.source, client_id);
  bzero(message.destination, 20);
  strcpy(message.destination, SERVER_ID);
  message.length = 0;
  message.message_id = 0;

  return message;
}

struct Message get_chat_message(char * source, char * destination, char * message_text, int message_id) {
  struct Message message;
  message.type = 5;
  bzero(message.source, 20);
  strcpy(message.source, source);
  bzero(message.destination, 20);
  strcpy(message.destination, destination);
  message.length = strlen(message_text);
  message.message_id = message_id;
  bzero(message.data, 400);
  strcpy(message.data, message_text);

  return message;
}

struct Message get_exit_message(char * source, char * destination) {
  struct Message message;
  message.type = 6;
  bzero(message.source, 20);
  strcpy(message.source, source);
  bzero(message.destination, 20);
  strcpy(message.destination, destination);
  message.length = 0;
  message.message_id = 0;

  return message;
}

/**
 * Prints a buffer in bytes.
 *
 * This is some rainman shit to verify endianness.
 */
void print_binary(char * buffer, int size_to_read, char * what_we_are_printing) {
  DEBUG_PRINT("Printing %s in binary.\n", what_we_are_printing);
  for (int i = 0; i < size_to_read; i++) {
    for (int j = 0; j < 8; j++) {
      printf("%d", !!((buffer[i] << j) & 0x80));
    }
    printf(" ");
  }
  printf("\n");
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

/**
 * Translate to network order (big endian). We have to do this when writing.
 */
void convert_message_hton(struct Message * ptr_to_message) {
  ptr_to_message->type = htons(ptr_to_message->type);  // Unsigned short
  ptr_to_message->length = htonl(ptr_to_message->length);  // Unsigned int
  ptr_to_message->message_id = htonl(ptr_to_message->message_id);  // Unsigned int
}

/**
 * Translate from network order (big endian). Done when reading.
 */
void convert_message_ntoh(struct Message * ptr_to_message) {
  ptr_to_message->type = ntohs(ptr_to_message->type);  // Unsigned short
  ptr_to_message->length = ntohl(ptr_to_message->length);  // Unsigned int
  ptr_to_message->message_id = ntohl(ptr_to_message->message_id);  // Unsigned int
}

int write_message(int file_descriptor, struct Message * ptr_to_message) {
  convert_message_hton(ptr_to_message);
  int message_byte_size =
      write(file_descriptor, ptr_to_message, sizeof((*ptr_to_message)));
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
 * Reads a full message into the message pointed to by ptr_to_message.
 */
int read_message(int file_descriptor, struct Message * ptr_to_message) {
  char buffer[MESSAGE_MAX_SIZE];
  bzero(buffer, MESSAGE_MAX_SIZE);
  DEBUG_PRINT("Gonna try to read\n");
  int message_byte_size = read(file_descriptor, buffer, MESSAGE_MAX_SIZE);
  DEBUG_PRINT("Read %d bytes\n", message_byte_size);
  // Print first two bytes
  DEBUG_PRINT("Read %d bytes\n", message_byte_size);
  if (message_byte_size < 0) {
    perror("read");
    exit(EXIT_FAILURE);
  }
  memcpy(ptr_to_message, buffer, message_byte_size);
  convert_message_ntoh(ptr_to_message);
  return message_byte_size;
}
