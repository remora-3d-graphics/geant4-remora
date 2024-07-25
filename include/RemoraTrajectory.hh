#ifndef REMORA_TRAJECTORY_HH
#define REMORA_TRAJECTORY_HH

#include <string>
#include <queue>
#include <unordered_map>
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
  TrajectoryManager(){
  };

  bool Exists(int key){
    return trajsInProgress.count(key);
  }

  bool AddTraj(int key, std::string name){
    if (Exists(key)) return false;

    trajsInProgress[key] = Trajectory(name);
    return true;
  }

  bool AddPoint(int key, G4ThreeVector pt){
    if (!Exists(key)) return false;

    trajsInProgress[key].AddPoint(pt);
    return true;
  }

  bool FinishTraj(int key){
    // moves from 'in progress' to 'finished'
    if (!Exists(key)) return false;

    finishedTrajs.push(trajsInProgress[key]);
    trajsInProgress.erase(key);

    return true;
  }
  

private:
  std::unordered_map<int, Trajectory> trajsInProgress;
  std::queue<Trajectory> finishedTrajs;
};

} // !remora

#endif