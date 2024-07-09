#ifndef GPS_TEST_SENSITIVE_DETECTOR_HH
#define GPS_TEST_SENSITIVE_DETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "globals.hh"
#include "G4SDManager.hh"

#include "G4THitsCollection.hh"
#include "G4VHit.hh"

#include "G4AnalysisManager.hh"

namespace gps_test {

	class Hit : public G4VHit {};
	class MyHitsCollection : public G4THitsCollection<Hit> {};

	class SensitiveDetector : public G4VSensitiveDetector {
	public:
		SensitiveDetector(
			G4String name,
			G4String hcName
			) : G4VSensitiveDetector(name) {

			collectionName.insert(hcName);
		};

		void Initialize(G4HCofThisEvent* hce) override;
		G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override;

	};



} // !gps_test

#endif // !GPS_TEST_SENSITIVE_DETECTOR_HH
