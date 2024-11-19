#include "EventAction.hh"
#include "G4Event.hh"
#include "G4ios.hh"

namespace lircst {
    void EventAction::BeginOfEventAction(const G4Event*) {
        // Do nothing
    }

    void EventAction::EndOfEventAction(const G4Event* event) {
        // Just some logging
        if (event->GetEventID() % 1000 == 0) {
            G4cout << "Event " << event->GetEventID() << " done" << G4endl;
        }
    }
}