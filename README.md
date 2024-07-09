# Geant4 Remora example

This is a simple Geant4 app that shows what the remora 3d graphics engine is capable of.

# Testing the server with docker
```
docker build -f Dockerfile.test -t remora_g4 .
docker run --rm -p 8080:8080 -it remora_g4
# note: that is <host port>:<container port>
# note for me: use -itd in the docker containers to keep them on standby
```


# Todo:
- [ ] If a socket disconnects, make sure and remove it from the sockets list
- [ ] client side: if server disconnects, disconnect as well. 
- [ ] Error handling for sending and receiving messages

## Message format:
- From server: (saves # bytes in message) `{Json_string}`
- From client: `{error?(0 or 1),bytes_received}`
- Server checks if error == 0 and if bytes_received is equal
- Server tries again a few times and then disconnects the socket

# Acknowledgements
- This product includes software developed by Members of the [Geant4 Collaboration](http://cern.ch/geant4).
