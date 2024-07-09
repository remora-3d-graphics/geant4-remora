#include "SensitiveDetector.hh"


namespace gps_test {

	void SensitiveDetector::Initialize(G4HCofThisEvent* hce) {
		// init the Hits Collection
		MyHitsCollection* newHC = new MyHitsCollection();

		G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);

		hce->AddHitsCollection(hcID, newHC);
	};


	G4bool SensitiveDetector::ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) {
		if (aStep->IsFirstStepInVolume()) {
			G4AnalysisManager::Instance()->FillH2(
				0,
				aStep->GetPreStepPoint()->GetPosition().getX(),
				aStep->GetPreStepPoint()->GetPosition().getY(),
				aStep->GetPreStepPoint()->GetKineticEnergy()
			);
		}

		return true;
	};




} // !gps_test