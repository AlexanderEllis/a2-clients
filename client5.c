/**
 * This is Client 5.
 *
 * Sends 1/2 of HELLO headers.
 * Sends second 1/2 of HELLO headers after 1 second.
 *
 * Takes in the server's port as an arg eventually.
 *
 * To use:
 * ./client5 9010
 *
 * To build:
 * gcc -g -o client5 client5.c
 */

#include "helpers.c"

#define CLIENT_ID "Client5"

/**
 * Note different write helper than other clients since we have to write partial
 * messages.
 *
 * Network-order MUST be done outside of this fn because otherwise we'll undo it
 * when we call it again.
 */
int write_partial_message(int file_descriptor,
                          struct Message * ptr_to_message,
                          int start_offset,
                          int size) {
  // Get size to write before converting endianness.
  int message_byte_size =
      write(file_descriptor, ((char * ) ptr_to_message) + start_offset, size);
  if (message_byte_size < 0) {
    perror("write");
    exit(EXIT_FAILURE);
  } else {
    DEBUG_PRINT("Message written\n");
    DEBUG_PRINT("bytes written: %d\n", message_byte_size);
  }

  return message_byte_size;
}

int main() {
  // Gotta set the sigpipe ignore setting so we can get -1 back from writes.
  // This will allow us to monitor when writes fail.
  signal(SIGPIPE, SIG_IGN);

  // Create socket
  int socket_file_descriptor = connect_to_server();

  // Start with hello.
  struct Message hello_message = get_hello_message(CLIENT_ID);
  DEBUG_PRINT("Writing hello.\n");
  convert_message_hton(&hello_message);
  int message_byte_size =
      write_partial_message(socket_file_descriptor, &hello_message, 0, 25);
  DEBUG_PRINT("Bytes written in first write: %d\n", message_byte_size);
  sleep(6);
  message_byte_size =
      write_partial_message(socket_file_descriptor, &hello_message, 25, 25);
  DEBUG_PRINT("Bytes written in second write: %d\n", message_byte_size);

  // Read from server. We should get a CLIENT_LIST.
  struct Message client_list_message;
  int bytes_read = read_message(socket_file_descriptor, &client_list_message);

  // Socket closed and read should fail.
  assert(bytes_read == -1);

  return 0;
}
