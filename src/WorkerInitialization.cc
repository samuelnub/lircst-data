#include "WorkerInitialization.hh"

#include "G4MTRunManager.hh"
#include "G4Threading.hh"

#include "RunManager.hh"
#include "DetectorConstruction.hh"

namespace lircst {
    void WorkerInitialization::WorkerRunStart() const {
        // Called before event loop, where Geometry and Physics are already set up (idle state in sequential mode)

        // We're really just here to make sure that the Sensitive Detector gets set up before each full rotation starts
        //auto runManager = static_cast<RunManager*>(G4MTRunManager::GetRunManager());
        //auto detectorConstruction = static_cast<DetectorConstruction*>(const_cast<G4VUserDetectorConstruction*>(runManager->GetUserDetectorConstruction()));
        // Apparently setting this up here is too early
        //detectorConstruction->ConstructSDandField();

        G4cout << "Worker thread " << G4Threading::G4GetThreadId() << " is about to start run!" << G4endl;
    }
}