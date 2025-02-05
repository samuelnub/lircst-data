#ifndef GROUNDTRUTHEXPORTER_HH
#define GROUNDTRUTHEXPORTER_HH

#include "G4Material.hh"
#include "G4ThreeVector.hh"

#include <vector>
#include <fstream>

namespace lircst {
    class GroundTruthExporter {
    public:
        GroundTruthExporter() = default;
        ~GroundTruthExporter() = default;

        void Export();

        static G4double CalculateElectronDensityPerMole(G4Material* material);

    private:
        G4Material* FindMaterialAt(G4ThreeVector pos);
        void WriteToFile(const std::vector<double>& electronDensityData);

        G4int fResolution = 1024;
        G4String fFilename = "gt.npy";
    };
}

#endif // GROUNDTRUTHEXPORTER_HH