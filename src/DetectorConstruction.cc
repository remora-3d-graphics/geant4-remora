#include "DetectorConstruction.hh"

#include "G4SystemOfUnits.hh"
#include "G4NistManager.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#include "G4Tubs.hh"
#include "G4Sphere.hh"

namespace gps_test {

	G4VPhysicalVolume* DetectorConstruction::Construct() {
		// order:
		// solid volume
		// logical volume
		// physical volume

		// solid
		G4int sizeX = 1 * m;
		G4int sizeY = 1 * m;
		G4int sizeZ = 1 * m;

		G4Box* solidWorld = new G4Box("solidWorld", sizeX, sizeY, sizeZ);

		// logic 
		auto nist = G4NistManager::Instance();
		
		G4Material* vacuum = nist->FindOrBuildMaterial("G4_Galactic");

		G4LogicalVolume* logicWorld = 
			new G4LogicalVolume(solidWorld, vacuum, "logicWorld");

		// phys
		G4VPhysicalVolume* physWorld = new G4PVPlacement(
			nullptr,
			G4ThreeVector(),
			logicWorld,
			"physWorld",
			nullptr,
			false,
			0
		);




		// place some volumes
		G4Box* solidObj = new G4Box("solidObj", 5 * cm, 5 * cm, 1 * mm);

		G4Material* Ti = nist->FindOrBuildMaterial("G4_Ti");
		flogicObj = new G4LogicalVolume(solidObj, Ti, "logicObj");

		new G4PVPlacement(
			nullptr,
			G4ThreeVector(0, 10 * cm, 5 * cm),
			flogicObj,
			"physObj",
			logicWorld,
			false,
			0
		);

		G4Box* solidObj2 = new G4Box("solidObj2", 3 * cm, 3 * cm, 3 * cm);

		G4LogicalVolume* logicObj2 = new G4LogicalVolume(solidObj2, Ti, "logicObj2");

		new G4PVPlacement(
			nullptr,
			G4ThreeVector(3 * cm, 0 * cm, -5 * cm),
			logicObj2,
			"physObj2",
			logicWorld,
			false,
			0
		);

    // testing different shapes
    G4Tubs* solidTub = new G4Tubs("solidTub", 3*cm, 5*cm, 10*cm, 0, 240*deg);

    G4LogicalVolume* logicTub = new G4LogicalVolume(solidTub, Ti, "logicTub");

    new G4PVPlacement(
      nullptr,
      G4ThreeVector(0, 0, 15*cm),
      logicTub,
      "physTub",
      logicWorld,
      false,
      0
    );

    // G4Sphere* solidSphere = new G4Sphere("solidSphere", 0, 30*cm, 0, 360*deg, 0, 360*deg);
    // G4LogicalVolume* logicSphere = new G4LogicalVolume(solidSphere, Ti, "logicSphere");
    // new G4PVPlacement(
    //   nullptr,
    //   G4ThreeVector(-10*cm, -10*cm, -10*cm),
    //   logicSphere,
    //   "physSphere",
    //   logicWorld,
    //   false,
    //   0
    // );

		return physWorld;
	}



	void DetectorConstruction::ConstructSDandField() {
		// create SD for the obj
		SensitiveDetector* SD = new SensitiveDetector("mySD", "myHC");
		SetSensitiveDetector(flogicObj, SD);

		G4SDManager::GetSDMpointer()->AddNewDetector(SD);
	}


} // !gps_test