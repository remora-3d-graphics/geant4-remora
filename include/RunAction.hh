#ifndef GPS_TEST_RUN_ACTION_HH
#define GPS_TEST_RUN_ACTION_HH

#include "G4UserRunAction.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"

namespace gps_test {

	class RunAction : public G4UserRunAction {
	public:
		RunAction();

		void BeginOfRunAction(const G4Run* aRun) override;
		void EndOfRunAction(const G4Run* aRun) override;
	};
}

#endif // !GPS_TEST_RUN_ACTION_HH
