#include "ActionInitialisation.hh"

#include "PrimaryGeneratorAction.hh"

namespace lircst {
    void ActionInitialisation::Build() const {
        this->SetUserAction(new PrimaryGeneratorAction);
    }
}