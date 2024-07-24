#ifndef REMORA_STEPPING_ACTION_HH
#define REMORA_STEPPING_ACTION_HH 1

#include "G4UserSteppingAction.hh"
#include "G4Step.hh"
#include <iostream>
#include <map>
#include <queue>

namespace remora {

typedef std::map<int, Trajectory> unfinishedTrajsContainer;
typedef std::queue<int> trajIDContainer;

class RemoraSteppingAction : public G4UserSteppingAction {
public:
  RemoraSteppingAction(
    G4UserSteppingAction* prevAction=nullptr
  );
  ~RemoraSteppingAction();

  void SetSteppingManagerPointer(G4SteppingManager* pValue) override;
  void UserSteppingAction(const G4Step*) override;

private:
  G4UserSteppingAction* pPrevAction = nullptr;
};

} // !namespace remora

#endif