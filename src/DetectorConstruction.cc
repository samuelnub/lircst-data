#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Trd.hh"
#include "G4Ellipsoid.hh"
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
        
        if (Util::GetGantryType() == GantryType::CST) {
            // We want a region where particles that enter it get killed, to save processing time
            auto scoringVolumeCullingSolidA = new G4Box("ScoringVolumeCRegionA", scoringVolumeSize * 1.4, scoringVolumeSize * 1.4, scoringVolumeSize * 1.4); // TODO: magic number
            auto scoringVolumeCullingSolidB = new G4Box("ScoringVolumeCRegionB", scoringVolumeSize * 1.2, scoringVolumeSize * 1.2, scoringVolumeSize * 1.2); // TODO: magic number
            // Translate culling solid A a tiny bit, but have culling solid B still be inside it centred around the world origin
            auto scoringVolumeCullingSolid = new G4SubtractionSolid("CullingVolume", scoringVolumeCullingSolidA, scoringVolumeCullingSolidB, 0, G4ThreeVector(scoringVolumeSize * 0.2, scoringVolumeSize * 0.2, 0));
            auto scoringVolumeCullingLogical = new G4LogicalVolume(scoringVolumeCullingSolid, G4NistManager::Instance()->FindOrBuildMaterial(fVoidMaterialName), "CullingVolume");
            // Attach it to the world volume, or else it won't be able to catch particles that are outside the gantry volume. But remember to rotate with gantry!
            fPhysicalCullingVolume = new G4PVPlacement(0, G4ThreeVector(0,0,0), scoringVolumeCullingLogical, "CullingVolume", worldLogical, false, 0);
        }    

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
            if (Util::GetGantryType() == GantryType::CST) {
                fPhysicalCullingVolume->SetRotation(fGantryRotation);            
            }
            
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
        auto phantomLogical = new G4LogicalVolume(phantomSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_LUNG_ICRP"), "Phantom");
        auto phantomPhysical = new G4PVPlacement(0, G4ThreeVector(), phantomLogical, "Phantom", fLogicalWorldVolume, false, 0, true);

        // Lung
        auto lungSize = phantomSize * 0.7;
        auto lungSolid = new G4Tubs("Lung", 0, lungSize, lungSize, 0, 360 * deg);
        auto lungLogical = new G4LogicalVolume(lungSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR"), "Lung");
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lungLogical, "Lung", phantomLogical, false, 0, true);

        // Rib
        auto ribSize = lungSize * 0.7;
        auto ribSolid = new G4Tubs("Rib", 0, ribSize * 0.2, ribSize, 0, 360 * deg);
        auto ribLogical = new G4LogicalVolume(ribSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU"), "Rib");
        new G4PVPlacement(new G4RotationMatrix(0, 90 * deg, 0), G4ThreeVector(lungSize * -0.3, 0, lungSize * -0.15), ribLogical, "Rib", lungLogical, false, 0, true);
    
        // Tumour. Let's have it be an oddly shaped small trapezoid, to make it more interesting. We'll use a G4Trd for this.
        auto tumourSize = lungSize * 0.25;
        auto tumourSolid = new G4Trd("Tumour", tumourSize * 0.6, tumourSize * 0.2, tumourSize * 0.5, tumourSize * 0.3, tumourSize);
        auto tumourLogical = new G4LogicalVolume(tumourSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_TISSUE_SOFT_ICRP"), "Tumour");
        new G4PVPlacement(new G4RotationMatrix(90 * deg, 45 * deg, 30 * deg), G4ThreeVector(lungSize * 0.25, 0, lungSize * 0.2), tumourLogical, "Tumour", lungLogical, false, 0, true);

        // Water droplet next to tumour, to make it more interesting. We'll use a G4Ellipsoid for this.
        auto dropletSize = tumourSize * 0.5;
        auto dropletSolid = new G4Ellipsoid("Droplet", dropletSize, dropletSize*0.6, dropletSize*0.8);
        auto dropletLogical = new G4LogicalVolume(dropletSolid, G4NistManager::Instance()->FindOrBuildMaterial("G4_WATER"), "Droplet");
        new G4PVPlacement(new G4RotationMatrix(0, 0, 0), G4ThreeVector(lungSize * 0.3, lungSize * 0.2, lungSize * 0.4), dropletLogical, "Droplet", lungLogical, false, 0, true);

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

    G4VPhysicalVolume* DetectorConstruction::ConstructPhanShepp() {
        // TODO: not quite up to scratch yet, some ellipsoids are rotated the wrong way, and materials aren't fully correct

        auto nist = G4NistManager::Instance();
        
        // Define Materials (Matching standard Shepp-Logan relative attenuation/densities scaled to H2O)
        auto water  = nist->FindOrBuildMaterial("G4_WATER"); // Scaling baseline
        auto bone   = nist->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");
        auto tissue = nist->FindOrBuildMaterial("G4_TISSUE_SOFT_ICRP");
        
        // Scale factor to map the standard [-1, 1] normalized coordinates to your desired scan size
        G4double scale = Util::GetPhantomSize(); // e.g., 100 * mm if GetPhantomSize() returns 100mm

        // -------------------------------------------------------------------------
        // 1. Base Phantom / Enclosing Ellipsoid (a)
        // -------------------------------------------------------------------------
        auto e1_solid = new G4Ellipsoid("E1_Skull_Outer", 0.69 * scale, 0.92 * scale, 0.9 * scale);
        auto e1_log   = new G4LogicalVolume(e1_solid, bone, "E1_Skull_Outer_Log");
        auto phantomPhysical = new G4PVPlacement(0, G4ThreeVector(0,0,0), e1_log, "Phantom", fLogicalWorldVolume, false, 0, true);

        // -------------------------------------------------------------------------
        // 2. Inner Skull / Brain Matrix (b) - Nesting overwrites the inside of E1
        // -------------------------------------------------------------------------
        auto e2_solid = new G4Ellipsoid("E2_Brain_Matrix", 0.6624 * scale, 0.874 * scale, 0.88 * scale);
        auto e2_log   = new G4LogicalVolume(e2_solid, water, "E2_Brain_Matrix_Log");
        new G4PVPlacement(0, G4ThreeVector(0, -0.0184 * scale, 0), e2_log, "E2_Brain_Matrix_Phys", e1_log, false, 0, true);

        // -------------------------------------------------------------------------
        // Inner structures (c through j) must be nested inside E2 (the Brain Matrix)
        // -------------------------------------------------------------------------

        // 3. Large Internal Structure (c)
        auto e3_solid = new G4Ellipsoid("E3_Structure", 0.11 * scale, 0.31 * scale, 0.21 * scale);
        auto e3_log   = new G4LogicalVolume(e3_solid, tissue, "E3_Log");
        auto rotE3    = new G4RotationMatrix(); rotE3->rotateZ(-18 * deg);
        new G4PVPlacement(rotE3, G4ThreeVector(0.22 * scale, 0, 0), e3_log, "E3_Phys", e2_log, false, 0, true);

        // 4. Large Internal Structure (d)
        auto e4_solid = new G4Ellipsoid("E4_Structure", 0.16 * scale, 0.41 * scale, 0.22 * scale);
        auto e4_log   = new G4LogicalVolume(e4_solid, tissue, "E4_Log");
        auto rotE4    = new G4RotationMatrix(); rotE4->rotateZ(18 * deg);
        new G4PVPlacement(rotE4, G4ThreeVector(-0.22 * scale, 0, 0), e4_log, "E4_Phys", e2_log, false, 0, true);

        // 5. Central Ventricle (e)
        auto e5_solid = new G4Ellipsoid("E5_Ventricle", 0.21 * scale, 0.25 * scale, 0.35 * scale);
        auto e5_log   = new G4LogicalVolume(e5_solid, tissue, "E5_Log");
        new G4PVPlacement(0, G4ThreeVector(0, 0.35 * scale, 0), e5_log, "E5_Phys", e2_log, false, 0, true);

        // 6. Right Ventricle / Structure (f)
        auto e6_solid = new G4Ellipsoid("E6_Structure", 0.046 * scale, 0.046 * scale, 0.1 * scale);
        auto e6_log   = new G4LogicalVolume(e6_solid, tissue, "E6_Log");
        new G4PVPlacement(0, G4ThreeVector(0, 0.1 * scale, -0.25 * scale), e6_log, "E6_Phys", e2_log, false, 0, true);

        // 7. Left Ventricle / Structure (g)
        auto e7_solid = new G4Ellipsoid("E7_Structure", 0.046 * scale, 0.046 * scale, 0.1 * scale);
        auto e7_log   = new G4LogicalVolume(e7_solid, tissue, "E7_Log");
        new G4PVPlacement(0, G4ThreeVector(0, -0.1 * scale, -0.25 * scale), e7_log, "E7_Phys", e2_log, false, 0, true);

        // 8. Bottom Internal Feature (h)
        auto e8_solid = new G4Ellipsoid("E8_Structure", 0.046 * scale, 0.023 * scale, 0.05 * scale);
        auto e8_log   = new G4LogicalVolume(e8_solid, tissue, "E8_Log");
        new G4PVPlacement(0, G4ThreeVector(-0.08 * scale, -0.605 * scale, 0), e8_log, "E8_Phys", e2_log, false, 0, true);

        // 9. Bottom Internal Feature (i)
        auto e9_solid = new G4Ellipsoid("E9_Structure", 0.023 * scale, 0.023 * scale, 0.02 * scale);
        auto e9_log   = new G4LogicalVolume(e9_solid, tissue, "E9_Log");
        new G4PVPlacement(0, G4ThreeVector(0, -0.605 * scale, 0), e9_log, "E9_Phys", e2_log, false, 0, true);

        // 10. Bottom Internal Feature (j)
        auto e10_solid = new G4Ellipsoid("E10_Structure", 0.023 * scale, 0.046 * scale, 0.02 * scale);
        auto e10_log   = new G4LogicalVolume(e10_solid, tissue, "E10_Log");
        new G4PVPlacement(0, G4ThreeVector(0.06 * scale, -0.605 * scale, 0), e10_log, "E10_Phys", e2_log, false, 0, true);

        return phantomPhysical;
    }
}