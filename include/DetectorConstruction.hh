#ifndef DETECTORCONSTRUCTION_HH
#define DETECTORCONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"

class G4LogicalVolume;

namespace lircst {
    class DetectorConstruction : public G4VUserDetectorConstruction {
    public:
        DetectorConstruction() = default;
        ~DetectorConstruction() override = default;

        G4VPhysicalVolume* Construct() override;
        void ConstructSDandField() override;

        G4LogicalVolume* GetScoringVolume() const { return fScoringVolume; }

        // Some different phantom options to construct
        void ConstructPhanRandom();
        void ConstructPhanLungTumour();
        void ConstructPhanTubes();

    protected:
        G4LogicalVolume* fWorldVolume = nullptr;
        G4LogicalVolume* fScoringVolume = nullptr;
    };
}

#endif // DETECTORCONSTRUCTION_HH