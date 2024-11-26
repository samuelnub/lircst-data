#include "EnergySpectScorer.hh"
#include "G4Step.hh"
#include "G4ios.hh"

#include "Util.hh"

namespace lircst {
    EnergySpectScorer::EnergySpectScorer(G4String name, G4int nx, G4int ny, G4int nBins, G4double eMin, G4double eMax) : G4VPrimitiveScorer(name) {
        fNx = nx;
        fNy = ny;
        fNbins = nBins;
        fEMin = eMin;
        fEMax = eMax;
    }

    void EnergySpectScorer::Initialize(G4HCofThisEvent* hce) {
        // Code inspired from https://github.com/Geant4/geant4/blob/e58e650b32b961c8093f3dd6a2c3bc917b2552be/source/digits_hits/scorer/src/G4PSEnergyDeposit.cc
        fHitsMap = new G4THitsMap<G4double>(GetMultiFunctionalDetector()->GetName(), GetName());
        G4int hcID = GetCollectionID(0);
        hce->AddHitsCollection(hcID, fHitsMap);
    }

    G4bool EnergySpectScorer::ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) {
        // Get energy deposited in this step
        G4double edep = aStep->GetTotalEnergyDeposit();
        if (edep == 0) return false;

        // Get pos of the step, and what pixel that corresponds to
        // Get local pos - local to touchable!
        auto touchable = aStep->GetPostStepPoint()->GetTouchable();
        G4ThreeVector worldPos = aStep->GetPostStepPoint()->GetPosition();
        G4ThreeVector pos = touchable->GetHistory()->GetTopTransform().TransformPoint(worldPos);

        G4int i = static_cast<G4int>((pos.x() - x_min) / pixel_size_x);
        G4int j = static_cast<G4int>((pos.y() - y_min) / pixel_size_y);
        if (i < 0 || i >= fNx || j < 0 || j >= fNy) return false; // Out of bounds

        // Determine enregy bin
        G4int bin = static_cast<G4int>((edep-fEMin) / (fEMax-fEMin) * fNbins);
        if (bin < 0 || bin >= fNbins) return false; // Out of bounds

        // Gen unique key for pixel and bin combination
        G4int key = Util::GenMapKey(i, j, bin);

        // Accumulate energy deposit in this pixel and bin
        G4double increment = 1.0f;
        fHitsMap->add(key, increment); 

        G4cout << "(i, j, bin, key, edep): " << i << ", " << j << ", " << bin << ", " << key << ", " << edep << G4endl;

        return true;
    }
}