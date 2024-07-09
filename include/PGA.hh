#ifndef GPS_TEST_PGA_HH
#define GPS_TEST_PGA_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"

#include "G4GeneralParticleSource.hh"

namespace gps_test {

	class PGA : public G4VUserPrimaryGeneratorAction {
	public:
		PGA() {
			gps = new G4GeneralParticleSource();
		};

		void GeneratePrimaries(G4Event* anEvent) override {
			gps->GeneratePrimaryVertex(anEvent);
		};

	private:
		G4GeneralParticleSource* gps;
	};

} // !gps_test

#endif // !GPS_TEST_PGA_HH
