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
 * ./client1 9010
 *
 * To build:
 * gcc -g -o client1 client1.c
 */

#include "helpers.c"

#define CLIENT_ID "Client1"

int main(int argc, char* argv[]) {
  int server_port;
  sscanf(argv[1], "%d", &server_port);
  // Create socket
  int socket_file_descriptor = connect_to_server(server_port);

  // Start with hello.
  struct Message hello_message = get_hello_message(CLIENT_ID);
  DEBUG_PRINT("Writing hello.\n");
  print_message(&hello_message);
  int message_byte_size = write_message(socket_file_descriptor, &hello_message);

  // Read from server. We should get a HELLO_ACK.
  struct Message hello_ack_message;
  read_message(socket_file_descriptor, &hello_ack_message);
  print_message(&hello_ack_message);

  assert(hello_ack_message.type == 2); // HELLO_ACK
  assert(hello_ack_message.length == 0); // Length should be zero

  // Read from server. We should get a CLIENT_LIST.
  struct Message client_list_message;
  read_message(socket_file_descriptor, &client_list_message);
  print_message(&client_list_message);
  assert(client_list_message.type == 4); // CLIENT_LIST
  assert(client_list_message.length != 0); // Length should be nonzero

  // Send another LIST_REQ because we're needy.
  struct Message list_req_message = get_list_req_message(CLIENT_ID, SERVER_ID);
  DEBUG_PRINT("Writing list req.\n");
  print_message(&list_req_message);
  message_byte_size = write_message(socket_file_descriptor, &list_req_message);

  // Read from server. We should get another CLIENT_LIST.
  read_message(socket_file_descriptor, &client_list_message);
  print_message(&client_list_message);
  assert(client_list_message.type == 4); // CLIENT_LIST

  // Send EXIT because we're done.
  struct Message exit_message = get_exit_message(CLIENT_ID, SERVER_ID);
  DEBUG_PRINT("Writing exit.\n");
  message_byte_size = write_message(socket_file_descriptor, &exit_message);

  // Close the active socket.
  close(socket_file_descriptor);

  return 0;
}
