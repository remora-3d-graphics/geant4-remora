#include "DetectorConstruction.hh"

#include "G4SystemOfUnits.hh"
#include "G4NistManager.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

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
			G4ThreeVector(0, 0, 5 * cm),
			flogicObj,
			"physObj",
			logicWorld,
			false,
			0
		);

    // a few more boxes for fun
    G4Box* solidBox2 = new G4Box("solidBox2", 10*cm, 10*cm, 10*cm);
    G4LogicalVolume* logicBox2 = new G4LogicalVolume(solidBox2, Ti, "logicBox2");
    new G4PVPlacement(
      nullptr,
      G4ThreeVector(5*cm, 5*cm, -5*cm),
      logicBox2,
      "physBox2",
      logicWorld,
      false,
      0
    );
    G4Box* solidBox3 = new G4Box("solidBox3", 5*cm, 10*cm, 13*cm);
    G4LogicalVolume* logicBox3 = new G4LogicalVolume(solidBox3, Ti, "logicBox3");
    new G4PVPlacement(
      nullptr,
      G4ThreeVector(5*cm, 5*cm, -15*cm),
      logicBox3,
      "physBox3",
      logicWorld,
      false,
      0
    );

		return physWorld;
	}



	void DetectorConstruction::ConstructSDandField() {
		// create SD for the obj
		SensitiveDetector* SD = new SensitiveDetector("mySD", "myHC");
		SetSensitiveDetector(flogicObj, SD);

		G4SDManager::GetSDMpointer()->AddNewDetector(SD);
	}


} // !gps_test