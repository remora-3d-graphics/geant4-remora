#ifndef REMORA_TRAJECTORY_HH
#define REMORA_TRAJECTORY_HH

#include <string>
#include <queue>
#include <map>
#include "G4ThreeVector.hh"

namespace remora {

static int trajectoryId = 0;

struct Trajectory {

  Trajectory(std::string name_) : name(name_) {
    id = ++trajectoryId;
  }

  void AddPoint(G4ThreeVector pt){
    points.push(pt);
  }

  // note: this id is purely for the remora client
  // and has nothing to do with Geant4 track id
  int id;
  std::string name;
  std::queue<G4ThreeVector> points;
};


class TrajectoryManager {
public:
  TrajectoryManager(){};

private:
  std::map<int, Trajectory> trajsInProgress;
  std::queue<Trajectory> finishedTrajs;
};

} // !remora

#endif