#include "RemoraSteppingAction.hh"

namespace remora
{

RemoraSteppingAction::RemoraSteppingAction(
  TrajectoryManager* trajManager,
  G4UserSteppingAction* prevAction)
: pTrajManager(trajManager),
  pPrevAction(prevAction) {}

RemoraSteppingAction::~RemoraSteppingAction(){
  if (pPrevAction) delete pPrevAction;
}

void RemoraSteppingAction::SetSteppingManagerPointer(G4SteppingManager* pValue){
  if (pPrevAction) pPrevAction->SetSteppingManagerPointer(pValue);
}

void RemoraSteppingAction::UserSteppingAction(const G4Step* aStep){
  if (pPrevAction) pPrevAction->UserSteppingAction(aStep);

  if (pTrajManager->IsLocked()) return;

  int id = aStep->GetTrack()->GetTrackID();

  if (!pTrajManager->Exists(id)){
    G4String name = aStep->GetTrack()->GetParticleDefinition()->GetParticleName();
    pTrajManager->AddTraj(id, name);
    // add the first point (pre)
    pTrajManager->AddPoint(id, G4ThreeVector(aStep->GetPreStepPoint()->GetPosition()));
  }

  // from now on, add the post step points
  pTrajManager->AddPoint(id, G4ThreeVector(aStep->GetPostStepPoint()->GetPosition()));

  if (aStep->GetTrack()->GetTrackStatus() != fAlive){
    pTrajManager->FinishTraj(id);
  }
}


  
} // namespace remora