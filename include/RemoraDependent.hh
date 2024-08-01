#ifndef REMORA_DEPENDENT_HH
#define REMORA_DEPENDENT_HH 1

#include "G4VStateDependent.hh"

class RemoraDependent : public G4VStateDependent {
public:
  RemoraDependent();
  
  G4bool Notify(G4ApplicationState requestedState) override;

};

#endif