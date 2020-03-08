# socket-chat
A tcp-socket chat client and server written in C using linux sockets without any libraries.

## Build instructions
The server and the client are in their own separate directories with their own Makefiles (although they have the same requirements). To build the client, run make in the client directory and vice versa for the server. There are no external libraries required and it's written in C99 compiled with the `-pedantic` flag in GCC 9.2.0 so it should work on old GCC versions and other compilers although I have not tried that.


## Network protocol for TCP chat server/client

### Request type
The first byte (char) of a request represents what type of request. The
following prefix-bytes are supported.
- c (Connection establishment)
- m (Message)
- q (Disconnecting)

To separate different data fields the ASCII "end of transmission block"
character (23, 0x17) is used. To signal the end of the entire request a
null-terminator (0, 0x0) is used.

#### Connection establishment (c)
These types of requests are sent from the server to the client in order to
request the data about the user when setting up a user on the server. And they
are used by the client to provide that data.

##### Client to server
Format:
```
c{username (1:40)}\0
```
After the first byte there are 1 to 40 bytes containing the client's desired
username terminated by the requests null-terminator.

Example:
```
cvincent-uden\0
```

##### Server to client
Format:
```
c{connection status (1)}\0
```
The second byte will be either an A or a D depending on if the clients request
to connect is accepted or declined. After this request is recieved the client
is expected to answer with a "Connection establishment" request of their own
as described below.

Example:
```
cA\0
```

#### Message (m)
This is the type of request a client sends to the server when it send a message
to the chat. It also the request the server uses when it notifies a client about
a message it recieved from another client.

##### Client to server
Format:
```
m{message (1:500)}\0
```
After the first byte there are 1 to 500 bytes containing the client's desired
message terminated by the requests null-terminator.

Example:
```
mHello World!\0
```

##### Server to client
Format:
```
m{username (1:40)}{message (1:500)}\0
```
The request first specifies the username from whom the message was sent and then
the message itself.

Example:
```
mvincent-uden\0x17Hello World!\0
```

#### Disconnecting (q)
This request is only two bytes long signifying that the connection is about to
be shut down.

##### Client to server
Format:
```
q\0
```

Example:
```
q\0
```

##### Server to client
Format:
```
q\0
```

Example:
```
q\0
```
