#ifndef EVENTACTION_HH
#define EVENTACTION_HH

#include "G4UserEventAction.hh"

namespace lircst {
    class EventAction : public G4UserEventAction {
    public:
        EventAction() = default;
        ~EventAction() override = default;

        void BeginOfEventAction(const G4Event*) override;
        void EndOfEventAction(const G4Event*) override;
    };
}

#endif // EVENTACTION_HH