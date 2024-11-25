#include "Run.hh"

#include "G4Event.hh"
#include "G4SDManager.hh"
#include "G4THitsMap.hh"
#include "EnergySpectScorer.hh"

namespace lircst {
    void Run::RecordEvent(const G4Event* event) {
        // Get hits from event

        G4SDManager* sdManager = G4SDManager::GetSDMpointer();

        auto colID = sdManager->GetCollectionID("mfd/ess");
        auto hitsMap = static_cast<G4THitsMap<G4double>*>(event->GetHCofThisEvent()->GetHC(colID));

        // Loop over hits and accumulate
        for (auto it = hitsMap->GetMap()->begin(); it != hitsMap->GetMap()->end(); ++it) {
            auto key = it->first;
            auto energy = *(it->second);
            
            
        }

        G4Run::RecordEvent(event); // Invoke base class method
    }

    void Run::Merge(const G4Run* aRun) {
        

        G4Run::Merge(aRun); // Invoke base class method
    }
}