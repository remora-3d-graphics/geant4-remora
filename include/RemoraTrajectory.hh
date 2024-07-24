#ifndef REMORA_TRAJECTORY_HH
#define REMORA_TRAJECTORY_HH

#include <string>
#include <queue>
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

  int id;
  std::string name;
  std::queue<G4ThreeVector> points;
};

} // !remora

#endif