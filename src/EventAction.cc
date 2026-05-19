#include "EventAction.hh"
#include "G4Event.hh"
#include "G4ios.hh"
#include "G4SDManager.hh"
#include "G4MTRunManager.hh"
#include "G4THitsMap.hh"

#include "RunManager.hh"
#include "RunAction.hh"

namespace lircst {
    void EventAction::BeginOfEventAction(const G4Event* event) {
    }

    void EventAction::EndOfEventAction(const G4Event* event) {
        // Just some logging
        if (event->GetEventID() % 1000000 == 0) {
            G4cout << "Event " << event->GetEventID() << " done" << G4endl;
        }

        // Important accumulating work

        auto hce = event->GetHCofThisEvent();
        if (!hce) return;

        auto hcID = G4SDManager::GetSDMpointer()->GetCollectionID("mfd/ess");
        auto hitsMap = static_cast<G4THitsMap<G4double>*>(hce->GetHC(hcID));
        if (!hitsMap) return;

        // So cursed
        auto runManager = static_cast<RunManager*>(G4MTRunManager::GetRunManager()); // We might want the worker run manager as opposed to the master one here
        auto runAction = static_cast<RunAction*>(const_cast<G4UserRunAction*>(runManager->GetUserRunAction()));

        runAction->AddHitsMap(hitsMap);
    }
}