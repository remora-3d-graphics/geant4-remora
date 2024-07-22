#ifndef REMORA_TRACKING_ACTION_HH
#define REMORA_TRACKING_ACTION_HH

#include "G4UserTrackingAction.hh"
#include "G4Track.hh"
#include <iostream>

class RemoraTrackingAction : public G4UserTrackingAction {
public:
  RemoraTrackingAction();

  void PostUserTrackingAction(const G4Track*) override;
};

#endif