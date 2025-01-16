#ifndef UTIL_HH
#define UTIL_HH

#include <vector>

#include "G4Types.hh"
#include "G4String.hh"
#include "G4SystemOfUnits.hh"

#include "AccumulableMap.hh"

using namespace std;

namespace lircst {
    template class AccumulableMap<G4int>;

    class Util {
    public:
        static G4int GenMapKey(G4int x, G4int y, G4int bin);

        static G4double BinToEnergy(G4int bin);

        static G4bool ExportData(AccumulableMap<G4int> data, G4String filename);

        static const G4int GetNumPixelsX() { return fNumPixelsX; } 
        static const G4int GetNumPixelsY() { return fNumPixelsY; }
        static const G4int GetNumBins() { return fNumBins; }
        static const G4double GetGunEnergy() { return fGunEnergy; }
        static const G4double GetEnergyMin() { return fEnergyMin; }
        static const G4double GetEnergyMax() { return fEnergyMax; }
        static const G4double GetScorerSize() { return fScorerSize; }
        static const G4double GetWorldSize() { return fWorldSize; }
        static const G4double GetPhantomSize() { return fPhantomSize; }
        static const G4double GetWorldGunSDRatio() { return fWorldGunSDRatio; }
        static const G4double GetCollSDToIncidentRatio() { return fCollSDToIncidentRatio; }

    private:
        static const G4int fNumPixelsX = 256;
        static const G4int fNumPixelsY = 256;
        static const G4int fNumBins = 100;
        static constexpr G4double fGunEnergy = 60.0 * keV;
        static constexpr G4double fEnergyMin = 0.0 * keV;
        static constexpr G4double fEnergyMax = 100.0 * keV;
        static constexpr G4double fScorerSize = 10.0 * cm; // Make sure that each pixel is roughly 1mm x 1mm
        static constexpr G4double fWorldSize = 30.0 * cm;
        static constexpr G4double fPhantomSize = 10.0 * cm;
        static constexpr G4double fWorldGunSDRatio = 0.9;
        static constexpr G4double fCollSDToIncidentRatio = 0.65;

    };
}

#endif // UTIL_HH