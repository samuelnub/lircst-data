#ifndef SENSITIVEDETECTOR_HH
#define SENSITIVEDETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4THitsMap.hh"
#include "G4SystemOfUnits.hh"

namespace lircst {
    class SensitiveDetector : public G4VSensitiveDetector {
    public:
        SensitiveDetector(const G4String& name);
        ~SensitiveDetector() override = default;
        
        void Initialize(G4HCofThisEvent* hce) override;
        G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* history) override;

    private:
        G4int fPixelsX, fPixelsY;
        G4THitsMap<G4double>* fHitsMap = nullptr;
        const G4double xBoundMin = -5.0 * cm;
        const G4double yBoundMin = -5.0 * cm;
        const G4double xPixelSize = 1 * mm;
        const G4double yPixelSize = 1 * mm;
    };
}

#endif // SENSITIVEDETECTOR_HH