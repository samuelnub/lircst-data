#ifndef ACTIONINITIALISATION_HH
#define ACTIONINITIALISATION_HH

#include "G4VUserActionInitialization.hh"

namespace lircst {
    class ActionInitialisation : public G4VUserActionInitialization {
    public:
        ActionInitialisation() = default;
        ~ActionInitialisation() override = default;

        void Build() const override;
        void BuildForMaster() const override;
        G4VSteppingVerbose* InitializeSteppingVerbose() const override;
    };
}

#endif // ACTIONINITIALISATION_HH