#ifndef DETECTORCONSTRUCTION_HH
#define DETECTORCONSTRUCTION_HH

#include <vector>

#include "G4VUserDetectorConstruction.hh"

// For importance biasing
#include "G4VIStore.hh"
#include "G4IStore.hh"

class G4LogicalVolume;

namespace lircst {
    class DetectorConstruction : public G4VUserDetectorConstruction {
    public:
        DetectorConstruction();
        ~DetectorConstruction() override = default;

        G4VPhysicalVolume* Construct() override;
        void ConstructSDandField() override;

        G4VPhysicalVolume* GetWorldVolume() const { return fPhysicalWorldVolume; }
        G4LogicalVolume* GetScoringVolume() const { return fLogicalScoringVolume; }

        void ConstructImportanceVolumes();
        G4VIStore* CreateImportanceStore(); // Caller is responsible for deletion

        // Some different phantom options to construct
        G4VPhysicalVolume* ConstructPhanRandom();
        G4VPhysicalVolume* ConstructPhanLungTumour();
        G4VPhysicalVolume* ConstructPhanTubes();

    protected:
        G4LogicalVolume* fLogicalWorldVolume = nullptr;
        G4LogicalVolume* fLogicalScoringVolume = nullptr;
        G4VPhysicalVolume* fPhysicalWorldVolume = nullptr;
        std::vector<G4VPhysicalVolume*> fPhyImportanceVolumes;
    };
}

#endif // DETECTORCONSTRUCTION_HH