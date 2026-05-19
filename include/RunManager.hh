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
        RunManager(long seedInstance, G4int currentGantryIndex=0);
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
        G4double GetCurrentGantryAngleRad() {
            G4double angle = minGantryAngle + fCurrentGantryIndex * (maxGantryAngle - minGantryAngle) / nThetaSteps;
            return angle;
        }

        G4int GetCurrentGantryIndex() {
            return fCurrentGantryIndex;
        }

        void IncrementGantryIndex() {
            fCurrentGantryIndex = (fCurrentGantryIndex + 1); //% nThetaSteps;
        }

        G4bool IsFullRotationComplete() {
            return GetCurrentGantryAngleRad() >= maxGantryAngle;
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

        const G4double minGantryAngle = 0.0 * rad;
        const G4double maxGantryAngle = CLHEP::pi * rad; // 180 degrees in radians
        const G4int nThetaSteps = 200;
        G4int fCurrentGantryIndex = 0;
    };
}


#endif // RUNMANAGER_HH