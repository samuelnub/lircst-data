#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"

namespace lircst {
    G4VPhysicalVolume* DetectorConstruction::Construct() {
        // World
        auto worldSize = 10.0 * cm;
        auto worldSolid = new G4Box("World", worldSize / 2, worldSize / 2, worldSize / 2);
        auto worldLogical = new G4LogicalVolume(worldSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR"), "World");
        auto worldPhysical = new G4PVPlacement(0, G4ThreeVector(), worldLogical, "World", 0, false, 0);

        // Scoring volume
        auto scoringVolumeSize = 1.0 * cm;
        auto scoringVolumeSolid = new G4Box("ScoringVolume", scoringVolumeSize / 2, scoringVolumeSize / 2, scoringVolumeSize / 2);
        fScoringVolume = new G4LogicalVolume(scoringVolumeSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_WATER"), "ScoringVolume");
        new G4PVPlacement(0, G4ThreeVector(), fScoringVolume, "ScoringVolume", worldLogical, false, 0);

        // Always return physical world
        return worldPhysical;
    }
}