#include "RunAction.hh"

#include "G4Run.hh"
#include "G4ios.hh"
#include "G4SDManager.hh"
#include "G4THitsMap.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4Event.hh"
#include "G4AccumulableManager.hh"

#include "EnergySpectScorer.hh"

namespace lircst {
    RunAction::RunAction() : G4UserRunAction() {
        G4AccumulableManager::Instance()->RegisterAccumulable(&fAccumulatedHitsMap);
        G4cout << "In RunAction constructor" << G4endl;
    }

    void RunAction::BeginOfRunAction(const G4Run* run) {
        if (IsMaster()) {
            // P.S. If you do multiple runs per session, we will accumulate over all runs
            G4cout << "Begin of global run action" << G4endl;
        }
    }

    void RunAction::EndOfRunAction(const G4Run* run) {
        auto nofEvents = run->GetNumberOfEvent();

        // Merge thread-local accumulables to one
        auto accumulableManager = G4AccumulableManager::Instance();
        accumulableManager->Merge();

        if (IsMaster())
        {
            G4cout << "End of global run... about to see how our data is like" << G4endl;

            // Save to final reading after master run
            const auto& finalAccumulableMap = static_cast<AccumulableMap*>(accumulableManager->GetAccumulable("AccumulatedHitsMap"));
            auto finalMap = finalAccumulableMap->GetMap();

            G4cout << "Final Hits Map Size: " << finalMap.size() << G4endl;

            for (const auto& entry : finalMap) {
                G4int index = entry.first;
                G4double value = entry.second;

                G4cout << "Index: " << index << ", Accumulated Value: " << value << G4endl;
            }

            G4cout
            << G4endl
            << "--------------------End of Global Run-----------------------"
            << G4endl
            << "  The run was " << nofEvents << " events" << G4endl;
        }
        else
        {
            G4cout
            << G4endl
            << "--------------------End of Local Run------------------------"
            << G4endl
            << "  The run was " << nofEvents << " events" << G4endl;
        }
    }

    void RunAction::AddHitsMap(G4THitsMap<G4double>* hitsMap) {
        for (auto iter = hitsMap->GetMap()->begin(); iter != hitsMap->GetMap()->end(); iter++) {
            G4int key = iter->first;
            G4double value = *(iter->second);
            fAccumulatedHitsMap.AddValue(key, value);
        }
    }
}