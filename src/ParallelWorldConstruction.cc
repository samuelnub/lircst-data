#include "ParallelWorldConstruction.hh"

#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Box.hh"
#include "G4LogicalVolumeStore.hh"

#include "BiasingOperator.hh"
#include "G4ParallelGeometriesLimiterProcess.hh"
#include "G4BiasingHelper.hh"

#include "Util.hh"

namespace lircst {
    ParallelWorldConstruction::ParallelWorldConstruction(G4String worldName) : G4VUserParallelWorld(worldName) {}

    void ParallelWorldConstruction::Construct() {
        // Construct the world
        G4cout << "Constructing parallel world" << G4endl;
        
        auto paraPhyWorld = GetWorld();
        auto paraLogWorld = paraPhyWorld->GetLogicalVolume();

        // Base phantom in our parallel world
        auto paraPhantomSize = Util::GetPhantomSize();
        auto paraPhantomSolid = new G4Box("ParaPhantom", paraPhantomSize, paraPhantomSize, paraPhantomSize);
        auto paraPhantomLogical = new G4LogicalVolume(paraPhantomSolid, 0, "ParaPhantom");
        auto paraPhantomPhysical = new G4PVPlacement(0, G4ThreeVector(), paraPhantomLogical, "Phantom", paraLogWorld, false, 0);

    }

    void ParallelWorldConstruction::ConstructSD() {
        // Create and link biasing operator
        auto biasingOperator = new BiasingOperator();

        biasingOperator->SetParallelWorld(GetWorld());

        // Attach to logical volume where biasing is to be applied
        auto paraPhantomLogical = G4LogicalVolumeStore::GetInstance()->GetVolume("ParaPhantom");
        
        biasingOperator->AttachTo(paraPhantomLogical);

        G4cout << "Biasing operator attached to ParaPhantom" << G4endl;
    }
}