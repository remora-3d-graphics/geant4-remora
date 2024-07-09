#ifndef GPS_TEST_DETECTOR_CONSTRUCTION_HH
#define GPS_TEST_DETECTOR_CONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4SDManager.hh"

#include "SensitiveDetector.hh"

namespace gps_test {
	class DetectorConstruction : public G4VUserDetectorConstruction {
	public:
		DetectorConstruction() : flogicObj(nullptr) {};

		G4VPhysicalVolume* Construct() override;
		void ConstructSDandField() override;

	private:
		G4LogicalVolume* flogicObj;
	};


} // !gps_test

#endif // !GPS_TEST_DETECTOR_CONSTRUCTION_HH
