#ifndef REMORA_STATE_MACHINE_HH
#define REMORA_STATE_MACHINE_HH 1

#include "G4VStateDependent.hh"

#include "G4ios.hh"

class RemoraStateMachine : public G4VStateDependent {
public:
  RemoraStateMachine() : G4VStateDependent() {};

  G4bool Notify(G4ApplicationState requestedState) override;

};

#endif