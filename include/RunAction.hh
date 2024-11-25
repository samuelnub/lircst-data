#ifndef RUNACTION_HH
#define RUNACTION_HH

#include "G4UserRunAction.hh"

namespace lircst {
    class RunAction : public G4UserRunAction {
    public:
        RunAction() = default;
        ~RunAction() override = default;

        void EndOfRunAction(const G4Run* run) override;
    };
}


#endif // RUNACTION_HH