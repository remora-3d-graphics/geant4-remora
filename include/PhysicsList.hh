#ifndef GPS_TEST_PHYSICS_LIST_HH
#define GPS_TEST_PHYSICS_LIST_HH

#include "G4VModularPhysicsList.hh"
#include "G4EmStandardPhysics.hh"

namespace gps_test {
	class PhysicsList : public G4VModularPhysicsList {
	
	public:
		PhysicsList() : G4VModularPhysicsList() {
			RegisterPhysics(new G4EmStandardPhysics());
		};
	
	};


} // !gps_test

#endif // !GPS_TEST_PHYSICS_LIST_HH