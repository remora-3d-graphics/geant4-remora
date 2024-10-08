#ifndef REMORA_G4_MESSENGER_HH
#define REMORA_G4_MESSENGER_HH

#include <string>

#include "G4UImessenger.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWith3Vector.hh"

#include "cp_server.hh"

namespace remora {

  class Server;

	class RemoraMessenger : public G4UImessenger {
	public:
		RemoraMessenger(Server* s);
		~RemoraMessenger() {};
	
		void SetNewValue(G4UIcommand* cmd, G4String newValues) override;
	
	private:
		Server* serverPtr;

		G4UIdirectory* fDirectory;
		G4UIcmdWithAString* fChangeTitleCmd;
    G4UIcmdWithoutParameter* fSendDetectorsCmd;
    G4UIcmdWithAString* fRmShapeWithNameCmd;
    G4UIcmdWith3Vector* fChangeColorCmd;
	
    G4UIcmdWithoutParameter* fActivateCmd;
    G4UIcmdWithoutParameter* fDeactivateCmd;

	};
}

#endif // ! REMORA_G4_MESSENGER_HH
