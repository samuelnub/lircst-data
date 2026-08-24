#include "SteppingAction.hh"
#include "G4VSensitiveDetector.hh"

#include "G4GeometryManager.hh"
#include "DetectorConstruction.hh"
#include "G4RunManager.hh"
#include "G4VUserDetectorConstruction.hh"

namespace lircst {
    SteppingAction::SteppingAction() {}
    SteppingAction::~SteppingAction() {}
    void SteppingAction::UserSteppingAction(const G4Step* step) {
        /*
        auto* prePV = step->GetPreStepPoint()
                            ->GetTouchableHandle()
                            ->GetVolume();

                            
        auto* postPV = step->GetPostStepPoint()
                            ->GetTouchableHandle()
                            ->GetVolume();
        
        auto* preLV = prePV ? prePV->GetLogicalVolume() : nullptr;
        auto* postLV = postPV ? postPV->GetLogicalVolume() : nullptr;

        G4cout
            << "STEP"
            << "\n  length = " << step->GetStepLength()
            << "\n  pre  PV = " << (prePV ? prePV->GetName() : "NULL")
            << "\n  pre  LV = " << (preLV ? preLV->GetName() : "NULL")
            << "\n  pre  SD = " << (preLV && preLV->GetSensitiveDetector()
                                    ? preLV->GetSensitiveDetector()->GetName()
                                    : "NULL")
            << "\n  post PV = " << (postPV ? postPV->GetName() : "NULL")
            << "\n  post LV = " << (postLV ? postLV->GetName() : "NULL")
            << "\n  post SD = " << (postLV && postLV->GetSensitiveDetector()
                                    ? postLV->GetSensitiveDetector()->GetName()
                                    : "NULL")
            << G4endl;
        */

        const auto* post = step->GetPostStepPoint();

        if(post->GetStepStatus() != fGeomBoundary) {
            return;
        }

        auto* detectorConstruction = static_cast<DetectorConstruction*>(const_cast<G4VUserDetectorConstruction*>(G4RunManager::GetRunManager()->GetUserDetectorConstruction()));
        auto* cullingVolume = detectorConstruction->GetCullingVolume();
        if(!cullingVolume) {
            G4cout << "Culling volume is NULL!" << G4endl;
            return;
        }
        if(post->GetPhysicalVolume() == cullingVolume) {
            // G4cout << "Particle entered culling volume, killing it!" << G4endl;
            step->GetTrack()->SetTrackStatus(fStopAndKill);
        }
    }
}