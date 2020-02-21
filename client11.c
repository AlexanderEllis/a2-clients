/**
 * This is Client 2. It is a problem client and does the following in order:
 * Sends HELLO
 * Returns without closing or anything. Rude.
 *
 * Takes in the server's port as an arg eventually.
 *
 * To use:
 * ./client11 9010
 *
 * To build:
 * gcc -g -o client11 client11.c
 */

#include "helpers.c"

#define CLIENT_ID "Client11"

int main(int argc, char* argv[]) {
  // Gotta set the sigpipe ignore setting so we can get -1 back from writes.
  // This will allow us to monitor when writes fail.
  signal(SIGPIPE, SIG_IGN);
  int server_port;
  sscanf(argv[1], "%d", &server_port);
  // Create socket
  int socket_file_descriptor = connect_to_server(server_port);

  // Start with hello.
  struct Message hello_message = get_hello_message(CLIENT_ID);
  // Mess up the message fields
  hello_message.length = 11;
  hello_message.message_id = 11;
  DEBUG_PRINT("Writing hello.\n");
  int message_byte_size = write_message(socket_file_descriptor, &hello_message);

  DEBUG_PRINT("Reading response\n");
  // Read from server. We should get a CLIENT_LIST.
  struct Message client_list_message;
  int bytes_read = read_message(socket_file_descriptor, &client_list_message);

  // Socket closed and read should fail.
  assert(bytes_read == 0);

  // Return without closing or sending exit.
  return 0;
}
