#include "RemoraStateMachine.hh"

G4bool RemoraStateMachine::Notify(G4ApplicationState requestedState){
  G4cout << "State changed: " << requestedState << G4endl;
}