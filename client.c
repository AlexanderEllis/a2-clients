/**
 * This is a test client for the chat app. On start, it will
 * ask for a clientID from the command line, then initiate a
 * HELLO with the server.
 *
 * I think I'm gonna have it take a few args. It would be rad if we had
 * a general client we could pass stuff to.
 *
 *
 * Client roles:
 * 1: Client that sends HELLO, reads HELLO_ACK and CLIENT_LIST, sends
 *    CLIENT_LIST_REQ, readds client list, then sends EXIT
 * 2: Client that sends HELLO, sends CHAT, then sends EXIT.
 * 3: Client that sends HELLO, reads message, then sends EXIT.
 *    Can be used in coordination with client 2, if you run this one before 2.
 *    Different client ID than the one before.
 * 4. Client that sends HELLO, 1/2 of CHAT, waits two seconds, then the other
 *    half of CHAT, then EXIT.
 * 5.
 *
 *
 * To use:
 * ./client 9010
 *
 * To build:
 * gcc -g -o client client.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>

#define BUFSIZE 1024
#define SERVER_PORT 9000
#define SERVER_ID "Server"

struct __attribute__((__packed__)) Message {
  unsigned short type;
  char source[20];
  char destination[20];
  unsigned int length;
  unsigned int message_id;
  char data[400];
};

struct Message get_list_message(char * client_id) {
  struct Message message;
  message.type = 3;
  bzero(message.source, 20);
  strcpy(message.source, client_id);
  bzero(message.destination, 20);
  strcpy(message.destination, SERVER_ID);
  message.length = 0;
  message.message_id = 0;

  return message;
}

struct Message get_chat_message(char * client_id, char * destination, char * message_text) {
  struct Message message;
  message.type = 5;
  bzero(message.source, 20);
  strcpy(message.source, client_id);
  bzero(message.destination, 20);
  strcpy(message.destination, destination);
  message.length = 10;
  message.message_id = 0;
  bzero(message.data, 20);
  strcpy(message.data, message_text);


  return message;
}

struct Message get_exit_message(char * client_id) {
  struct Message message;
  message.type = 6;
  bzero(message.source, 20);
  strcpy(message.source, client_id);
  bzero(message.destination, 20);
  strcpy(message.destination, SERVER_ID);
  message.length = 0;
  message.message_id = 0;

  return message;
}

int write_message(int file_descriptor, struct Message * message) {
  int message_byte_size =
      write(file_descriptor, message, sizeof((*message)));
  if (message_byte_size < 0) {
    perror("write");
    exit(EXIT_FAILURE);
  } else {
    printf("MEssage written\n");
    printf("bytes written: %d\n", message_byte_size);
  }

  return message_byte_size;
}

int main() {
  // Create socket
  int socket_file_descriptor = socket(AF_INET,
                                      SOCK_STREAM,
                                      0);
  if (socket_file_descriptor == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  // Build proxy address
  struct sockaddr_in server_address;
  bzero((char *) &server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;  // This will be internet address
  server_address.sin_addr.s_addr =
      htonl(INADDR_ANY); // Bind socket to available interfaces.
  server_address.sin_port =
      htons((unsigned short) SERVER_PORT);

  // Connect to server
  if (connect(socket_file_descriptor,
              (struct sockaddr *) &server_address,
              sizeof(server_address)) < 0) {
    perror("connect");
    exit(EXIT_FAILURE);
  }

  int message_byte_size;

  char client_id[20];
  printf("Enter client id: ");
  scanf("%s", client_id);

  // Start with hello.
  struct Message hello_message;
  hello_message.type = 1;
  bzero(hello_message.source, 20);
  strcpy(hello_message.source, client_id);
  bzero(hello_message.destination, 20);
  strcpy(hello_message.destination, "Server");
  hello_message.length = 0;
  hello_message.message_id = 0;

  printf("Writing hello.\n");
  // Write to proxy
  message_byte_size = write_message(socket_file_descriptor, &hello_message);

  fd_set readfds;
  int fd_stdin = fileno(stdin);

  FD_ZERO(&readfds);
  FD_SET(fileno(stdin), &readfds);
  FD_SET(socket_file_descriptor, &readfds);

  struct timeval tv;
  tv.tv_sec = 5;
  tv.tv_usec = 0;


  while (1) {
    int num_readable =
        select(fd_stdin + socket_file_descriptor, &readfds, NULL, NULL, &tv);
    if (num_readable == 2) {
      // Get message from server first
    } else if (num_readable == 1) {
      if (FD_ISSET(socket_file_descriptor, &readfds)) {
        } else {
          printf("What would you like to do? Enter 'list', 'chat', or 'exit'.\n");
          char command[20];
          printf("Enter command: ");
          scanf("%s", command);

          if (strcmp(command, "list") == 0) {
            // Send a list command
            struct Message list_message = get_list_message(client_id);
            message_byte_size = write_message(socket_file_descriptor, &list_message);
          } else if (strcmp(command, "chat") == 0) {
            char destination[20];
            printf("Who to send to?: ");
            scanf("%s", destination);
            char message_text[400];
            printf("What do you want to say?: ");
            scanf("%s", message_text);
            struct Message chat_message =
                get_chat_message(client_id, destination, message_text);
            printf("MEssage data: %s\n", chat_message.data);
            message_byte_size = write_message(socket_file_descriptor, &chat_message);
          } else if (strcmp(command, "exit") == 0) {
            struct Message exit_message = get_exit_message(client_id);
            message_byte_size = write_message(socket_file_descriptor, &exit_message);
            break;
          } else {
            printf("Sorry, didn't get that.\n\n");
          }
        }
      }
    }
  }

  // Read from proxy
  char buffer[BUFSIZE];
  bzero(buffer, BUFSIZE);
  printf("Gonna try to read\n");
  message_byte_size = read(socket_file_descriptor, buffer, BUFSIZE);
  if (message_byte_size < 0) {
    perror("read");
    exit(EXIT_FAILURE);
  }
  printf("We got this from the proxy: %s\n", buffer);

  // Close the active socket.
  close(socket_file_descriptor);

  return 0;
}
