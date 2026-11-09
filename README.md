# Chatroom

## About
This is a chatroom I made entirely with C only using ENet for UDP networking and ncurses which is a famous library for TUI.
Within days and following tutorials, I was able to make this. That project showed what I have learned in a short timespan about networking and the client server system.

## Screenshots
### Client 0 (John)
![john](img/john.png)
### Client 1 (Tom)
![tom](img/tom.png)

## Features
### Client
- write messages with up to 31 other clients in the chatroom
- pick any username you want with unique client id to differentiate
- encryption between clients by all using the same key

Some more info can be found [here](Client/README.md)
  
### Server
- let connect up to 32 clients
- some information which is not sensitive due to encryption
- only functions as manager and service provider
