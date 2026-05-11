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



            G4cout << "Starting run " << fRunsThisInstance << " with random seed " << GetRandomSeed() << G4endl;

            // Call base class BeamOn
            G4MTRunManager::BeamOn(nEventsPerRun);

            ResetRun();
            
            fRunsThisInstance++;
        }
    }

    void RunManager::ExecuteFullRotation(G4int nEventsPerTheta) {
        G4double deltaTheta = (maxGantryAngle - minGantryAngle) / nThetaSteps;

        
    }

    void RunManager::ResetRun() {
        // Reset the dangling things that we need to reset before we do another run

        G4cout << "Resetting run" << G4endl;


        // https://geant4.web.cern.ch/documentation/dev/bfad_html/ForApplicationDevelopers/Fundamentals/run.html#changing-the-detector-geometry

        //InitializeGeometry();
        // (already called when ReinitGeom is called) GeometryHasBeenModified(); // Let the kernel know that the geometry has been modified
        ReinitializeGeometry(true); // Force geometry to be reinitialized
        // Initialize();
    }
}