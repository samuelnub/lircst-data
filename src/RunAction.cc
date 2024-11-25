#include "RunAction.hh"

#include "G4Run.hh"
#include "G4ios.hh"
#include "G4SDManager.hh"
#include "G4THitsMap.hh"
#include "G4MultiFunctionalDetector.hh"

#include "EnergySpectScorer.hh"

namespace lircst {
    void RunAction::EndOfRunAction(const G4Run* run) {
        auto nofEvents = run->GetNumberOfEvent();
        if (IsMaster())
        {
            G4cout << "End of global run... about to see how our data is like" << G4endl;

            // Save to final reading after master run
            auto sdManager = G4SDManager::GetSDMpointer();
            G4int hcID = sdManager->GetCollectionID("mfd/ess");
            auto mfd = static_cast<G4MultiFunctionalDetector*>(sdManager->FindSensitiveDetector("mfd"));
            auto ess = static_cast<EnergySpectScorer*>(mfd->GetPrimitive(0));
            auto hitsMap = ess->GetHitsMap();

            for (int x = 0; x < Util::GetNumPixelsX(); x++)
            {
                for (int y = 0; y < Util::GetNumPixelsY(); y++)
                {
                    for (int bin = 0; bin < Util::GetNumBins(); bin++)
                    {
                        G4int key = Util::GenMapKey(x, y, bin);
                        G4double value = *((*hitsMap)[key]);
                        G4cout << "Pixel (" << x << ", " << y << ") Bin " << bin << " : " << value << G4endl;
                    }
                }
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
}