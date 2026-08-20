#ifndef STEPPINGACTION_HH
#define STEPPINGACTION_HH

#include "G4UserSteppingAction.hh"
#include "G4Step.hh"

namespace lircst {
    class SteppingAction : public G4UserSteppingAction {
    public:
        SteppingAction();
        virtual ~SteppingAction();
        virtual void UserSteppingAction(const G4Step* step);
    };
}

#endif