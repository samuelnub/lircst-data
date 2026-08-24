#include "ActionInitialisation.hh"

#include "PrimaryGeneratorAction.hh"
#include "EventAction.hh"
#include "RunAction.hh"
#include "StackingAction.hh"
#include "SteppingAction.hh"

namespace lircst {
    void ActionInitialisation::BuildForMaster() const {
        SetUserAction(new RunAction);
    }

    void ActionInitialisation::Build() const {
        SetUserAction(new RunAction);
        SetUserAction(new EventAction);
        SetUserAction(new PrimaryGeneratorAction);
        SetUserAction(new StackingAction);
        SetUserAction(new SteppingAction);
    }
}