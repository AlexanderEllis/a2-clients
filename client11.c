/**
 * This is Client 11. It is a problem client and does the following in order:
 * Sends HELLO
 * Sends chat as Client 6
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
#define PRETENDING_TO_BE_CLIENT_ID "Client6"
#define DESTINATION_ID "Client2"
#define MESSAGE_ID 27

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
  DEBUG_PRINT("Writing hello.\n");
  int message_byte_size = write_message(socket_file_descriptor, &hello_message);

  DEBUG_PRINT("Reading response\n");
  // Read from server. We should get a HELLO_ACK and CLIENT_LIST.
  struct Message hello_ack_message;
  int bytes_read = read_message(socket_file_descriptor, &hello_ack_message);
  if (bytes_read == 50) {
    // We read hello ack and now have to read client list
    struct Message client_list_message;
    bytes_read = read_message(socket_file_descriptor, &client_list_message);
  }

  // Send a message to client2.
  char * message_data = "Hey Client2 how you doing";
  struct Message chat_message =
      get_chat_message(CLIENT_ID, DESTINATION_ID, message_data, MESSAGE_ID);
  // Pretend to be client3
  bzero(chat_message.source, 20);
  strcpy(chat_message.source, PRETENDING_TO_BE_CLIENT_ID);
  DEBUG_PRINT("Writing CHAT.\n");
  message_byte_size = write_message(socket_file_descriptor, &chat_message);

  // Try to from server. Shouldn't work.
  struct Message error_message;
  bytes_read = read_message(socket_file_descriptor, &error_message);
  // Socket closed and read should fail.
  assert(bytes_read <= 0);

  // Return without closing or sending exit.
  return 0;
}
