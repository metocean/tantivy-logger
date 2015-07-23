# tantivy-logger
A single threaded poll based logging server (using libuv) as well as mixed language clients.

Project status: Aplah 0.1

This project is currently just a prototype, usally I'd keep something closed until its good enought, but making work pay for my repos could be troublesome.

Project description:

This program contains a logger server and clients of other langauges.

Client Process.A >
Client Process.B > unix socket > tantivy logger server > log_file.txt
Client Process.C >

The server is:
* Written in C.
* Connections via unix-socket.
* Single threaded poll based via use of the awesome libuv library.

Clients currently supported are:
* Python

Performance: 
100,000 + log inserts per/second.

API for talking to server:

* Saving a log entry:

[ SOL ][ length ][ EOL ][ log entry ]
[ 1 char ][ variable chars ][ 1 char ][ variable chars ]

[ 1 char ] = start of length marker, currently is the ascii char '~'
[ variable chars ] = length of the preceding log entry. The length is in ascii enodced. 
[ 1 char ] = end of length marker, currently is the ascii char ';'
[ variable chars ] = the actual log entry. If your log entry does not have a '\n' at the end of it the server will automatically insert one. This will become a configurable option.

exmaples:
"~9;some error"
"~13;beer is great"

TODO:
* add command line args, and config files.
* add automake or something similar.
* add configuration / init mode, so when clients first connects they can change options on how the logs get save.
* add unit tests using valgrind (I'm currently testing manually with valgrind).
* add python logging handler 
* add TCP socket interface.
* make output file for insertion into elasticsearch.
* make c client.
* make c++ client.
* make java client.
* make bash client.
* make c# client.
