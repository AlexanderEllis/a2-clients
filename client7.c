/**
 * This is Client 7. It is a valid client and does the following in order:
 * Sends HELLO
 * Confirms it got a HELLO_ACK
 * Confirms it got a CLIENT_LIST
 * Sends CHAT to a predefined client id (see Client 3).
 * Confirms it got an ERROR(CANNOT_DELIVER) message
 * Exits
 *
 * I use this to test the CANNOT_DELIVER message, since this client is sending
 * to a client that is not connected.
 * (See README for more details on other clients)
 *
 * Takes in the server's port as an arg eventually.
 *
 * To use:
 * ./client7 9010
 *
 * To build:
 * gcc -g -o client7 client7.c
 */

#include "helpers.c"

#define CLIENT_ID "Client7"
#define DESTINATION_ID "ASDFASDFASDF"
#define TEST_MESSAGE_ID 112
// Message max size is 2 + 20 + 20 + 4 + 4 + 400
#define MESSAGE_MAX_SIZE 450

int main(int argc, char* argv[]) {
  int server_port;
  sscanf(argv[1], "%d", &server_port);
  // Create socket
  int socket_file_descriptor = connect_to_server(server_port);
  int message_byte_size;

  // Start with hello.
  struct Message hello_message = get_hello_message(CLIENT_ID);
  DEBUG_PRINT("Writing hello.\n");
  message_byte_size = write_message(socket_file_descriptor, &hello_message);

  // Read from server. We should get a HELLO_ACK and CLIENT_LIST.
  struct Message hello_ack_message;
  int bytes_read = read_message(socket_file_descriptor, &hello_ack_message);
  if (bytes_read == 50) {
    // We read hello ack and now have to read client list
    struct Message client_list_message;
    bytes_read = read_message(socket_file_descriptor, &client_list_message);
  }

  // Let's chat it up, except we're sending to a junk destination.
  char * message_data = "Hey ASDFASDFASDF how you doing";
  struct Message chat_message =
      get_chat_message(CLIENT_ID,
                       DESTINATION_ID,
                       message_data,
                       TEST_MESSAGE_ID);
  DEBUG_PRINT("Writing CHAT.\n");
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
