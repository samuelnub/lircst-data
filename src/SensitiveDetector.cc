#include "SensitiveDetector.hh"

namespace lircst{
    SensitiveDetector::SensitiveDetector(const G4String& name) : G4VSensitiveDetector(name) {}

    void SensitiveDetector::Initialize(G4HCofThisEvent* hce) {
        fHitsMap = new G4THitsMap<G4double>(GetName(), "EnergyMap");
        if (fHitsMap == nullptr) {
            G4ExceptionDescription description;
            description << "Unable to allocate hits map for " << GetName();
            G4Exception("SensitiveDetector::Initialize", "SensitiveDetector", FatalException, description);
        }
        hce->AddHitsCollection(GetCollectionID(0), fHitsMap);
    }

    G4bool SensitiveDetector::ProcessHits(G4Step* aStep, G4TouchableHistory* history) {
        G4double eDep = aStep->GetTotalEnergyDeposit();
        if (eDep == 0) {
            return false;
        }

        G4ThreeVector pos = aStep->GetPreStepPoint()->GetPosition();
        int i = static_cast<int>((pos.x() - xBoundMin) / xPixelSize);
        int j = static_cast<int>((pos.y() - yBoundMin) / yPixelSize);

        // Check bounds
        if (i < 0 || i >= fPixelsX || j < 0 || j >= fPixelsY) {
            return false;
        }

        G4int key = i + fPixelsX * j;
        fHitsMap->add(key, eDep);

        return true;
    }
}