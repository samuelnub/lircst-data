#include "SteppingAction.hh"
#include "G4VSensitiveDetector.hh"

namespace lircst {
    SteppingAction::SteppingAction() {}
    SteppingAction::~SteppingAction() {}
    void SteppingAction::UserSteppingAction(const G4Step* step) {
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
    }
}