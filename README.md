Part of the testing is using multiple clients that do a handful of different things. I started with a general client that would do everything, but it got busy quick. We can break it down into multiple clients.

All these clients share a ton of code in helpers.c. I put as much as I could in there to keep each one simple.

## Clients:

Client 1: Client that sends HELLO, reads HELLO_ACK and CLIENT_LIST, sends CLIENT_LIST_REQ, reads client list, then sends EXIT
- This is a normal client that just connects and reads the client list.

Client 2: Client that sends HELLO, then reads messages forever.
- This is super helpful client that just prints out whatever messages it gets. Run this while running all of the others to test whether messages are getting through.

Client 3: Client that sends HELLO to client2, sends CHAT, then sends EXIT.

Client 4: Client that sends 1/2 of HELLO headers, then second half of HELLO headers. the 1/2 of CHAT, then second half of chat.
- This will test the partial message for headers.

Client 5: Client that sends HELLO, 1/2 of CHAT, waits 65 seconds, then ensures connection was closed.
- This will test the server's timeout when waiting for partial messages. After a minute the connection should be closed.

Client 6: Client that sends HELLO, then tries to read, then just hangs out. Running one instance of this should succeed and print the hello_ack and clinet list. Running another instance without stopping the first one should print out the ERROR message it receives.
- This will test the server's ERROR(CLIENT_ALREADY_PRESENT)

Client 7: Client that sends HELLO, then CHAT to clientID that is not connected, then reads the error message.
- This will test a nonexistant clientID and the server's ERROR(CANNOT_DELIVER)

Client 8: Client that sends HELLO, then CHAT to itself, then reads the error message.
- This will test the server's ability to gracefully handle a client sending a message to themself. As per the handout, the socket will be closed.

Client 9: Client that sends HELLO, then CHAT with junk length (too long) specified in header. It will then ensure that the connection was closed.
- This will test the server's ability to handle malformed data gracefully, since the server should close the connection.

Client 10: Client that sends HELLO, then closes the connection without sending anything (very rude).
- This will test the server's ability to handle a client randomly closing.

Client 11: Client that sends HELLO, then impersonates Client3.
- This will test impersonation.
- Run both client2 and client3 before running this one.
