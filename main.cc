
#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "G4ios.hh"
#include "G4GeometrySampler.hh"
#include "G4ImportanceBiasing.hh"
#include "G4GeometryManager.hh"

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
        // runManager->SetNumberOfThreads(256);

        // Set must-have user init classes
        auto detector = new DetectorConstruction();
        runManager->SetUserInitialization(detector);
        // For importance biasing
        G4GeometrySampler geomSampler(detector->GetWorldVolume(), "gamma");
        auto physList = new PhysicsList();
        physList->RegisterPhysics(new G4ImportanceBiasing(&geomSampler));
        runManager->SetUserInitialization(physList);
        runManager->SetUserInitialization(new ActionInitialisation);

        // Init G4 kernel
        runManager->Initialize();

        // For importance biasing
        detector->CreateImportanceStore();

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
            int noOfEvents = 10000000; // 10 teeny tiny mil
            runManager->BeamOn(noOfEvents);
            G4cout << "End of run tee hee" << G4endl;
        }

        // For importance biasing clean-up
        G4GeometryManager::GetInstance()->OpenGeometry();

        // Terminate job
        delete visManager;
        delete runManager;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}