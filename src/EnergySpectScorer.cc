#include "EnergySpectScorer.hh"
#include "G4Step.hh"

namespace lircst {
    EnergySpectScorer::EnergySpectScorer(G4String name, G4int nx, G4int ny, G4int nBins, G4double eMin, G4double eMax) : G4VPrimitiveScorer(name, nx * ny) {
        fNx = nx;
        fNy = ny;
        fNbins = nBins;
        fEMin = eMin;
        fEMax = eMax;
    }

    G4bool EnergySpectScorer::ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) {
        // Get energy deposited in this step
        G4double edep = aStep->GetTotalEnergyDeposit();
        if (edep == 0) return false;

        // Get pos of the step, and what pixel that corresponds to
        G4ThreeVector pos = aStep->GetPreStepPoint()->GetPosition();
        G4int i = static_cast<G4int>((pos.x() - x_min) / pixel_size_x);
        G4int j = static_cast<G4int>((pos.y() - y_min) / pixel_size_y);

        if (i < 0 || i >= fNx || j < 0 || j >= fNy) return false; // Out of bounds

        // Determine enregy bin
        
        
    }
}