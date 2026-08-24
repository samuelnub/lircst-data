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
#include "G4GeometryManager.hh"
#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
#include "G4SubtractionSolid.hh"
#include "G4UserLimits.hh"

#include "G4PSDoseDeposit.hh" // TODO: temp test to see if our ESS is what's broken

#include "EnergySpectScorer.hh"
#include "RandPhanGen.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunManager.hh"

#include "Util.hh"

#include <cmath>
#include <memory>

namespace lircst {
    DetectorConstruction::DetectorConstruction() : G4VUserDetectorConstruction() {}

    G4VPhysicalVolume* DetectorConstruction::Construct() {
        G4cout << "Constructing geometry..." << G4endl;

        // World
        auto worldSize = Util::GetWorldSize();
        auto worldSolid = new G4Box("World", worldSize, worldSize, worldSize);
        auto worldLogical = new G4LogicalVolume(worldSolid, G4NistManager::Instance()->FindOrBuildMaterial(fVoidMaterialName), "World");
        auto worldPhysical = new G4PVPlacement(0, G4ThreeVector(), worldLogical, "World", 0, false, 0);
        // For importance biasing
        //fPhyImportanceVolumes.push_back(worldPhysical);
        fPhysicalWorldVolume = worldPhysical;

        fLogicalWorldVolume = worldLogical;

        // =============================================
        // Call your phantom construction here
        auto phantomPhysical = ConstructPhanLungTumour();
        // =============================================

        // For importance biasing
        //fPhyImportanceVolumes.push_back(phantomPhysical);

        // TODO: construct importance geometries (also attach highest importance to SD!!!)
        //ConstructImportanceVolumes();
        // P.S. construct SD last so that we can have highest importance assigned to it

        // Sensitive / Multi-func Detector & scoring volume geometries
        auto scoringVolumeSize = Util::GetScorerSize();

        auto scoringVolumeDistFromCentre = Util::GetDetecDistIsocenter(); 
        auto scoringVolumeRotation = new G4RotationMatrix();

        // Gantry that we attach the scoring volume to, so that rotations are a lot easier
        auto gantrySolid = new G4Tubs(
            "Gantry",
            scoringVolumeDistFromCentre - (scoringVolumeSize), // magic number to allow leeway so that entire scorer should be enveloped by this tub
            scoringVolumeDistFromCentre + (scoringVolumeSize), // magic number to allow leeway so that entire scorer should be enveloped by this tub
            scoringVolumeSize * 1.2,
            0 * rad,
            2 * CLHEP::pi * rad // TODO: magic number
        );
        auto gantryLogical = new G4LogicalVolume(gantrySolid, G4NistManager::Instance()->FindOrBuildMaterial(fVoidMaterialName), "Gantry");
        fPhysicalGantryVolume = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), gantryLogical, "Gantry", worldLogical, false, 0);

        auto scoringVolumeSolid = new G4Box("ScoringVolume", scoringVolumeSize, scoringVolumeSize / 8, scoringVolumeSize); // TODO: magic number
        fLogicalScoringVolume = new G4LogicalVolume(scoringVolumeSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_Pb"), "ScoringVolume");
        fPhysicalScoringVolume = new G4PVPlacement(scoringVolumeRotation, G4ThreeVector(0, scoringVolumeDistFromCentre, 0), fLogicalScoringVolume, "ScoringVolume", gantryLogical, false, 0);
        
        // We want a region where particles that enter it get killed, to save processing time
        auto scoringVolumeCullingSolidA = new G4Box("ScoringVolumeCRegionA", scoringVolumeSize * 1.4, scoringVolumeSize * 1.4, scoringVolumeSize * 1.4); // TODO: magic number
        auto scoringVolumeCullingSolidB = new G4Box("ScoringVolumeCRegionB", scoringVolumeSize * 1.2, scoringVolumeSize * 1.2, scoringVolumeSize * 1.2); // TODO: magic number
        // Translate culling solid A a tiny bit, but have culling solid B still be inside it centred around the world origin
        auto scoringVolumeCullingSolid = new G4SubtractionSolid("CullingVolume", scoringVolumeCullingSolidA, scoringVolumeCullingSolidB, 0, G4ThreeVector(scoringVolumeSize * 0.2, scoringVolumeSize * 0.2, 0));
        auto scoringVolumeCullingLogical = new G4LogicalVolume(scoringVolumeCullingSolid, G4NistManager::Instance()->FindOrBuildMaterial(fVoidMaterialName), "CullingVolume");
        // Attach it to the world volume, or else it won't be able to catch particles that are outside the gantry volume. But remember to rotate with gantry!
        fPhysicalCullingVolume = new G4PVPlacement(0, G4ThreeVector(0,0,0), scoringVolumeCullingLogical, "CullingVolume", worldLogical, false, 0);
        

        // For importance biasing
        //fPhyImportanceVolumes.push_back(scoringVolumePhysical);

        // TODO: temp
        //this->SetGantryAngle(0 * rad, false);

        // Always return physical world
        return worldPhysical;
    }

    void DetectorConstruction::SetGantryAngle(G4double angle, G4bool updateGeom) {
        if(fPhysicalGantryVolume) {
            if (updateGeom) {
                G4GeometryManager::GetInstance()->OpenGeometry(fPhysicalGantryVolume);
            }

            if (!fGantryRotation) {
                fGantryRotation = new G4RotationMatrix();
            }
            fGantryRotation->set(0, 0, 0); // Reset rotation to identity
            fGantryRotation->rotateZ(-angle); // Minus angle because it didn't seem to align with the particle generator
            fPhysicalGantryVolume->SetRotation(fGantryRotation);
            fPhysicalCullingVolume->SetRotation(fGantryRotation);
            // Needs a geometry reinitialisation after this!

            if (updateGeom) {
                G4GeometryManager::GetInstance()->CloseGeometry(fPhysicalGantryVolume);
            }
        } else {
            G4cerr << "Gantry volume not constructed yet!" << G4endl;
        }
    }

    void DetectorConstruction::ConstructSDandField() {
        // Do we need to construct a new SD for each theta in a rotation? 
        G4cout << "SD and field construction... runmanager type: " << G4RunManager::GetRunManager()->GetRunManagerType() << " current SD pointer: " << G4SDManager::GetSDMpointer()->FindSensitiveDetector("mfd", false) << G4endl;

        // Unreliable way of checking if mfd already exists, as worker threads may report a non-nullptr address for this mfd, when in fact it has never been initialised
        //auto oldMfd = G4SDManager::GetSDMpointer()->FindSensitiveDetector("mfd", false);
        /*
        if (G4RunManager::GetRunManager()->GetRunManagerType() == G4RunManager::masterRM ) {
            // G4RunManager::GetRunManager()->GetRunManagerType() == G4RunManager::sequentialRM) { // We might want to test non-MT geant4
            G4cout << "In non-worker thread, not constructing SD" << G4endl;
            return;
        }*/
        /*
        if (fMFDConstructed) {
            G4cout << "MFD already constructed, skipping..." << G4endl;
            return;
        }*/

        // Setup MFD and Primitive Scorer(s)
        auto mfd = new G4MultiFunctionalDetector("mfd");
        G4SDManager::GetSDMpointer()->AddNewDetector(mfd);
        // Add primitive scorer(s)
        
        // TODO: testing primitive scorer
        //auto energySpectScorer = new G4PSDoseDeposit("ess");

        
        auto energySpectScorer = new EnergySpectScorer(
                                                        "ess",
                                                        Util::GetNumPixelsX(),
                                                        Util::GetNumPixelsY(),
                                                        Util::GetNumBins(),
                                                        Util::GetEnergyMin(),
                                                        Util::GetEnergyMax()); // pretty low (medical) 
        mfd->RegisterPrimitive(energySpectScorer);
        SetSensitiveDetector("ScoringVolume", mfd); // Give pointer to ScoringVolume? Or str name?
        G4cout << "SD and field construction done! Current MFD pointer: " << mfd << " and SD pointer " << G4SDManager::GetSDMpointer()->FindSensitiveDetector("mfd", false) << G4endl;
    }

    void DetectorConstruction::ConstructImportanceVolumes() {
        // UNUSED
        return;
        // Logical slabs
        int numSlabs = 16;
        G4double boundUpper = Util::GetDetecDistIsocenter() - (Util::GetScorerSize() / 8); // TODO: magic number
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
        auto phantomSolid = new G4Tubs("Phantom", 0, phantomSize, phantomSize, 0, 360 * deg);
        auto phantomLogical = new G4LogicalVolume(phantomSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_WATER"), "Phantom");
        auto phantomPhysical = new G4PVPlacement(0, G4ThreeVector(), phantomLogical, "Phantom", fLogicalWorldVolume, false, 0, true);

        // Lung
        auto lungSize = phantomSize * 0.8;
        auto lungSolid = new G4Tubs("Lung", 0, lungSize, lungSize, 0, 360 * deg);
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
        auto phantomSolid = new G4Tubs("Phantom", 0, phantomSize, phantomSize, 0, 360 * deg);
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