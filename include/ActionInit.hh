#ifndef GPS_TEST_ACTION_INIT_HH
#define GPS_TEST_ACTION_INIT_HH

#include "G4VUserActionInitialization.hh"
#include "PGA.hh"
#include "RunAction.hh"

#include "cp_server.hh"

using namespace remora;

namespace gps_test {

	class ActionInit : public G4VUserActionInitialization {
	public:
		ActionInit(Server* pRemoraServer_) : pRemoraServer(pRemoraServer_) {};

		void Build() const override;
		void BuildForMaster() const override;

  private:
    Server* pRemoraServer;
	};

} // !gps_test

#endif // !GPS_TEST_ACTION_INIT_HH
