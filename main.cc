
#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "G4ios.hh"

#include "PhysicsList.hh"
#include "DetectorConstruction.hh"
#include "ActionInitialisation.hh"

#include "G4FastSimulationPhysics.hh"
#include "CeleritasGlobals.hh"

using namespace lircst;

// Based on https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/GettingStarted/mainProgram.html
int main(int argc,char** argv) {
    try {
        G4UIExecutive* ui = nullptr;
        if ( argc > 1 ) { ui = new G4UIExecutive(argc, argv); }

        // Default run manager - manages flow of program, and event loop(s) in a run
        auto runManager = G4RunManagerFactory::CreateRunManager();
        runManager->SetNumberOfThreads(64);

        // Set must-have user init classes
        runManager->SetUserInitialization(new DetectorConstruction);

        // Celeritas physics for FastSimulationPhysics
        auto physicsList = new PhysicsList();
        auto fastPhysics = new G4FastSimulationPhysics();
        fastPhysics->ActivateFastSimulation("e-");
        fastPhysics->ActivateFastSimulation("e+");
        fastPhysics->ActivateFastSimulation("gamma");
        physicsList->RegisterPhysics(fastPhysics);
        runManager->SetUserInitialization(physicsList);

        runManager->SetUserInitialization(new ActionInitialisation);

        // Celeritas options
        CeleritasGlobals::setup_options.max_num_tracks = 1024;
        CeleritasGlobals::setup_options.initializer_capacity = 1024 * 128;


        // Init G4 kernel
        runManager->Initialize();

        auto visManager = new G4VisExecutive(argc, argv);
        visManager->Initialize();

        // UI manager pointer
        auto uiManager = G4UImanager::GetUIpointer();
        // Set verbosities for UI
        uiManager->ApplyCommand("/run/verbose 0");
        uiManager->ApplyCommand("/event/verbose 0");
        uiManager->ApplyCommand("/tracking/verbose 0");

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
            int noOfEvents = 10000000; // 1 Billion 10 mil actually lol
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