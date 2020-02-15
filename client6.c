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

#include "helpers.c"

#define CLIENT_ID "Client6"

int main() {
  // Create socket
  int socket_file_descriptor = connect_to_server();

  // Start with hello.
  struct Message hello_message = get_hello_message(CLIENT_ID);
  DEBUG_PRINT("Writing hello.\n");
  // Write to server.
  int message_byte_size = write_message(socket_file_descriptor, &hello_message);
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
  convert_message_ntoh(&hello_ack_message);
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
  convert_message_ntoh(&client_list_message);
  print_message(&client_list_message);

  // If this is the first run, we want to just hang and keep the connection.
  DEBUG_PRINT("We're just gonna hang out now\n");
  while(1) {
  }

  // Close the active socket.
  close(socket_file_descriptor);

  return 0;
}
