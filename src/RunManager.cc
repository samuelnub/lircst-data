#include "RunManager.hh"

#include "G4VisManager.hh"
#include "G4UImanager.hh"
#include "G4SDManager.hh"

#include "G4TransportationManager.hh"

namespace lircst {
    RunManager::RunManager(long seedInstance) : G4MTRunManager(), fSeedInstance(seedInstance) {
        // Set number of threads
        SetNumberOfThreads(20);

        fMessenger = new G4GenericMessenger(this, "/lircst/runmanager/", "LircstData Run manager control");
        fMessenger->DeclareMethod("beamOn", &RunManager::BeamOn, "Beam on nEvents");
    }

    void RunManager::BeamOn(G4int nEvents) {
        ExecuteSimulations(1, nEvents);
    }

    void RunManager::ExecuteSimulations(G4int nRuns, G4int nEventsPerRun) {
        for (G4int i = 0; i < nRuns; i++) {
            SetRandomSeed(fSeedInstance + fRunsThisInstance);

            ResetRun();

            G4cout << "Starting run " << fRunsThisInstance << " with random seed " << GetRandomSeed() << G4endl;

            // Call base class BeamOn
            G4MTRunManager::BeamOn(nEventsPerRun);

            fRunsThisInstance++;
        }
    }

    void RunManager::ResetRun() {
        // Reset the dangling things that we need to reset before we do another run

        G4cout << "Resetting run" << G4endl;

        auto visManager = G4VisManager::GetConcreteInstance();
        visManager->GeometryHasChanged();

        //InitializeGeometry();
        GeometryHasBeenModified(); // Let the kernel know that the geometry has been modified
        ReinitializeGeometry(true); // Force geometry to be reinitialized
        Initialize();

        visManager->DrawGeometry(G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume());
    }
}