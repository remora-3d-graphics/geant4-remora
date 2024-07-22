#include "RemoraTrackingAction.hh"

RemoraTrackingAction::RemoraTrackingAction(){
  std::cout << "Remora tracking action initialized" << std::endl;

}

RemoraTrackingAction::~RemoraTrackingAction(){
  std::cout << "Goodbye Remora Tracking Action" << std::endl;
}

void RemoraTrackingAction::PostUserTrackingAction(const G4Track* aTrack){
  std::cout << "Tracking: " << aTrack->GetParticleDefinition()->GetParticleName() << std::endl;
}

