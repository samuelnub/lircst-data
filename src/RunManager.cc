#include "RunManager.hh"

#include "G4VisManager.hh"
#include "G4UImanager.hh"
#include "G4SDManager.hh"
#include "G4VisManager.hh"
#include "DetectorConstruction.hh"
#include "GroundTruthExporter.hh"

#include "G4TransportationManager.hh"

namespace lircst {
    RunManager::RunManager(long seedInstance, G4int currentGantryIndex) : G4MTRunManager(), fSeedInstance(seedInstance), fCurrentGantryIndex(currentGantryIndex) {
        // Set number of threads
        SetNumberOfThreads(20);
        SetRandomSeed(fSeedInstance);

        fMessenger = new G4GenericMessenger(this, "/lircst/runmanager/", "LircstData Run manager control");
        fMessenger->DeclareMethod("beamOn", &RunManager::BeamOn, "Beam on nEvents");
        fMessenger->DeclareMethod("executeFullRotation", &RunManager::ExecuteFullRotation, "Execute a full rotation with nEventsPerTheta events per angle step");
    }

    void RunManager::BeamOn(G4int nEvents) {
        G4MTRunManager::BeamOn(nEvents);

        // Old
        // ExecuteSimulations(1, nEvents);
    }

    void RunManager::ExecuteSimulations(G4int nRuns, G4int nEventsPerRun) {
        // TODO: Somewhat outdated as we now want to run one full rotation instead of multiple runs at the same angle, but we can still use this for testing and stuff
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
        // Export ground truth phantom volume before starting the runs
        // TODO: bugged? GroundTruthExporter().ExportFullVolume();


        while (!IsFullRotationComplete()) {
            G4cout << "Starting rotational run at gantry angle " << GetCurrentGantryAngleRad() << " (index " << GetCurrentGantryIndex() << ") radians with random seed " << GetRandomSeed() << G4endl;
            
            // Set gantry angle in detector construction
            DetectorConstruction* detector = static_cast<DetectorConstruction*>(const_cast<G4VUserDetectorConstruction*>(GetUserDetectorConstruction()));
            detector->SetGantryAngle(GetCurrentGantryAngleRad());

            // Optional? 
            GeometryHasBeenModified();
            if(G4VisManager::GetConcreteInstance()) {
                G4VisManager::GetConcreteInstance()->GeometryHasChanged();
            }

            // Call base class BeamOn
            G4MTRunManager::BeamOn(nEventsPerTheta);

            // Remember to increment!
            IncrementGantryIndex();

            //ResetRun();
        }

        G4cout << "Completed full rotation!" << G4endl;
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