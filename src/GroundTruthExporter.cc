#include "GroundTruthExporter.hh"

#include "G4Navigator.hh"
#include "G4TransportationManager.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PhysicalVolumeStore.hh"
#include "CLHEP/Units/PhysicalConstants.h"

#include "Util.hh"

#include "npy.hh"

namespace lircst {
    void GroundTruthExporter::Export(G4int slice) {
        // To prevent segfaults after each run
        fNavigator->SetWorldVolume(G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume());

        // Our imaging plane is a square encompassing the phantom
        // We will divide this plane into a grid of fResolution x fResolution
        // We will then calculate the electron density per pixel
        auto imagingPlaneWidth = Util::GetPhantomSize(); // Geant4 box dimensions are half-widths, remember!

        G4double pixelWidth = 2 * imagingPlaneWidth / fResolution;
        G4double halfPixelWidth = pixelWidth / 2;

        std::vector<double> elecDensAndLinAttenData(2 * fResolution * fResolution, 0.0f); // 2 channels: 0 = electron density, 1 = linear attenuation


        G4EmCalculator emCalc;

        G4double sliceZ = -imagingPlaneWidth + halfPixelWidth + slice * pixelWidth; // Calculate Z position of the slice we want to export

        // Loop over all voxels in imaging plane
        for (int i = 0; i < fResolution; i++) {
            for (int j = 0; j < fResolution; j++) {
                // Slice dimension goes along Z axis, so we want to image across X and Y
                G4ThreeVector pos = G4ThreeVector(-imagingPlaneWidth + halfPixelWidth + i * pixelWidth, -imagingPlaneWidth + halfPixelWidth + j * pixelWidth, sliceZ);
                G4Material* material = FindMaterialAt(pos);
                elecDensAndLinAttenData[0 * fResolution * fResolution + i * fResolution + j] = CalculateElectronDensityPerMole(material);
                elecDensAndLinAttenData[1 * fResolution * fResolution + i * fResolution + j] = CalculateLinearAttenuation(material, Util::GetGunEnergy(), emCalc); // Assumes usage of the monochromatic source
            }
        }

        WriteToFile(elecDensAndLinAttenData);
    }

    G4Material* GroundTruthExporter::FindMaterialAt(G4ThreeVector pos) {
        // G4Navigator* navigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
        G4VPhysicalVolume* volume = fNavigator->LocateGlobalPointAndSetup(pos);
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

            electronDensity += fraction * (atomicNumber / atomicWeight) * CLHEP::Avogadro * density; // electrons per cm^3
        }

        return electronDensity;
    }

    G4double GroundTruthExporter::CalculateLinearAttenuation(G4Material* material, G4double energy, G4EmCalculator& emCalc) {
        G4double len = emCalc.ComputeGammaAttenuationLength(energy, material);
        if (len == 0) return 0; // Prevent division by zero, though I'm not sure if this is physically meaningful
        return 1.0 / (len * cm); // the cm is to get it to the right magnitude
    }

    void GroundTruthExporter::WriteToFile(const std::vector<double>& elecDensAndLinAttenData) {
        G4String folder = "output/";
        
        npy::npy_data_ptr<double> data;

        data.data_ptr = elecDensAndLinAttenData.data();

        // 2 channels: 0 = electron density, 1 = linear attenuation
        // Followed by x and y dimensions of the image
        data.shape = {2, static_cast<unsigned long>(fResolution), static_cast<unsigned long>(fResolution)};
        data.fortran_order = false;
        
        const string path{folder + Util::GenUniqueInstanceRunName() + fFilename};

        npy::write_npy(path, data);
    }
}