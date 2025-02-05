#include "GroundTruthExporter.hh"

#include "G4Navigator.hh"
#include "G4TransportationManager.hh"
#include "G4VPhysicalVolume.hh"

#include "Util.hh"

#include "npy.hh"

namespace lircst {
    void GroundTruthExporter::Export() {
        // Our imaging plane is a square encompassing the phantom at the slice at 0,0,0 with the normal facing upwards
        // We will divide this plane into a grid of fResolution x fResolution
        // We will then calculate the electron density per pixel
        auto imagingPlaneWidth = Util::GetPhantomSize(); // Geant4 box dimensions are half-widths, remember!

        G4double pixelWidth = 2 * imagingPlaneWidth / fResolution;
        G4double halfPixelWidth = pixelWidth / 2;

        std::vector<double> electronDensityMap(fResolution * fResolution, 0.0f);
    
        // Loop over all voxels in imaging plane
        for (int i = 0; i < fResolution; i++) {
            for (int j = 0; j < fResolution; j++) {
                G4ThreeVector pos = G4ThreeVector(-imagingPlaneWidth + halfPixelWidth + i * pixelWidth, 0, -imagingPlaneWidth + halfPixelWidth + j * pixelWidth);
                G4Material* material = FindMaterialAt(pos);
                electronDensityMap[i * fResolution + j] = CalculateElectronDensityPerMole(material);
            }
        }

        WriteToFile(electronDensityMap);
    }

    G4Material* GroundTruthExporter::FindMaterialAt(G4ThreeVector pos) {
        G4Navigator* navigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
        G4VPhysicalVolume* volume = navigator->LocateGlobalPointAndSetup(pos);
        if (!volume) return nullptr;
        return volume->GetLogicalVolume()->GetMaterial();
    }

    G4double GroundTruthExporter::CalculateElectronDensityPerMole(G4Material* material) {
        G4double density = material->GetDensity(); // (g / cm^3)
        G4double electronDensity = 0.0f;

        const G4ElementVector* elements = material->GetElementVector();
        const G4double* fractions = material->GetFractionVector();
        size_t numElements = material->GetNumberOfElements();

        for (size_t i = 0; i < numElements; i++) {
            const G4Element* element = (*elements)[i];
            G4double fraction = fractions[i];
            G4double atomicNumber = element->GetZ();
            G4double atomicWeight = element->GetA();

            electronDensity += fraction * (atomicNumber / atomicWeight);
        }

        electronDensity = electronDensity * density * cm3; // the cm3 is to get it to the right magnitude

        return electronDensity;
    }

    void GroundTruthExporter::WriteToFile(const std::vector<double>& electronDensityData) {
        G4String folder = "output/";
        
        npy::npy_data_ptr<double> data;
        data.data_ptr = electronDensityData.data();
        data.shape = {static_cast<unsigned long>(fResolution), static_cast<unsigned long>(fResolution)};
        data.fortran_order = false;
        
        const string path{folder + Util::GenUniqueInstanceRunName() + fFilename};

        npy::write_npy(path, data);
    }
}