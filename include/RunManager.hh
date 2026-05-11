#ifndef RUNMANAGER_HH
#define RUNMANAGER_HH

#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
#include "Randomize.hh"
#include "G4GenericMessenger.hh"
#include "G4SystemOfUnits.hh"

namespace lircst {
    class RunManager : public G4MTRunManager { // We going multi-threaded
    public:
        RunManager(long seedInstance);
        ~RunManager() override = default;

        // We treat this as an ExecuteSimulations but just 1 run
        void BeamOn(G4int nEvents);

        void ExecuteSimulations(G4int nRuns, G4int nEventsPerRun);

        void ExecuteFullRotation(G4int nEventsPerTheta);

        void ResetRun();

        // This does NOT change per run - constant throughout the program lifetime
        long GetSeedInstance() {
            return fSeedInstance;
        }

        long GetRunsThisInstance() {
            return fRunsThisInstance;
        }

        G4double GetMinGantryAngle() {
            return minGantryAngle;
        }
        G4double GetMaxGantryAngle() {
            return maxGantryAngle;
        }
        G4int GetNThetaSteps() {
            return nThetaSteps;
        }
        G4double GetCurrentGantryAngle() {
            return fCurrentGantryAngle;
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

        const G4double minGantryAngle = 0 * deg;
        const G4double maxGantryAngle = 180 * deg;
        const G4int nThetaSteps = 200;
        G4double fCurrentGantryAngle = minGantryAngle;
    };
}


#endif // RUNMANAGER_HH