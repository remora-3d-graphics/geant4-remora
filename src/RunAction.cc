#include "RunAction.hh"

namespace gps_test {

	RunAction::RunAction() {
		// Create analysis h2
		auto analysisManager = G4AnalysisManager::Instance();
		analysisManager->CreateH2(
			"Intensity", "Intensity", 100, -6 * mm, 6 * mm, 100, -5 * mm, 5 * mm, "mm", "mm");
		G4cout << "Run Action Initialized!" << G4endl;
	}

	void RunAction::BeginOfRunAction(const G4Run* aRun) {
		// open analysis file
		auto analysisManager = G4AnalysisManager::Instance();
		analysisManager->OpenFile("Output.root");
	}

	void RunAction::EndOfRunAction(const G4Run* aRun){
		auto analysisManager = G4AnalysisManager::Instance();
		analysisManager->Write();
		analysisManager->CloseFile();
	}
}