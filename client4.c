/**
 * This is Client 4. Easily the worst client to handle.
 *
 * Sends 1/2 of HELLO headers.
 * Sends second 1/2 of HELLO headers after 1 second.
 * Confirms it got a HELLO_ACK
 * Confirms it got a CLIENT_LIST
 *
 * Sends 1/2 of CHAT headers
 * Sends the rest of chat headers AND the chat body
 *
 * Sends CHAT headers and 1/2 of body
 * Sends rest of body
 *
 * Sends 1/2 of CHAT headers.
 * Sends other 1/2 of CHAT headers and 1/2 of body.
 * Sends other half of CHAT body
 *
 * Sends CHAT headers
 * Sends CHAT body
 *
 *
 * This is supposed to handle a ton of test cases related to partial reads.
 * If you run client3 before this, you should see all the messages get to
 * client3.
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

#define CLIENT_ID "Client4"
#define DESTINATION_ID "Client3"
#define MESSAGE_ID 112  // Arbitrary

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
  //DEBUG_PRINT("Pointer to message: %p\n", ptr_to_message);
  //DEBUG_PRINT("Preparing to write %d bytes from address %p\n", size, ptr_to_message + (start_offset * sizeof(char)));
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
  // Create socket
  int socket_file_descriptor = connect_to_server();

  // Start with hello.
  struct Message hello_message = get_hello_message(CLIENT_ID);
  DEBUG_PRINT("Writing hello.\n");
  convert_message_hton(&hello_message);
  int message_byte_size =
      write_partial_message(socket_file_descriptor, &hello_message, 0, 25);
  DEBUG_PRINT("Bytes written in first write: %d\n", message_byte_size);
  sleep(2);
  message_byte_size =
      write_partial_message(socket_file_descriptor, &hello_message, 25, 25);
  DEBUG_PRINT("Bytes written in second write: %d\n", message_byte_size);

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


  int message_id = 1;

  // Let's chat it up. Send a message to client3.
  // Start with rough edge case: 1/2 of CHAT headers, then rest and body
  DEBUG_PRINT("\nFirst chat message\n");
  char * message_data = "First chat message";
  struct Message chat_message =
      get_chat_message(CLIENT_ID, DESTINATION_ID, message_data, message_id);
  message_id++;
  int body_length = chat_message.length;
  print_message(&chat_message);
  convert_message_hton(&chat_message);
  message_byte_size =
      write_partial_message(socket_file_descriptor, &chat_message, 0, 25);
  DEBUG_PRINT("Bytes written in first write: %d\n", message_byte_size);
  sleep(2);
  message_byte_size =
      write_partial_message(socket_file_descriptor, &chat_message, 25, 25 + body_length);
  DEBUG_PRINT("Bytes written in second write: %d\n", message_byte_size);

  // Send another message to client3.
  // CHAT headers and part of body, then rest of body
  DEBUG_PRINT("\nSecond chat message\n");
  message_data = "Second chat message";
  chat_message =
      get_chat_message(CLIENT_ID, DESTINATION_ID, message_data, message_id);
  message_id++;
  print_message(&chat_message);
  body_length = chat_message.length;
  convert_message_hton(&chat_message);
  message_byte_size =
      write_partial_message(socket_file_descriptor,
                            &chat_message,
                            0,
                            HEADER_SIZE + 5); // 5 bytes of body
  DEBUG_PRINT("Bytes written in first write: %d\n", message_byte_size);
  sleep(2);
  message_byte_size =
      write_partial_message(socket_file_descriptor,
                            &chat_message,
                            HEADER_SIZE + 5,
                            body_length - 5);
  DEBUG_PRINT("Bytes written in second write: %d\n", message_byte_size);


  // Send yet another message to client3.
  // 1/2 CHAT headers, second 1/2 and part of body, then rest of body
  DEBUG_PRINT("\nThird chat message\n");
  message_data = "Third chat message";
  chat_message =
      get_chat_message(CLIENT_ID, DESTINATION_ID, message_data, message_id);
  message_id++;
  print_message(&chat_message);
  body_length = chat_message.length;
  convert_message_hton(&chat_message);
  message_byte_size =
      write_partial_message(socket_file_descriptor,
                            &chat_message,
                            0,
                            25);
  DEBUG_PRINT("Bytes written in first write: %d\n", message_byte_size);
  sleep(2);
  message_byte_size =
      write_partial_message(socket_file_descriptor,
                            &chat_message,
                            25,
                            25 + 5);
  DEBUG_PRINT("Bytes written in second write: %d\n", message_byte_size);
  sleep(2);
  message_byte_size =
      write_partial_message(socket_file_descriptor,
                            &chat_message,
                            55,
                            body_length - 5);
  DEBUG_PRINT("Bytes written in third write: %d\n", message_byte_size);

  // Last one.
  // CHAT headers, then rest body
  DEBUG_PRINT("\nFourth chat message...\n");
  message_data = "Fourth chat message";
  chat_message =
      get_chat_message(CLIENT_ID, DESTINATION_ID, message_data, message_id);
  print_message(&chat_message);
  body_length = chat_message.length;
  convert_message_hton(&chat_message);
  message_byte_size =
      write_partial_message(socket_file_descriptor,
                            &chat_message,
                            0,
                            HEADER_SIZE);
  DEBUG_PRINT("Bytes written in first write: %d\n", message_byte_size);
  sleep(2);
  message_byte_size =
      write_partial_message(socket_file_descriptor,
                            &chat_message,
                            50,
                            body_length);
  DEBUG_PRINT("Bytes written in second write: %d\n", message_byte_size);

  // Send EXIT because we're done. Note that we don't need network order
  // conversion here because it still uses write_message.
  struct Message exit_message = get_exit_message(CLIENT_ID, SERVER_ID);
  DEBUG_PRINT("Writing exit.\n");

  message_byte_size = write_message(socket_file_descriptor, &exit_message);



  // Close the active socket.
  //close(socket_file_descriptor);

  return 0;
}
