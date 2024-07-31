#include "RemoraG4Messenger.hh"

namespace remora {
	RemoraMessenger::RemoraMessenger(Server* s) 
  : G4UImessenger(), serverPtr(s) {
		fDirectory = new G4UIdirectory("/remora/");
	
		fChangeTitleCmd = new G4UIcmdWithAString("/remora/setTitle", this);
    fSendDetectorsCmd = new G4UIcmdWithoutParameter("/remora/sendDetectors", this);
    fRmShapeWithNameCmd = new G4UIcmdWithAString("/remora/removeShapeWithName", this);
    fRmShapeWithNameCmd->SetGuidance("Removes all shapes who's name start with the inputted string. Useful for removing all particle trajectories. e.g.: /remora/removeShapeWithName particle");
    fChangeColorCmd = new G4UIcmdWith3Vector("/remora/changeColor", this);
    fChangeColorCmd->SetGuidance("Changes color based on the R, G, B, values input.");

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
    if (cmd == fChangeColorCmd){
      G4ThreeVector rgb = fChangeColorCmd->GetNew3VectorValue(newValues);

      // just making sure it's between 0 and 255
      if (
        rgb.getX() > 255 || rgb.getY() > 255 || rgb.getZ() > 255 ||
        rgb.getX() < 0 || rgb.getY() < 0 || rgb.getZ() < 0
        ) {
          G4cout << "Error sending change color command: "
          << "rgb values must be between 0 and 255. " << G4endl;
        }
      else {
        G4String changeColorCmd = "Color";
        changeColorCmd += std::to_string(int(rgb.getX()));
        changeColorCmd += ",";
        changeColorCmd += std::to_string(int(rgb.getY()));
        changeColorCmd += ",";
        changeColorCmd += std::to_string(int(rgb.getZ()));
        serverPtr->QueueMessageToBeSent(changeColorCmd);
      }

    }
	}
}