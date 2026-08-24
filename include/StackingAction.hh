#ifndef STACKINGACTION_HH
#define STACKINGACTION_HH

#include "G4UserStackingAction.hh"

namespace lircst {
    class StackingAction : public G4UserStackingAction {
    public:
        StackingAction() = default;
        ~StackingAction() override = default;

        G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* aTrack) override;
    };
}

#endif // STACKINGACTION_HH