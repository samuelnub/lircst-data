#ifndef ENERGYSPRECTSCORER_HH
#define ENERGYSPRECTSCORER_HH

#include "G4VPrimitiveScorer.hh"
#include "G4THitsMap.hh"
#include "G4SystemOfUnits.hh"

#include "Util.hh"

namespace lircst {
    enum ScorerType {
        Pinhole = 0,
        Parallel = 1,
    };

    class EnergySpectScorer : public G4VPrimitiveScorer {
    public:
        EnergySpectScorer(G4String name, G4int nx, G4int ny, G4int nBins, G4double eMin, G4double eMax);
        ~EnergySpectScorer() override = default;

        void Initialize(G4HCofThisEvent* hce) override;
        G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override;

        G4THitsMap<G4double>* GetHitsMap() { return fHitsMap; }

    private:
        ScorerType fScorerType = ScorerType::Parallel;

        G4ThreeVector fParallelCollNormal = G4ThreeVector(0, 1, 0);

        G4int fNx, fNy, fNbins;    // Pixel grid and energy bins
        G4double fEMin, fEMax;     // Energy range
        const G4double x_min = -(Util::GetScorerSize() / 2), y_min = -(Util::GetScorerSize() / 2);
        const G4double pixel_size_x = Util::GetScorerSize() / Util::GetNumPixelsX(), pixel_size_y = Util::GetScorerSize() / Util::GetNumPixelsY();

        G4THitsMap<G4double>* fHitsMap = nullptr;

        G4ThreeVector fCollPosition;
        G4double fCollTolerance;
    };
}

#endif // ENERGYSPRECTSCORER_HH