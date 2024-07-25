#include "RemoraTrajectory.hh"

namespace remora {

static int trajectoryId = 0;

Trajectory::Trajectory(std::string name_) : name(name_) {
  id = ++trajectoryId;
}

void Trajectory::AddPoint(G4ThreeVector pt) {
  points.push(pt);
}

TrajectoryManager::TrajectoryManager() {}

bool TrajectoryManager::Exists(int key) {
  return trajsInProgress.count(key);
}

bool TrajectoryManager::AddTraj(int key, std::string name) {
  if (Exists(key)) return false;
  trajsInProgress[key] = Trajectory(name);
  return true;
}

bool TrajectoryManager::AddPoint(int key, G4ThreeVector pt) {
  if (!Exists(key)) return false;
  trajsInProgress[key].AddPoint(pt);
  return true;
}

bool TrajectoryManager::FinishTraj(int key) {
  if (!Exists(key)) return false;
  finishedTrajs.push(trajsInProgress[key]);
  trajsInProgress.erase(key);
  if (finishedTrajs.size() > maxTrajs) {
    locked = true;
  }
  return true;
}

bool TrajectoryManager::IsLocked() {
  return locked;
}

int TrajectoryManager::GetNTrajectories() {
  return finishedTrajs.size();
}

Trajectory TrajectoryManager::GetNextTrajectory() {
  return finishedTrajs.front();
}

bool TrajectoryManager::PopNextTrajectory() {
  if (finishedTrajs.empty()) return false;
  finishedTrajs.pop();
  if (finishedTrajs.size() < maxTrajs) locked = false;
  return true;
}

} // namespace remora
