#include "ActionInitialisation.hh"

#include "G4MultiEventAction.hh"
#include "PrimaryGeneratorAction.cc"

namespace lircst {
    void ActionInitialisation::Build() const {
        // Just slap em all into a multi action in case we want to expand it in future
        //auto multiAction = new G4MultiEventAction();
        // multiAction->AddAction(new EventAction);
        //SetUserAction(multiAction);

        SetUserAction(new PrimaryGeneratorAction);
    }
}