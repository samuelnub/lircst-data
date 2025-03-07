
#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "G4ios.hh"
#include "G4GeometrySampler.hh"
#include "G4ImportanceBiasing.hh"
#include "G4GeometryManager.hh"
#include "G4GenericBiasingPhysics.hh"

#include "RunManager.hh"
#include "PhysicsList.hh"
#include "DetectorConstruction.hh"
#include "ActionInitialisation.hh"
#include "ParallelWorldConstruction.hh"

#include <chrono>

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "Util.hh"

using namespace lircst;

// Based on https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/GettingStarted/mainProgram.html
int main(int argc,char** argv) {
    // Install a segfault handler
    signal(SIGSEGV, [](int sig) {
        void *array[10];
        size_t size = backtrace(array, 10);
        fprintf(stderr, "Error: signal %d:\n", sig);
        backtrace_symbols_fd(array, size, STDERR_FILENO);
        exit(1);
    });

    try {
        G4UIExecutive* ui = nullptr;
        if ( argc > 1 ) { ui = new G4UIExecutive(argc, argv); }

        // Our run manager - manages flow of program, and event loop(s) in a run
        auto runManager = new RunManager(std::chrono::system_clock::now().time_since_epoch().count());

        // Set must-have user init classes
        auto detector = new DetectorConstruction();

        if(Util::GetEnableSolidAngleBiasing()) {
            auto parallelWorld = new ParallelWorldConstruction("ParaWorld"); // TODO: for biasing
            detector->RegisterParallelWorld(parallelWorld); // TODO: for biasing
        }
        runManager->SetUserInitialization(detector);

        auto physList = new PhysicsList();

        if(Util::GetEnableSolidAngleBiasing()) {
            auto biasingPhysics = new G4GenericBiasingPhysics();
            biasingPhysics->BeVerbose();
            biasingPhysics->NonPhysicsBias("gamma");
            biasingPhysics->AddParallelGeometry("gamma", "ParaWorld");
            physList->RegisterPhysics(biasingPhysics); // TODO: for biasing
        }
        runManager->SetUserInitialization(physList);

        runManager->SetUserInitialization(new ActionInitialisation);

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

            auto timestampStart = (unsigned long)time(NULL);

            int noOfEvents = 1000000000; // 1bil
            // Our version of beamOn
            runManager->ExecuteSimulations(100, noOfEvents);

            auto timestampEnd = (unsigned long)time(NULL);

            G4cout << "End of runs tee hee, took " << timestampEnd - timestampStart << " seconds" << G4endl;
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