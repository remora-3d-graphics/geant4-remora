#ifndef REMORA_TRAJECTORY_HH
#define REMORA_TRAJECTORY_HH

#include <string>
#include <queue>
#include <unordered_map>
#include "G4ThreeVector.hh"

namespace remora {

class Trajectory {
public:
  Trajectory(std::string name_);
  void AddPoint(G4ThreeVector pt);

  int id;
  std::string name;
  std::queue<G4ThreeVector> points;
};

class TrajectoryManager {
public:
  TrajectoryManager();
  bool Exists(int key);
  bool AddTraj(int key, std::string name);
  bool AddPoint(int key, G4ThreeVector pt);
  bool FinishTraj(int key);
  bool IsLocked();

private:
  int GetNTrajectories();
  Trajectory GetNextTrajectory();
  bool PopNextTrajectory();

  std::unordered_map<int, Trajectory> trajsInProgress;
  std::queue<Trajectory> finishedTrajs;
  int maxTrajs = 50;
  bool locked = false;

  friend class Server;
};

} // namespace remora

#endif // REMORA_TRAJECTORY_HH
