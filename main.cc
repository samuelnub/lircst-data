
#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"

#include "PhysicsList.hh"
#include "DetectorConstruction.hh"
#include "ActionInitialisation.hh"

using namespace lircst;

// Based on https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/GettingStarted/mainProgram.html
int main() {
    // Default run manager - manages flow of program, and event loop(s) in a run
    auto runManager = G4RunManagerFactory::CreateRunManager();

    // Set must-have user init classes
    runManager->SetUserInitialization(new DetectorConstruction);
    runManager->SetUserInitialization(new PhysicsList);
    runManager->SetUserInitialization(new ActionInitialisation);

    // Init G4 kernel
    runManager->Initialize();

    // UI manager pointer
    auto uiManager = G4UImanager::GetUIpointer();
    // Set verbosities for UI
    uiManager->ApplyCommand("/run/verbose 1");
    uiManager->ApplyCommand("/event/verbose 1");
    uiManager->ApplyCommand("/tracking/verbose 1");

    // Start a run
    int noOfEvents = 10000;
    runManager->BeamOn(noOfEvents);

    // Terminate job
    delete runManager;
    return 0;
}