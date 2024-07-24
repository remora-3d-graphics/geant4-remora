#include "RemoraSteppingAction.hh"

namespace remora
{

RemoraSteppingAction::RemoraSteppingAction(G4UserSteppingAction* prevAction)
: pPrevAction(prevAction) {}

RemoraSteppingAction::~RemoraSteppingAction(){
  if (pPrevAction) delete pPrevAction;
}

void RemoraSteppingAction::SetSteppingManagerPointer(G4SteppingManager* pValue){
  if (pPrevAction) pPrevAction->SetSteppingManagerPointer(pValue);
}

void RemoraSteppingAction::UserSteppingAction(const G4Step* aStep){
  if (pPrevAction) pPrevAction->UserSteppingAction(aStep);

  std::cout << "Stepping: " << aStep->GetTrack()->GetTrackID() << std::endl;
}


  
} // namespace remora