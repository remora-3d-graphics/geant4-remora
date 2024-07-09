#ifndef GPS_TEST_ACTION_INIT_HH
#define GPS_TEST_ACTION_INIT_HH

#include "G4VUserActionInitialization.hh"
#include "PGA.hh"
#include "RunAction.hh"

namespace gps_test {

	class ActionInit : public G4VUserActionInitialization {
	public:
		ActionInit() {};

		void Build() const override;
		void BuildForMaster() const override;
	};

} // !gps_test

#endif // !GPS_TEST_ACTION_INIT_HH
