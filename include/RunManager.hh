#ifndef RUNMANAGER_HH
#define RUNMANAGER_HH

#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
#include "Randomize.hh"
#include "G4GenericMessenger.hh"

namespace lircst {
    class RunManager : public G4MTRunManager { // We going multi-threaded
    public:
        RunManager(long seedInstance);
        ~RunManager() override = default;

        // We treat this as an ExecuteSimulations but just 1 run
        void BeamOn(G4int nEvents);

        void ExecuteSimulations(G4int nRuns, G4int nEventsPerRun);

        void ResetRun();

        // This does NOT change per run - constant throughout the program lifetime
        long GetSeedInstance() {
            return fSeedInstance;
        }

        long GetRunsThisInstance() {
            return fRunsThisInstance;
        }

    private:
        void SetRandomSeed(long seed) {
            G4Random::setTheSeed(seed);
        }

        long GetRandomSeed() {
            return G4Random::getTheSeed();
        }

        const long fSeedInstance;
        G4int fRunsThisInstance = 0;
        G4GenericMessenger* fMessenger;
    };
}


#endif // RUNMANAGER_HH