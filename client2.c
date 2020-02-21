/**
 * This is Client 2. It is a valid client and does the following in order:
 * Sends HELLO
 * Confirms it got a HELLO_ACK
 * Confirms it got a CLIENT_LIST
 * Waits for a chat
 *
 * (See README for more details on other clients)
 *
 * Takes in the server's port as an arg eventually.
 *
 * To use:
 * ./client2 9010
 *
 * To build:
 * gcc -g -o client2 client2.c
 */

#include "helpers.c"

#define CLIENT_ID "Client2"

int main(int argc, char* argv[]) {
  int server_port;
  sscanf(argv[1], "%d", &server_port);
  // Create socket
  int socket_file_descriptor = connect_to_server(server_port);

  // Start with hello.
  struct Message hello_message = get_hello_message(CLIENT_ID);
  DEBUG_PRINT("Writing hello.\n");
  // Write to server.
  int message_byte_size = write_message(socket_file_descriptor, &hello_message);

  // Read from server. We should get a HELLO_ACK.
  struct Message hello_ack_message;
  read_message(socket_file_descriptor, &hello_ack_message);
  print_message(&hello_ack_message);

  // Read from server. We should get a CLIENT_LIST.
  struct Message client_list_message;
  read_message(socket_file_descriptor, &client_list_message);
  print_message(&client_list_message);

  // Loop forever
  while(1) {
    // Read from server. We should get a CHAT.
    struct Message chat_message;
    bzero(&chat_message, sizeof(chat_message));
    int bytes_read = read_message(socket_file_descriptor, &chat_message);
    if (bytes_read == 0) {
      close(socket_file_descriptor);
      return 0;
    }
    DEBUG_PRINT("Got a message!\n");
    print_message(&chat_message);
  }

  // Close the active socket.
  close(socket_file_descriptor);

  return 0;
}
