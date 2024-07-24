#include "ActionInit.hh"

namespace gps_test {

	void ActionInit::Build() const {
		SetUserAction(new PGA());
		SetUserAction(new RunAction());
    SetUserAction(pRemoraServer->GetRemoraSteppingAction());
	}

	void ActionInit::BuildForMaster() const {
		SetUserAction(new RunAction());
	}

} // !gps_test