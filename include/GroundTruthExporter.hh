#ifndef GROUNDTRUTHEXPORTER_HH
#define GROUNDTRUTHEXPORTER_HH

#include "G4Material.hh"
#include "G4ThreeVector.hh"
#include "G4Navigator.hh"
#include "G4TouchableHistoryHandle.hh"
#include "G4EmCalculator.hh"

#include <vector>
#include <fstream>

namespace lircst {
    class GroundTruthExporter {
    public:
        GroundTruthExporter() = default;
        ~GroundTruthExporter() = default;

        void Export(G4int slice = 0); // Exports the first slice by default

        static G4double CalculateElectronDensityPerMole(G4Material* material);
        static G4double CalculateLinearAttenuation(G4Material* material, G4double energy, G4EmCalculator& emCalc);

    private:
        G4Material* FindMaterialAt(G4ThreeVector pos);
        void WriteToFile(const std::vector<double>& elecDensAndLinAttenData);

        G4Navigator* fNavigator = new G4Navigator();

        G4int fResolution = 128;
        G4String fFilename = "gt.npy";
    };
}

#endif // GROUNDTRUTHEXPORTER_HH