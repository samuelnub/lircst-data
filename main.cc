
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
#include "WorkerInitialization.hh"

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
        auto visMacro = "vis.mac";

        G4UIExecutive* ui = nullptr;
        if (argc > 1 && strcmp(argv[1], visMacro) == 0 ) { ui = new G4UIExecutive(argc, argv); }

        auto seed = std::chrono::system_clock::now().time_since_epoch().count();
        auto gantryIndex = 0;

        if (argc > 1 && strcmp(argv[1], visMacro) != 0) {
            assert(argc == 5 && "Usage: ./lircstData <seed> <gantryIndex> <noOfEvents> <isGeneratePhan>");
            seed = atoi(argv[1]);

            gantryIndex = atoi(argv[2]);
        }

        // Our run manager - manages flow of program, and event loop(s) in a run
        auto runManager = new RunManager(seed, gantryIndex);
        //runManager->SetUserInitialization(new WorkerInitialization); // For worker thread setup before each run 

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

        if (argc > 1 && strcmp(argv[1], visMacro) == 0) {
            // Assume it's a vis ui session
            //auto ui = new G4UIExecutive(argc, argv);
            G4String command = "/control/execute ";
            G4String fileName = argv[1];
            uiManager->ApplyCommand(command+fileName);
            ui->SessionStart();
            delete ui;
        }

        if (argc == 1) {
            // Start a regular run if not in vis ui session

            auto timestampStart = (unsigned long)time(NULL);

            int noOfEvents = 1*pow(10, 8);
            // Our version of beamOn
            runManager->ExecuteFullRotation(noOfEvents, 0);

            auto timestampEnd = (unsigned long)time(NULL);

            G4cout << "End of run(s) tee hee, took " << timestampEnd - timestampStart << " seconds" << G4endl;
        }

        // Our jobbed version
        if (argc > 1 && argv[1] != visMacro) {

            assert(argc == 5 && "Usage: ./lircstData <seed> <gantryIndex> <noOfEvents> <isGeneratePhan>");

            // Seed already set above...

            int noOfEvents = atoi(argv[3]);
            bool isGeneratePhan = argv[4] == std::string("true") || argv[4] == std::string("1");

            // For this jobbed version, we just execute one projection at the specified gantry index
            runManager->BeamOn(noOfEvents, isGeneratePhan); // Export ground truth only once for all projections, and update geometry to the specified gantry angle
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