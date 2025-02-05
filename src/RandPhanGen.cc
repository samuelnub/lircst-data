#include "RandPhanGen.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4Torus.hh"
#include "G4Ellipsoid.hh"

#include "GroundTruthExporter.hh"

namespace lircst {
    G4PVPlacement* RandPhanGen::GeneratePhantom() {
        G4cout << "Generating random phantom with seed " << G4Random::getTheSeed() << G4endl;

        auto phantomSize = Util::GetPhantomSize();

        // Generate random material for root phantom volume, but check if its electron density is < a threshold
        G4double phantomMaterialMaxRhoE = 1;
        auto phantomMaterial = GetRandomMaterial();
        while (GroundTruthExporter::CalculateElectronDensityPerMole(phantomMaterial) > phantomMaterialMaxRhoE) {
            phantomMaterial = GetRandomMaterial();
        }

        auto phantomSolid = new G4Box("Phantom", phantomSize, phantomSize, phantomSize);
        auto phantomLogical = new G4LogicalVolume(phantomSolid, phantomMaterial, "Phantom");
        auto phantomPhysical = new G4PVPlacement(0, G4ThreeVector(), phantomLogical, "Phantom", fLogicalWorldVol, false, 0, true);

        G4int numInserts = Util::GenRandomInt(1, fMaxNumInserts);

        for (G4int i = 0; i < numInserts; i++) {
            bool placementSuccess = false;

            for (int retry = 0; retry < fMaxRetries; retry++) {
                G4VSolid* solid = GetRandomSolid(i);
                G4Material* material = GetRandomMaterial();

                // Remember to delete if placement fails
                G4LogicalVolume* logical = new G4LogicalVolume(solid, material, "RandInsertLogVol" + std::to_string(i));

                G4LogicalVolume* parent = SelectParent(logical, i, phantomLogical);

                G4ThreeVector position = GetRandomPosition();
                G4RotationMatrix* rotation = GetRandomRotation();

                // Place volume and check for overlaps
                G4PVPlacement* placement = new G4PVPlacement(
                    rotation, 
                    position, 
                    logical, 
                    "RandInsertPhyVol" + std::to_string(i), 
                    parent, 
                    false, 
                    0, 
                    true);

                if (!placement->CheckOverlaps()) {
                    fParentInserts.push_back(logical);
                    placementSuccess = true;
                    break;
                } else {
                    delete placement;
                    delete rotation;
                    delete logical;
                }
            }

            if (!placementSuccess) {
                G4cout << "Failed to place insert " << i << " after " << fMaxRetries << " retries" << G4endl;
            }
        }

        return phantomPhysical;
    }

    G4Material* RandPhanGen::GetRandomMaterial() {
        const auto& materialNames = G4NistManager::Instance()->GetNistMaterialNames();
        auto materialName = materialNames[Util::GenRandomInt(0, materialNames.size() - 1)];
        return G4NistManager::Instance()->FindOrBuildMaterial(materialName);
    }

    G4VSolid* RandPhanGen::GetRandomSolid(G4int n /*current index of insert we are generating*/) {
        auto phSize = Util::GetPhantomSize();

        auto shrinkFac = n > 1 ? Util::GenRandomInt(1, n) : 1;

        switch(Util::GenRandomInt(0, 4)) {
            case 0:
                return new G4Box("Box", 
                Util::GenRandomDouble(phSize*0.1, phSize*0.8) / shrinkFac, 
                Util::GenRandomDouble(phSize*0.1, phSize*0.8) / shrinkFac, 
                Util::GenRandomDouble(phSize*0.1, phSize*0.8) / shrinkFac);
            case 1:
                return new G4Tubs("Tubs", 
                Util::GenRandomDouble(0, phSize*0.09) / shrinkFac, 
                Util::GenRandomDouble(phSize*0.1, phSize*0.8) / shrinkFac, 
                Util::GenRandomDouble(phSize*0.1, phSize*0.8) / shrinkFac, 
                0, 
                Util::GenRandomInt(1, 360) * deg);
            case 2:
                return new G4Sphere("Sphere", 
                Util::GenRandomDouble(0, phSize*0.09) / shrinkFac, 
                Util::GenRandomDouble(phSize*0.1, phSize*0.8) / shrinkFac, 
                0, 
                Util::GenRandomInt(1, 360) * deg,
                0, 
                Util::GenRandomInt(1, 360) * deg);
            case 3:
                return new G4Torus("Torus", 
                Util::GenRandomDouble(0, phSize*0.09) / shrinkFac, 
                Util::GenRandomDouble(phSize*0.1, phSize*0.29) / shrinkFac, 
                Util::GenRandomDouble(phSize*0.3, phSize*0.8) / shrinkFac, 
                0, 
                Util::GenRandomInt(1, 360) * deg);
            case 4:
                return new G4Ellipsoid("Ellipsoid", 
                Util::GenRandomDouble(phSize*0.1, phSize*0.8) / shrinkFac, 
                Util::GenRandomDouble(phSize*0.1, phSize*0.8) / shrinkFac, 
                Util::GenRandomDouble(phSize*0.1, phSize*0.8) / shrinkFac);
            default:
                return new G4Box("Box", 1, 1, 1);
        }
    }

    G4ThreeVector RandPhanGen::GetRandomPosition() {
        return G4ThreeVector( // We want our y value to be a gauss distribution instead of uniform
            Util::GenRandomDouble(-Util::GetPhantomSize(), Util::GetPhantomSize()), 
            Util::GenRandomDoubleGauss(0.0f, Util::GetPhantomSize() / 4), // TODO: assumes imaging plane is always at 0
            Util::GenRandomDouble(-Util::GetPhantomSize(), Util::GetPhantomSize())
        );
    }

    G4RotationMatrix* RandPhanGen::GetRandomRotation() {
        G4RotationMatrix* rot = new G4RotationMatrix();
        rot->rotateX(Util::GenRandomInt(0, 360) * deg);
        rot->rotateY(Util::GenRandomInt(0, 360) * deg);
        rot->rotateZ(Util::GenRandomInt(0, 360) * deg);
        return rot;
    }

    G4LogicalVolume* RandPhanGen::SelectParent(G4LogicalVolume* candidateLogVol, G4int n, G4LogicalVolume* defaultParent) {
        G4bool assignDaughter = Util::GenRandomInt(0, fMaxNumInserts - n) == 0 && !fParentInserts.empty();

        if (assignDaughter) {
            return fParentInserts[Util::GenRandomInt(0, fParentInserts.size() - 1)];
        }

        return defaultParent; // Default to root phantom as parent
    }
}