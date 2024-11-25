#ifndef RUNACTION_HH
#define RUNACTION_HH

#include "G4UserRunAction.hh"
#include "G4Accumulable.hh"
#include "G4THitsMap.hh"
#include "AccumulableMap.hh"

namespace lircst {
    class RunAction : public G4UserRunAction {
    public:
        RunAction();
        ~RunAction() override = default;

        void BeginOfRunAction(const G4Run* run) override;
        void EndOfRunAction(const G4Run* run) override;
        void AddHitsMap(G4THitsMap<G4double>* hitsMap);

    private:
        G4Accumulable<AccumulableMap> fAccumulatedHitsMap = AccumulableMap();
    };
}


#endif // RUNACTION_HH