Part of the testing is using multiple clients that do a handful of different things. I started with a general client that would do everything, but it got busy quick. We can break it down into multiple clients.

All these clients share a ton of code but whatever.

## Clients:

Client 1: Client that sends HELLO, reads HELLO_ACK and CLIENT_LIST, sends CLIENT_LIST_REQ, reads client list, then sends EXIT
- This is a normal client that just connects and reads the client list.

Client 2: Client that sends HELLO, sends CHAT, then sends EXIT.

Client 3: Client that sends HELLO, reads message, then sends EXIT.
- Can be used in coordination with client 2, if you run this one before 2.
- Different client ID than the one before.

Client 4: Client that sends HELLO, 1/2 of CHAT, waits two seconds, then the other half of CHAT, then EXIT.
- This will test the partial message

Client 5: Client that sends HELLO, 1/2 of CHAT, waits 61 seconds, then ensures connection was closed.
- This will test the server's timeout when waiting for partial messages. After a minute the connection should be closed.

Client 6: "Client" that actually creates two connections with the same client ID. First one should succeed, second one gets error message.
- This will test the server's ERROR(CLIENT_ALREADY_PRESENT)

Client 7: Client that sends HELLO, then CHAT to clientID that is not connected, then reads the error message.
- This will test a nonexistant clientID and the server's ERROR(CANNOT_DELIVER)

Client 8: Client that sends HELLO, then CHAT to itself, then reads the error message.
- This will test the server's ability to gracefully handle a client sending a message to themself. Should probably send error message.

Client 9: Client that sends HELLO, then CHAT with data length that doesn't match length specified in header. It will then ensure that the connection was closed.
- This will test the server's ability to handle malformed data gracefully, since the server should close the connection.

Client 10: Client that sends HELLO, then closes the connection without sending anything (very rude).
- This will test the server's ability to handle a client randomly closing.