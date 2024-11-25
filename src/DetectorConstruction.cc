#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"

#include "G4MultiFunctionalDetector.hh"
#include "G4SDManager.hh"

#include "EnergySpectScorer.hh"

namespace lircst {
    G4VPhysicalVolume* DetectorConstruction::Construct() {
        // World
        auto worldSize = 40.0 * cm;
        auto worldSolid = new G4Box("World", worldSize / 2, worldSize / 2, worldSize / 2);
        auto worldLogical = new G4LogicalVolume(worldSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR"), "World");
        auto worldPhysical = new G4PVPlacement(0, G4ThreeVector(), worldLogical, "World", 0, false, 0);

        // TODO: Phantom


        // Sensitive / Multi-func Detector & scoring volume geometries
        auto scoringVolumeSize = 10.0 * cm;

        // Scoring volume
        // TODO: This shouldn't be water, and this shouldn't be where the phantom should go
        auto scoringVolumeSolid = new G4Box("ScoringVolume", scoringVolumeSize / 2, scoringVolumeSize / 2, scoringVolumeSize / 2);
        fScoringVolume = new G4LogicalVolume(scoringVolumeSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_WATER"), "ScoringVolume");
        new G4PVPlacement(0, G4ThreeVector(), fScoringVolume, "ScoringVolume", worldLogical, false, 0);

        // Always return physical world
        return worldPhysical;
    }



    void DetectorConstruction::ConstructSDandField() {
        // Setup MFD and Primitive Scorer(s)
        auto mfd = new G4MultiFunctionalDetector("mfd");
        G4SDManager::GetSDMpointer()->AddNewDetector(mfd);
        // Add primitive scorer(s)
        auto energySpectScorer = new EnergySpectScorer(
                                                        "ess",
                                                        256,
                                                        256,
                                                        100,
                                                        0.0 * keV,
                                                        100.0 * keV); // pretty low (medical)
        mfd->RegisterPrimitive(energySpectScorer);
        SetSensitiveDetector(fScoringVolume, mfd);
    }
}