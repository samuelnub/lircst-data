#ifndef UTIL_HH
#define UTIL_HH

#include <vector>

#include "G4Types.hh"
#include "G4String.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

#include "AccumulableMap.hh"

using namespace std;

namespace lircst {
    template class AccumulableMap<G4int>;

    class Util {
    public:
        static G4int GenMapKey(G4int x, G4int y, G4int bin);

        static G4int GenRandomInt(G4int min, G4int max);
        static G4double GenRandomDouble(G4double min, G4double max);
        static G4double GenRandomDoubleGauss(G4double mean, G4double sigma);

        static G4ThreeVector GetPosFromThetaRotation(G4double theta, G4double radius);

        static G4String GenUniqueInstanceRunName();

        static G4double BinToEnergy(G4int bin);

        static G4bool ExportData(AccumulableMap<G4int> data);

        static const G4int GetNumPixelsX() { return fNumPixelsX; } 
        static const G4int GetNumPixelsY() { return fNumPixelsY; }
        static const G4int GetNumBins() { return fNumBins; }
        static const G4double GetGunEnergy() { return fGunEnergy; }
        static const G4double GetEnergyMin() { return fEnergyMin; }
        static const G4double GetEnergyMax() { return fEnergyMax; }
        static const G4double GetScorerSize() { return fScorerSize; }
        static const G4double GetWorldSize() { return fWorldSize; }
        static const G4double GetPhantomSize() { return fPhantomSize; }
        static const G4double GetDetecDistIsocenter() { return fDetecDistIsocenter; }
        static const G4double GetSourceDistIsocenter() { return fSourceDistIsocenter; }
        static const G4double GetWorldGunSDRatio() { return fWorldGunSDRatio; }
        static const G4double GetCollSDToIncidentRatio() { return fCollSDToIncidentRatio; }
        static const G4double GetCollCosAcceptanceDeg() { return fCollCosAcceptanceDeg; }
        static const G4bool GetEnableSolidAngleBiasing() { return fEnableSolidAngleBiasing; }

    private:
        static const G4int fNumPixelsX = 128;
        static const G4int fNumPixelsY = 128;
        static const G4int fNumBins = 100;
        static constexpr G4double fGunEnergy = 1.0 * MeV; // Mimics peak of a 6MeV radiotherapy photon spectrum
        static constexpr G4double fEnergyMin = 0.324 * MeV;
        static constexpr G4double fEnergyMax = 0.353 * MeV; // According to angle alpha based on how far the source is from the isocentre and the width of the phantom
        static constexpr G4double fScorerSize = 12.8 * cm / 2.0; // Make sure that each pixel is roughly 1mm x 1mm
        static constexpr G4double fWorldSize = 150.0 * cm;
        static constexpr G4double fPhantomSize = 12.8 * cm / 2.0; // Geant4 box dimensions are half-widths, remember!
        static constexpr G4double fDetecDistIsocenter = 50.0 * cm;
        static constexpr G4double fSourceDistIsocenter = 100.0 * cm;
        static constexpr G4double fWorldGunSDRatio = 0.9; // TODO: deprecated
        static constexpr G4double fCollSDToIncidentRatio = 0.65; // TODO: deprecated
        static constexpr G4double fCollCosAcceptanceDeg = 2.29;
        static const G4bool fEnableSolidAngleBiasing = false;
    };
}

#endif // UTIL_HH