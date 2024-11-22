#ifndef ENERGYSPRECTSCORER_HH
#define ENERGYSPRECTSCORER_HH

#include "G4VPrimitiveScorer.hh"
#include "G4THitsMap.hh"
#include "G4SystemOfUnits.hh"

namespace lircst {
    class EnergySpectScorer : public G4VPrimitiveScorer {
    public:
        EnergySpectScorer(G4String name, G4int nx, G4int ny, G4int nBins, G4double eMin, G4double eMax);
        ~EnergySpectScorer() override = default;

        void Initialize(G4HCofThisEvent* HCE) override;
        G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override;
        void EndOfEvent(G4HCofThisEvent* HCE) override;

    private:
        G4int fNx, fNy, fNbins;    // Pixel grid and energy bins
        G4double fEMin, fEMax;     // Energy range
        const G4double x_min = -50.0 * mm, y_min = -50.0 * mm;
        const G4double pixel_size_x = 1.0 * mm, pixel_size_y = 1.0 * mm;

        G4THitsMap<G4double>* fHitsMap = nullptr;
    };
}

#endif // ENERGYSPRECTSCORER_HH