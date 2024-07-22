#ifndef REMORA_ACTION_INIT_HH
#define REMORA_ACTION_INIT_HH

#include "G4VUserActionInitialization.hh"

#include "RemoraTrackingAction.hh"

class RemoraActionInit : public G4VUserActionInitialization {
public:
  RemoraActionInit(G4VUserActionInitialization* previousActionInit) : previousActionInit_(previousActionInit) {};

  void Build() const override;
	void BuildForMaster() const override;

private:
  G4VUserActionInitialization* previousActionInit_ = nullptr;
};

#endif