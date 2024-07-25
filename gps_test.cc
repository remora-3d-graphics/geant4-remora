#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4RunManagerFactory.hh"
#include "G4VisExecutive.hh"

#include "ActionInit.hh"
#include "DetectorConstruction.hh"
#include "PhysicsList.hh"

#include "cp_server.hh"

using namespace gps_test;
using namespace remora;

// TODO:
// 1. Get the Action Initialization, Physics list,
//	  Detector Construction, and Primary Generator Action
// 2. Get a vis.mac file going that opens an OGL viewer

int main(int argc, char** argv)
{

  // remora server:
  
  Server server;
	
  // Start (or don't) a UI
	G4UIExecutive* ui = nullptr;
	if (argc == 1) {
		ui = new G4UIExecutive(argc, argv);
	}

	// get pointer to UI manager
	G4UImanager* UImanager = G4UImanager::GetUIpointer();

	// create a runmanager
	auto runManager =
		G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);

	// set our three initializations
	runManager->SetUserInitialization(new PhysicsList());
	runManager->SetUserInitialization(new DetectorConstruction());
	runManager->SetUserInitialization(new ActionInit(&server));


	// include a vismanager
	G4VisExecutive* visExec = new G4VisExecutive;
	visExec->Initialize();
	

	// Run macro or start UI
	if (!ui) {
		// batch mode
		G4String command = "/control/execute ";
		G4String fileName = argv[1];
		UImanager->ApplyCommand(command + fileName);
	}
	else {

		// use UI
		ui->SessionStart();

		delete ui;
	}

	return 0;
}
