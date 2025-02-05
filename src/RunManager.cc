#include "RunManager.hh"

#include "G4VisManager.hh"
#include "G4UImanager.hh"

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

            ReinitializeGeometry(true); // Force geometry to be reinitialized
            Initialize();
            InitializeGeometry();
            GeometryHasBeenModified(); // Let the kernel know that the geometry has been modified

            G4cout << "Starting run " << fRunsThisInstance << " with random seed " << GetRandomSeed() << G4endl;

            // Call base class BeamOn
            G4MTRunManager::BeamOn(nEventsPerRun);

            fRunsThisInstance++;
        }
    }

}