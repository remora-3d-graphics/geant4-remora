#include "RemoraTrackingAction.hh"

RemoraTrackingAction::RemoraTrackingAction(){
  std::cout << "Remora tracking action initialized" << std::endl;

}

void RemoraTrackingAction::PostUserTrackingAction(const G4Track* aTrack){
  std::cout << "Tracking: " << aTrack->GetParticleDefinition()->GetParticleName() << std::endl;
}

