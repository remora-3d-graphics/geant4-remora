#include "RemoraTrajectory.hh"

namespace remora {


// Trajectory

static std::atomic<int> trajectoryId = 0;

Trajectory::Trajectory(std::string name_) : name(name_) {
  id = ++trajectoryId;
}

void Trajectory::AddPoint(G4ThreeVector pt) {
  std::lock_guard<std::mutex> lock(ptsMutex);
  points.push(pt);
}


// Trajectory Manager

TrajectoryManager::TrajectoryManager() {}

bool TrajectoryManager::Exists(int key) {
  return trajsInProgress.count(key);
}

bool TrajectoryManager::AddTraj(int key, std::string name) {
  std::lock_guard<std::mutex> lock(trajInProgressMutex);

  if (Exists(key)) return false;
  trajsInProgress[key] = new Trajectory(name);
  return true;
}

bool TrajectoryManager::AddPoint(int key, G4ThreeVector pt) {
  std::lock_guard<std::mutex> lock(trajInProgressMutex);

  if (!Exists(key)) return false;
  trajsInProgress[key]->AddPoint(pt);
  return true;
}

bool TrajectoryManager::FinishTraj(int key) {
  std::lock_guard<std::mutex> lock1(trajInProgressMutex);
  std::lock_guard<std::mutex> lock2(finishedTrajsMutex);

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
  std::lock_guard<std::mutex> lock(finishedTrajsMutex);

  return finishedTrajs.size();
}

Trajectory* TrajectoryManager::GetNextTrajectory() {
  std::lock_guard<std::mutex> lock(finishedTrajsMutex);

  return finishedTrajs.front();
}

bool TrajectoryManager::PopNextTrajectory() {
  std::lock_guard<std::mutex> lock(finishedTrajsMutex);

  if (finishedTrajs.empty()) return false;

  delete finishedTrajs.front();
  finishedTrajs.pop();
  
  if (finishedTrajs.size() < maxTrajs) locked = false;
  return true;
}

} // namespace remora
