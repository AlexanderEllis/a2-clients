/**
 * This is Client 9. It is problem client and does the following in order:
 * Sends HELLO
 * Reads twice to get through hello_ack and client list
 * Sends CHAT to a predefined client id (see Client 2) with a bad length.
 * Tries to read and gets no bytes.
 * EXITs.
 *
 * Takes in the server's port as an arg eventually.
 *
 * To use:
 * ./client9 9010
 *
 * To build:
 * gcc -g -o client9 client9.c
 */

#include "helpers.c"

#define CLIENT_ID "Client9"
#define DESTINATION_ID "Client2"
#define MESSAGE_ID 112  // Arbitrary

int main(int argc, char* argv[]) {
  int server_port;
  sscanf(argv[1], "%d", &server_port);
  // Create socket
  int socket_file_descriptor = connect_to_server(server_port);

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

  // Let's chat it up. Send a message to client2.
  char * message_data = "Hey Client2 how you doing";
  struct Message chat_message =
      get_chat_message(CLIENT_ID, DESTINATION_ID, message_data, MESSAGE_ID);
  chat_message.length = 401;
  DEBUG_PRINT("Writing CHAT with bad length.\n");
  message_byte_size = write_message(socket_file_descriptor, &chat_message);

  // Read from server. We should get an error since the server should remove
  // client.
  struct Message error_message;
  int bytes_read = read_message(socket_file_descriptor, &error_message);
  DEBUG_PRINT("Bytes read: %d\n", bytes_read);
  assert(bytes_read <= 0);

  // Close the active socket.
  close(socket_file_descriptor);

  return 0;
}
