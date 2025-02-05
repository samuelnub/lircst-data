#ifndef RANDPHANGEN_HH
#define RANDPHANGEN_HH

#include "Randomize.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4VSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"

#include "Util.hh"

namespace lircst {
    class RandPhanGen {
    public:
        RandPhanGen(G4LogicalVolume* logicalWorldVolume) : fLogicalWorldVol(logicalWorldVolume) {}
        ~RandPhanGen() = default;

        // Returns physical volume of our root phantom
        G4PVPlacement* GeneratePhantom();

    private:
        G4int fMaxNumInserts = 10;
        G4int fMaxRetries = 10;
        G4LogicalVolume* fLogicalWorldVol;
        std::vector<G4LogicalVolume*> fParentInserts; // Potential parent volumes for inserts

        G4Material* GetRandomMaterial();
        G4VSolid* GetRandomSolid(G4int n);
        G4ThreeVector GetRandomPosition();
        G4RotationMatrix* GetRandomRotation();
        G4LogicalVolume* SelectParent(G4LogicalVolume* candidateLogVol, G4int n, G4LogicalVolume* defaultParent);
    };
}


#endif // RANDPHANGEN_HH