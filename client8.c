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

#include "helpers.c"

#define CLIENT_ID "Client8"
#define DESTINATION_ID "Client8" // Same as previous line
#define TEST_MESSAGE_ID 112

int main() {
  // Gotta set the sigpipe ignore setting so we can get -1 back from writes.
  // This will allow us to monitor when writes fail.
  signal(SIGPIPE, SIG_IGN);
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
  DEBUG_PRINT("Reading hello ack.\n");
  read_message(socket_file_descriptor, &hello_ack_message);
  print_message(&hello_ack_message);
  // Read from server. We should get a CLIENT_LIST.
  struct Message client_list_message;
  DEBUG_PRINT("Reading client_list.\n");
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
  DEBUG_PRINT("Writing chat to myself.\n");
  message_byte_size = write_message(socket_file_descriptor, &chat_message);

  // Read from server. Server will have disconnected socket and removed from
  // lists (see assignment handout).
  struct Message message;
  DEBUG_PRINT("Attempting to read error, but we'll get 0 bytes read.\n");
  int bytes_read = read_message(socket_file_descriptor, &message);
  assert(bytes_read == 0);

  // Loop until we kill program to ensure we're not closing this client's
  // connection.
  while(1) {
  }

  // Close the active socket.
  close(socket_file_descriptor);

  return 0;
}
