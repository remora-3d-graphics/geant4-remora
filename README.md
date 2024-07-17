# Geant4 Remora example

This is a simple Geant4 app that shows what the remora 3d graphics engine is capable of.

For now, only supports one client at a time.

# Testing the server with docker
```
docker build -f Dockerfile.test -t remora_g4 .
docker run --rm -p 8080:8080 -it remora_g4
# note: that is <host port>:<container port>
# note for me: use -itd in the docker containers to keep them on standby
```


# Todo:
- [x] Make a client recv thread and send thread. The issue is that the server waits to hear back from the client.
- [x] If a socket disconnects, make sure and remove it from the sockets list SOLUTION: Next message they try to send, if they don't get anything or they get 'bye' they kill the socket.
- [ ] client side: if server disconnects, disconnect as well. 
- [ ] Error handling for sending and receiving messages
- [ ] Basically graceful disconnection needs to be addressed from server and client sides.
- [ ] If a socket joins before runManager is initialized, make sure they can get the detectors once it initializes.

## Message format:
- From server: (saves # bytes in message) `{Json_string}`
- From client: `{error?(0 or 1),bytes_received}`
- Server checks if error == 0 and if bytes_received is equal
- Server tries again a few times and then disconnects the socket

## Need for mutexes:
- `int nThreads = 0;`
- `int nClientsReceived = 0;`
- `std::queue<std::string> messagesToBeSent;`

# Issues:
- [ ] Geant4 app can't quit, I think because of the server running. Maybe because the messenger is still there?
- [x] Even using the public version of the runManager, we can't get the world object. Fixed using RunManagerFactory
- [ ] If the shape json is too long (e.g. G4Sphere) the connection needs to be able to receive the whole thing. The client can't get the whole thing in time and it crashes. This means the server needs to send it in chunks.
- [ ] TODO: FIGURE OUT HOW TO SEND IN CHUNKS. maybe the wrapper that the server sends has "CMD{JSON}DONE" or something and the client will keep receiving until "DONE" is read. 
- [ ] If client presses the 'x' the server doesn't register that it left.

## New approach to multiple clients messages
- instead of doing all this mutex stuff, I will have each new client get it's own list (on the master thread). if the message queue gets a new message, it gets added to all the lists. Then once it is popped from all the lists, you can pop it from the messages.

# Useful websites
- [Helpful tips about mutexes](https://stackoverflow.com/questions/4989451/mutex-example-tutorial)

# Acknowledgements
- This product includes software developed by Members of the [Geant4 Collaboration](http://cern.ch/geant4).
