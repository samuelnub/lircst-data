
#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "G4ios.hh"

#include "PhysicsList.hh"
#include "DetectorConstruction.hh"
#include "ActionInitialisation.hh"

using namespace lircst;

// Based on https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/GettingStarted/mainProgram.html
int main(int argc,char** argv) {
    try {
        G4UIExecutive* ui = nullptr;
        if ( argc > 1 ) { ui = new G4UIExecutive(argc, argv); }

        // Default run manager - manages flow of program, and event loop(s) in a run
        auto runManager = G4RunManagerFactory::CreateRunManager();

        // Set must-have user init classes
        runManager->SetUserInitialization(new DetectorConstruction);
        runManager->SetUserInitialization(new PhysicsList);
        runManager->SetUserInitialization(new ActionInitialisation);

        // Init G4 kernel
        runManager->Initialize();

        auto visManager = new G4VisExecutive(argc, argv);
        visManager->Initialize();

        // UI manager pointer
        auto uiManager = G4UImanager::GetUIpointer();
        // Set verbosities for UI
        uiManager->ApplyCommand("/run/verbose 1");
        uiManager->ApplyCommand("/event/verbose 1");
        uiManager->ApplyCommand("/tracking/verbose 1");

        if (argc > 1) {
            // Assume it's a vis ui session
            //auto ui = new G4UIExecutive(argc, argv);
            G4String command = "/control/execute ";
            G4String fileName = argv[1];
            uiManager->ApplyCommand(command+fileName);
            ui->SessionStart();
            delete ui;
        }

        if (argc == 1) {
            // Start a run if not in vis ui session
            int noOfEvents = 100000;
            runManager->BeamOn(noOfEvents);
            G4cout << "End of run tee hee" << G4endl;
        }

        // Terminate job
        delete visManager;
        delete runManager;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}