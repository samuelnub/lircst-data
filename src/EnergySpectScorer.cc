#include "EnergySpectScorer.hh"
#include "G4Step.hh"
#include "G4ios.hh"

#include "RunManager.hh"
#include "Util.hh"

namespace lircst {
    EnergySpectScorer::EnergySpectScorer(G4String name, G4int nx, G4int ny, G4int nBins, G4double eMin, G4double eMax) : G4VPrimitiveScorer(name) {
        G4cout << "Constructing EnergySpectScorer with name: " << name << " nx: " << nx << " ny: " << ny << " nBins: " << nBins << " eMin: " << eMin << " eMax: " << eMax << G4endl;
        
        fNx = nx;
        fNy = ny;
        fNbins = nBins;
        fEMin = eMin;
        fEMax = eMax;

        fCollTolerance = std::cos(Util::GetCollCosAcceptanceDeg() * deg);
        G4cout << "EnergySpectScorer collimation tolerance (cosine): " << fCollTolerance << G4endl;
    }

    void EnergySpectScorer::Initialize(G4HCofThisEvent* hce) {
        // Code inspired from https://github.com/Geant4/geant4/blob/e58e650b32b961c8093f3dd6a2c3bc917b2552be/source/digits_hits/scorer/src/G4PSEnergyDeposit.cc
        fHitsMap = new G4THitsMap<G4double>(GetMultiFunctionalDetector()->GetName(), GetName());
        // G4cout << "In EnergySpectScorer::Initialize, the MFD name we got was " << GetMultiFunctionalDetector()->GetName() << " and self->GetName() is " << GetName() << G4endl;
        G4int hcID = GetCollectionID(0);
        hce->AddHitsCollection(hcID, fHitsMap);
    }

    G4bool EnergySpectScorer::ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) {
        const auto* pre = aStep->GetPreStepPoint();

        const G4double energy =
            pre->GetKineticEnergy();
        if (energy == 0.)
            return false;

        auto touchable = pre->GetTouchableHandle();
        const auto* transform = touchable->GetHistory()->GetPtrTopTransform();

        G4ThreeVector momentumWorld = aStep->GetPreStepPoint()->GetMomentumDirection();
        G4ThreeVector momentumLocal = transform->TransformAxis(momentumWorld);

        // Assuming +Y is the detector normal in detector-local coordinates.
        G4ThreeVector detectorNormalLocal(0., 1., 0.);

        G4double alignment =
            detectorNormalLocal.dot(momentumLocal);

        //G4cout << "EnergySpectScorer: alignment (cosine): " << alignment << ", fCollTolerance: " << fCollTolerance  << ", momentumLocal: (" << momentumLocal.x() << ", " << momentumLocal.y() << ", " << momentumLocal.z() << ")" << G4endl;

        switch(fScorerType) {
            case ScorerType::Parallel: {
                if (alignment < fCollTolerance) return false;
                break;
            }
            case ScorerType::Pinhole: {
                G4cout << "EnergySpectScorer: Pinhole collimation not implemented yet!" << G4endl;
                break;
            }
            default: {
                break;
            }
        }

        // Get pos of the step, and what pixel that corresponds to
        // Get local pos - local to touchable!
        G4ThreeVector worldPos = aStep->GetPreStepPoint()->GetPosition();
        G4ThreeVector pos = touchable->GetHistory()->GetTopTransform().TransformPoint(worldPos);

        G4int i = static_cast<G4int>((pos.x() - x_min) / pixel_size_w);
        G4int j = static_cast<G4int>((pos.z() - y_min) / pixel_size_h);
        if (i < 0 || i >= fNx || j < 0 || j >= fNy) return false; // Out of bounds

        // Determine enregy bin
        G4int bin = static_cast<G4int>((energy-fEMin) / (fEMax-fEMin) * fNbins);
        if (bin < 0 || bin >= fNbins) {
            // G4cout << "EnergySpectScorer: (SUSPICIOUS) Energy " << energy << " out of bounds for binning, bin index: " << bin << ", pos x and z: (" << pos.x() << ", " << pos.z() << ")" << G4endl;
            return false;
        } // Out of bounds

        // Gen unique key for pixel and bin combination
        G4int key = Util::GenMapKey(i, j, bin);

        G4cout << "EnergySpectScorer accepted hit with energy: " << energy << " and pixel coordinates: (" << i << ", " << j << ", " << bin << ")" << ", pos x and z: (" << pos.x() << ", " << pos.z() << ")" << G4endl;

        G4double valueToAdd = 1; // For now, we just count photons, so we add 1 for each hit. In the future, we might want to add energy deposit or other metrics.

        // Accumulate energy deposit in this pixel and bin
        fHitsMap->add(key, valueToAdd); // P.S. Eventually, when aggregating, we just care about photon count

        return true;
    }
}