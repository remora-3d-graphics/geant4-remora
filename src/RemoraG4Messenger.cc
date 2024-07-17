#include "RemoraG4Messenger.hh"

namespace remora {
	RemoraMessenger::RemoraMessenger(Server* s) 
  : G4UImessenger(), serverPtr(s) {
		fDirectory = new G4UIdirectory("/remora/");
	
		fChangeTitleCmd = new G4UIcmdWithAString("/remora/setTitle", this);
    fSendDetectorsCmd = new G4UIcmdWithoutParameter("/remora/sendDetectors", this);
    fRmShapeWithNameCmd = new G4UIcmdWithAString("/remora/removeShapeWithName", this);
    fRmShapeWithNameCmd->SetGuidance("Removes all shapes who's name start with the inputted string. Useful for removing all particle trajectories. e.g.: /remora/removeShapeWithName particle");


	}

	void RemoraMessenger::SetNewValue(G4UIcommand* cmd, G4String newValues) {
		if (cmd == fChangeTitleCmd) {
			G4String setTitleCommand = "SetTitle" + newValues;
			serverPtr->QueueMessageToBeSent(setTitleCommand);
		}
    if (cmd == fSendDetectorsCmd){
      serverPtr->SendDetectors();
    }
    if (cmd == fRmShapeWithNameCmd){
      G4String rmShapesCommand = "RmShape" + newValues;
      serverPtr->QueueMessageToBeSent(rmShapesCommand);
    }
	}
}