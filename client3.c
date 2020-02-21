/**
 * This is Client 3. It is a valid client and does the following in order:
 * Sends HELLO
 * Confirms it got a HELLO_ACK
 * Confirms it got a CLIENT_LIST
 * Sends CHAT to a predefined client id (see Client 3).
 * EXITs.
 *
 * Takes in the server's port as an arg eventually.
 *
 * To use:
 * ./client2 9010
 *
 * To build:
 * gcc -g -o client3 client3.c
 */

#include "helpers.c"

#define CLIENT_ID "Client3"
#define DESTINATION_ID "Client2"
#define MESSAGE_ID 112  // Arbitrary

int main() {
  // Create socket
  int socket_file_descriptor = connect_to_server();

  // Start with hello.
  struct Message hello_message = get_hello_message(CLIENT_ID);
  DEBUG_PRINT("Writing hello.\n");
  int message_byte_size = write_message(socket_file_descriptor, &hello_message);

  // Read from server. We should get a HELLO_ACK.
  struct Message hello_ack_message;
  read_message(socket_file_descriptor, &hello_ack_message);
  print_message(&hello_ack_message);

  // Read from server. We should get a CLIENT_LIST.
  struct Message client_list_message;
  read_message(socket_file_descriptor, &client_list_message);
  print_message(&client_list_message);

  // Let's chat it up. Send a message to client3.
  char * message_data = "Hey Client3 how you doing";
  struct Message chat_message =
      get_chat_message(CLIENT_ID, DESTINATION_ID, message_data, MESSAGE_ID);
  DEBUG_PRINT("Writing CHAT.\n");
  message_byte_size = write_message(socket_file_descriptor, &chat_message);

  // Send EXIT because we're done.
  struct Message exit_message = get_exit_message(CLIENT_ID, SERVER_ID);
  DEBUG_PRINT("Writing exit.\n");
  message_byte_size = write_message(socket_file_descriptor, &exit_message);

  // Close the active socket.
  close(socket_file_descriptor);

  return 0;
}
