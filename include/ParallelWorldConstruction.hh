#ifndef PARA_WORLD_CONSTRUCTION_HH
#define PARA_WORLD_CONSTRUCTION_HH

#include "G4VUserParallelWorld.hh"

namespace lircst {
    class ParaWorldConstruction : public G4VUserParallelWorld {
    public:
        ParaWorldConstruction(G4String worldName);
        ~ParaWorldConstruction() override = default;

        virtual void Construct();
        virtual void ConstructSD();
    };
}

#endif // PARA_WORLD_CONSTRUCTION_HH