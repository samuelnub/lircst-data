#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4SDManager.hh"

#include "EnergySpectScorer.hh"

#include "Util.hh"

namespace lircst {
    G4VPhysicalVolume* DetectorConstruction::Construct() {
        // World
        auto worldSize = Util::GetWorldSize();
        auto worldSolid = new G4Box("World", worldSize, worldSize, worldSize);
        auto worldLogical = new G4LogicalVolume(worldSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR"), "World");
        auto worldPhysical = new G4PVPlacement(0, G4ThreeVector(), worldLogical, "World", 0, false, 0);

        // Base phantom
        auto phantomSize = Util::GetPhantomSize();
        auto phantomSolid = new G4Box("Phantom", phantomSize, phantomSize, phantomSize);
        auto phantomLogical = new G4LogicalVolume(phantomSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_WATER"), "Phantom");
        new G4PVPlacement(0, G4ThreeVector(), phantomLogical, "Phantom", worldLogical, false, 0, true);

        // Randomised spherical inserts in phantom
        for (int i = 0; i < 5; i++) {
            // TODO: randomise size
            auto insertSize = phantomSize / 10;
            auto insertSolid = new G4Box("Insert", insertSize, insertSize, insertSize);
            // TODO: randomise material
            auto insertLogical = new G4LogicalVolume(insertSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU"), "Insert");
            auto x = G4UniformRand() * phantomSize - phantomSize / 2;
            auto y = G4UniformRand() * phantomSize - phantomSize / 2;
            auto z = G4UniformRand() * phantomSize - phantomSize / 2;
            new G4PVPlacement(0, G4ThreeVector(x, y, z), insertLogical, "Insert", phantomLogical, false, 0, true);
        }



        // Sensitive / Multi-func Detector & scoring volume geometries
        auto scoringVolumeSize = Util::GetScorerSize();

        auto scoringVolumeDistFromCentre = worldSize * 0.8;
        auto scoringVolumeRotation = new G4RotationMatrix();
        scoringVolumeRotation->rotateX(180 * deg); // Face down

        auto scoringVolumeSolid = new G4Box("ScoringVolume", scoringVolumeSize, scoringVolumeSize / 8, scoringVolumeSize);
        fScoringVolume = new G4LogicalVolume(scoringVolumeSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_Pb"), "ScoringVolume");
        new G4PVPlacement(scoringVolumeRotation, G4ThreeVector(0, scoringVolumeDistFromCentre, 0), fScoringVolume, "ScoringVolume", worldLogical, false, 0);

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
                                                        Util::GetNumPixelsX(),
                                                        Util::GetNumPixelsY(),
                                                        Util::GetNumBins(),
                                                        Util::GetEnergyMin(),
                                                        Util::GetEnergyMax()); // pretty low (medical)
        mfd->RegisterPrimitive(energySpectScorer);
        SetSensitiveDetector(fScoringVolume, mfd);
    }
}