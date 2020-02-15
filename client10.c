/**
 * This is Client 2. It is a problem client and does the following in order:
 * Sends HELLO
 * Returns without closing or anything. Rude.
 *
 * Takes in the server's port as an arg eventually.
 *
 * To use:
 * ./client2 9010
 *
 * To build:
 * gcc -g -o client10 client10.c
 */

#include "helpers.c"

#define CLIENT_ID "Client10"

int main() {
  // Create socket
  int socket_file_descriptor = connect_to_server();

  // Start with hello.
  struct Message hello_message = get_hello_message(CLIENT_ID);
  DEBUG_PRINT("Writing hello.\n");
  int message_byte_size = write_message(socket_file_descriptor, &hello_message);

  // Return without closing or sending exit.
  return 0;
}
