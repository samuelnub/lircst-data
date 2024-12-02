#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
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

        fWorldVolume = worldLogical;

        // Call your phantom construction here
        ConstructPhanLungTumour();

        // Sensitive / Multi-func Detector & scoring volume geometries
        auto scoringVolumeSize = Util::GetScorerSize();

        auto scoringVolumeDistFromCentre = worldSize * Util::GetGunSDRatio();
        auto scoringVolumeRotation = new G4RotationMatrix();
        scoringVolumeRotation->rotateX(-90 * deg); // Face down

        auto scoringVolumeSolid = new G4Box("ScoringVolume", scoringVolumeSize, scoringVolumeSize, scoringVolumeSize / 8);
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

    void DetectorConstruction::ConstructPhanRandom() {
        // Base phantom
        auto phantomSize = Util::GetPhantomSize();
        auto phantomSolid = new G4Box("Phantom", phantomSize, phantomSize, phantomSize);
        auto phantomLogical = new G4LogicalVolume(phantomSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_WATER"), "Phantom");
        new G4PVPlacement(0, G4ThreeVector(), phantomLogical, "Phantom", fWorldVolume, false, 0, true);

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
    }

    void DetectorConstruction::ConstructPhanLungTumour() {
        // Base phantom
        auto phantomSize = Util::GetPhantomSize();
        auto phantomSolid = new G4Box("Phantom", phantomSize, phantomSize, phantomSize);
        auto phantomLogical = new G4LogicalVolume(phantomSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_WATER"), "Phantom");
        new G4PVPlacement(0, G4ThreeVector(), phantomLogical, "Phantom", fWorldVolume, false, 0, true);

        // Lung
        auto lungSize = phantomSize * 0.8;
        auto lungSolid = new G4Box("Lung", lungSize, lungSize, lungSize * 0.7);
        auto lungLogical = new G4LogicalVolume(lungSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_LUNG_ICRP"), "Lung");
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lungLogical, "Lung", phantomLogical, false, 0, true);

        // Tumour
        auto tumourSize = lungSize * 0.8;
        auto tumourSolid = new G4Tubs("Tumour", 0, tumourSize * 0.2, tumourSize, 0, 360 * deg);
        auto tumourLogical = new G4LogicalVolume(tumourSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_Pb"), "Tumour");
        new G4PVPlacement(new G4RotationMatrix(0, 90 * deg, 0), G4ThreeVector(lungSize * 0.3, 0, 0), tumourLogical, "Tumour", lungLogical, false, 0, true);
    }

    void DetectorConstruction::ConstructPhanTubes() {
        // TODO: autogenerated lol
        
        // Base phantom
        auto phantomSize = Util::GetPhantomSize();
        auto phantomSolid = new G4Box("Phantom", phantomSize, phantomSize, phantomSize);
        auto phantomLogical = new G4LogicalVolume(phantomSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_WATER"), "Phantom");
        new G4PVPlacement(0, G4ThreeVector(), phantomLogical, "Phantom", fWorldVolume, false, 0, true);

        // Tubes
        auto tubeSize = phantomSize / 10;
        auto tubeSolid = new G4Tubs("Tube", 0, tubeSize / 2, phantomSize / 2, 0, 360 * deg);
        auto tubeLogical = new G4LogicalVolume(tubeSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU"), "Tube");
        for (int i = 0; i < 5; i++) {
            auto x = G4UniformRand() * phantomSize - phantomSize / 2;
            auto y = G4UniformRand() * phantomSize - phantomSize / 2;
            new G4PVPlacement(0, G4ThreeVector(x, y, 0), tubeLogical, "Tube", phantomLogical, false, 0, true);
        }
    }
}