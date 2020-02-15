/**
 * This is Client 8 is a dingus that does the following:
 * Sends HELLO
 * Confirms it got a HELLO_ACK
 * Confirms it got a CLIENT_LIST
 * Sends CHAT to itself.
 * Confirms it got an ERROR(CANNOT_DELIVER) message
 * Exits
 *
 * I use this to test the CANNOT_DELIVER message when sending a message to
 * itself.
 * (See README for more details on other clients)
 *
 * Takes in the server's port as an arg eventually.
 *
 * To use:
 * ./client8 9010
 *
 * To build:
 * gcc -g -o client8 client8.c
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>

#include"helpers.c"

#define CLIENT_ID "Client8"
#define DESTINATION_ID "Client8" // Same as previous line
#define TEST_MESSAGE_ID 112

int main() {
  // Create socket
  int socket_file_descriptor = connect_to_server();

  int message_byte_size;

  // Start with hello.
  struct Message hello_message = get_hello_message(CLIENT_ID);
  DEBUG_PRINT("Writing hello.\n");
  // Write to server.
  message_byte_size = write_message(socket_file_descriptor, &hello_message);

  // Read from server. We should get a HELLO_ACK.
  struct Message hello_ack_message;
  read_message(socket_file_descriptor, &hello_ack_message);
  print_message(&hello_ack_message);
  // Read from server. We should get a CLIENT_LIST.
  struct Message client_list_message;
  read_message(socket_file_descriptor, &client_list_message);
  print_message(&client_list_message);

  // Let's chat it up.
  char * message_data = "Literally talking to myself";
  struct Message chat_message =
      get_chat_message(CLIENT_ID,
                       DESTINATION_ID,
                       message_data,
                       TEST_MESSAGE_ID);
  // Write to server.
  message_byte_size = write_message(socket_file_descriptor, &chat_message);

  // Read from server. We should get a ERROR(CANNOT_DELIVER).
  struct Message error_message;
  read_message(socket_file_descriptor, &error_message);
  print_message(&error_message);
  assert(strcmp(error_message.destination, CLIENT_ID) == 0);
  assert(strcmp(error_message.source, SERVER_ID) == 0);
  assert(error_message.type == 8);  // Type for ERROR(CANNOT_DELIVER)
  assert(error_message.message_id == TEST_MESSAGE_ID);

  // Loop until we kill program to ensure we're not closing this client's
  // connection.
  while(1) {
  }

  // Close the active socket.
  close(socket_file_descriptor);

  return 0;
}
