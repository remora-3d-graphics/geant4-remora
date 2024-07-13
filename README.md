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
- [ ] Make a client recv thread and send thread. The issue is that the server waits to hear back from the client.
- [ ] If a socket disconnects, make sure and remove it from the sockets list
- [ ] client side: if server disconnects, disconnect as well. 
- [ ] Error handling for sending and receiving messages
- [ ] Basically graceful disconnection needs to be addressed from server and client sides.
- [ ] If a socket joins before runManager is initialized, make sure they can get the detectors once it initializes.

## Message format:
- From server: (saves # bytes in message) `{Json_string}`
- From client: `{error?(0 or 1),bytes_received}`
- Server checks if error == 0 and if bytes_received is equal
- Server tries again a few times and then disconnects the socket

## How to get dimensions from a G4VSolid:
Possible methods:
```cpp
virtual G4ThreeVector SurfaceNormal(const G4ThreeVector& p) const = 0;
      // Returns the outwards pointing unit normal of the shape for the
      // surface closest to the point at offset p.

virtual G4GeometryType  GetEntityType() const = 0;
      // Provide identification of the class of an object.
      // (required for persistency and STEP interface)

virtual G4ThreeVector GetPointOnSurface() const;
      // Returns a random point located on the surface of the solid.
      // Points returned are not necessarily uniformly distributed.

virtual G4Polyhedron* CreatePolyhedron () const;
      // Create a G4Polyhedron.  (It is the caller's responsibility
      // to delete it).  A null pointer means "not created".
```

The G4Polyhedron class is interesting:
```cpp
class G4Polyhedron : public HepPolyhedron, public G4Visible

```
The following is from HepPolydedron
```cpp
  // Get next vertex index of the quadrilateral
  G4bool GetNextVertexIndex(G4int & index, G4int & edgeFlag) const;

  // Get vertex by index
  G4Point3D GetVertex(G4int index) const;

  // Get next vertex + edge visibility of the quadrilateral
  G4bool GetNextVertex(G4Point3D & vertex, G4int & edgeFlag) const;

  // Get next vertex + edge visibility + normal of the quadrilateral
  G4bool GetNextVertex(G4Point3D & vertex, G4int & edgeFlag,
                       G4Normal3D & normal) const;

  // Get indices of the next edge with indices of the faces
  G4bool GetNextEdgeIndices(G4int & i1, G4int & i2, G4int & edgeFlag,
                            G4int & iface1, G4int & iface2) const;
  G4bool GetNextEdgeIndeces(G4int & i1, G4int & i2, G4int & edgeFlag,
                            G4int & iface1, G4int & iface2) const
  {return GetNextEdgeIndices(i1,i2,edgeFlag,iface1,iface2);}  // Old spelling

  // Get indices of the next edge
  G4bool GetNextEdgeIndices(G4int & i1, G4int & i2, G4int & edgeFlag) const;
  G4bool GetNextEdgeIndeces(G4int & i1, G4int & i2, G4int & edgeFlag) const
  {return GetNextEdgeIndices(i1,i2,edgeFlag);}  // Old spelling.


```

# Issues:
- [ ] Geant4 app can't quit, I think because of the server running. Maybe because the messenger is still there?
- [x] Even using the public version of the runManager, we can't get the world object. Fixed using RunManagerFactory
- [ ] If the shape json is too long (e.g. G4Sphere) the connection needs to be able to receive the whole thing. The client can't get the whole thing in time and it crashes. This means the server needs to send it in chunks.

# Acknowledgements
- This product includes software developed by Members of the [Geant4 Collaboration](http://cern.ch/geant4).
