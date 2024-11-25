#include "EventAction.hh"
#include "G4Event.hh"
#include "G4ios.hh"
#include "G4SDManager.hh"
#include "G4RunManager.hh"
#include "G4THitsMap.hh"

#include "RunAction.hh"

namespace lircst {
    void EventAction::BeginOfEventAction(const G4Event*) {
        // Do nothing
    }

    void EventAction::EndOfEventAction(const G4Event* event) {
        // Just some logging
        if (event->GetEventID() % 1000 == 0) {
            G4cout << "Event " << event->GetEventID() << " done" << G4endl;
        }

        // Important accumulating work

        auto hce = event->GetHCofThisEvent();
        if (!hce) return;

        auto hcID = G4SDManager::GetSDMpointer()->GetCollectionID("mfd/ess");
        auto hitsMap = static_cast<G4THitsMap<G4double>*>(hce->GetHC(hcID));
        if (!hitsMap) return;

        // So cursed
        auto runAction = static_cast<RunAction*>(const_cast<G4UserRunAction*>(G4RunManager::GetRunManager()->GetUserRunAction()));
        runAction->AddHitsMap(hitsMap);
    }
}