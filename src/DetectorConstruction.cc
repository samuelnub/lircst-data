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
#include "RandPhanGen.hh"

#include "Util.hh"

#include <cmath>

namespace lircst {
    DetectorConstruction::DetectorConstruction() : G4VUserDetectorConstruction() {}

    G4VPhysicalVolume* DetectorConstruction::Construct() {
        // World
        auto worldSize = Util::GetWorldSize();
        auto worldSolid = new G4Box("World", worldSize, worldSize, worldSize);
        auto worldLogical = new G4LogicalVolume(worldSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR"), "World");
        auto worldPhysical = new G4PVPlacement(0, G4ThreeVector(), worldLogical, "World", 0, false, 0);
        // For importance biasing
        fPhyImportanceVolumes.push_back(worldPhysical);
        fPhysicalWorldVolume = worldPhysical;

        fLogicalWorldVolume = worldLogical;

        // Call your phantom construction here
        auto phantomPhysical = ConstructPhanRandom();
        // For importance biasing
        fPhyImportanceVolumes.push_back(phantomPhysical);

        // TODO: construct importance geometries (also attach highest importance to SD!!!)
        ConstructImportanceVolumes();
        // P.S. construct SD last so that we can have highest importance assigned to it

        // Sensitive / Multi-func Detector & scoring volume geometries
        auto scoringVolumeSize = Util::GetScorerSize();

        auto scoringVolumeDistFromCentre = worldSize * Util::GetWorldGunSDRatio();
        auto scoringVolumeRotation = new G4RotationMatrix();
        scoringVolumeRotation->rotateX(-90 * deg); // Face down

        auto scoringVolumeSolid = new G4Box("ScoringVolume", scoringVolumeSize, scoringVolumeSize, scoringVolumeSize / 8); // TODO: magic number
        fLogicalScoringVolume = new G4LogicalVolume(scoringVolumeSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_Pb"), "ScoringVolume");
        auto scoringVolumePhysical = new G4PVPlacement(scoringVolumeRotation, G4ThreeVector(0, scoringVolumeDistFromCentre, 0), fLogicalScoringVolume, "ScoringVolume", worldLogical, false, 0);
        // For importance biasing
        fPhyImportanceVolumes.push_back(scoringVolumePhysical);

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
        SetSensitiveDetector(fLogicalScoringVolume, mfd);
    }

    void DetectorConstruction::ConstructImportanceVolumes() {

        // UNUSED
        return;

        // Logical slabs
        int numSlabs = 16;

        G4double boundUpper = (Util::GetWorldSize() * Util::GetWorldGunSDRatio()) - (Util::GetScorerSize() / 8); // TODO: magic number
        G4double boundLower = Util::GetPhantomSize(); // Remember! Geant4 box dimensions are half-lengths! Why? IDK
        G4double slabY = ((boundUpper - boundLower) / numSlabs) / 2; // Half-size

        for(int i = 0; i < numSlabs; i++) {
            auto slabXZ = Util::GetScorerSize(); // TODO: placeholder until i can get it to look nice
            
            auto slabSolid = new G4Box("ISlab", slabXZ, slabY, slabXZ);
            auto slabLogical = new G4LogicalVolume(slabSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR"), "ISlab");
            auto slabPhysical = new G4PVPlacement(0, G4ThreeVector(0, boundLower + slabY + i * 2 * slabY, 0), slabLogical, "ISlab", fLogicalWorldVolume, false, 0, true);

            fPhyImportanceVolumes.push_back(slabPhysical);
        }
    }

    G4VIStore* DetectorConstruction::CreateImportanceStore() {

        // UNUSED
        return nullptr;
        
        if(!fPhyImportanceVolumes.size()) {
            G4cerr << "No importance volumes to create store for!" << G4endl;
            return nullptr;
        }
        G4IStore* iStore = G4IStore::GetInstance();

        /* TODO: Clear this up because we won't be using importance biasing */
        return iStore;

        for(int i = 0; i < fPhyImportanceVolumes.size(); i++) {
            G4cout << "Adding importance volume " << i << G4endl;
            iStore->AddImportanceGeometryCell(std::pow(2, i), *fPhyImportanceVolumes[i]);
        }

        return iStore;
    }

    G4VPhysicalVolume* DetectorConstruction::ConstructPhanRandom() {
        auto randPhanGen = new RandPhanGen(fLogicalWorldVolume);
        return randPhanGen->GeneratePhantom();
    }

    G4VPhysicalVolume* DetectorConstruction::ConstructPhanLungTumour() {
        // Base phantom
        auto phantomSize = Util::GetPhantomSize();
        auto phantomSolid = new G4Box("Phantom", phantomSize, phantomSize, phantomSize);
        auto phantomLogical = new G4LogicalVolume(phantomSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_WATER"), "Phantom");
        auto phantomPhysical = new G4PVPlacement(0, G4ThreeVector(), phantomLogical, "Phantom", fLogicalWorldVolume, false, 0, true);

        // Lung
        auto lungSize = phantomSize * 0.8;
        auto lungSolid = new G4Box("Lung", lungSize, lungSize, lungSize * 0.7);
        auto lungLogical = new G4LogicalVolume(lungSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_LUNG_ICRP"), "Lung");
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lungLogical, "Lung", phantomLogical, false, 0, true);

        // Tumour
        auto tumourSize = lungSize * 0.8;
        auto tumourSolid = new G4Tubs("Tumour", 0, tumourSize * 0.2, tumourSize, 0, 360 * deg);
        auto tumourLogical = new G4LogicalVolume(tumourSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU"), "Tumour");
        new G4PVPlacement(new G4RotationMatrix(0, 90 * deg, 0), G4ThreeVector(lungSize * -0.3, 0, lungSize * -0.15), tumourLogical, "Tumour", lungLogical, false, 0, true);
    
        return phantomPhysical;
    }

    G4VPhysicalVolume* DetectorConstruction::ConstructPhanTubes() {
        // TODO: autogenerated lol
        
        // Base phantom
        auto phantomSize = Util::GetPhantomSize();
        auto phantomSolid = new G4Box("Phantom", phantomSize, phantomSize, phantomSize);
        auto phantomLogical = new G4LogicalVolume(phantomSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_WATER"), "Phantom");
        auto phantomPhysical = new G4PVPlacement(0, G4ThreeVector(), phantomLogical, "Phantom", fLogicalWorldVolume, false, 0, true);

        // Tubes
        auto tubeSize = phantomSize / 10;
        auto tubeSolid = new G4Tubs("Tube", 0, tubeSize / 2, phantomSize / 2, 0, 360 * deg);
        auto tubeLogical = new G4LogicalVolume(tubeSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU"), "Tube");
        for (int i = 0; i < 5; i++) {
            auto x = G4UniformRand() * phantomSize - phantomSize / 2;
            auto y = G4UniformRand() * phantomSize - phantomSize / 2;
            new G4PVPlacement(0, G4ThreeVector(x, y, 0), tubeLogical, "Tube", phantomLogical, false, 0, true);
        }

        return phantomPhysical;
    }
}