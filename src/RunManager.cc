#include "RunManager.hh"

#include "G4VisManager.hh"
#include "G4UImanager.hh"
#include "G4SDManager.hh"
#include "G4VisManager.hh"
#include "DetectorConstruction.hh"
#include "GroundTruthExporter.hh"
#include "G4Timer.hh"

#include "G4TransportationManager.hh"

namespace lircst {
    RunManager::RunManager(long seedInstance, G4int currentGantryIndex) : G4MTRunManager(), fSeedInstance(seedInstance), fCurrentGantryIndex(currentGantryIndex) {
        // Set number of threads
        SetNumberOfThreads(20);
        SetRandomSeed(fSeedInstance);

        fMessenger = new G4GenericMessenger(this, "/lircst/runmanager/", "LircstData Run manager control");
        fMessenger->DeclareMethod("beamOn", &RunManager::BeamOn, "Beam on nEvents");
        fMessenger->DeclareMethod("executeFullRotation", &RunManager::ExecuteFullRotation, "Execute a full rotation with nEventsPerTheta events per angle step");
        fMessenger->DeclareMethod("setGantryAngleByIndex", &RunManager::SetGantryAngleByIndex, "Set gantry angle by index");
    }

    void RunManager::BeamOn(G4int nEvents) {
        // Profile timing
        G4Timer timer;
        timer.Start();

        G4MTRunManager::BeamOn(nEvents);

        timer.Stop();
        G4cout << "BeamOn completed in " << timer.GetRealElapsed() << " seconds" << G4endl;
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

    void RunManager::SetGantryAngleByIndex(G4int index, G4bool updateGeom) {
        fCurrentGantryIndex = index;
        if(updateGeom) {
            DetectorConstruction* detector = static_cast<DetectorConstruction*>(const_cast<G4VUserDetectorConstruction*>(GetUserDetectorConstruction()));
            detector->SetGantryAngle(GetCurrentGantryAngleRad());
            //ReinitializeGeometry(true); // Force geometry to be reinitialized // Not needed: https://geant4.web.cern.ch/documentation/pipelines/master/bfad_html/ForApplicationDevelopers/Detector/Geometry/geomDynamic.html
            GeometryHasBeenModified();
            if(G4VisManager::GetConcreteInstance()) {
                G4VisManager::GetConcreteInstance()->GeometryHasChanged();
            }
        }
    }

    void RunManager::ExecuteFullRotation(G4int nEventsPerTheta, G4int startGantryIndex) {
        // Export ground truth phantom volume before starting the runs
        GroundTruthExporter().ExportFullVolume();

        // Set gantry angle to start index
        if (fCurrentGantryIndex != startGantryIndex) {
            G4cout << "Setting gantry angle to start index " << startGantryIndex << " (current index: " << fCurrentGantryIndex << ")" << G4endl;
            SetGantryAngleByIndex(startGantryIndex, true);
        }

        while (!IsFullRotationComplete()) {
            G4cout << "Starting rotational run at gantry angle " << GetCurrentGantryAngleRad() << " (index " << GetCurrentGantryIndex() << ") radians with random seed " << GetRandomSeed() << G4endl;
            
            // Call base class BeamOn
            this->BeamOn(nEventsPerTheta);

            // Set up for next run
            SetGantryAngleByIndex(GetCurrentGantryIndex() + 1, true);

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