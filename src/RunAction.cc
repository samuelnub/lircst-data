#include "RunAction.hh"

#include "G4Run.hh"
#include "G4ios.hh"

namespace lircst {
    void RunAction::EndOfRunAction(const G4Run* run) {


        
        auto nofEvents = run->GetNumberOfEvent();
        if (IsMaster())
        {
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