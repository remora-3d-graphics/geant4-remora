#include "RemoraG4Messenger.hh"

namespace remora {
	RemoraMessenger::RemoraMessenger(Server* s) 
  : G4UImessenger(), serverPtr(s) {
		fDirectory = new G4UIdirectory("/remora/");
	
		fChangeTitleCmd = new G4UIcmdWithAString("/remora/setTitle", this);
    fSendDetectorsCmd = new G4UIcmdWithoutParameter("/remora/sendDetectors", this);

	}

	void RemoraMessenger::SetNewValue(G4UIcommand* cmd, G4String newValues) {
		if (cmd == fChangeTitleCmd) {
			G4String setTitleCommand = "SetTitle" + newValues;
			serverPtr->QueueMessageToBeSent(setTitleCommand);
		}
    if (cmd == fSendDetectorsCmd){
      serverPtr->SendDetectors();
    }
	}
}