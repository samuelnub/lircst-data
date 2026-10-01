#include "GroundTruthExporter.hh"

#include "G4Navigator.hh"
#include "G4TransportationManager.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PhysicalVolumeStore.hh"
#include "CLHEP/Units/PhysicalConstants.h"

#include "G4GenericMessenger.hh"

#include "Util.hh"

#include "npy.hh"

namespace lircst {
    std::vector<double>& GroundTruthExporter::Export(G4int slice) {
        fNavigator->SetWorldVolume(G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume());

        auto imagingPlaneWidth = Util::GetPhantomSize(); 
        G4double pixelWidth = 2 * imagingPlaneWidth / fResolution;
        G4double halfPixelWidth = pixelWidth / 2;

        fElecDensAndLinAttenData.assign(2 * fResolution * fResolution, 0.0f); 

        G4EmCalculator emCalc;
        G4double sliceZ = -imagingPlaneWidth + halfPixelWidth + slice * pixelWidth; 

        int sliceArea = fResolution * fResolution;

        for (int i = 0; i < fResolution; i++) {
            for (int j = 0; j < fResolution; j++) {
                G4ThreeVector pos = G4ThreeVector(-imagingPlaneWidth + halfPixelWidth + i * pixelWidth, 
                                                -imagingPlaneWidth + halfPixelWidth + j * pixelWidth, 
                                                sliceZ);
                G4Material* material = FindMaterialAt(pos);
                
                // Layout within a single slice: Keep Ch0 and Ch1 contiguous for this slice
                // Indexing for (C, X, Y) within this slice:
                fElecDensAndLinAttenData[0 * sliceArea + i * fResolution + j] = CalculateElectronDensityPerMole(material);
                fElecDensAndLinAttenData[1 * sliceArea + i * fResolution + j] = CalculateLinearAttenuation(material, Util::GetGunEnergy(), emCalc);
            }
        }

        return fElecDensAndLinAttenData;
    }

    void GroundTruthExporter::ExportFullVolume() {
        G4cout << "Exporting full volume..." << G4endl;
        
        // Total size: 2 * 128 * 128 * 128
        std::vector<double> fullVolumeData(2 * fResolution * fResolution * fResolution, 0.0f);   
        
        int sliceArea = fResolution * fResolution;
        int channelVolume = fResolution * fResolution * fResolution; // Offset for Channel 1

        for (int slice = 0; slice < fResolution; slice++) {
            std::vector<double>& sliceData = Export(slice);
            
            // Target Layout: C, X, Y, Z
            // For a given slice (Z), its data elements are written to:
            // Channel 0 destination offset: (i * fResolution * fResolution) + (j * fResolution) + slice
            // Channel 1 destination offset: channelVolume + (i * fResolution * fResolution) + (j * fResolution) + slice
            
            for (int i = 0; i < fResolution; i++) {
                for (int j = 0; j < fResolution; j++) {
                    int sliceIdx = i * fResolution + j;
                    
                    // Read from sliceData
                    double ch0_val = sliceData[0 * sliceArea + sliceIdx];
                    double ch1_val = sliceData[1 * sliceArea + sliceIdx];
                    
                    // Write to fullVolumeData matching (C, X, Y, Z) stride
                    int destIdx = i * fResolution * fResolution + j * fResolution + slice;
                    
                    fullVolumeData[destIdx] = ch0_val;                  // Channel 0 block
                    fullVolumeData[channelVolume + destIdx] = ch1_val;  // Channel 1 block
                }
            }
        }
        WriteToFile(fullVolumeData);
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
        G4String folder = "output/" + Util::GetInstanceRunName() + "/"; // Create a subfolder for this run
        Util::CreateDirectory(folder); // Ensure the output directory exists before writing files

        npy::npy_data_ptr<double> data;

        data.data_ptr = elecDensAndLinAttenData.data();

        // 2 channels: 0 = electron density, 1 = linear attenuation
        // Followed by x and y and z dimensions of the phantom
        data.shape = {2, static_cast<unsigned long>(fResolution), static_cast<unsigned long>(fResolution), static_cast<unsigned long>(fResolution)}; // Note: Geant4 box dimensions are half-widths, remember!
        data.fortran_order = false;
        
        const string path{folder + "phan" + fFilenameSuffix};

        npy::write_npy(path, data);
    }
}