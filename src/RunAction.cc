#include "RunAction.hh"

#include "G4Run.hh"
#include "G4ios.hh"
#include "G4SDManager.hh"
#include "G4THitsMap.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4Event.hh"
#include "G4AccumulableManager.hh"

#include "EnergySpectScorer.hh"

#include <typeinfo> // TODO: for debugging

namespace lircst {
    RunAction::RunAction() : G4UserRunAction() {
        G4AccumulableManager::Instance()->RegisterAccumulable(fAccumulatedHitsMap);
    }

    void RunAction::BeginOfRunAction(const G4Run* run) {
        G4AccumulableManager::Instance()->Reset();

        G4cout << "Begin of run action: Just reset accumulable" << G4endl;
    }

    void RunAction::EndOfRunAction(const G4Run* run) {
        auto nofEvents = run->GetNumberOfEvent();
        if (IsMaster())
        {
            G4cout << "End of global run... about to see how our data is like" << G4endl;

            // Save to final reading after master run
            const auto& finalHitsMap = fAccumulatedHitsMap.GetValue().GetMap();

            for (const auto& entry : finalHitsMap) {
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
            fAccumulatedHitsMap.GetValue().AddValue(key, value);
        }
    }
}